#ifndef IN_GAME_NBOMB_H
#define IN_GAME_NBOMB_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

Gfx *nbombCreateSphere(Gfx *gdl, int depth);
void nbombReset(struct nbomb *nbomb);
int nbombCalculateAlpha(struct nbomb *nbomb);
Gfx *nbombCreateGdl(void);
Gfx *nbombRender(Gfx *gdl, struct nbomb *nbomb, Gfx *subgdl);
void nbombClearAllNBombs(void);
void nbombInflictDamage(struct nbomb *nbomb);
void nbombTick(struct nbomb *nbomb);
void nbombsTick(void);
Gfx *nbombsRender(Gfx *gdl);
void nbombCreateStorm(struct coord *pos, struct prop *ownerprop);
float gasGetDoorFrac(int tagnum);
Gfx *nbombRenderOverlay(Gfx *gdl);
Gfx *gasRender(Gfx *gdl);

#endif
