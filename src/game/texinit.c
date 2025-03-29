#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

int g_ReplacementTextureList[4000];

void texInit(void)
{
	extern uint8_t EXT_SEG _textureslistSegmentRomStart;
	extern uint8_t EXT_SEG _textureslistSegmentRomEnd;

	uint32_t len = ((REF_SEG _textureslistSegmentRomEnd - REF_SEG _textureslistSegmentRomStart) + 15) & -16;

	g_Textures = mempAlloc(len, MEMPOOL_PERMANENT);

	dmaExec(g_Textures, (romptr_t) REF_SEG _textureslistSegmentRomStart, len);

	int i = 0;
	for(i = 0; i < ARRAYCOUNT(g_ReplacementTextureList); i++) {
		g_ReplacementTextureList[i] = -1;
	}
}
