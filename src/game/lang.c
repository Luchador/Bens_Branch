#include <ultra64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "game/file.h"
#include "game/lang.h"
#include "game/debug.h"
#include "game/mplayer/mplayer.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"
#include "platform.h"
#ifndef PLATFORM_N64
#include "video.h"
#endif

u16 *g_FrameBuffers[NUM_FRAMEBUFFERS];

/**
 * Officially, the NTSC versions are American English only, while the PAL
 * versions support British English, French, German, Italian and Spanish, and
 * the Japanese version is Japanese only. However all versions actually have
 * support for all 7 languages, despite some being inaccessible and not even
 * translated.
 *
 * All strings are categorised into areas, which we call banks. There is one
 * bank per stage, as well as some common banks such as menus and weapon names.
 * Each bank contains 7 files; one for each translation.
 *
 * An example language filename is LoptionsE. "options" is the bank name, and E
 * means it's the American English file.
 *
 * The list of suffixes are:
 * E - American English
 * J - Japanese
 * P - British English (P is for PAL)
 * _str_f = French
 * _str_g = German
 * _str_i = Italian
 * _str_s = Spanish
 *
 * Banks are loaded based on the current stage. When loading a bank, only the
 * current translation is loaded.
 *
 * Text is referenced using a 16 bit text ID. The text ID is made up of a 7-bit
 * bank index and a 9-bit string index within that bank:
 *
 *     bbbbbbbs ssssssss
 *
 * For versions prior to PAL, the language files are loaded during stage load
 * and allocated from memp. This method doesn't work for PAL because PAL
 * supports changing the language mid-stage and memp allocations cannot be freed
 * until a new stage is loaded. So for PAL, during stage load a single buffer is
 * allocated from memp to hold all loaded language files. The buffer is
 * repopulated when the language is changed, and the lang banks point to their
 * relevant offsets within that buffer.
 */

uintptr_t *g_LangBanks[69];

u16 g_LangFiles[] = {
	/* 0*/ 0,
	/* 1*/ FILE_LAMEE,
	/* 2*/ FILE_LARCHE,
	/* 3*/ FILE_LARKE,
	/* 4*/ FILE_LASHE,
	/* 5*/ FILE_LAZTE,
	/* 6*/ FILE_LCATE,
	/* 7*/ FILE_LCAVEE,
	/* 8*/ FILE_LARECE,
	/* 9*/ FILE_LCRADE,
	/*10*/ FILE_LCRYPE,
	/*11*/ FILE_LDAME,
	/*12*/ FILE_LDEPOE,
	/*13*/ FILE_LDESTE,
	/*14*/ FILE_LDISHE,
	/*15*/ FILE_LEARE,
	/*16*/ FILE_LELDE,
	/*17*/ FILE_LIMPE,
	/*18*/ FILE_LJUNE,
	/*19*/ FILE_LLEEE,
	/*20*/ FILE_LLENE,
	/*21*/ FILE_LLIPE,
	/*22*/ FILE_LLUEE,
	/*23*/ FILE_LOATE,
	/*24*/ FILE_LPAME,
	/*25*/ FILE_LPETEE,
	/*26*/ FILE_LREFE,
	/*27*/ FILE_LRITE,
	/*28*/ FILE_LRUNE,
	/*29*/ FILE_LSEVBE,
	/*30*/ FILE_LSEVE,
	/*31*/ FILE_LSEVXE,
	/*32*/ FILE_LSEVXBE,
	/*33*/ FILE_LSHOE,
	/*34*/ FILE_LSILOE,
	/*35*/ FILE_LSTATE,
	/*36*/ FILE_LTRAE,
	/*37*/ FILE_LWAXE,
	/*38*/ FILE_LGUNE,
	/*39*/ FILE_LTITLEE,
	/*40*/ FILE_LMPMENUE,
	/*41*/ FILE_LPROPOBJE,
	/*42*/ FILE_LMPWEAPONSE,
	/*43*/ FILE_LOPTIONSE,
	/*44*/ FILE_LMISCE,
	/*45*/ FILE_LUFFE,
	/*46*/ FILE_LOLDE,
	/*47*/ FILE_LATEE,
	/*48*/ FILE_LLAME,
	/*49*/ FILE_LMP1E,
	/*50*/ FILE_LMP2E,
	/*51*/ FILE_LMP3E,
	/*52*/ FILE_LMP4E,
	/*53*/ FILE_LMP5E,
	/*54*/ FILE_LMP6E,
	/*55*/ FILE_LMP7E,
	/*56*/ FILE_LMP8E,
	/*57*/ FILE_LMP9E,
	/*58*/ FILE_LMP10E,
	/*59*/ FILE_LMP11E,
	/*60*/ FILE_LMP12E,
	/*61*/ FILE_LMP13E,
	/*62*/ FILE_LMP14E,
	/*63*/ FILE_LMP15E,
	/*64*/ FILE_LMP16E,
	/*65*/ FILE_LMP17E,
	/*66*/ FILE_LMP18E,
	/*67*/ FILE_LMP19E,
	/*68*/ FILE_LMP20E,
};

