#include <ultra64.h>
#include <stdint.h>
#include <string.h>
#include "constants.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

void texInit(void)
{
	extern uint8_t *_textureslistSegmentRomStart;
	extern uint8_t *_textureslistSegmentRomEnd;

	uint32_t len = ((_textureslistSegmentRomEnd - _textureslistSegmentRomStart) + 15) & -16;

	g_Textures = mempAlloc(len, MEMPOOL_PERMANENT);

	memcpy(g_Textures, (const void *) ((romptr_t) _textureslistSegmentRomStart), len);
}
