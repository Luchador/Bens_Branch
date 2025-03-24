#ifndef _IN_GAME_ARTIFACTS_H
#define _IN_GAME_ARTIFACTS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void artifactsClear(void);
void artifactsTick(void);
u16 func0f13c574(f32 arg0);
s32 floatToSafeS32(f32 arg0);
void artifactsCalculateGlaresForRoom(s32 roomnum);
u8 artifactClampU8Within7(u8 arg0, u8 arg1);
Gfx *artifactsConfigureForGlares(Gfx *gdl);
Gfx *artifactsUnconfigureForGlares(Gfx *gdl);
Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, s32 roomnum);
#ifndef PLATFORM_N64
bool artifactTestLos(struct coord *spec, struct coord *roompos, s32 xi, s32 yi);
#endif

#endif
