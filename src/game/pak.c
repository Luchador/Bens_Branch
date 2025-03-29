#include "versions.h"
#include <ultra64.h>
#include "constants.h"
#include "game/bossfile.h"
#include "game/filelist.h"
#include "game/menu.h"
#include "game/crc.h"
#include "game/gamefile.h"
#include "game/lv.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/joy.h"
#include "lib/lib_06440.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "string.h"
#include "data.h"
#include "types.h"
#include "string.h"
#include "input.h"

/**
 * Perfect Dark supports saving to an in-cartridge EEPROM chip, as well as to
 * controller paks which can be inserted in any of the four controllers.
 *
 * This file provides an abstraction layer between a generic "pak" and the
 * backend device it uses, and also manages the structure of data within the
 * pak.
 *
 * -- EEPROM --
 *
 * The EEPROM chip is 2KB (0x800 bytes) and is rather simple: The game reads and
 * writes to it using address and length arguments to osEeprom functions.
 *
 * -- Controller Paks --
 *
 * The controller paks are accessed via osPfs functions which are provided by
 * Nintendo. Controller paks use a filesystem and can hold data from other games
 * which is why these functions must be used.
 *
 * Controller paks have a capacity of 256Kbits (32KB). Each controller pak
 * consists of 128 "pages", where each page is a block of 256 bytes. The first
 * 5 pages are reserved for the file allocation table, leaving 123 pages
 * available for game data.
 *
 * Games use osPfsAllocateFile to create a file, also known as a game note.
 * Controller paks can hold up to 16 game notes. Perfect Dark's game note uses
 * 28 pages (7168 bytes). This single game note holds all saved information
 * (game files, MP players and MP games).
 *
 * -- Data Structure --
 *
 * Regardless of whether the data is being written to EEPROM or to a controller
 * pak, the format of it is the same. The data is a list of files, with
 * different lengths based on their file type.
 *
 * Each file has a 16-byte header, followed by its variable length body.
 * The header contains a checksum of the body data, as well as its filetype,
 * size and identifiers.
 *
 * The only way to iterate the files in the filesystem is to read the first
 * file's header (at offset 0), then read its file length out of that header and
 * add it to the offset. Repeat until a PAKFILETYPE_TERMINATOR is found which
 * marks the end of the filesystem. The filesystem may be smaller than the size
 * of the EEPROM or controller pak note.
 *
 * The effective file types are:
 *
 * BOS (length 0x70) - The "boss" file stores things global to all game files,
 *     such as the alternative title setting and chosen language if PAL.
 * GAM (length 0xb0) - Single player game files
 * MPP (length 0x60) - Multiplayer player files
 * MPG (length 0x50) - Multiplayer game setup files
 *
 * Each device can store 4 GAM, MPG and MPP files, and one BOS file. There is
 * additionally a single swap space per game type, making the total usage
 * 1984 bytes (0x7c0), which is 0x30 short of the EEPROM capacity.
 *
 * Controller paks, however, use 28 pages which is 20 pages more than necessary.
 * This is likely because they were going to hold PerfectHead photos, but when
 * the feature was removed the controller pak allocation was not adjusted.
 *
 * -- GUIDs --
 *
 * GUID is an abbreviation for globally unique identifier. GUIDs are used to
 * minimise the chance of the game overwriting a wrong file in the event that
 * a player loads a file from a controller pak, then swaps the controller pak
 * for another during gameplay. By using GUIDs, the game is very likely to
 * detect when this has happened and will prompt the player to reinsert the
 * original pak.
 *
 * When creating a game note on a controller pak, the game generates a serial
 * number for the controller pak. This serial number persists throughout the
 * life of the note. The serial number is saved into the header of every file
 * in that note.
 *
 * Additionally, when creating a file on a pak, the file is given an
 * incrementing ID number which is unique to that pak. That ID is also saved
 * into the header of that file.
 *
 * The combination of the device serial and file ID is the GUID.
 */

#define NUM_PAGES 28

#define MAX_HEADERCACHE_ENTRIES 50

#define LINE_825  822
#define LINE_1058 1055
#define LINE_1551 1551
#define LINE_1802 1788
#define LINE_3486 3290
#define LINE_3495 3299
#define LINE_3599 3403
#define LINE_3654 3459
#define LINE_3668 3473
#define LINE_3829 3634
#define LINE_3865 3670
#define LINE_3889 3694
#define LINE_3948 3753
#define LINE_4140 3945
#define LINE_4394 4199
#define LINE_4742 4547
#define LINE_4801 4606

/**
 * In NTSC Beta the functions joyDisableCyclicPolling and joyEnableCyclicPolling
 * take two arguments: __LINE__ and __FILE__. In newer versions of the game
 * these functions take no arguments. This macro is here to avoid using VERSION
 * checks everywhere where these are called.
 */
#define JOYARGS(line)

#define SETBANNER(banner) if (g_ShowPakMenuBanner) { menuSetBanner(banner, true); }

#define PAKFEATURE_MEMORY  0x01
#define PAKFEATURE_RUMBLE  0x02
#define PAKFEATURE_GAMEBOY 0x04

const char g_N64FontCodeMap[] = "\0************** 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#'*+,-./:=?@";

struct pak g_Paks[5]; // controller paks + EEPROM

OSPfs g_Pfses[MAX_PLAYERS];

uint32_t var80075ccc = 0x00000400;
uint32_t g_PakHasEeprom = false;
uint32_t g_PakDebugForceCrc = 0;
uint32_t g_PakDebugForceScrub = 0;
uint32_t g_PakDebugPakDump = 0;
uint32_t g_PakDebugPakCache = 1;
uint32_t g_PakDebugPakInit = 0;

uint32_t g_PakDebugWipeEeprom = 0;

char g_PakNoteGameName[] = {
	N64CHAR('P'),
	N64CHAR('E'),
	N64CHAR('R'),
	N64CHAR('F'),
	N64CHAR('E'),
	N64CHAR('C'),
	N64CHAR('T'),
	N64CHAR(' '),
	N64CHAR('D'),
	N64CHAR('A'),
	N64CHAR('R'),
	N64CHAR('K'),
	0, // fill to 16 bytes
	0,
	0,
	0,
};

char g_PakNoteExtName[] = {0, 0, 0, 0};

uint8_t g_PaksPlugged = 0;

bool g_ShowPakMenuBanner = true;

bool g_ValidGbcRomFound = false;

uint32_t pakGetBlockSize(int8_t device)
{
	return device == SAVEDEVICE_GAMEPAK ? 0x10 : 0x20;
}

uint32_t pakAlign(int8_t device, uint32_t size)
{
	return pakGetBlockSize(device) == 0x20 ? align32(size) : align16(size);
}

int pakGetAlignedFileLenByBodyLen(int8_t device, uint32_t bodylen)
{
	return pakAlign(device, sizeof(struct pakfileheader) + bodylen);
}

uint32_t pakGetBodyLenByFileLen(uint32_t filelen)
{
	return filelen - sizeof(struct pakfileheader);
}

uint32_t pakGenerateSerial(int8_t device)
{
	int value;
	int rand;
	int count;

	if (device == SAVEDEVICE_GAMEPAK) {
		return 0xbaa;
	}

	value = g_Paks[device].unk2c8;
	rand = (rngRandom() % 496) + 16; // range 16-511
	count = utilsGetCount();

	return value ^ rand ^ count;
}

bool mempakIsOkay(int8_t device)
{
	if (g_Paks[device].type == PAKTYPE_MEMORY) {
		switch (g_Paks[device].state) {
		case PAKSTATE_MEM_ENTER_DEVICEERROR:
		case PAKSTATE_MEM_ENTER_CORRUPT:
		case PAKSTATE_MEM_DEVICEERROR:
		case PAKSTATE_MEM_CORRUPT:
		case PAKSTATE_22:
			return false;
		}

		return true;
	}

	return false;
}

int pakGetFileIdsByType(int8_t device, uint32_t filetype, uint32_t *fileids)
{
	return _pakGetFileIdsByType(device, filetype, fileids);
}

int pak0f1167d8(int8_t device)
{
	return pak0f119298(device);
}

int pakReadBodyAtGuid(int8_t device, int fileid, uint8_t *body, int arg3)
{
	return _pakReadBodyAtGuid(device, fileid, body, arg3);
}

int pakSaveAtGuid(int8_t device, int fileid, int filetype, uint8_t *body, int *outfileid, uint8_t *olddata)
{
	return _pakSaveAtGuid(device, fileid, filetype, body, outfileid, olddata);
}

bool pakDeleteFile(int8_t device, int fileid)
{
	return _pakDeleteFile(device, fileid);
}

PakErr1 pakDeleteGameNote(int8_t device, uint16_t company_code, uint32_t game_code, char *game_name, char *ext_name)
{
	return _pakDeleteGameNote(device, company_code, game_code, game_name, ext_name);
}

PakErr1 pak0f1168c4(int8_t device, struct pakdata **arg1)
{
	return pak0f116df0(device, arg1);
}

int pakGetType(int8_t device)
{
	return _pakGetType(device);
}

int pakGetSerial(int8_t device)
{
	return _pakGetSerial(device);
}

void pak0f116994(void)
{
	if (g_Vars.stagenum == STAGE_BOOTPAKMENU) {
		g_Vars.pakstocheck = 0xf8;
	}
}

void pak0f1169c8(int8_t device, bool tick)
{
	uint8_t prevvalue = g_Vars.paksneededformenu;

	g_Vars.paksneededformenu = 0x1f;

	if ((g_Vars.paksneededforgame | g_Vars.paksneededformenu) & (1 << device)) {
		g_PaksPlugged &= ~(1 << device);

		pakCheckPlugged();
		pakCheckPlugged();

		if (tick) {
			g_JoyPfsPollMasterEnabled = false;

			pakTickState(device);
			pakTickState(device);
			pakTickState(device);
			pakTickState(device);
			pakTickState(device);
			pakTickState(device);
			pakTickState(device);

			g_JoyPfsPollMasterEnabled = true;
		}
	}

	g_Vars.paksneededformenu = prevvalue;
}

bool mempakIsReady(int8_t device)
{
	if (g_Paks[device].state == PAKSTATE_READY && g_Paks[device].type == PAKTYPE_MEMORY) {
		return true;
	}

	return false;
}

bool mempakIsReadyOrFull(int8_t device)
{
	if ((g_Paks[device].state == PAKSTATE_READY
				|| g_Paks[device].state == PAKSTATE_MEM_ENTER_FULL
				|| g_Paks[device].state == PAKSTATE_MEM_FULL)
			&& g_Paks[device].type == PAKTYPE_MEMORY) {
		return true;
	}

	return false;
}

uint16_t _pakGetSerial(int8_t device)
{
	return g_Paks[device].serial;
}

uint32_t _pakGetType(int8_t device)
{
	return g_Paks[device].type;
}

void pakSetState(int8_t device, int state)
{
	g_Paks[device].state = state;
}

PakErr1 pak0f116df0(int8_t device, struct pakdata **pakdata)
{
	*pakdata = NULL;

	if (mempakIsReadyOrFull(device)) {
		if (pakQueryTotalUsage(device)) {
			*pakdata = &g_Paks[device].pakdata;
			return PAK_ERR1_OK;
		}

		return PAK_ERR1_NEWPAK;
	}

	return PAK_ERR1_NOPAK;
}

PakErr1 _pakDeleteGameNote(int8_t device, uint16_t company_code, uint32_t game_code, char *game_name, char *ext_name)
{
	int result;

	if (mempakIsReadyOrFull(device)) {
		joyDisableCyclicPolling(JOYARGS(738));
		result = pakDeleteGameNote3(PFS(device), company_code, game_code, game_name, ext_name);
		joyEnableCyclicPolling(JOYARGS(740));

		if (pakHandleResult(result, device, true, LINE_825)) {
			g_Paks[device].unk2b8_02 = 1;
			return PAK_ERR1_OK;
		}

		return PAK_ERR1_NEWPAK;
	}

	return PAK_ERR1_NOPAK;
}

