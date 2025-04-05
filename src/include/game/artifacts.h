#pragma once

#include "data.h"
#include "types.h"

void artifactsClear(void);
void artifactsTick(void);
void artifactsCalculateGlaresForRoom(int roomnum);
uint8_t func0f13d3c4(uint8_t arg0, uint8_t arg1);
Gfx *artifactsConfigureForGlares(Gfx *gdl);
Gfx *artifactsUnconfigureForGlares(Gfx *gdl);
Gfx *artifactsRenderGlaresForRoom(Gfx *gdl, int roomnum);
bool artifactTestLos(struct coord *spec, struct coord *roompos, int xi, int yi);