u16 g_TextFiles[] = {
	/* 0*/ 0,
	/* 1*/ FILE_LAMEE,
	/* 2*/ FILE_LARCHE,
	/* 3*/ FILE_LARKE,
	/* 4*/ FILE_LASHE,
	/* 5*/ FILE_LAZTE,
	/* 6*/ FILE_LCATE,
	/* 7*/ FILE_LCAVEE,
	/* 8*/ FILE_LARECE,
	/* 9*/ FILE_LCRADE,
	/*10*/ FILE_LCRYPE,
	/*11*/ FILE_LDAME,
	/*12*/ FILE_LDEPOE,
	/*13*/ FILE_LDESTE,
	/*14*/ FILE_LDISHE,
	/*15*/ FILE_LEARE,
	/*16*/ FILE_LELDE,
	/*17*/ FILE_LIMPE,
	/*18*/ FILE_LJUNE,
	/*19*/ FILE_LLEEE,
	/*20*/ FILE_LLENE,
	/*21*/ FILE_LLIPE,
	/*22*/ FILE_LLUEE,
	/*23*/ FILE_LOATE,
	/*24*/ FILE_LPAME,
	/*25*/ FILE_LPETEE,
	/*26*/ FILE_LREFE,
	/*27*/ FILE_LRITE,
	/*28*/ FILE_LRUNE,
	/*29*/ FILE_LSEVBE,
	/*30*/ FILE_LSEVE,
	/*31*/ FILE_LSEVXE,
	/*32*/ FILE_LSEVXBE,
	/*33*/ FILE_LSHOE,
	/*34*/ FILE_LSILOE,
	/*35*/ FILE_LSTATE,
	/*36*/ FILE_LTRAE,
	/*37*/ FILE_LWAXE,
	/*38*/ FILE_LGUNE,
	/*39*/ FILE_LTITLEE,
	/*40*/ FILE_LMPMENUE,
	/*41*/ FILE_LPROPOBJE,
	/*42*/ FILE_LMPWEAPONSE,
	/*43*/ FILE_LOPTIONSE,
	/*44*/ FILE_LMISCE,
	/*45*/ FILE_LUFFE,
	/*46*/ FILE_LOLDE,
	/*47*/ FILE_LATEE,
	/*48*/ FILE_LLAME,
	/*49*/ FILE_LMP1E,
	/*50*/ FILE_LMP2E,
	/*51*/ FILE_LMP3E,
	/*52*/ FILE_LMP4E,
	/*53*/ FILE_LMP5E,
	/*54*/ FILE_LMP6E,
	/*55*/ FILE_LMP7E,
	/*56*/ FILE_LMP8E,
	/*57*/ FILE_LMP9E,
	/*58*/ FILE_LMP10E,
	/*59*/ FILE_LMP11E,
	/*60*/ FILE_LMP12E,
	/*61*/ FILE_LMP13E,
	/*62*/ FILE_LMP14E,
	/*63*/ FILE_LMP15E,
	/*64*/ FILE_LMP16E,
	/*65*/ FILE_LMP17E,
	/*66*/ FILE_LMP18E,
	/*67*/ FILE_LMP19E,
	/*68*/ FILE_LMP20E,
};