int _pakDeleteFile(int8_t device, int fileid)
{
	struct pakfileheader header;
	int result = pakFindFile(device, fileid, &header);

	if (result == -1) {
		return 1;
	}

	result = pakWriteFileAtOffset(device, result, header.filetype, NULL, 0, NULL, NULL, 0, header.generation + 1);

	if (result) {
		return result;
	}

	return 0;
}

int pakGetPlugCount(int8_t device)
{
	return g_Paks[device].plugcount;
}

uint32_t pakGetMaxFileSize(int8_t device)
{
	if (device != SAVEDEVICE_GAMEPAK) {
		return 0x4c0;
	}

	return 0x100;
}

int pakGetBodyLenByType(int8_t device, uint32_t filetype)
{
	int len = 0;

	switch (filetype) {
	case PAKFILETYPE_001:
	case PAKFILETYPE_BLANK:
		break;
	case PAKFILETYPE_TERMINATOR:
		len = pakGetMaxFileSize(device) - sizeof(struct pakfileheader);
		break;
	case PAKFILETYPE_BOSS:
		len = 0x5b;
		break;
	case PAKFILETYPE_MPPLAYER:
		len = 0x4e;
		break;
	case PAKFILETYPE_MPSETUP:
		len = 0x31;
		break;
	case PAKFILETYPE_CAMERA:
		len = 0x4a0;
		break;
	case PAKFILETYPE_GAME:
		len = 0xa0;
		break;
	}

	return len;
}

bool pakRetrieveBlockFromCache(int8_t device, uint32_t offset, uint8_t *dst)
{
	uint32_t blocksize = pakGetBlockSize(device);
	uint32_t stack;
	int i;

	if (g_Paks[device].headercachecount < MAX_HEADERCACHE_ENTRIES) {
		for (i = 0; i < g_Paks[device].headercachecount; i++) {
			if (offset / blocksize == g_Paks[device].headercache[i].blocknum) {
				memcpy(dst, g_Paks[device].headercache[i].payload, pakGetBlockSize(device));
				return true;
			}
		}
	}

	return false;
}

PakErr2 pakReadHeaderAtOffset(int8_t device, uint32_t offset, struct pakfileheader *header)
{
	struct pakfileheader localheader;
	struct pakfileheader *headerptr;
	uint32_t blocknum;
	int result;
	uint16_t checksum[2];
	uint8_t sp38[0x20];

	headerptr = header ? header : &localheader;

	blocknum = offset / pakGetBlockSize(device);

	if (blocknum >= g_Paks[device].pdnumblocks) {
		return PAK_ERR2_BADOFFSET;
	}

	if (!pakRetrieveHeaderFromCache(device, blocknum, headerptr)) {
		result = pakReadWriteBlock(device, PFS(device), g_Paks[device].pdnoteindex, 0, offset, sizeof(sp38), sp38);

		if (pakHandleResult(result, device, true, LINE_1058) == 0) {
			if (result == PAK_ERR1_NOPAK) {
				return PAK_ERR2_NOPAK;
			}

			return PAK_ERR2_BADOFFSET;
		}

		memcpy(headerptr, sp38, sizeof(struct pakfileheader));
		pakCalculateChecksum(&sp38[0x08], &sp38[0x10], checksum);

		if (headerptr->headersum[0] != checksum[0] || headerptr->headersum[1] != checksum[1]) {
			return PAK_ERR2_CHECKSUM;
		}

		if (!headerptr->writecompleted) {
			return PAK_ERR2_INCOMPLETE;
		}

		if ((argFindByPrefix(1, "-forceversion") ? 1 : 0) != headerptr->version) {
			return PAK_ERR2_VERSION;
		}

		if (g_PakDebugPakCache) {
			pakSaveHeaderToCache(device, blocknum, (struct pakfileheader *) sp38);

			if (!pakRetrieveHeaderFromCache(device, blocknum, headerptr)) {
				return PAK_ERR2_CORRUPT;
			}
		}
	}

	if (headerptr->filelen == 0) {
		return PAK_ERR2_CORRUPT;
	}

	return PAK_ERR2_OK;
}

/**
 * Overwrite the save file which has the specified fileid. This is typically an
 * earlier version of the same logical save file. A new fileid will be generated
 * and returned to *outfileid.
 *
 * The function doesn't literally overwrite the old file. On the pak, there is
 * a swap file reserved for atomic writes. The new file is written into the
 * swap file, then the old file is marked as swap.
 */
int _pakSaveAtGuid(int8_t device, int fileid, int filetype, uint8_t *newdata, int *outfileid, uint8_t *olddataptr)
{
	struct pakfileheader header;
	struct pakfileheader swapheader;
	int result;
	int oldoffset;
	int i;
	int swapoffset;
	uint32_t fileids[1024];
	int swapfileid = 0;
	uint8_t olddata[0x800];

	// Find the file to be "replaced"
	oldoffset = pakFindFile(device, fileid, &header);

	if (oldoffset && (!oldoffset || oldoffset >= pakGetPdNumBytes(device) || ((pakGetBlockSize(device) - 1) & oldoffset))) {
		return 3;
	}

	if (filetype != header.filetype) {
		return 12;
	}

	// Find all files on the pak of the same filetype,
	// then iterate them to find the swap file
	pakGetFileIdsByType(device, header.filetype, fileids);

	// NTSC Beta initialises swapoffset to -1 so it can detect if the loop below
	// has been entered. But in NTSC 1.0 they realised that pakFindFile can
	// return -1, so they initialised the variable to 0xeeeeeeee instead and
	// added the check for -1 in the loop below.
	swapoffset = 0xeeeeeeee;

	for (i = 0; fileids[i] != 0; i++) {
		swapoffset = pakFindFile(device, fileids[i], &swapheader);

		if (swapoffset == -1) {
			return 1;
		}

		if (!swapheader.occupied && swapheader.fileid != fileid) {
			// Found the swap file
			swapfileid = swapheader.fileid;
			break;
		}
	}

	// For the game pak, don't trust the olddataptr argument and instead
	// populate it by loading the data at the swap file (olddataptr is used to
	// skip writes if any old and new blocks are matching).
	// @bug? Shouldn't this also apply to controller paks? How would the caller
	// know which swap space was going to be used? Maybe controller paks don't
	// use the olddataptr optimisation?
	if (device == SAVEDEVICE_GAMEPAK) {
		result = pakReadBodyAtGuid(device, swapfileid, olddata, -1);

		// NTSC 1.0 just writes the same thing a different way
		if (result == 0) {
			olddataptr = olddata;
		} else if (result == 10) {
			olddataptr = olddata;
		} else {
			olddataptr = NULL;
		}
	}

	// Write the new file into the swap space
	result = pakWriteFileAtOffset(device, swapoffset, filetype, newdata, 0, outfileid, olddataptr, fileid, header.generation + 1);

	if (result != 0) {
		return 4;
	}

	// NTSC Beta skips marking the old file as vacant if the file wasn't found
	// and returns an OK value. NTSC Final makes it return an error instead.
	// @bug: The 0xeeeeeeee check should have been done earlier for swapoffset
	// instead. As it turns out, if swap space wasn't found then
	// pakWriteFileAtOffset would have returned an error above and this function
	// would have returned before this check occurs. And oldoffset will never be
	// 0xeeeeeeee (even with 4GB of storage) because 0xeeeeeeee is not aligned
	// to a 16-byte boundary. So this bug is harmless.
	if (oldoffset == -1) {
		return 1;
	}

	if (oldoffset != 0xeeeeeeee) {
		pakWriteFileAtOffset(device, oldoffset, filetype, NULL, 0, NULL, NULL, swapfileid, header.generation);
	}

	return 0;
}

PakErr1 pakInitPak(OSPfs *pfs, int channel, int *arg3)
{
	if (pfs) {
		return inputRumbleSupported(channel) ? 11 : 1;
	}

	if (!g_PakHasEeprom) {
		return 0x80;
	}

	return 0;
}

PakErr1 _pakReadWriteBlock(OSPfs *pfs, int file_no, uint8_t flag, uint32_t address, uint32_t len, uint8_t *buffer)
{
	uint32_t newaddress;

	joyPollPfs(2);

	if (pfs) {
		return 1;
	}

	newaddress = address / 8;

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	if (flag == 1) { // Write
		return pakWriteEeprom(newaddress, buffer, len);
	}

	if (flag == 0) { // Read
		return pakReadEeprom(newaddress, buffer, len);
	}

	return PAK_ERR1_EEPROMINVALIDOP;
}

