#ifndef _IN_GAME_GFXMEMORY_H
#define _IN_GAME_GFXMEMORY_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern u8 *g_GfxBuffers[3];

void gfxReset(void);
Gfx *gfxGetMasterDisplayList(void);
Vtx *gfxAllocateVertices(u32 count);
void *gfxAllocateMatrix(void);
LookAt *gfxAllocateLookAt(int count);
Col *gfxAllocateColours(int count);
void *gfxAllocate(u32 size);
void gfxSwapBuffers(void);
int gfxGetFreeGfx(Gfx *gdl);

#endif
