#ifndef _IN_GAME_SMOKE_H
#define _IN_GAME_SMOKE_H
#include "data.h"
#include "types.h"

void smokesInit(void);

void smokeReset(void);

void smokeStop(void);

Gfx *smokeRenderPart(struct smoke *smoke, struct smokepart *part, Gfx *gdl, struct coord *coord, float size);
struct smoke *smokeCreate(struct coord *pos, RoomNum *rooms, int16_t type);
bool smokeCreateForHand(struct coord *pos, RoomNum *rooms, int16_t type, int handnum);
bool smokeCreateWithSource(void *source, struct coord *pos, RoomNum *rooms, int16_t type, bool srcispadeffect);
void smokeCreateAtProp(struct prop *prop, int16_t type);
void smokeCreateAtPadEffect(struct padeffectobj *effect, struct coord *pos, RoomNum *rooms, int16_t type);
void smokeClearForProp(struct prop *prop);
struct smoke *smokeCreateSimple(struct coord *pos, RoomNum *rooms, int16_t type);
u32 smokeTick(struct prop *prop);
u32 smokeTickPlayer(struct prop *prop);
Gfx *smokeRender(struct prop *prop, Gfx *gdl, bool xlupass);
void smokeClearSomeTypes(void);

#endif