PakErr1 pakQueryNumNotes(OSPfs *pfs, int *max_files, int *files_used)
{
	if (pfs) {
		int result;

		joyDisableCyclicPolling(JOYARGS(1308));
		result = 1;
		joyEnableCyclicPolling(JOYARGS(1310));

		return result;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	*max_files = 1;
	*files_used = 1;

	return PAK_ERR1_OK;
}

PakErr1 pakQueryNumFreeBytes(OSPfs *pfs, int *bytes_not_used)
{
	if (pfs) {
		int result;

		joyDisableCyclicPolling(JOYARGS(1337));
		result = 1;
		joyEnableCyclicPolling(JOYARGS(1339));

		return result;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	*bytes_not_used = 0;

	return PAK_ERR1_OK;
}

PakErr1 pakQueryNoteState(OSPfs *pfs, int file_no, OSPfsState *note)
{
	if (pfs) {
		int result;

		joyDisableCyclicPolling(JOYARGS(1363));
		result = 1;
		joyEnableCyclicPolling(JOYARGS(1365));

		return result;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	if (file_no) {
		return PAK_ERR1_EEPROMINVALIDARG;
	}

	note->file_size = 0x800;
	note->company_code = ROM_COMPANYCODE;
	strcpy(note->game_name, g_PakNoteGameName);
	strcpy(note->ext_name, g_PakNoteExtName);

	return PAK_ERR1_OK;
}

PakErr1 pakAllocateNote(OSPfs *pfs, uint16_t company_code, uint32_t game_code, char *game_name, char *ext_name, int size, int *file_no)
{
	if (pfs) {
		return 1;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	*file_no = 0;

	return PAK_ERR1_OK;
}

PakErr1 pakDeleteGameNote3(OSPfs *pfs, uint16_t company_code, uint32_t game_code, char *game_name, char *ext_name)
{
	if (pfs) {
		return 1;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	return PAK_ERR1_OK;
}

PakErr1 pakFindNote(OSPfs *pfs, uint16_t company_code, uint32_t game_code, char *game_name, char *ext_name, int *file_no)
{
	if (pfs) {
		return 1;
	}

	if (g_PakHasEeprom) {
		*file_no = 0;
		return PAK_ERR1_OK;
	}

	return PAK_ERR1_EEPROMMISSING;
}

PakErr1 _pakResizeNote(OSPfs *pfs, uint16_t company_code, uint32_t game_code, uint8_t *game_name, uint8_t *ext_name, uint32_t numbytes)
{
	if (pfs) {
		int result;

		joyDisableCyclicPolling(JOYARGS(1496));
		result = 1;
		joyEnableCyclicPolling(JOYARGS(1498));

		return result;
	}

	if (!g_PakHasEeprom) {
		return PAK_ERR1_EEPROMMISSING;
	}

	return PAK_ERR1_OK;
}

int pakGetPdNumPages(int8_t device)
{
	return g_Paks[device].pdnumpages;
}

uint32_t pakGetPdNumBytes(int8_t device)
{
	return g_Paks[device].pdnumbytes;
}

int pakQueryNumFreePages(int8_t device)
{
	int bytesfree;

	pakQueryNumFreeBytes(PFS(device), &bytesfree);

	return bytesfree / 256;
}

bool pakResizeNote(int8_t device, int numpages)
{
	int errnum;
	struct pak *devicedata;
	OSPfsState *note;
	uint32_t numbytes;

	pakGetPdNumPages(device);
	pakQueryNumFreePages(device);

	numbytes = numpages * 256;
	errnum = _pakResizeNote(PFS(device), ROM_COMPANYCODE, ROM_GAMECODE, g_PakNoteGameName, g_PakNoteExtName, numbytes);
	pakHandleResult(errnum, device, true, LINE_1802);

	if (errnum == PAK_ERR1_OK) {
		devicedata = &g_Paks[device];
		note = &devicedata->pakdata.notes[devicedata->pdnoteindex];

		devicedata->pakdata.pagesfree -= numpages - devicedata->pdnumpages;
		devicedata->pakdata.pagesused += numpages - devicedata->pdnumpages;

		note->file_size = devicedata->pakdata.pagesused * 256;

		devicedata->pdnumbytes = numbytes;
		devicedata->pdnumblocks = devicedata->pdnumbytes / pakGetBlockSize(device);
		devicedata->pdnumpages = devicedata->pdnumbytes / 256;

		return true;
	}

	return false;
}

/**
 * Find a spot for the given filetype and write it.
 *
 * Replace a blank spot or extend the filesystem if needed and possible.
 */
uint32_t pak0f118674(int8_t device, uint32_t filetype, int *outfileid)
{
	struct pakfileheader header;
	int ret;
	int zero = 0;
	int filelen = pakGetAlignedFileLenByBodyLen(device, pakGetBodyLenByType(device, filetype));
	int bestoffset = -1;
	uint32_t offset = 0;
	bool foundperfectblank = false;
	bool foundblank = false;

	if (pak0f1167d8(device)) {
		return pak0f1167d8(device);
	}

	while (offset < g_Paks[device].pdnumbytes) {
		ret = pakReadHeaderAtOffset(device, offset, &header);

		if (ret == PAK_ERR2_OK) {
			if (header.filetype & PAKFILETYPE_TERMINATOR) {
				if (offset + filelen > g_Paks[device].pdnumbytes - 0x20) {
					return 14;
				}

				bestoffset = offset;
				break;
			}

			if (header.filetype & PAKFILETYPE_BLANK) {
				if (header.filelen == filelen) {
					foundperfectblank = true;
					bestoffset = offset;
					break;
				}

				foundblank = true;
				bestoffset = offset;
				break;
			}

			offset += header.filelen;
		}
		else if (ret == PAK_ERR2_NOPAK) {
			return 1;
		}
		else {
			offset += pakGetBlockSize(device);
		}
	}

	// This is optimised out, but it invalidates the the register that stores &g_Paks[device],
	// which makes it get recalculated. This is required for a match.
	if (zero) {
		device ^= 0;
		device ^= 0;
	}

	if (offset == 0 ||
			(offset && offset < pakGetPdNumBytes(device) && ((pakGetBlockSize(device) - 1) & offset) == 0)) {
		if (bestoffset == -1) {
			return 14;
		}

		// Write the file
		if (pakWriteFileAtOffset(device, bestoffset, filetype, NULL, 0, outfileid, NULL, 0, 1) == 0) {
			if (foundblank) {
				uint32_t endoffset = bestoffset + filelen;
				pakRepairAsBlank(device, &endoffset, NULL);
				return 0;
			}

			if (foundperfectblank || foundblank) {
				return 0;
			}

			// Write new terminator after file
			bestoffset += pakGetAlignedFileLenByBodyLen(device, pakGetBodyLenByType(device, filetype));

			if (pakWriteFileAtOffset(device, bestoffset, PAKFILETYPE_TERMINATOR, NULL, 0, NULL, NULL, 0, 1) == 0) {
				return 0;
			}

			return 4;
		}

		return 4;
	}

	g_Paks[device].state = PAKSTATE_MEM_ENTER_FULL;
	g_Paks[device].type = PAKTYPE_MEMORY;

	return 4;
}

void paksInit(void)
{
	uint8_t prevvalue = g_Vars.paksneededformenu;
	int8_t i;

	g_Vars.pakstocheck = 0;

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		pakSetDefaults(i);
	}

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		pakSetFeatures(i, PAKFEATURE_MEMORY | PAKFEATURE_RUMBLE | PAKFEATURE_GAMEBOY, 2049, "pak/pak.c");
	}

	pakProbeEeprom();
	joyRecordPfsState(0x10);

	g_Vars.paksneededformenu = 0x10;

	pak0f1169c8(SAVEDEVICE_GAMEPAK, true);
	bossfileLoadFull();

	gamefileLoadDefaults(&g_GameFile);
	gamefileApplyOptions(&g_GameFile);

	g_GameFileGuid.deviceserial = 0;
	g_Vars.pakstocheck = 0xf5;
	g_Vars.paksneededformenu = prevvalue;
}

void pakCalculateChecksum(uint8_t *start, uint8_t *end, uint16_t *checksum)
{
	crcCalculateU16Pair(start, end, checksum);
}

int pak0f118b04(int8_t device, uint32_t fileid)
{
	int offset;
	int result = pak0f1167d8(device);

	if (result == 0) {
		offset = pakFindFile(device, fileid, 0);

		if (offset == -1) {
			return 1;
		}

		if (offset == 0 || (offset != 0 && offset < pakGetPdNumBytes(device) && ((pakGetBlockSize(device) - 1) & offset) == 0)) {
			if (!pakReplaceFileAtOffsetWithBlank(device, offset)) {
				return 4;
			}
		} else {
			return 3;
		}
	} else {
		return 6;
	}

	return 0;
}

int _pakReadBodyAtGuid(int8_t device, int fileid, uint8_t *body, int arg3)
{
	int offset;
	struct pakfileheader header;
	int result;
	uint16_t checksum[2];

	if (!pak0f1167d8(device)) {
		offset = pakFindFile(device, fileid, NULL);

		if (offset == -1) {
			return 1;
		}

		if (offset == 0 || (offset && offset < pakGetPdNumBytes(device) && ((pakGetBlockSize(device) - 1) & offset) == 0)) {
			result = pak0f11b86c(device, offset, body, &header, arg3);

			if (result) {
				return result;
			}

			if (arg3 == -1) {
				arg3 = 0;
			}

			if (header.occupied) {
				if (!arg3) {
					pakCalculateChecksum(body, body + header.bodylen, checksum);

					if (header.bodysum[0] != checksum[0] || header.bodysum[1] != checksum[1]) {
						return 8;
					}
				}
			} else {
				return 10;
			}
		} else {
			return 3;
		}
	} else {
		return 6;
	}

	return 0;
}

int _pakGetFileIdsByType(int8_t device, uint32_t filetype, uint32_t *fileids)
{
	struct pakfileheader header;
	uint32_t offset = 0;
	uint32_t fslen;
	int len = 0;
	int result = pak0f119298(device);

	if (result != 0) {
		return result;
	}

	result = pakGetFilesystemLength(device, &fslen);

	if (result != 0) {
		return result;
	}

	result = pak0f1167d8(device);

	if (result != 0) {
		return pak0f1167d8(device);
	}

	result = pakReadHeaderAtOffset(device, offset, &header);

	while (result == PAK_ERR2_OK) {
		if ((filetype & PAKFILETYPE_ALL) || (filetype & header.filetype)) {
			fileids[len] = header.fileid;
			len++;
		}

		offset += header.filelen;

		if (offset >= fslen) {
			break;
		}

		result = pakReadHeaderAtOffset(device, offset, &header);
	}

	fileids[len] = 0;

	if (result == PAK_ERR2_CHECKSUM) {
		return 7;
	}

	if (result == PAK_ERR2_NOPAK) {
		return 1;
	}

	return 0;
}

int pakDefrag(int8_t device)
{
	int result = pak0f1167d8(device);

	if (result != 0) {
		return result;
	}

	pakMergeBlanks(device);

	return 0;
}

/**
 * Calculate the number of times the given filetype can fit in the note.
 *
 * Return 0 if it can fit, otherwise 5.
 */
int pakCheckFileCanFitInNote(int8_t device, int filetype, int *numspaces)
{
	struct pakfileheader header;
	uint32_t filelen;
	bool hasspace;
	uint32_t fslen;
	uint32_t offset;
	uint32_t roomtogrow;

	filelen = pakGetAlignedFileLenByBodyLen(device, pakGetBodyLenByType(device, filetype));

	hasspace = false;

	pakGetFilesystemLength(device, &fslen);

	if (numspaces != NULL) {
		*numspaces = 0;
	}

	for (offset = 0; pakReadHeaderAtOffset(device, offset, &header) == PAK_ERR2_OK && offset < fslen; offset += header.filelen) {
		if (PAKFILETYPE_BLANK == header.filetype && header.filelen >= filelen) {
			hasspace = true;

			if (numspaces != NULL) {
				*numspaces = *numspaces + 1;
			}
		}
	}

	roomtogrow = g_Paks[device].pdnumbytes - fslen;

	if (numspaces != NULL) {
		*numspaces += roomtogrow / filelen;
	}

	if (!hasspace && roomtogrow >= pakGetMaxFileSize(device)) {
		hasspace = true;
	}

	return (hasspace ? 0 : 5);
}

uint32_t pak0f119298(int8_t device)
{
	if (g_Paks[device].type != PAKTYPE_MEMORY) {
		return 1;
	}

	switch (g_Paks[device].state) {
	case PAKSTATE_READY:
		return 0;
	case PAKSTATE_17:
		return 14;
	case PAKSTATE_18:
		return 4;
	case PAKSTATE_MEM_DISPATCH:
	case PAKSTATE_MEM_PRE_PREPARE:
	case PAKSTATE_MEM_PREPARE:
	case PAKSTATE_MEM_POST_PREPARE:
	case PAKSTATE_07:
		return 13;
	}

	return 1;
}

int pakFindFile(int8_t device, uint32_t fileid, struct pakfileheader *headerptr)
{
	struct pakfileheader header;
	int offset = 0;
	uint32_t fslen;
	int ret;

	pakGetFilesystemLength(device, &fslen);

	ret = pakReadHeaderAtOffset(device, offset, &header);

	while (ret == PAK_ERR2_OK && offset < fslen) {
		if (fileid == header.fileid) {
			if (headerptr) {
				memcpy(headerptr, &header, sizeof(struct pakfileheader));
			}

			return offset;
		}

		offset += header.filelen;

		ret = pakReadHeaderAtOffset(device, offset, &header);
	}

	if (ret == PAK_ERR2_NOPAK) {
		return -1;
	}

	return 0xffff;
}

bool pakWriteBlankFile(int8_t device, uint32_t offset, struct pakfileheader *header)
{
	if (pakWriteFileAtOffset(device, offset, PAKFILETYPE_BLANK, NULL, pakGetBodyLenByFileLen(header->filelen), NULL, NULL, 0, 1) == 0) {
		return true;
	}

	return false;
}

/**
 * Repair the pak by writing a blank file from the given offset up until the
 * start of the next file.
 *
 * If done successfully, update the value at the offset pointer with the end
 * offset of the blank file and return true.
 *
 * For controller paks, enforce a max file size. If the blank file would exceed
 * the max file size, store the current search offset in the pointer and return
 * false.
 *
 * If the end of the note/device is reached without finding the subequent file,
 * write a terminator file at the start address and return true. A blank file
 * is not written in this case.
 *
 * If an error code is returned by called functions, the value at the offset
 * pointer is not changed and the function returns false.
 *
 * If the header argument is not NULL, it's assumed it's the header
 * corresponding to the starting offset and the function takes a shortcut by
 * starting the scan at the end of the header.
 */
bool pakRepairAsBlank(int8_t device, uint32_t *offsetptr, struct pakfileheader *header)
{
	struct pakfileheader iterheader;

	uint32_t maxfilesize = pakGetMaxFileSize(device);
	uint32_t start = *offsetptr;
	uint32_t start2 = *offsetptr;
	uint32_t offset = *offsetptr;
	int result;
	uint32_t bodylen;

	// Skip past the header if given
	if (header != NULL) {
		offset += header->filelen;
	}

	while (offset < g_Paks[device].pdnumbytes) {
		result = pakReadHeaderAtOffset(device, offset, &iterheader);

		if (result == PAK_ERR2_OK) {
			// Found a valid header
			if ((iterheader.filetype & PAKFILETYPE_BLANK) == 0 && offset > start2) {
				break;
			}
		} else if (result == PAK_ERR2_NOPAK) {
			return false;
		}

		// No header at this offset
		offset += pakGetBlockSize(device);

		// For controller paks, consider giving up
		if (device != SAVEDEVICE_GAMEPAK && offset - start > maxfilesize) {
			*offsetptr = offset;
			return false;
		}

		// If the end was reached, write a terminator at the starting offset
		if (offset >= g_Paks[device].pdnumbytes) {
			pakWriteFileAtOffset(device, start, PAKFILETYPE_TERMINATOR, NULL, 0, NULL, NULL, 0, 1);
			return true;
		}
	}

	bodylen = pakGetBodyLenByFileLen(offset - start);

	// Write the blank file ranging from to the start to the current offset
	result = pakWriteFileAtOffset(device, start, PAKFILETYPE_BLANK, NULL, bodylen, NULL, NULL, 0, 1);

	if (result != 0) {
		*offsetptr = offset;
		return false;
	}

	*offsetptr = offset;
	return true;
}

struct serialcount {
	uint32_t serial;
	int count;
};

/**
 * Attempt to repair the filesystem on the pak.
 *
 * ntsc-final:
 * - Ensures the same device serial is used across all files
 *
 * ntsc-1.0:
 * - Fixes the terminator if missing
 * - Changes return code to -1, 0 or 1, where:
 *     -1 means a fatal error occurred and the filesystem is hosed
 *     0 means the filesystem was fine or is now repaired
 *     1 means the pak is not inserted or the repair failed
 *
 * ntsc-beta:
 * - Removes duplicate files
 * - Removes files if header checksum mismatches
 * - Removes files if partially written
 * - Returns 1 if filesystem is good, or 0 if unrepairable
 */
int pakRepairFilesystem(int8_t device)
{
	int ret;
	bool fatal = false;
	bool foundotherversion = false;
	struct pakfileheader headers[50];
	struct pakfileheader header;
	int numheaders = 0;
	uint32_t headeroffsets[50];
	uint32_t offset;
	int i;
	bool foundduplicate;
	struct serialcount serials[100];

	g_Paks[device].serial = 0xbaba;
	g_Paks[device].headercachecount = 0;

	if (pak0f1167d8(device) != 0) {
		return 1;
	}

	// Iterate the headers on the pak and copy each one onto the stack.
	// As each header is read, check if a duplicate exists on the stack.
	// If a duplicate is found, blank the oldest one.
	// Stop when the end is reached, or a blank file or terminator is found.
	// If any headers are unreadable, replace them with a blank file.
	offset = 0;

	while (!fatal && offset < g_Paks[device].pdnumbytes) {
		ret = pakReadHeaderAtOffset(device, offset, &header);

		if (ret == PAK_ERR2_OK) {
			if (header.filetype & PAKFILETYPE_BLANK) {
				break;
			}

			if (header.filetype & PAKFILETYPE_TERMINATOR) {
				break;
			}

			if (offset + header.filelen >= g_Paks[device].pdnumbytes)
			{
				// File overflows the device length -> replace with terminator
				ret = pakWriteFileAtOffset(device, offset, PAKFILETYPE_TERMINATOR, NULL, 0, NULL, NULL, 0, 1);

				if (ret != 0) {
					fatal = true;
				} else {
					break;
				}
			} else {
				// Check for duplicates
				int i;
				foundduplicate = false;

				for (i = 0; i != numheaders; i++) {
					if (headeroffsets[i] != -1 && header.fileid == headers[i].fileid) {
						foundduplicate = true;

						if (header.generation < headers[i].generation) {
							// The header that was just read is older -> delete it
							fatal = pakRepairAsBlank(device, &offset, &header) == 0;
						} else {
							// The header that was just read is newer -> delete the older one
							fatal = pakRepairAsBlank(device, &headeroffsets[i], &headers[i]) == 0;
							headeroffsets[i] = -1;

							// Add this header to the list
							memcpy(&headers[numheaders], &header, sizeof(header));
							headeroffsets[numheaders] = offset;
							numheaders++;

							offset += header.filelen;
						}

						break;
					}
				}

				if (!foundduplicate && !fatal) {
					memcpy(&headers[numheaders], &header, sizeof(header));
					headeroffsets[numheaders] = offset;
					numheaders++;

					offset += header.filelen;
				}
			}
		} else {
			if (ret == PAK_ERR2_NOPAK) {
				return 1;
			} else if (ret == PAK_ERR2_CHECKSUM) {
				fatal = pakRepairAsBlank(device, &offset, NULL) == false;
			} else if (ret == PAK_ERR2_INCOMPLETE) {
				if (!pakRepairAsBlank(device, &offset, &header)) {
					fatal = true;
					break;
				}
			} else if (ret == PAK_ERR2_VERSION) {
				foundotherversion = true;
				break;
			} else if (ret == PAK_ERR2_BADOFFSET) {
				fatal = true;
				break;
			} else if (ret == PAK_ERR2_CORRUPT) {
				fatal = true;
				break;
			} else {
				fatal = true;
				break;
			}
		}
	}

	// Recheck all the headers.
	// Return 0 if the first file was a terminator (ie. pak is empty).
	// Return 1 if any header still has problems.
	offset = 0;

	while (!foundotherversion && !fatal && offset < g_Paks[device].pdnumbytes) {
		ret = pakReadHeaderAtOffset(device, offset, &header);

		if (ret == 0) { // success
			if (header.filetype & PAKFILETYPE_BLANK) {
				// empty
			} else if (offset) {
			} else {
				g_Paks[device].serial = header.deviceserial;

				if (header.filetype & PAKFILETYPE_TERMINATOR) {
					return 0;
				}
			}

			if ((header.filetype & PAKFILETYPE_TERMINATOR) == 0) {
				offset += header.filelen;
			} else {
				break;
			}
		} else if (ret == PAK_ERR2_VERSION) {
			foundotherversion = true;
			offset += header.filelen;
		} else if (ret == PAK_ERR2_NOPAK) {
			return 1;
		} else {
			return 1;
		}
	}

	// NTSC Final ensures serials are all the same
	if (!foundotherversion && !fatal) {
		// Build list of serials and how many times each was found.
		// There should only be one serial across all files.
		int numserials = 0;
		offset = 0;

		while (offset < g_Paks[device].pdnumbytes) {
			ret = pakReadHeaderAtOffset(device, offset, &header);

			if (ret == PAK_ERR2_NOPAK);

			if (ret != PAK_ERR2_OK) {
				break;
			}

			if ((header.filetype & PAKFILETYPE_BLANK) == 0) {
				bool found = false;

				if (header.filetype & PAKFILETYPE_TERMINATOR) {
					break;
				}

				for (i = 0; i < numserials; i++) {
					if (serials[i].serial == header.deviceserial) {
						found = true;
						serials[i].count++;
					}
				}

				if (!found) {
					serials[numserials].serial = header.deviceserial;
					serials[numserials].count = 1;
					numserials++;
				}
			}

			offset += header.filelen;

			if (offset);
		}

		if (numserials >= 2) {
			// Decide which serial to use based on majority
			int bestindex = -1;
			int bestcount = -1;

			for (i = 0; i < numserials; i++) {
				if (serials[i].count > bestcount) {
					bestindex = i;
					bestcount = serials[i].count;
				}
			}

			if (bestindex != -1) {
				// Apply the chosen serial
				g_Paks[device].serial = serials[bestindex].serial;

				offset = 0;

				while (offset < g_Paks[device].pdnumbytes) {
					ret = pakReadHeaderAtOffset(device, offset, &header);

					if (ret != PAK_ERR2_OK) {
						break;
					}

					if ((header.filetype & PAKFILETYPE_BLANK) == 0) {
						if (header.filetype & PAKFILETYPE_TERMINATOR) {
							break;
						}

						if (header.deviceserial != g_Paks[device].serial) {
							pakWriteBlankFile(device, offset, &header);
						}
					}

					offset += header.filelen;

					if (offset);
				}
			}
		} else {
			g_Paks[device].serial = serials[0].serial;
		}
	}

	if (fatal) {
		return -1;
	}

	if (foundotherversion) {
		return -1;
	}

	if (device != SAVEDEVICE_GAMEPAK && g_Paks[device].serial == 0) {
		g_Paks[device].serial = pakGenerateSerial(device);
		return -1;
	}

	return (VERSION >= VERSION_NTSC_1_0 ? 0 : 1);
}

/**
 * Create the initial files on a pak. Return true if all good.
 *
 * NTSC Beta forgets to include return values.
 */
#if VERSION >= VERSION_NTSC_1_0
bool pakCreateInitialFiles(int8_t device)
#else
void pakCreateInitialFiles(int8_t device)
#endif
{
	struct pakfileheader header;
	int i;
	uint32_t fileids[1024];
	int j;
	uint32_t stack[2];

	uint32_t filetypes[] = {
		PAKFILETYPE_BOSS,
		PAKFILETYPE_CAMERA,
		PAKFILETYPE_MPPLAYER,
		PAKFILETYPE_MPSETUP,
		PAKFILETYPE_GAME,
	};

	uint32_t filecounts[] = { 2, 3, 5, 5, 5 };

#if VERSION >= VERSION_NTSC_1_0
	char *filenames[] = { "BOS\n", "CAM\n", "MPP\n", "MPG\n", "GAM" };
#else
	char *filenames[] = { "BOS", "CAM", "MPP", "MPG", "GAM" };
#endif

	// Iterate all files on the pak and decrease the counts per filetype
	if (pakGetFileIdsByType(device, PAKFILETYPE_ALL, fileids) != 0) {
#if VERSION >= VERSION_NTSC_1_0
		return false;
#else
		return;
#endif
	}

	for (i = 0; fileids[i] != 0; i++) {
#if VERSION >= VERSION_NTSC_1_0
		if (pakFindFile(device, fileids[i], &header) == -1) {
			return false;
		}
#else
		pakFindFile(device, fileids[i], &header);
#endif

		for (j = 0; j < ARRAYCOUNT(filetypes); j++) {
			if (header.filetype == filetypes[j]) {
				if (filecounts[j]) {
					filecounts[j]--;
				}
				break;
			}
		}
	}

	// Create files
	for (i = 0; i < ARRAYCOUNT(filetypes); i++) {
		// Skip creating camera files on the game pak (they are controller pak only)
		if (filecounts[i] != 0 && !(device == SAVEDEVICE_GAMEPAK && i == 1)) {
			for (j = 0; j < filecounts[i]; j++) {
				int ret = pak0f118674(device, filetypes[i], NULL);

				if (ret != 0) {
					if (ret == 14) {
						return true;
					}
					return false;
				}
			}
		}
	}

	return true;
}

int pakFindMaxFileId(int8_t device)
{
	struct pakfileheader header;
	uint32_t fileids[1025];
	int result;
	int max = 0;
	int i;

	result = pakGetFileIdsByType(device, PAKFILETYPE_ALL, fileids);

	if (result == 0) {
		for (i = 0; fileids[i] != 0; i++) {
			int offset = pakFindFile(device, fileids[i], &header);

			if (offset == -1) {
				return -1;
			}

			if (header.fileid > max) {
				max = header.fileid;
			}
		}
	} else {
		return -1;
	}

	return max;
}

void pakMergeBlanks(int8_t device)
{
	struct pakfileheader header;
	uint32_t offset = 0;
	uint32_t nextoffset;
	int mergestartoffset = 0xffff;

	while (pakReadHeaderAtOffset(device, offset, &header) == PAK_ERR2_OK) {
		nextoffset = offset + header.filelen;

		if (offset);

		if (PAKFILETYPE_BLANK == header.filetype) {
			if (mergestartoffset != 0xffff) {
				uint32_t filelen = offset - mergestartoffset + header.filelen - sizeof(struct pakfileheader);
				nextoffset = 0;
				mergestartoffset = 0xffff;
			} else {
				mergestartoffset = offset;
			}
		} else {
			mergestartoffset = 0xffff;
		}

		offset = nextoffset;
	}
}

int pakGetFeatures(int8_t device)
{
	return g_Paks[device].features;
}

void pakSetFeatures(int8_t device, uint8_t features, uint32_t line, char *file)
{
	if (g_Paks[device].features == 0) {
		g_Paks[device].features = features;

		if ((g_Paks[device].features & PAKFEATURE_MEMORY) && g_Paks[device].headercache == NULL) {
			g_Paks[device].headercachecount = 0;
			g_Paks[device].headercache = mempAlloc(align32(sizeof(struct pakheadercache) * MAX_HEADERCACHE_ENTRIES), MEMPOOL_PERMANENT);

			// This would have been used in an osSyncPrintf call.
			// Perhaps using the strings at var7f1b4318 through var7f1b43ac?
			align32(sizeof(struct pakheadercache) * MAX_HEADERCACHE_ENTRIES);
		}
	}
}

void pakRemoveAllFeatures(int8_t device, uint32_t arg1, uint32_t arg2)
{
	if (g_Paks[device].features) {
		g_Paks[device].features = 0;
	}

	if (g_Paks[device].features);
}

/*const char var7f1b4294[] = "Pak %d - Pak_StartOne called from line %d in %s -> Flags = %0x\n";
const char var7f1b42d4[] = "\nPak_StartOne -> Pak%d, Modes -\n";
const char var7f1b42f8[] = "Memory,";
const char var7f1b4300[] = "Rumble,";
const char var7f1b4308[] = "Game Boy";
const char var7f1b4314[] = "\n";
const char var7f1b4318[] = "Pak %d -> %u Bytes of scratch for cache 2 memory at %0x\n";
const char var7f1b4354[] = "\nPak%d -> Pak_EndOne - Called from line %d in %s : Modes -\n";
const char var7f1b4390[] = "Memory,";
const char var7f1b4398[] = "Rumble,";
const char var7f1b43a0[] = "Game Boy";
const char var7f1b43ac[] = "\n";
const char var7f1b43b0[] = "Pak -> FATAL ERROR -> MEMORY INSTANCE ENDING IS NO LONGER SUPPORTED\n";
const char var7f1b43f8[] = "Pak -> Pak_MakeOne - Id=%d is finished\n";*/

void pakSetDefaults(int8_t device)
{
	g_Paks[device].unk274 = 3;
	g_Paks[device].features = 0;
	g_Paks[device].type = PAKTYPE_NONE;
	g_Paks[device].unk008 = PAK008_01;
	g_Paks[device].rumblestate = RUMBLESTATE_1;
	g_Paks[device].unk00c = PAK00C_03;
	g_Paks[device].state = PAKSTATE_NOPAK;
	g_Paks[device].pdnoteindex = -1;
	g_Paks[device].unk2b8_01 = 0;
	g_Paks[device].unk2b8_05 = 0;
	g_Paks[device].isgbcamera = 0;
	g_Paks[device].unk2b8_02 = 0;
	g_Paks[device].unk2bd = 128;
	g_Paks[device].plugcount = 0;
	g_Paks[device].unk2b8_06 = 0;
	g_Paks[device].showdatalost = false;
	g_Paks[device].headercache = NULL;
	g_Paks[device].unk2c4 = NULL;
	g_Paks[device].maxfileid = 8;
	g_Paks[device].serial = 0;
	g_Paks[device].rumblettl = -1;
	g_Paks[device].unk2c8 = 0;
}

PakErr1 pakReadWriteBlock(int8_t device, OSPfs *pfs, int file_no, uint8_t flag, uint32_t address, uint32_t len, uint8_t *buffer)
{
	int result;
	len = pakAlign(device, len);

	joyDisableCyclicPolling(JOYARGS(3096));
	result = _pakReadWriteBlock(pfs, file_no, flag, address, len, buffer);
	joyEnableCyclicPolling(JOYARGS(3098));

	return result;
}

bool pakQueryTotalUsage(int8_t device)
{
	struct pak *pak = &g_Paks[device];
	int noteerrors[16];
	int bytesfree;
	int ret;
	int i;

	if (!pak->unk2b8_02) {
		return true;
	}

	ret = pakQueryNumNotes(PFS(device), &pak->notestotal, &pak->notesused);

	if (!pakHandleResult(ret, device, true, LINE_3486)) {
		pak->unk2b8_02 = false;
		return false;
	}

	ret = pakQueryNumFreeBytes(PFS(device), &bytesfree);
	pak->pakdata.pagesfree = ((bytesfree + 255) & 0xffff) >> 8;

	if (!pakHandleResult(ret, device, true, LINE_3495)) {
		pak->unk2b8_02 = false;
		return false;
	}

	for (i = 0; i < ARRAYCOUNT(noteerrors); i++) {
		noteerrors[i] = pakQueryNoteState(PFS(device), i, &pak->pakdata.notes[i]);

		if (noteerrors[i] != PAK_ERR1_OK) {
			pak->pakdata.notesinuse[i] = false;
		} else {
			pak->pakdata.notesinuse[i] = true;
		}
	}

	for (i = 0, pak->pakdata.pagesused = 0; i < ARRAYCOUNT(noteerrors); i++) {
		if (noteerrors[i] == PAK_ERR1_OK) {
			pak->pakdata.pagesused += (pak->pakdata.notes[i].file_size + 255) >> 8;
		}
	}

	pak->unk2b8_02 = false;

	return true;
}

void pakQueryPdSize(int8_t device)
{
	uint32_t stack;
	OSPfsState note;
	int result;

	joyDisableCyclicPolling(JOYARGS(3242));
	result = pakQueryNoteState(PFS(device), g_Paks[device].pdnoteindex, &note);
	joyEnableCyclicPolling(JOYARGS(3244));

	if (pakHandleResult(result, device, true, LINE_3599)) {
		g_Paks[device].pdnumbytes = note.file_size;
		g_Paks[device].pdnumblocks = g_Paks[device].pdnumbytes / pakGetBlockSize(device);
		g_Paks[device].pdnumpages = g_Paks[device].pdnumbytes / 256;
		g_Paks[device].pdnumnotes = g_Paks[device].pdnumbytes / (256 * NUM_PAGES);
	}
}

/**
 * Prepare a controller pak for use by making sure a note is allocated and that
 * the filesystem is good, among other things.
 */
bool mempakPrepare(int8_t device)
{
	uint32_t stack1;
	struct pak *pak;
	bool error1 = false;
	bool error2 = false;
	uint32_t fileids[1024];
	int serial;
	int sp48;
	int notesize;
	int maxfileid;
	uint32_t stack2;

	g_Paks[device].type = PAKTYPE_MEMORY;
	g_Paks[device].unk2b8_02 = true;

	pakQueryTotalUsage(device);

	if (g_Paks[device].state == PAKSTATE_UNPLUGGING) {
		return false;
	}

	// Find the PD note if it exists
	joyDisableCyclicPolling(JOYARGS(3319));
	sp48 = pakFindNote(PFS(device), ROM_COMPANYCODE, ROM_GAMECODE, g_PakNoteGameName, g_PakNoteExtName, &g_Paks[device].pdnoteindex);
	joyEnableCyclicPolling(JOYARGS(3321));

	// If it doesn't exist, allocate it
	if (sp48 != PAK_ERR1_OK) {
		pak = &g_Paks[device];

		pakHandleResult(sp48, device, false, LINE_3654);

		g_Paks[device].pdnumnotes = (pak->pakdata.pagesfree > 128) ? 2 : 1;

		notesize = g_Paks[device].pdnumnotes * (256 * NUM_PAGES);

		joyDisableCyclicPolling(JOYARGS(3336));
		sp48 = pakAllocateNote(PFS(device), ROM_COMPANYCODE, ROM_GAMECODE, g_PakNoteGameName, g_PakNoteExtName, notesize, &g_Paks[device].pdnoteindex);
		joyEnableCyclicPolling(JOYARGS(3338));

		g_Paks[device].unk2b8_02 = true;

		if (pakHandleResult(sp48, device, true, LINE_3668)) {
			error1 = true;
		} else {
			return false;
		}
	}

	pakQueryTotalUsage(device);
	pakQueryPdSize(device);

	g_Paks[device].showdatalost = false;
	g_Paks[device].headercachecount = 0;
	g_Paks[device].state = PAKSTATE_READY;

	// If it's a new note, create the filesystem
	if (error1) {
		serial = pakCreateFilesystem(device);

		if (serial != -1) {
			g_Paks[device].serial = serial;
		} else {
			error2 = true;
		}
	}

	// Check the filesystem for errors and try to recreate it if broken
	if (!error2) {
		if (pakRepairFilesystem(device) == -1) {
			serial = pakCreateFilesystem(device);

			if (serial != -1) {
				g_Paks[device].serial = serial;
			} else {
				error2 = true;
			}

			if (device != SAVEDEVICE_GAMEPAK) {
				g_Paks[device].showdatalost = true;
			}
		}
	}

	if (!error2) {
		maxfileid = pakFindMaxFileId(device);

		if (maxfileid != -1) {
			g_Paks[device].maxfileid = maxfileid;

			if (pakGetFileIdsByType(device, PAKFILETYPE_TERMINATOR, fileids) == 0 && pakCreateInitialFiles(device)) {
				g_Paks[device].state = (device == SAVEDEVICE_GAMEPAK) ? PAKSTATE_READY : PAKSTATE_MEM_POST_PREPARE;

				filelistInvalidatePak(device);

				return true;
			}
		}
	}

	g_Paks[device].state = PAKSTATE_22;

	filelistInvalidatePak(device);

	return false;
}

bool pakProbe(int8_t device)
{
	bool plugged = false;
	int ret;
	bool done = false;

	joyDisableCyclicPolling();

	// Try memory pak
	ret = pakInitPak(PFS(device), device, NULL);

	if (pakHandleResult(ret, device, true, LINE_3829)) {
		g_Paks[device].state = PAKSTATE_MEM_DISPATCH;

		if (device == SAVEDEVICE_GAMEPAK) {
			pakExecuteDebugOperations();
			pakExecuteDebugOperations();
			pakExecuteDebugOperations();
		}

		plugged = true;
		done = true;
	} else if (ret == PAK_ERR1_NOPAK) {
		done = true;
	}

	if (!done) {
		if (device == SAVEDEVICE_GAMEPAK) {
			plugged = false;
			done = true;
		}

		if (!done) {
			// Try rumble pak
			ret = osMotorProbe(PFS(device), device);

			if (pakHandleResult(ret, device, false, LINE_3865)) {
				g_Paks[device].type = PAKTYPE_RUMBLE;
				g_Paks[device].state = PAKSTATE_READY;
				g_Paks[device].rumblestate = RUMBLESTATE_1;
				g_Paks[device].plugcount++;

				plugged = true;
				done = true;
			} else if (ret == PAK_ERR1_NOPAK) {
				plugged = false;
				done = true;
			}

			if (!done) {
				// Try game boy pak
				ret = 1;

				if (pakHandleResult(ret, device, false, LINE_3889)) {
					g_Paks[device].type = PAKTYPE_GAMEBOY;
					g_Paks[device].state = PAKSTATE_GB_PRE_PREPARE;
					g_Paks[device].unk2b8_01 = false;
					g_Paks[device].plugcount++;
					plugged = true;
				} else if (ret == PAK_ERR1_NOPAK) {
					plugged = false;
				}
			}
		}
	}

	joyEnableCyclicPolling();

	return plugged;
}

/**
 * Replace data between the given blocks with '!'.
 */
void pakWipe(int8_t device, uint32_t blocknumstart, uint32_t blocknumend)
{
	uint8_t buffer[128];
	uint32_t i;

	for (i = 0; i < pakGetBlockSize(device); i++) {
		buffer[i] = '!';
	}

	for (i = blocknumstart; i < blocknumend; i++) {
		int result = pakReadWriteBlock(device, PFS(device), g_Paks[device].pdnoteindex, 1, i * pakGetBlockSize(device), pakGetBlockSize(device), buffer); // Write

		g_Paks[device].headercachecount = 0;

		if (!pakHandleResult(result, device, true, LINE_3948)) {
			g_Paks[device].pdnoteindex = -1;
			break;
		}
	}
}

void pakSaveHeaderToCache(int8_t device, int blocknum, struct pakfileheader *header)
{
	struct pak *pak = &g_Paks[device];
	int count;
	int overview[1024];
	uint32_t stack[2];
	int j;
	int k;
	int i;
	int endblocknum = header->filelen / pakGetBlockSize(device) + blocknum;

	for (i = 0; i < ARRAYCOUNT(overview); i++) {
		overview[i] = -1;
	}

	// Iterate existing cache items and write their indexes into the overview array,
	// where the index in the overview array is determined by the cache item's blocknum.
	for (i = 0; i < pak->headercachecount; i++) {
		struct pakfileheader *tmp = (struct pakfileheader *) pak->headercache[i].payload;

		for (j = 0; j < tmp->filelen / pakGetBlockSize(device); j++) {
			overview[pak->headercache[i].blocknum + j] = i;
		}
	}

	// Examine the overview range where the new cache entry is going to go.
	// If any cache entries are there then they're likely an older version of
	// the cache header that's being inserted, so invalidate them.
	for (k = blocknum; k < endblocknum; k++) {
		if (overview[k] != -1) {
			pak->headercache[overview[k]].blocknum = -1;
		}
	}

	// Save the header into the cache
	pak->headercache[pak->headercachecount].blocknum = blocknum;
	memcpy(pak->headercache[pak->headercachecount].payload, header, pakGetBlockSize(device));

	pak->headercachecount++;

	// Close any gaps in the cache list and recount for good measure
	count = 0;

	for (i = 0; i < pak->headercachecount; i++) {
		if (pak->headercache[i].blocknum != -1) {
			pak->headercache[count].blocknum = pak->headercache[i].blocknum;
			memcpy(&pak->headercache[count].payload, &pak->headercache[i].payload, pakGetBlockSize(device));
			count++;
		}
	}

	pak->headercachecount = count;
}

bool pakRetrieveHeaderFromCache(int8_t device, int blocknum, struct pakfileheader *dst)
{
	struct pak *pak = &g_Paks[device];
	int i;

	if (pak->headercachecount < MAX_HEADERCACHE_ENTRIES) {
		for (i = 0; i < pak->headercachecount; i++) {
			if (blocknum == pak->headercache[i].blocknum) {
				memcpy(dst, &pak->headercache[i].payload, sizeof(struct pakfileheader));
				return true;
			}
		}
	}

	return false;
}

/**
 * Initialise a game pak or controller pak note's filesystem from scratch.
 *
 * - A serial is generated for the pak.
 * - A terminator file (containing the serial) is then written at the start of the pak.
 * - Random bytes are then written into a block after the terminator.
 *
 * Return the pak's serial on success, or -1 on failure.
 */
int pakCreateFilesystem(int8_t device)
{
	uint8_t data[32];
	int address;
	int result;
	int i;

	for (i = 0; i < 32; i++) {
		data[i] = rngRandom() & 0xff;
	}

	address = pakGetAlignedFileLenByBodyLen(device, pakGetBodyLenByType(device, PAKFILETYPE_TERMINATOR));

	g_Paks[device].maxfileid = 0x10;
	g_Paks[device].serial = pakGenerateSerial(device);
	g_Paks[device].headercachecount = 0;

	pakWriteFileAtOffset(device, 0, PAKFILETYPE_TERMINATOR, NULL, 0, NULL, NULL, 0, 1);

	result = pakReadWriteBlock(device, PFS(device), g_Paks[device].pdnoteindex, 1, address, pakGetBlockSize(device), data); // Write

	if (pakHandleResult(result, device, true, LINE_4140) == 0) {
		return -1;
	}

	return g_Paks[device].serial;
}

int pak0f11b6ec(int8_t device)
{
	if (g_Paks[device].state == PAKSTATE_READY && g_Paks[device].type == PAKTYPE_MEMORY) {
		return 3;
	}

	return 0;
}

bool pakGetFilesystemLength(int8_t device, uint32_t *outlen)
{
	struct pakfileheader header;
	int offset = 0;

	while (offset < g_Paks[device].pdnumbytes) {
		int ret = pakReadHeaderAtOffset(device, offset, &header);
		offset += header.filelen;

		if (ret == PAK_ERR2_NOPAK) {
			return true;
		}

		if (PAKFILETYPE_TERMINATOR == header.filetype) {
			*outlen = offset;
			return false;
		}

		if (ret != PAK_ERR2_OK) {
			return false;
		}
	}

	return false;
}

//const char var7f1b4508[] = "RWI : Warning : tOffset > gPakObj[PakNum].GameFileSize\n";

/**
 * Read a file from cache or from the pak and write it to *data.
 */
int pak0f11b86c(int8_t device, uint32_t offset, uint8_t *data, struct pakfileheader *header, int bodylen)
{
	struct pakfileheader headerstack;
	int s0;
	int negativebodylen2;
	bool negativebodylen;
	int ret;
	bool isoneblock;
	uint32_t i;
	uint32_t filelen;
	uint32_t blocksize;
	uint32_t alignedfilelen;
	uint32_t stack[3];
	uint8_t buffer[16];
	int offsetinblock;
	int blocknum;
	uint8_t sp58[128];

	if (bodylen == -1) {
		negativebodylen = true;
		negativebodylen2 = true;
		bodylen = 0;
	} else {
		negativebodylen = false;
		negativebodylen2 = false;
	}

	blocksize = pakGetBlockSize(device);
	isoneblock = bodylen && data && (bool)(bodylen + sizeof(struct pakfileheader) <= blocksize);

	if (header == NULL) {
		header = &headerstack;
	}

	ret = pakReadHeaderAtOffset(device, offset, header);

	if (ret != 0) {
		return ret;
	}

	if (!negativebodylen2 && !header->occupied) {
		return 10;
	}

	if (isoneblock) {
		if (pakRetrieveBlockFromCache(device, offset, buffer)) {
			for (i = 0; i < bodylen; i++) {
				data[i] = buffer[sizeof(struct pakfileheader) + i];
			}

			return 0;
		}
	}

	if (bodylen == header->bodylen) {
		bodylen = 0;
	}

	alignedfilelen = pakGetAlignedFileLenByBodyLen(device, header->bodylen);

	filelen = (bodylen == 0 ? header->bodylen : bodylen) + sizeof(struct pakfileheader);

	if (negativebodylen) {
		filelen = alignedfilelen;
	}

	joyDisableCyclicPolling(JOYARGS(4008));

	for (i = 0; i != filelen; i++) {
		offsetinblock = i % pakGetBlockSize(device);
		blocknum = i / pakGetBlockSize(device);

		if (offsetinblock == 0) {
			int absoluteoffset = pakGetBlockSize(device) * blocknum + offset;
			int ret;

			ret = pakReadWriteBlock(device, PFS(device), g_Paks[device].pdnoteindex, 0, absoluteoffset, pakGetBlockSize(device), sp58); // Read

			if (!pakHandleResult(ret, device, true, LINE_4394)) {
				joyEnableCyclicPolling(JOYARGS(4032));

				if (ret == 1) {
					return 1;
				}

				return 4;
			}
		}

		if (i >= 0x10 && data != NULL) {
			*data = sp58[offsetinblock];
			data++;
		}
	}

	joyEnableCyclicPolling(JOYARGS(4054));

	return 0;
}

/*const char var7f1b4540[] = "Pak %d -> Pak_DeleteFile_Offset - DataSize = %u\n";
const char var7f1b4574[] = "Pak %d -> Delete file offset (file id %d) failed\n";
const char var7f1b45a8[] = "Pak %d -> Delete file offset failed - Bad Offset passed\n";*/

bool pakReplaceFileAtOffsetWithBlank(int8_t device, uint32_t offset)
{
	struct pakfileheader header;
	int result;

	result = pakReadHeaderAtOffset(device, offset, &header);

	if (result == PAK_ERR2_OK) {
		result = pakWriteFileAtOffset(device, offset, PAKFILETYPE_BLANK, NULL, header.filelen - sizeof(struct pakfileheader), NULL, NULL, 0, 1);

		if (result == 0) {
			return true;
		}
	}

	return false;
}

int pakWriteFileAtOffset(int8_t device, uint32_t offset, uint32_t filetype, uint8_t *newdata, int bodylenarg, int *outfileid, uint8_t *olddata, uint32_t fileid, uint32_t generation)
{
	uint8_t headerbytes[sizeof(struct pakfileheader)];
	struct pakfileheader *headerptr;
	uint32_t blocksize;
	int filelen;
	int bodylen;
	int paddinglen;
	uint8_t newfilebytes[4096];
	uint8_t oldfilebytes[4096];
	uint32_t numblocks;
	int i = 0;
	int j;
	int k;
	int result;
	uint8_t version;
	struct pakfileheader *newheader;
	struct pakfileheader *oldheader;
	uint8_t *oldfileu8ptr;
	uint8_t *newfileu8ptr;
	uint8_t *headeru8ptr;

	blocksize = pakGetBlockSize(device);

	generation &= 0x1ff;
	bodylen = bodylenarg ? bodylenarg : pakGetBodyLenByType(device, filetype);
	filelen = pakGetAlignedFileLenByBodyLen(device, bodylen);

	// Build the header bytes on the stack
	headerptr = (struct pakfileheader *) headerbytes;
	headerptr->fileid = fileid ? fileid : ++g_Paks[device].maxfileid;
	headerptr->deviceserial = g_Paks[device].serial;
	headerptr->filelen = filelen;

	version = argFindByPrefix(1, "-forceversion") ? 1 : 0;

	headerptr->version = version;
	headerptr->bodylen = bodylen;
	headerptr->generation = generation;
	headerptr->filetype = filetype;
	headerptr->fileid &= 0x7f;

	if (outfileid != NULL) {
		*outfileid = headerptr->fileid;
	}

	headerptr->occupied = newdata ? 1 : 0;

	if (headerptr->occupied) {
		pakCalculateChecksum(newdata, newdata + headerptr->bodylen, headerptr->bodysum);
	} else {
		headerptr->bodysum[0] = 0xffff;
		headerptr->bodysum[1] = 0xffff;
	}

	// Build "old" and "new" versions of the complete file on the stack.
	// "old" is what is believed to be on the pak already, and "new" is what is
	// needed to be written if any. Either the olddata or newdata pointers can
	// be null when creating or deleting files, so substitute their bytes with
	// a plus sign if so.
	// These will then be compared block by block to decide which blocks need
	// to be written to the pak.
	newheader = (struct pakfileheader *) newfilebytes;
	paddinglen = filelen - bodylen - sizeof(struct pakfileheader);
	headeru8ptr = headerbytes;
	newfileu8ptr = newfilebytes;
	oldfileu8ptr = oldfilebytes;

	// Header
	for (i = 0; i < sizeof(struct pakfileheader); i++) {
		*newfileu8ptr = headeru8ptr[i];
		*oldfileu8ptr = '+';
		newfileu8ptr++;
		oldfileu8ptr++;
	}

	// Data
	for (i = 0; i != bodylen; i++) {
		*newfileu8ptr = newdata ? newdata[i] : '+';
		*oldfileu8ptr = olddata ? olddata[i] : '+';
		newfileu8ptr++;
		oldfileu8ptr++;
	}

	// Data padding to reach alignment
	for (i = 0; i != paddinglen; i++) {
		*newfileu8ptr = newdata ? newdata[i] : '+';
		*oldfileu8ptr = olddata ? olddata[i] : '+';
		newfileu8ptr++;
		oldfileu8ptr++;
	}

	numblocks = filelen / blocksize;

	if ((filelen % blocksize) != 0) {
		numblocks++;
	}

	joyDisableCyclicPolling(JOYARGS(4292));

	// Write the header with writecompleted = 0, followed by the data, then
	// rewrite the header with writecompleted = 1. This allows the game to
	// detect if data on the pak was only partially written.
	for (j = 0; j < 2; j++) {
		newheader->writecompleted = j ? 1 : 0;

		// Checksum the header part after the checksums themselves
		pakCalculateChecksum((uint8_t *) (newheader->bodysum + 2), (uint8_t *) (newheader + 1), newheader->headersum);

		for (i = 0; i != numblocks; i++) {
			int offsetinfile = pakGetBlockSize(device) * i;
			bool writethisblock = false;

			if (offsetinfile < sizeof(struct pakfileheader)) {
				// Header - always write it
				writethisblock = true;
			} else {
				// Don't write data on the second iteration
				if (j == 1) {
					break;
				}

				// Don't write data for blank files or if the file is being deleted
				if (newheader->filetype == PAKFILETYPE_BLANK || newdata == NULL) {
					break;
				}

				if (olddata) {
					// Check if any bytes in the old and new blocks are different
					for (k = 0; k < blocksize; k++) {
						int index = i * blocksize + k;

						if (newfilebytes[index] != oldfilebytes[index]) {
							writethisblock = true;
							break;
						}
					}
				} else {
					writethisblock = true;
				}
			}

			if (writethisblock) {
				result = pakReadWriteBlock(device, PFS(device), g_Paks[device].pdnoteindex, 1, offset + i * blocksize, pakGetBlockSize(device), &newfilebytes[offsetinfile]); // Write

				if (!pakHandleResult(result, device, true, LINE_4742)) {
					joyEnableCyclicPolling(JOYARGS(4380));

					if (result == PAK_ERR1_NOPAK) {
						return 1;
					}

					return 4;
				}
			}
		}
	}

	joyEnableCyclicPolling(JOYARGS(4393));

	if (g_PakDebugPakCache) {
		pakSaveHeaderToCache(device, offset / pakGetBlockSize(device), newheader);
	}

	return 0;
}

bool pakRepair(int8_t device)
{
	int result;

	switch (g_Paks[device].state) {
	case PAKSTATE_MEM_ENTER_DEVICEERROR:
	case PAKSTATE_MEM_DEVICEERROR:
		break;
	default:
		joyDisableCyclicPolling(JOYARGS(4425));
		result = 1;
		joyEnableCyclicPolling(JOYARGS(4427));

		if (result == PAK_ERR1_OK) {
			g_Paks[device].state = PAKSTATE_PROBE;
			return true;
		}

		pakHandleResult(result, device, false, LINE_4801);

		g_Paks[device].state = PAKSTATE_22;
		break;
	}

	return false;
}

/*const char var7f1b45f4[] = "PakMac_PaksLive()=%x\n";
const char var7f1b460c[] = "paksNeedToBeLive4Game=%x\n";
const char var7f1b4628[] = "paksNeedToBeLive4Menu=%x\n";
const char var7f1b4644[] = "g_LastPackPattern=%x\n";*/

bool pakHandleResult(int err1, int8_t device, bool arg2, uint32_t line)
{
	if (err1 == PAK_ERR1_OK) {
		return true;
	}

	if (arg2) {
		switch (err1) {
		case PAK_ERR1_NOPAK:
			g_Paks[device].type = PAKTYPE_MEMORY;
			g_Paks[device].state = PAKSTATE_UNPLUGGING;
			break;
		case PAK_ERR1_DEVICE:
			g_Paks[device].type = PAKTYPE_MEMORY;
			g_Paks[device].state = PAKSTATE_MEM_ENTER_DEVICEERROR;
			break;
		case PAK_ERR1_INCONSISTENT:
		case PAK_ERR1_IDFATAL:
			g_Paks[device].type = PAKTYPE_MEMORY;
			g_Paks[device].state = PAKSTATE_MEM_ENTER_CORRUPT;
			break;
		case PAK_ERR1_DATAFULL:
		case PAK_ERR1_DIRFULL:
			g_Paks[device].type = PAKTYPE_MEMORY;
			g_Paks[device].state = PAKSTATE_MEM_ENTER_FULL;
			break;
		}
	}

	switch (err1) {
	case PAK_ERR1_NOPAK:
	case PAK_ERR1_NEWPAK:
	case PAK_ERR1_INCONSISTENT:
	case PAK_ERR1_CONTRFAIL:
	case PAK_ERR1_INVALID:
	case PAK_ERR1_BADDATA:
	case PAK_ERR1_DATAFULL:
	case PAK_ERR1_DIRFULL:
	case PAK_ERR1_EXIST:
	case PAK_ERR1_IDFATAL:
	case PAK_ERR1_DEVICE:
	case PAK_ERR1_NOGBCART:
	case PAK_ERR1_NEWGBCART:
	case PAK_ERR1_EEPROMMISSING:
	case PAK_ERR1_EEPROMREADFAILED:
	case PAK_ERR1_EEPROMWRITEFAILED:
	case PAK_ERR1_EEPROMINVALIDOP:
	case PAK_ERR1_EEPROMINVALIDARG:
		break;
	}

	return false;
}

void paksTick(void)
{
	int i;

	if (g_Vars.pakstocheck) {
		g_MpPlayerNum = 0;

		menuSetBanner(MENUBANNER_CHECKINGPAK, true);

		g_ShowPakMenuBanner = false;

		if (g_Vars.pakstocheck & 0x0f) {
			joySetPfsPollEnabled(0);

			// Waiting for some timer
			if ((g_Vars.pakstocheck & 0x0f) >= 10) {
				if ((g_Vars.lvframenum % 7) == 0) {
					g_Vars.pakstocheck--;
				}
			} else {
				g_Vars.pakstocheck--;
			}
		} else {
			joyPollPfs(2);

			for (i = 0; i < 4; i++) {
				if (g_Vars.pakstocheck & (1 << (i + 4))) {
					pak0f1169c8(i, true);
					g_Vars.pakstocheck &= ~(1 << (i + 4));
				} else if (g_Vars.pakstocheck & (1 << (i + 8))) {
					pak0f1169c8(i, false);
					g_Vars.pakstocheck &= ~(1 << (i + 8));
				}
			}

			if (!joyIsPfsPollEnabled()) {
				joySetPfsPollEnabled(true);
				joySetDefaultPfsPollInterval();
			}

			menuSetBanner(-1, true);

			g_ShowPakMenuBanner = true;
		}
	}
}

void pak0f11c6d0(void)
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		switch (g_Paks[i].state) {
		case PAKSTATE_PROBE:
		case PAKSTATE_MEM_DISPATCH:
		case PAKSTATE_MEM_PRE_PREPARE:
		case PAKSTATE_MEM_PREPARE:
		case PAKSTATE_MEM_POST_PREPARE:
			g_Paks[i].state = PAKSTATE_UNPLUGGING;
			g_PaksPlugged &= ~(1 << i);
			g_MpPlayerNum = i;
			menuSetBanner(-1, true);
			break;
		}
	}

	g_JoyPfsPollMasterEnabled = true;
}

void pakExecuteDebugOperations(void)
{
	bool disablepolling = false;
	int8_t i;

	if (g_PakDebugPakDump) {
		g_PakDebugPakDump = false;
	}

	if (g_PakDebugWipeEeprom) {
		pakWipe(SAVEDEVICE_GAMEPAK, 0, 0x80);
		g_PakDebugWipeEeprom = false;
	}

	if (g_PakDebugPakInit) {
		int device = g_PakDebugPakInit - 1;

		joyDisableCyclicPolling();
		pakInitPak(PFS(device), device, 0);
		joyEnableCyclicPolling();

		g_PakDebugPakInit = false;
	}

	if (g_PakDebugForceScrub) {
		pakCreateFilesystem(SAVEDEVICE_GAMEPAK);
		g_PakDebugForceScrub = false;
	}

	pakCheckPlugged();

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		if (g_Paks[i].features) {
			pakTickState(i);
		}
	}

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		switch (g_Paks[i].state) {
		case PAKSTATE_PROBE:
		case PAKSTATE_MEM_DISPATCH:
		case PAKSTATE_MEM_PRE_PREPARE:
		case PAKSTATE_MEM_PREPARE:
		case PAKSTATE_MEM_POST_PREPARE:
			disablepolling = true;
			break;
		}
	}

	if (disablepolling) {
		g_JoyPfsPollMasterEnabled = false;
	} else {
		g_JoyPfsPollMasterEnabled = true;
	}
}

