#ifndef IN_GAME_BONDGRAB_H
#define IN_GAME_BONDGRAB_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void bgrabInit(void);
void bgrabExit(void);
void bgrab0f0ccbf0(struct coord *delta, float angle, struct defaultobj *obj);
bool bgrabTryMoveUpwards(float y);
int bgrabCalculateNewPosition(struct coord *delta, float angle, bool arg2);
bool bgrabCalculateNewPositiontWithPush(struct coord *delta, float angle, bool arg2);
bool bgrab0f0cdb04(float angle, bool arg2);
bool bgrabTryPushObject(float angle);
void bgrab0f0cdef0(void);
bool bgrab0f0cdf64(struct coord *delta, struct coord *arg1, struct coord *arg2);
int bgrab0f0cdfbc(struct coord *delta, struct coord *arg1, struct coord *arg2);
void bgrab0f0ce0bc(struct coord *arg0);
void bgrabUpdatePrevPos(void);
void bgrab0f0ce178(void);
void bgrabUpdateVertical(void);
void bgrabHandleActivate(void);
void bgrabUpdateSpeedSideways(float targetspeed, float accelspeed, int mult);
void bgrabUpdateSpeedForwards(float target, float speed);
void bgrabApplyMoveData(struct movedata *data);
void bgrabUpdateSpeedTheta(void);
void bgrabUpdatePlayerMovement(void);
void bgrabTick(void);

#endif
