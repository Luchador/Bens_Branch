#pragma once

#include "data.h"
#include "types.h"

void bwalkInit(void);
void bwalkSetSwayTarget(int value);
void bwalkSetSwayTargetf(float value);
void bwalkAdjustCrouchPos(int value);
void bwalk0f0c3b38(struct coord *param_1, struct defaultobj *obj);
int bwalkTryMoveUpwards(float amount);
bool bwalkCanMoveUpwards(float amount);
bool bwalkCalculateNewPosition(struct coord *vel, float rotateamount, bool apply, float extrawidth, int arg4);
bool bwalkCalculateNewPositionWithPush(struct coord *delta, float rotateamount, bool apply, float extrawidth, int types);
int bwalk0f0c4764(struct coord *delta, struct coord *arg1, struct coord *arg2, int types);
int bwalk0f0c47d0(struct coord *a, struct coord *b, struct coord *c, struct coord *d, struct coord *e, int types);
int bwalk0f0c494c(struct coord *a, struct coord *b, struct coord *c, int types);
int bwalk0f0c4a5c(struct coord *a, struct coord *b, struct coord *c, int types);
void bwalkUpdateSpeedSideways(float targetspeed, float accelspeed, int mult);
void bwalkUpdateSpeedForwards(float targetspeed, float accelspeed);
void bwalkUpdateVertical(void);
void bwalkApplyCrouchSpeed(void);
bool bwalkCanUncrouch(void);
void bwalkUpdateCrouchOffsetReal(void);
void bwalkUpdateCrouchOffset(void);
void bwalkUpdateTheta(void);
void bwalk0f0c63bc(struct coord *arg0, uint32_t arg1, int types);
void bwalkUpdatePrevPos(void);
void bwalkHandleActivate(void);
void bwalkApplyMoveData(struct movedata *data);
void bwalk0f0c69b8(void);
void bwalkTick(void);