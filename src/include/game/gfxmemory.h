#ifndef _IN_GAME_GFXMEMORY_H
#define _IN_GAME_GFXMEMORY_H
#include "data.h"
#include "types.h"

extern uint8_t *g_GfxBuffers[3];

void gfxReset(void);
Gfx *gfxGetMasterDisplayList(void);
Vtx *gfxAllocateVertices(uint32_t count);
void *gfxAllocateMatrix(void);
LookAt *gfxAllocateLookAt(int count);
Col *gfxAllocateColours(int count);
void *gfxAllocate(uint32_t size);
void gfxSwapBuffers(void);

#endif
