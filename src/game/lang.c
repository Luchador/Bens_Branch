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
#include "fs.h"
#ifndef PLATFORM_N64
#include "video.h"
#endif

u16 *g_FrameBuffers[NUM_FRAMEBUFFERS];

TextData *g_TextGunData;
TextData *g_TextTitleData;
TextData *g_TextMPMenuData;
TextData *g_TextPropObjData;
TextData *g_TextMPWeaponsData;
TextData *g_TextOptionsData;
TextData *g_TextMiscData;
TextData *g_TextameData;
TextData *g_TextearData;
TextData *g_TextarkData;
TextData *g_TexteldData;
TextData *g_TextpeteData;
TextData *g_TextdepoData;
TextData *g_TextlueData;
TextData *g_TextlipData;
TextData *g_TexttraData;
TextData *g_TextcaveData;
TextData *g_TextritData;
TextData *g_TextaztData;
TextData *g_TextdamData;
TextData *g_TextpamData;
TextData *g_TextimpData;
TextData *g_TextleeData;
TextData *g_TextshoData;
TextData *g_TextwaxData;
TextData *g_TextsevData;
TextData *g_TextstatData;
TextData *g_TextateData;
TextData *g_TextdishData;

// Each language bank by ID, a beginning offset, and an end offset to match with the language enums in lang.h
struct langbank g_LangBanks[29] = {
	//       id,                     begin,     end
	/*0x00*/ LANGBANK_GUN,           19456,     19700,
	/*0x01*/ LANGBANK_TITLE,         19968,     20120,
	/*0x02*/ LANGBANK_MPMENU,        20480,     20976,
	/*0x03*/ LANGBANK_PROPOBJ,       20992,     21044,
	/*0x04*/ LANGBANK_MPWEAPONS,     21504,     21768,
	/*0x05*/ LANGBANK_OPTIONS,       22016,     22512,
	/*0x06*/ LANGBANK_MISC,          22528,     23028,
	/*0x07*/ LANGBANK_AME,           512,       624,
	/*0x08*/ LANGBANK_EAR,           7680,      7788,
	/*0x09*/ LANGBANK_ARK,           1536,      1612,
	/*0x10*/ LANGBANK_ELD,           8192,      8244,
	/*0x11*/ LANGBANK_PETE,          12800,     12876,
	/*0x12*/ LANGBANK_DEPO,          6144,      6228,
	/*0x13*/ LANGBANK_LUE,           11264,     11340,
	/*0x14*/ LANGBANK_LIP,           10752,     10860,
	/*0x15*/ LANGBANK_TRA,           18432,     18520,
	/*0x16*/ LANGBANK_CAVE,          3584,      3652,
	/*0x17*/ LANGBANK_RIT,           13824,     13912,
	/*0x18*/ LANGBANK_AZT,           2560,      2608,
	/*0x19*/ LANGBANK_DAM,           5632,      5680,
	/*0x20*/ LANGBANK_PAM,           12288,     12336,
	/*0x21*/ LANGBANK_IMP,           8704,      8768,
	/*0x22*/ LANGBANK_LEE,           9728,      9784,
	/*0x23*/ LANGBANK_SHO,           16896,     16952,
	/*0x24*/ LANGBANK_WAX,           18944,     18972,
	/*0x25*/ LANGBANK_SEV,           15360,     15376,
	/*0x26*/ LANGBANK_STAT,          17920,     17932,
	/*0x27*/ LANGBANK_ATE,           24064,     24080,
	/*0x28*/ LANGBANK_DISH,          7168,      7296,
};

// Replaces asterisks with newline characters. Asterisks are used to represent new lines in the PD text files.
void langReplaceAsterisk(TextData *data) {
    
	int i = 0;
	for(i = 0; i < data->count; i++)
	{
		char *str = data->lines[i];
		while (*str) {
			if (*str == '*') {
				*str = '\n';
			}
			str++;
		}
	}
}

char* langRemoveNewline(char *str) {
    char *pos = strchr(str, '\n');  // Find the first newline
    if (pos) {
        *pos = '\0';  // Replace it with null terminator
    }

	return str;
}


