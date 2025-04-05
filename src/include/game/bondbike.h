#pragma once

#include "data.h"
#include "types.h"

void bbikeInit(void);
void bbikeExit(void);
void bbikeUpdateVehicleOffset(void);
void bbikeTryDismountAngle(float relativeangle, float distance);
void bbikeHandleActivate(void);
void bbikeApplyMoveData(struct movedata *data);
void bbike0f0d2b40(struct defaultobj *bike, struct coord *arg1, float arg2, struct defaultobj *obstacle);
int bbikeCalculateNewPosition(struct coord *arg0, float arg1);
int bbikeCalculateNewPositionWithPush(struct coord *arg0, float arg1);
void bbikeUpdateVertical(struct coord *pos);
int bbike0f0d3680(struct coord *arg0, struct coord *arg1, struct coord *arg2);
int bbike0f0d36d4(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4);
int bbike0f0d3840(struct coord *arg0, struct coord *arg1, struct coord *arg2);
int bbike0f0d3940(struct coord *arg0, struct coord *arg1, struct coord *arg2);
void bbike0f0d3c60(struct coord *arg0);
void bbikeTick(void);