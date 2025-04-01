#ifndef IN_GAME_EXPLOSIONS_H
#define IN_GAME_EXPLOSIONS_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void explosionsReset(void);

void explosionsStop(void);

bool explosionCreateSimple(struct prop *prop, struct coord *pos, RoomNum *rooms, int16_t type, int playernum);
bool explosionCreateComplex(struct prop *prop, struct coord *pos, RoomNum *rooms, int16_t type, int playernum);
float explosionGetHorizontalRangeAtFrame(struct explosion *exp, int frame);
float explosionGetVerticalRangeAtFrame(struct explosion *exp, int frame);
void explosionGetBboxAtFrame(struct coord *lower, struct coord *upper, int frame, struct prop *prop);
void explosionAlertChrs(float *radius, struct coord *noisepos);
bool explosionCreate(struct prop *prop, struct coord *pos, RoomNum *rooms, int16_t type, int playernum, bool makescorch, struct coord *arg6, RoomNum room, struct coord *arg8);
void explosionsUpdateShake(struct coord *cameraPos, struct coord *cameraForward);
bool explosionOverlapsProp(struct explosion *exp, struct prop *prop, struct coord *pos1, struct coord *pos2);
void explosionInflictDamage(struct prop *prop);
uint32_t explosionTick(struct prop *prop);
uint32_t explosionTickPlayer(struct prop *prop);
Gfx *explosionRender(struct prop *prop, Gfx *gdl, bool xlupass);
Gfx *explosionRenderPart(struct explosion *exp, struct explosionpart *part, Gfx *gdl, struct coord *coord, int arg4);

#endif
