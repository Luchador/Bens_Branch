#ifndef _IN_GAME_WALLHIT_H
#define _IN_GAME_WALLHIT_H
#include "data.h"
#include "types.h"

void wallhitReset(void);

int16_t wallhitFinaliseAxis(float arg0);
void wallhitFree(struct wallhit *wallhit);
void wallhitsFreeByProp(struct prop *prop, int8_t layer);
bool chrIsUsingPaintball(struct chrdata *chr);
void wallhitChooseBloodColour(struct prop *prop);
void wallhitFade(struct wallhit *wallhit, uint32_t arg1);
bool wallhitRemoveOneInRoom(int arg0);
void wallhitRemoveOne(void);
void wallhitsTick(void);

void wallhitCreate(struct coord *relpos, struct coord *arg1, struct coord *arg2, int16_t arg3[3],
		int16_t arg4[3], int16_t texnum, RoomNum room, struct prop *objprop,
		int8_t mtxindex, int8_t arg9, struct chrdata *chr, bool xlu);

void wallhitCreateWith20Args(struct coord *relpos, struct coord *arg1, struct coord *arg2, int16_t arg3[3],
		int16_t arg4[3], int16_t texnum, RoomNum room, struct prop *objprop,
		struct prop *chrprop, int8_t mtxindex, int8_t arg10, struct chrdata *chr,
		float width, float height, uint8_t minalpha, uint8_t maxalpha,
		int rotdeg, uint32_t timermax, uint32_t timerspeed, bool xlu);

Gfx *wallhitRenderPropHits(Gfx *gdl, struct prop *prop, bool xlupass);
Gfx *wallhitRenderBgHits(int roomnum, Gfx *gdl);
void wallhitsRecolour(void);
void wallhitFadeSplatsForRemovedChr(struct prop *chrprop);
void wallhitRemoveOldestWoundedSplatByChr(struct prop *chrprop);

#endif
