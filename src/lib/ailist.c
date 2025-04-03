#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/ailist.h"
#include "data.h"
#include "types.h"

int g_NumGlobalAilists = 0;
int g_NumLvAilists = 0;

u8 *ailistFindById(int ailistid)
{
	int lower;
	int upper;
	int index;

	if (ailistid >= 0x401) {
		if (g_StageSetup.ailists) {
			lower = 0;
			upper = g_NumLvAilists;
			index;

			while (upper >= lower) {
				index = (lower + upper) / 2;

				if (g_StageSetup.ailists[index].id == ailistid) {
					return g_StageSetup.ailists[index].list;
				}

				if (ailistid < g_StageSetup.ailists[index].id) {
					upper = index - 1;
				} else {
					lower = index + 1;
				}
			}
		}
	} else {
		lower = 0;
		upper = g_NumGlobalAilists;
		index;

		while (upper >= lower) {
			index = (lower + upper) / 2;

			if (g_GlobalAilists[index].id == ailistid) {
				return g_GlobalAilists[index].list;
			}

			if (ailistid < g_GlobalAilists[index].id) {
				upper = index - 1;
			} else {
				lower = index + 1;
			}
		}
	}

	return NULL;
}