void pakCheckPlugged(void)
{
	if (g_Vars.tickmode != TICKMODE_CUTSCENE || g_MenuData.count > 0) {
		uint8_t oldplugged = g_PaksPlugged;
		uint8_t newplugged = g_PaksPlugged;
		uint8_t paksconnected = 0xff;
		int i;

		if ((g_Vars.pakstocheck & 0xf) == 0) {
			for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
				if ((g_Vars.paksneededforgame | g_Vars.paksneededformenu) & (1 << i)) {
					if (paksconnected == 0xff) {
						paksconnected = joyShiftPfsStates();
					}

					if (((paksconnected & (1 << i)) != (oldplugged & (1 << i)))) {
						if ((paksconnected & (1 << i))) {
							// pak connected
							g_Paks[i].state = PAKSTATE_PROBE;
							newplugged |= (1 << i);
						} else {
							// pak unplugged
							g_Paks[i].state = PAKSTATE_UNPLUGGING;
							newplugged &= ~(1 << i);
							filelistInvalidatePak(i);
						}
					}
				}
			}

			g_PaksPlugged = newplugged;
		}
	}
}

void gbpakHandleError(uint32_t err)
{
	switch (err) {
		case PFS_ERR_NOPACK:
		case PFS_ERR_NEW_PACK:
		case PFS_ERR_INCONSISTENT:
		case PFS_ERR_CONTRFAIL:
		case PFS_ERR_INVALID:
		case PFS_ERR_BAD_DATA:
		case PFS_DATA_FULL:
		case PFS_DIR_FULL:
		case PFS_ERR_EXIST:
		case PFS_ERR_ID_FATAL:
		case PFS_ERR_DEVICE:
		case PFS_ERR_NO_GBCART:
		case PFS_ERR_NEW_GBCART:
			break;
	}
}

