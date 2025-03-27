#ifndef IN_GAME_BONDHEAD_H
#define IN_GAME_BONDHEAD_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void bheadReset(void);

void bheadFlipAnimation(void);
void bheadUpdateIdleRoll(void);
void bheadUpdatePos(struct coord *vel);
void bheadUpdateRot(struct coord *lookvel, struct coord *upvel);
void bheadSetDamp(float headdamp);
void bheadUpdate(float arg0, float arg1);
void bheadAdjustAnimation(float speed);
void bheadStartDeathAnimation(int16_t animnum, uint32_t flip, float fstarttime, float speed);
void bheadSetSpeed(float speed);
float bheadGetBreathingValue(void);

#endif
