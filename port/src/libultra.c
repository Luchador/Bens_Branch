#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <PR/os.h>
#include <PR/R4300.h>
#include <PR/ultratypes.h>
#include "platform.h"
#include "system.h"
#include "input.h"
#include "video.h"
#include "audio.h"
#include "fs.h"

#define EEPROM_SIZE (256 * 8)
#define EEPROM_FNAME "eeprom.bin"
#define EEPROM_PATH "$S/" EEPROM_FNAME

#define OS_COUNTER_RATE 46875000ULL
#define OS_COUNTER_NUM (OS_COUNTER_RATE / 1000ULL)
#define OS_COUNTER_DEN (1000000ULL / 1000ULL)

static u8 eeprom[EEPROM_SIZE];
static char eepromPath[FS_MAXPATH + 1];
static s32 eepromLoaded = 0;

/* Time */

OSTime osGetTime(void)
{
	// u64 should be enough to last a while
	return (sysGetMicroseconds() * OS_COUNTER_NUM) / OS_COUNTER_DEN;
}

u32 osGetCount(void)
{
	return (u32)osGetTime();
}

/* Ai */

u32 osAiGetLength(void)
{
	return audioGetBytesBuffered();
}

s32 osAiSetNextBuffer(void *bufPtr, u32 size)
{
	audioSetNextBuffer(bufPtr, size);
	return 0;
}

/* Cont */

s32 osContInit(OSMesgQueue *mesgq, u8 *bitpattern, OSContStatus *data)
{
	if (bitpattern) {
		*bitpattern = inputControllerMask();
	}
	if (data) {
		osContGetQuery(data);
	}
	return 0;
}

void osContGetReadData(OSContPad *pad)
{
	// game always passes in an array of 4 OSContPads
	for (s32 i = 0; i < MAXCONTROLLERS; ++i, ++pad) {
		pad->button = 0;
		pad->stick_x = 0;
		pad->stick_y = 0;
		pad->rstick_x = 0;
		pad->rstick_y = 0;
		if (inputReadController(i, pad) < 0) {
			pad->errnum = CONT_NO_RESPONSE_ERROR;
		} else {
			pad->errnum = 0;
		}
	}
}

void osContGetQuery(OSContStatus *status)
{
	// also always 4 status structs here
	for (s32 i = 0; i < MAXCONTROLLERS; ++i, ++status) {
		if (inputControllerConnected(i)) {
			status->errnum = 0;
			status->type = CONT_ABSOLUTE;
			status->status = CONT_CARD_ON;
		} else {
			status->errnum = CONT_NO_RESPONSE_ERROR;
			status->type = 0;
			status->status = 0;
		}
	}
}

/* Motor */

s32 osMotorProbe(OSMesgQueue *ctrlrqueue, OSPfs* pfs, s32 channel)
{
	if (pfs && inputRumbleSupported(channel)) {
		pfs->queue = ctrlrqueue;
		pfs->channel = channel;
		pfs->activebank = 0xff;
		pfs->status = 0x8; // PFS_MOTOR_INITIALIZED
		return 0;
	}
	return PFS_ERR_NOPACK;
}

s32 __osMotorAccess(OSPfs *pfs, s32 cmd)
{
	if (!pfs || pfs->channel < 0 || pfs->channel >= INPUT_MAX_CONTROLLERS) {
		return PFS_ERR_NOPACK;
	}

	const f32 strength = (f32)(cmd == MOTOR_START);
	inputRumble(pfs->channel, strength, 5.f); // hope someone turns it off in those 5 seconds

	return 0;
}

/* Eeprom */

static inline void osEepromSetPath(void)
{
	const char *extPath = sysArgGetString("--eeprom-file");
	if (extPath && extPath[0]) {
		if (extPath[0] == '$' || fsPathIsAbsolute(extPath) || fsPathIsCwdRelative(extPath)) {
			strncpy(eepromPath, extPath, FS_MAXPATH);
		} else {
			// just a filename, look for it in the save dir
			snprintf(eepromPath, FS_MAXPATH, "$S/%s", extPath);
		}
	} else {
		strncpy(eepromPath, EEPROM_PATH, FS_MAXPATH);
	}
}

static inline void osEeepromLoad(const char *fname)
{
	if (!eepromLoaded) {
		eepromLoaded = 1;
		FILE *fp = fsFileOpenRead(fname);
		if (fp) {
			fread(eeprom, 1, EEPROM_SIZE, fp);
			fsFileFree(fp);
		} else {
			sysLogPrintf(LOG_NOTE, "could not read EEPROM from `%s`: %s", fsFullPath(fname), strerror(errno));
		}
	}
}

static inline void osEeepromSave(const char *fname)
{
	FILE* fp = fsFileOpenWrite(fname);
	if (fp) {
		fwrite(eeprom, 1, EEPROM_SIZE, fp);
		fsFileFree(fp);
	} else {
		sysLogPrintf(LOG_ERROR, "could not save EEPROM to `%s`: %s", fsFullPath(fname), strerror(errno));
	}
}

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(buffer, eeprom + address * 8, nbytes);

	return 0;
}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, int nbytes)
{
	if (!eepromPath[0]) {
		osEepromSetPath();
	}

	osEeepromLoad(eepromPath);

	memcpy(eeprom + address * 8, buffer, nbytes);

	osEeepromSave(eepromPath);

	return 0;
}

/* Pfs */

s32 osPfsIsPlug(OSMesgQueue *queue, u8 *pattern)
{
	if (pattern) {
		*pattern = 0;
		for (s32 i = 0; i < MAXCONTROLLERS; ++i) {
			if (inputRumbleSupported(i)) {
				*pattern |= 1 << i;
			}
		}
	}
	return 0;
}

/* Gbpak */

s32 osGbpakInit(OSMesgQueue *queue, OSPfs *pfs, s32 ch)
{
	return 1;
}

s32 osGbpakPower(OSPfs *pfs, s32 flag)
{
	return 1;
}

s32 osGbpakReadWrite(OSPfs *pfs, u16 flag, u16 addr, u8 *buf, u16 size)
{
	return 1;
}

s32 osGbpakReadId(OSPfs *pfs, OSGbpakId *id, u8 *status)
{
	return 1;
}

s32 osPiStartDma(OSIoMesg *mb, uintptr_t devAddr, void *vAddr, u32 nbytes, OSMesgQueue *mq)
{
	memcpy(vAddr, (const void *)devAddr, nbytes);
	return 0;
}

uintptr_t osVirtualToPhysical(void *addr)
{
	return (uintptr_t)addr;
}

u32 osGetMemSize(void)
{
	return 16 * 1024 * 1024; /* expansion pak installed plus some extra */
}

/* libc compatibility wrappers */

#ifndef PLATFORM_OSX

void bzero(void *ptr, size_t size)
{
	memset(ptr, 0, size);
}

void bcopy(const void *src, void *dst, size_t n)
{
	memcpy(dst, src, n);
}

s32 bcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}

#endif