void pakRumble(int device, float numsecs, int onduration, int offduration)
{
	if (g_Paks[device].state == PAKSTATE_READY
			&& g_Paks[device].type == PAKTYPE_RUMBLE
			&& g_Paks[device].rumblestate != RUMBLESTATE_DISABLED_STOPPING
			&& g_Paks[device].rumblestate != RUMBLESTATE_DISABLED_STOPPED
			&& g_Paks[device].rumblettl < 60 * numsecs) {
		g_Paks[device].rumblestate = RUMBLESTATE_ENABLED_STARTING;
		g_Paks[device].rumblettl = 60 * numsecs;
		g_Paks[device].rumblepulsestopat = onduration;
		g_Paks[device].rumblepulselen = onduration + offduration;
		g_Paks[device].rumblepulsetimer = 0;
	}
}

void paksStop(bool disablepolling)
{
	int8_t i;

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		int type = g_Paks[i].type;

		if (type);

		if (type != PAKTYPE_MEMORY && type != PAKTYPE_GAMEBOY) {
			joyStopRumble(i, disablepolling);
		}
	}
}

void pakDisableRumbleForPlayer(int8_t playernum)
{
	int i;
	int tmp = playernum;
	int contpads[2];

	joyGetContpadNumsForPlayer(tmp, &contpads[0], &contpads[1]);

	for (i = 0; i < 2; i++) {
		if (contpads[i] >= 0 && g_Paks[contpads[i]].type == PAKTYPE_RUMBLE) {
			g_Paks[contpads[i]].rumblestate = RUMBLESTATE_DISABLED_STOPPING;
			joyStopRumble(contpads[i], true);
		}
	}
}