u32 langGetLangBankIndexFromStagenum(s32 stagenum)
{
	u32 bank;

	switch (stagenum) {
	case STAGE_PELAGIC:       bank = LANGBANK_DAM; break;
	case STAGE_EXTRACTION:    bank = LANGBANK_ARK; break;
	case STAGE_MAIANSOS:      bank = LANGBANK_SEV; break;
	case STAGE_WAR:           bank = LANGBANK_STAT; break;
	case STAGE_CHICAGO:       bank = LANGBANK_PETE; break;
	case STAGE_G5BUILDING:    bank = LANGBANK_DEPO; break;
	case STAGE_ESCAPE:        bank = LANGBANK_TRA; break;
	case STAGE_MP_TEMPLE:     bank = LANGBANK_JUN; break;
	case STAGE_MP_PIPES:      bank = LANGBANK_CRAD; break;
	case STAGE_MP_G5BUILDING: bank = LANGBANK_CRYP; break;
	case STAGE_CITRAINING:    bank = LANGBANK_DISH; break;
	case STAGE_MP_COMPLEX:    bank = LANGBANK_REF; break;
	case STAGE_MP_SKEDAR:     bank = LANGBANK_OAT; break;
	case STAGE_DEFECTION:     bank = LANGBANK_AME; break;
	case STAGE_VILLA:         bank = LANGBANK_ELD; break;
	case STAGE_DEFENSE:       bank = LANGBANK_IMP; break;
	case STAGE_INVESTIGATION: bank = LANGBANK_EAR; break;
	case STAGE_ATTACKSHIP:    bank = LANGBANK_LEE; break;
	case STAGE_RESCUE:        bank = LANGBANK_LIP; break;
	case STAGE_INFILTRATION:  bank = LANGBANK_LUE; break;
	case STAGE_DEEPSEA:       bank = LANGBANK_PAM; break;
	case STAGE_SKEDARRUINS:   bank = LANGBANK_SHO; break;
	case STAGE_AIRFORCEONE:   bank = LANGBANK_RIT; break;
	case STAGE_MP_RAVINE:     bank = LANGBANK_AREC; break;
	case STAGE_CRASHSITE:     bank = LANGBANK_AZT; break;
	case STAGE_AIRBASE:       bank = LANGBANK_CAVE; break;
	case STAGE_DUEL:          bank = LANGBANK_ATE; break;
	case STAGE_MP_BASE:       bank = LANGBANK_MP1; break;
	case STAGE_MP_AREA52:     bank = LANGBANK_MP3; break;
	case STAGE_MP_WAREHOUSE:  bank = LANGBANK_MP4; break;
	case STAGE_MP_CARPARK:    bank = LANGBANK_MP5; break;
	case STAGE_MP_RUINS:      bank = LANGBANK_MP9; break;
	case STAGE_MP_SEWERS:     bank = LANGBANK_MP10; break;
	case STAGE_MP_FELICITY:   bank = LANGBANK_MP11; break;
	case STAGE_MP_FORTRESS:   bank = LANGBANK_MP12; break;
	case STAGE_MP_VILLA:      bank = LANGBANK_MP13; break;
	case STAGE_MP_GRID:       bank = LANGBANK_MP15; break;
	case STAGE_MBR:           bank = LANGBANK_WAX; break;
	default:
		while (true) {
			// empty
		}
	}

	return bank;
}

TextData* loadFileIntoMemory(const char *filename) {
	FILE *file = fopen(filename, "r");
	if(!file) {
		perror("Error opening file");
		return NULL;
	}

	TextData *filedata = malloc(sizeof(TextData));
	filedata->lines = NULL;
	filedata-> count = 0;

	char buffer[MAX_LINE_LENGTH];
	while (fgets(buffer, sizeof(buffer), file)) {
		filedata->count++;

		filedata->lines = realloc(filedata->lines, filedata->count * sizeof(char *));
		filedata->lines[filedata->count - 1] = strdup(buffer);
	}

	fclose(file);
	return filedata;
}

// Fetch the text data for a particular stage
void langInit(u8 stagenum)
{
	debug_log("lang init on stage: %d \n", stagenum);
}

// Free up the memory used in langInit after the stage ends
void langClear()
{
	debug_log("lang clear\n", 0);
}

s32 langGetFileId(s32 bank)
{
	return g_LangFiles[bank];
}

void langLoad(s32 bank)
{
	s32 file_id = langGetFileId(bank);
	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[bank] = fileLoadToNew(file_id, FILELOADMETHOD_DEFAULT, LOADTYPE_LANG);
}

void langLoadToAddr(s32 bank, u8 *dst, s32 size)
{
	s32 file_id = langGetFileId(bank);
	g_LoadType = LOADTYPE_LANG;
	g_LangBanks[bank] = fileLoadToAddr(file_id, FILELOADMETHOD_DEFAULT, dst, size);
}

void langClearBank(s32 bank)
{
	g_LangBanks[bank] = NULL;
}

/**
 * Resolve a text ID to a string.
 *
 * g_LangBanks is an array of pointers to language file data in RAM. Many of
 * those pointers will be NULL because only the necessary language files are
 * loaded at any given time.
 *
 * The language file data consists of a variable-length array of offsets into
 * the file. Not to be confused with pointers.
 */
char *langGet(s32 textid)
{
	s32 bankindex = textid >> 9;
	s32 textindex = textid & 0x1ff;
	uintptr_t *bank = (uintptr_t*)g_LangBanks[bankindex];
	uintptr_t addr;

	if (bank && bank[textindex]) {
		addr = (uintptr_t)bank + bank[textindex];
	} else {
		addr = 0;
	}

	return (char *)addr;
}

const char* langGetText(TextData *filedata, int line) {
	if (line < 1 || line > filedata->count) {
		return NULL;
	}
	return filedata->lines[line - 1];
}