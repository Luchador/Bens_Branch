#include <ultra64.h>
#include "constants.h"
#include "game/gfxmemory.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/rzip.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "string.h"
#include <stdlib.h>
#include "data.h"
#include "types.h"
#include "platform.h"

/**
 * This file handles memory usage for graphics related tasks.
 *
 * There are two pools, "gfx" and "vtx", which are used to store different data.
 *
 * The gfx pool (g_GfxBuffers) is sized based on the stage's -mgfx and -mgfxtra
 * arguments. It contains only the master display list's GBI bytecode.
 * The master gdl is passed through all rendering functions in the game engine,
 * where each appends to the display list.
 *
 * The vtx pool (g_VtxBuffers) is sized based on the stage's -mvtx argument.
 * It is used for auxiliary graphics data such as vertex arrays, matrices and
 * colours.
 *
 * Both the gfx and vtx pools are split into two buffers of equal size.
 * Only one buffer is active at a time - the other is being drawn to the screen
 * while the active one is being built. Each time a frame is finished the active
 * buffer index is swapped to the other one.
 *
 * Both the gfx and vtx pools have a third element in them, but this is just a
 * marker for the end of the second element's allocation.
 */

/**
 * On 64-bit platforms the Gfx struct is twice as large.
*/
#ifdef PLATFORM_64BIT
#define GFX_SIZE_MULTIPLIER 2
#else
#define GFX_SIZE_MULTIPLIER 1
#endif

uint8_t *g_GfxBuffers[NUM_GFXTASKS + 1];
uint32_t var800aa58c;
uint8_t *g_VtxBuffers[NUM_GFXTASKS + 1];
uint8_t *g_GfxMemPos;
uint8_t g_GfxActiveBufferIndex;
uint32_t g_GfxRequestedDisplayList;

// Ben's comment: originally there was an array that changed this based on player count. I'm just using a constant value, the one used for 4 players.
uint32_t g_GfxSize = 0x00028000 * GFX_SIZE_MULTIPLIER;

uint32_t g_VtxSize = 0x00028000;

int g_GfxNumSwapsPerBuffer[NUM_GFXTASKS] = {0, 1};
uint32_t g_GfxNumSwaps = 2;

/**
 * Allocate graphics memory from the heap. Presumably called on stage load.
 *
 * Comments in this function are strings that appear in an XBLA debug build.
 * They were likely in the N64 version but ifdeffed out.
 */
void gfxReset(void)
{
	if (argFindByPrefix(1, "-mgfx")) {
		// Argument specified master_dl_size\n
		int gfx;
		int gfxtra = 0;

		gfx = strtol(argFindByPrefix(1, "-mgfx"), NULL, 0) * 1024;

		if (argFindByPrefix(1, "-mgfxtra")) {
			// ******** Extra specified but are we in the correct game mode I wonder???\n
			if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) && PLAYERCOUNT() == 2) {
				// ******** Extra Display List Memeory Required\n
				// ******** Shall steal from video buffer\n
				// ******** If you try and run hi-res then\n
				// ******** you're gonna shafted up the arse\n
				// ******** so don't blame me\n
				gfxtra = strtol(argFindByPrefix(1, "-mgfxtra"), NULL, 0) * 1024;
			} else {
				// ******** No we're not so there\n
			}
		}

		// ******** Original Amount required = %dK ber buffer\n
		// ******** Extra Amount required = %dK ber buffer\n
		// ******** Total of %dK (Double Buffered)\n
		g_GfxSize = (gfx + gfxtra) * GFX_SIZE_MULTIPLIER;
	}

	if (argFindByPrefix(1, "-mvtx")) {
		// Argument specified mtxvtx_size\n
		g_VtxSize = strtol(argFindByPrefix(1, "-mvtx"), NULL, 0) * 1024;
	}

	// %d Players : Allocating %d bytes for master dl's\n
	g_GfxBuffers[0] = mempAlloc(g_GfxSize * NUM_GFXTASKS, MEMPOOL_STAGE);
	g_GfxBuffers[1] = g_GfxBuffers[0] + g_GfxSize;
	g_GfxBuffers[2] = g_GfxBuffers[1] + g_GfxSize;

	// Allocating %d bytes for mtxvtx space\n
	g_VtxBuffers[0] = mempAlloc(g_VtxSize * NUM_GFXTASKS, MEMPOOL_STAGE);
	g_VtxBuffers[1] = g_VtxBuffers[0] + g_VtxSize;
	g_VtxBuffers[2] = g_VtxBuffers[1] + g_VtxSize;

	g_GfxActiveBufferIndex = 0;
	g_GfxRequestedDisplayList = false;
	g_GfxMemPos = g_VtxBuffers[0];
}

Gfx *gfxGetMasterDisplayList(void)
{
	g_GfxRequestedDisplayList = true;

	return (Gfx *)g_GfxBuffers[g_GfxActiveBufferIndex];
}

Vtx *gfxAllocateVertices(uint32_t count)
{
	void *ptr = g_GfxMemPos;
	g_GfxMemPos += count * sizeof(Vtx);
	g_GfxMemPos = (uint8_t *)ALIGN16((uintptr_t)g_GfxMemPos);

	return ptr;
}

void *gfxAllocateMatrixF(void)
{
	void *ptr = g_GfxMemPos;
	g_GfxMemPos += sizeof(Mtx);

	return ptr;
}

Mtx *gfxAllocateMatrix(void) 
{
    Mtx *ptr = (Mtx *)g_GfxMemPos;
    g_GfxMemPos += sizeof(Mtx);
    return ptr;
}

/**
 * sizeof(LookAt) is 0x10 and it consists of two Light structs of 0x8 each.
 * The function allocates 0x8 for every count, so it could be allocating lights
 * instead, however it's only used for LookAts so it's named as LookAt.
 */
LookAt *gfxAllocateLookAt(int count)
{
	void *ptr = g_GfxMemPos;
#ifdef PLATFORM_64BIT
	g_GfxMemPos += count * (sizeof(LookAt) * 2);
#else
	g_GfxMemPos += count * (sizeof(LookAt) / 2);
#endif

	return ptr;
}

Col *gfxAllocateColours(int count)
{
	void *ptr = g_GfxMemPos;
	count = ALIGN16(count * sizeof(Col));
	g_GfxMemPos += count;

	return ptr;
}

void *gfxAllocate(uint32_t size)
{
	void *ptr = g_GfxMemPos;
	size = ALIGN16(size);
	g_GfxMemPos += size;

	return ptr;
}

void gfxSwapBuffers(void)
{
	g_GfxActiveBufferIndex ^= 1;
	g_GfxRequestedDisplayList = false;
	g_GfxMemPos = g_VtxBuffers[g_GfxActiveBufferIndex];
	g_GfxNumSwapsPerBuffer[g_GfxActiveBufferIndex] = g_GfxNumSwaps;
	g_GfxNumSwaps++;

	if (g_GfxNumSwaps == -1) {
		g_GfxNumSwaps = 2;
	}
}

int gfxGetFreeGfx(Gfx *gdl)
{
	return (Gfx *)g_GfxBuffers[g_GfxActiveBufferIndex + 1] - gdl;
}

uint32_t gfxGetFreeVtx(void)
{
	return g_VtxBuffers[g_GfxActiveBufferIndex + 1] - g_GfxMemPos;
}
