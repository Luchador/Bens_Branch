#ifndef _IN_GAME_ARTIFACTS_H
#define _IN_GAME_ARTIFACTS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void artifactsClear(void);
void artifactsTick(void);
uint16_t func0f13c574(float arg0);
int func0f13c710(float arg0);
void artifactsCalculateGlaresForRoom(int roomnum);
uint8_t func0f13d3c4(uint8_t arg0, uint8_t arg1);
Gfx *artifactsConfigureForGlares(Gfx *gdl);
Gfx *artifactsUnconfigureForGlares(Gfx *gdl);
Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, int roomnum);
#ifndef PLATFORM_N64
bool artifactTestLos(struct coord *spec, struct coord *roompos, int xi, int yi);
#endif

#endif