void pakEnableRumbleForPlayer(int8_t playernum)
{
	int i;
	int tmp = playernum;
	int contpads[2];

	joyGetContpadNumsForPlayer(tmp, &contpads[0], &contpads[1]);

	for (i = 0; i < 2; i++) {
		if (contpads[i] >= 0
				&& g_Paks[contpads[i]].type == PAKTYPE_RUMBLE
				&& g_Paks[contpads[i]].rumblestate == RUMBLESTATE_DISABLED_STOPPED) {
			g_Paks[contpads[i]].rumblestate = RUMBLESTATE_ENABLING;
		}
	}
}

void pakDisableRumbleForAllPlayers(void)
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		if (g_Paks[i].type == PAKTYPE_RUMBLE) {
			g_Paks[i].rumblestate = RUMBLESTATE_DISABLED_STOPPING;
			joyStopRumble(i, true);
		}
	}
}

void pakEnableRumbleForAllPlayers(void)
{
	int i;

	for (i = 0; i < MAX_PLAYERS; i++) {
		if (g_Paks[i].type == PAKTYPE_RUMBLE && g_Paks[i].rumblestate == RUMBLESTATE_DISABLED_STOPPED) {
			g_Paks[i].rumblestate = RUMBLESTATE_ENABLING;
		}
	}
}