const char* langGetText(TextData *filedata, int line) {
	if (line < 1 || line > filedata->count) {
		return NULL;
	}

    if(filedata->lines[line - 1]) {
		return filedata->lines[line - 1];
	}
	else {
		return "no string found \n";
	}
}


// Ben's comment: Fetch the text data for everything. Unlike the original we won't be swapping out text banks on stage loads. This makes things far simpler.
void langInit()
{
	char *fullpath = "./" DEFAULT_BASEDIR_NAME "/text"; // ./data/text

	g_TextGunData = loadFileIntoMemory(buildDynamicPath(fullpath, "LgunE.txt"));
	g_TextMiscData = loadFileIntoMemory(buildDynamicPath(fullpath, "LmiscE.txt"));
	g_TextMPMenuData = loadFileIntoMemory(buildDynamicPath(fullpath, "LmpmenuE.txt"));
	g_TextMPWeaponsData = loadFileIntoMemory(buildDynamicPath(fullpath, "LmpweaponsE.txt"));
	g_TextOptionsData = loadFileIntoMemory(buildDynamicPath(fullpath, "LoptionsE.txt"));
	g_TextPropObjData = loadFileIntoMemory(buildDynamicPath(fullpath, "LpropobjE.txt"));
	g_TextTitleData = loadFileIntoMemory(buildDynamicPath(fullpath, "LtitleE.txt"));
	g_TextameData = loadFileIntoMemory(buildDynamicPath(fullpath, "LameE.txt"));
	g_TextearData = loadFileIntoMemory(buildDynamicPath(fullpath, "LearE.txt"));
	g_TextarkData = loadFileIntoMemory(buildDynamicPath(fullpath, "LarkE.txt"));
	g_TexteldData = loadFileIntoMemory(buildDynamicPath(fullpath, "LeldE.txt"));
	g_TextpeteData = loadFileIntoMemory(buildDynamicPath(fullpath, "LpeteE.txt"));
	g_TextdepoData = loadFileIntoMemory(buildDynamicPath(fullpath, "LdepoE.txt"));
	g_TextlueData = loadFileIntoMemory(buildDynamicPath(fullpath, "LlueE.txt"));
	g_TextlipData = loadFileIntoMemory(buildDynamicPath(fullpath, "LlipE.txt"));
	g_TexttraData = loadFileIntoMemory(buildDynamicPath(fullpath, "LtraE.txt"));
	g_TextcaveData = loadFileIntoMemory(buildDynamicPath(fullpath, "LcaveE.txt"));
	g_TextritData = loadFileIntoMemory(buildDynamicPath(fullpath, "LritE.txt"));
	g_TextaztData = loadFileIntoMemory(buildDynamicPath(fullpath, "LaztE.txt"));
	g_TextdamData = loadFileIntoMemory(buildDynamicPath(fullpath, "LdamE.txt"));
	g_TextpamData = loadFileIntoMemory(buildDynamicPath(fullpath, "LpamE.txt"));
	g_TextimpData = loadFileIntoMemory(buildDynamicPath(fullpath, "LimpE.txt"));
	g_TextleeData = loadFileIntoMemory(buildDynamicPath(fullpath, "LleeE.txt"));
	g_TextshoData = loadFileIntoMemory(buildDynamicPath(fullpath, "LshoE.txt"));
	g_TextwaxData = loadFileIntoMemory(buildDynamicPath(fullpath, "LwaxE.txt"));
	g_TextsevData = loadFileIntoMemory(buildDynamicPath(fullpath, "LsevE.txt"));
	g_TextstatData = loadFileIntoMemory(buildDynamicPath(fullpath, "LstatE.txt"));
	g_TextateData = loadFileIntoMemory(buildDynamicPath(fullpath, "LateE.txt"));
	g_TextdishData = loadFileIntoMemory(buildDynamicPath(fullpath, "LdishE.txt"));

	// The original language files use an asterisk to represent a new line. Those asterisks need to be swapped for a newline character.
	langReplaceAsterisk(g_TextGunData);
	langReplaceAsterisk(g_TextMiscData);
	langReplaceAsterisk(g_TextMPMenuData);
	langReplaceAsterisk(g_TextMPWeaponsData);
	langReplaceAsterisk(g_TextOptionsData);
	langReplaceAsterisk(g_TextPropObjData);
	langReplaceAsterisk(g_TextTitleData);
	langReplaceAsterisk(g_TextameData);
	langReplaceAsterisk(g_TextearData);
	langReplaceAsterisk(g_TextarkData);
	langReplaceAsterisk(g_TexteldData);
	langReplaceAsterisk(g_TextpeteData);
	langReplaceAsterisk(g_TextdepoData);
	langReplaceAsterisk(g_TextlueData);
	langReplaceAsterisk(g_TextlipData);
	langReplaceAsterisk(g_TexttraData);
	langReplaceAsterisk(g_TextcaveData);
	langReplaceAsterisk(g_TextritData);
	langReplaceAsterisk(g_TextaztData);
	langReplaceAsterisk(g_TextdamData);
	langReplaceAsterisk(g_TextpamData);
	langReplaceAsterisk(g_TextimpData);
	langReplaceAsterisk(g_TextleeData);
	langReplaceAsterisk(g_TextshoData);
	langReplaceAsterisk(g_TextwaxData);
	langReplaceAsterisk(g_TextsevData);
	langReplaceAsterisk(g_TextstatData);
	langReplaceAsterisk(g_TextateData);
	langReplaceAsterisk(g_TextdishData);

}

