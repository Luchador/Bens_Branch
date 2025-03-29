#ifndef _IN_GAME_SIGHT_H
#define _IN_GAME_SIGHT_H
#include "data.h"
#include "types.h"

bool sightIsPropFriendly(struct prop *prop);
bool sightCanTargetProp(struct prop *prop, int max);
bool sightIsReactiveToProp(struct prop *prop);
int sightFindFreeTargetIndex(int max);
void func0f0d7364(void);
void sightTick(bool sighton);
int sightCalculateBoxBound(int arg0, int arg1, int arg2, int arg3);
Gfx *sightDrawTargetBox(Gfx *gdl, struct trackedprop *trackedprop, int textid, int time);
Gfx *sightDrawAimer(Gfx *gdl, int x, int y, int radius, int cornergap, uint32_t colour);
Gfx *sightDrawDelayedAimer(Gfx *gdl, int x, int y, int radius, int cornergap, uint32_t colour);
Gfx *sightDrawDefault(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawClassic(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawType2(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawSkedarTriangle(Gfx *gdl, int x, int y, int dir, uint32_t colour);
Gfx *sightDrawSkedar(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawZoom(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawMaian(Gfx *gdl, bool sighton, float crossx, float crossy);
Gfx *sightDrawTarget(Gfx *gdl, float crossx, float crossy);
bool sightHasTargetWhileAiming(int sight);
Gfx *sightDraw(Gfx *gdl, bool sighton, int sight);

#endif