void pakTickState(int8_t device)
{
	switch (g_Paks[device].state) {
	case PAKSTATE_NOPAK:
		break;
	case PAKSTATE_UNPLUGGING:
		g_Paks[device].state = PAKSTATE_NOPAK;
		g_Paks[device].plugcount++;
		g_Paks[device].showdatalost = false;
		g_Paks[device].type = PAKTYPE_NONE;

		SETBANNER(-1);
		break;
	case PAKSTATE_PROBE:
		SETBANNER(-1);
		pakProbe(device);
		break;
	case PAKSTATE_MEM_DISPATCH:
		if (g_Vars.paksneededformenu & (1 << device))
		{
			if (device == SAVEDEVICE_GAMEPAK) {
				g_Paks[device].state = PAKSTATE_MEM_PREPARE;
			} else {
				g_Paks[device].state = PAKSTATE_MEM_PRE_PREPARE;
			}
		} else {
			g_Paks[device].state = PAKSTATE_07;
		}
		break;
	case PAKSTATE_07:
		if (g_Vars.paksneededformenu & (1 << device))
		{
			g_Paks[device].state = PAKSTATE_PROBE;
		}
		break;
	default:
		break;
	case PAKSTATE_MEM_PRE_PREPARE:
		g_MpPlayerNum = device;

		SETBANNER(MENUBANNER_CHECKINGPAK);

		g_Paks[device].state = PAKSTATE_MEM_PREPARE;
		break;
	case PAKSTATE_MEM_PREPARE:
		joyDisableCyclicPolling();
		mempakPrepare(device);
		joyEnableCyclicPolling();
		break;
	case PAKSTATE_MEM_POST_PREPARE:
		SETBANNER(-1);
		g_Paks[device].state = PAKSTATE_READY;
		break;
	case PAKSTATE_GB_PRE_PREPARE:
		// Related to PerfectHead
		/*if (func0f14aea0(device)) {
			g_Paks[device].state = PAKSTATE_GB_PREPARE;
		}*/
		break;
	case PAKSTATE_GB_PREPARE:
		g_Paks[device].unk008 = PAK008_00;
		g_Paks[device].state = PAKSTATE_GB_POST_PREPARE1;
		break;
	case PAKSTATE_GB_POST_PREPARE1:
		g_Paks[device].state = PAKSTATE_GB_POST_PREPARE2;
		break;
	case PAKSTATE_GB_POST_PREPARE2:
		g_Paks[device].state = PAKSTATE_GB_POST_PREPARE3;
		break;
	case PAKSTATE_GB_POST_PREPARE3:
		g_Paks[device].state = PAKSTATE_READY;
		break;
	case PAKSTATE_MEM_ENTER_CORRUPT:
		SETBANNER(-1);

		if ((g_Vars.paksneededformenu & (1 << device)))
		{
			if (menuIsReadyForPakError(device, PAKERRORDIALOG_CORRUPT)) {
				menuPushPakErrorDialog(device, PAKERRORDIALOG_CORRUPT);
				g_Paks[device].state = PAKSTATE_MEM_CORRUPT;
			}
		}
		break;
	case PAKSTATE_GB_OPEN_UNREADABLE:
		if (menuIsReadyForPakError(device, PAKERRORDIALOG_GB_UNREADABLE)) {
			menuPushPakErrorDialog(device, PAKERRORDIALOG_GB_UNREADABLE);
			g_Paks[device].state = PAKSTATE_GB_IDLE_UNREADABLE;
		}
		break;
	case PAKSTATE_GB_IDLE_UNREADABLE:
		g_Paks[device].state = PAKSTATE_READY;
		break;
	case PAKSTATE_MEM_ENTER_DEVICEERROR:
		SETBANNER(-1);

		if (g_Vars.paksneededformenu & (1 << device))
		{
			if (menuIsReadyForPakError(device, PAKERRORDIALOG_DEVICEERROR)) {
				menuPushPakErrorDialog(device, PAKERRORDIALOG_DEVICEERROR);
				g_Paks[device].state = PAKSTATE_MEM_DEVICEERROR;
			}
		}
		break;
	case PAKSTATE_MEM_ENTER_FULL:
		SETBANNER(-1);

		if (g_Vars.paksneededformenu & (1 << device))
		{
			if (menuIsReadyForPakError(device, PAKERRORDIALOG_FULL)) {
				menuPushPakErrorDialog(device, PAKERRORDIALOG_FULL);
				g_Paks[device].state = PAKSTATE_MEM_FULL;
			}
		}
		break;
	case PAKSTATE_MEM_FULL:
		SETBANNER(-1);
		break;
	case PAKSTATE_22:
		SETBANNER(-1);
		break;
	}

	if (g_Paks[device].showdatalost) {
		SETBANNER(-1);

		if (menuIsReadyForPakError(device, PAKERRORDIALOG_DATALOST)) {
			menuPushPakErrorDialog(device, PAKERRORDIALOG_DATALOST);
			g_Paks[device].showdatalost = false;
		}
	}
}