char *langGet(s32 textid)
{
	if(textid == L_MPWEAPONS_129)
	{
		return "Difficulty\n"; // Stop the "Difficulty" text from creating a new line on the end screen
	}

	int i = 0;
	for(i = 0; i < ARRAYCOUNT(g_LangBanks); i++)
	{
		if(textid >= g_LangBanks[i].begin && textid < g_LangBanks[i].end)
		{
			textid -= g_LangBanks[i].begin - 1;
			switch(i){
				case 0:
					return langGetText(g_TextGunData, textid);
				case 1:
					return langGetText(g_TextTitleData, textid);
				case 2:
					return langGetText(g_TextMPMenuData, textid);
				case 3:
					return langGetText(g_TextPropObjData, textid);
				case 4:
					return langGetText(g_TextMPWeaponsData, textid);
				case 5:
					return langGetText(g_TextOptionsData, textid);
				case 6:
					return langGetText(g_TextMiscData, textid);
				case 7:
					return langGetText(g_TextameData, textid);
				case 8:
					return langGetText(g_TextearData, textid);
				case 9:
					return langGetText(g_TextarkData, textid);
				case 10:
					return langGetText(g_TexteldData, textid);
				case 11:
					return langGetText(g_TextpeteData, textid);
				case 12:
					return langGetText(g_TextdepoData, textid);
				case 13:
					return langGetText(g_TextlueData, textid);
				case 14:
					return langGetText(g_TextlipData, textid);
				case 15:
					return langGetText(g_TexttraData, textid);
				case 16:
					return langGetText(g_TextcaveData, textid);
				case 17:
					return langGetText(g_TextritData, textid);
				case 18:
					return langGetText(g_TextaztData, textid);
				case 19:
					return langGetText(g_TextdamData, textid);
				case 20:
					return langGetText(g_TextpamData, textid);
				case 21:
					return langGetText(g_TextimpData, textid);
				case 22:
					return langGetText(g_TextleeData, textid);
				case 23:
					return langGetText(g_TextshoData, textid);
				case 24:
					return langGetText(g_TextwaxData, textid);
				case 25:
					return langGetText(g_TextsevData, textid);
				case 26:
					return langGetText(g_TextstatData, textid);
				case 27:
					return langGetText(g_TextateData, textid);
				case 28:
					return langGetText(g_TextdishData, textid);
				default:
					return("error \n");
			}
		}
	}
	
	return "string not found \n";
}