void pakProbeEeprom(void)
{
	joyDisableCyclicPolling(JOYARGS(6199));
	joyEnableCyclicPolling(JOYARGS(6201));

	g_PakHasEeprom = true;

	if (argFindByPrefix(1, "-scrub")) {
		pakCreateFilesystem(SAVEDEVICE_GAMEPAK);
	}
}

PakErr1 pakReadEeprom(uint8_t address, uint8_t *buffer, uint32_t len)
{
	int result;

	joyDisableCyclicPolling(JOYARGS(6234));
	result = osEepromLongRead(address, buffer, len);
	joyEnableCyclicPolling(JOYARGS(6236));

	return result == PAK_ERR1_OK ? PAK_ERR1_OK : PAK_ERR1_EEPROMREADFAILED;
}

PakErr1 pakWriteEeprom(uint8_t address, uint8_t *buffer, uint32_t len)
{
	int result;

	joyDisableCyclicPolling(JOYARGS(6269));
	result = osEepromLongWrite(address, buffer, len);
	joyEnableCyclicPolling(JOYARGS(6271));

	return result == PAK_ERR1_OK ? PAK_ERR1_OK : PAK_ERR1_EEPROMWRITEFAILED;
}

void pakSetBitflag(int flagnum, uint8_t *bitstream, bool set)
{
	uint32_t byteindex = (uint32_t)flagnum / 8;
	uint8_t mask = 1 << ((uint32_t)flagnum % 8);

	if (set) {
		bitstream[byteindex] |= mask;
	} else {
		bitstream[byteindex] &= (uint8_t)~mask;
	}
}

bool pakHasBitflag(uint32_t flagnum, uint8_t *bitstream)
{
	uint32_t byteindex = flagnum / 8;
	uint8_t mask = 1 << (flagnum % 8);

	return bitstream[byteindex] & mask ? 1 : 0;
}

void pakClearAllBitflags(uint8_t *flags)
{
	int i;

	for (i = 0; i <= GAMEFILEFLAG_4E; i++) {
		pakSetBitflag(i, flags, false);
	}
}

/**
 * The note name and note extension are stored on the pak using N64 font code.
 * This is different to ASCII.
 *
 * This function expects src to be a pointer to an N64 font code string.
 * It converts it to ASCII and writes it to dst. Characters are replaced with
 * an asterisk if they are invalid font codes or if the character doesn't exist
 * in PD's font.
 */
void pakN64FontCodeToAscii(char *src, char *dst, int len)
{
	char buffer[256];
	int i;
	int in;
	char c;
	char *ptr = buffer;

	for (i = 0; i < len;) {
		in = *src;
		src++;
		i++;
		c = '*';

		// @bug: The length check of the map is off by 1. The last char in the
		// list is '@', so if an @ sign appears in a note name then PD will
		// incorrectly replace it with '*' when displaying the name.
		// The original source likely used a literal here instead of sizeof().
		if (in < (int)(sizeof(g_N64FontCodeMap) - 1)) {
			c = g_N64FontCodeMap[in];
		}

		// PD has a double quote in its fonts, but I guess it doesn't render
		// very well. So it gets replaced with two single quotes.
		if ((uint32_t)c == '"') {
			*ptr = '\'';
			ptr++;
			*ptr = '\'';
		} else {
			*ptr = c;
		}

		ptr++;
	}

	*ptr = '\0';

	strcpy(dst, buffer);
}

int8_t pakFindBySerial(int findserial)
{
	int8_t device = -1;
	int i;

	for (i = 0; i < ARRAYCOUNT(g_Paks); i++) {
		if (mempakIsReady(i)) {
			int serial = pakGetSerial(i);

			if (findserial == serial) {
				device = i;
			}
		}
	}

	return device;
}

bool gbpakIsAnyPerfectDark(void)
{
	return g_ValidGbcRomFound;
}