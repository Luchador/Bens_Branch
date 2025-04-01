#ifndef IN_GAME_BONDEYESPY_H
#define IN_GAME_BONDEYESPY_H
#include "data.h"
#include "types.h"

float eyespyFindGround(RoomNum *floorroom);
int eyespyTryMoveUpwards(float yvel);
int eyespyCalculateNewPosition(struct coord *vel);
bool eyespyCalculateNewPositionWithPush(struct coord *vel);
int eyespy0f0cf890(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4);
int eyespy0f0cf9f8(struct coord *arg0, struct coord *arg1, struct coord *arg2);
int eyespyTrySidePush(struct coord *moveDir, struct coord *targetA, struct coord *targetB);
int eyespy0f0cfdd0(struct coord *vel, struct coord *arg1, struct coord *arg2);
void eyespyUpdateVertical(void);
bool eyespyTryLaunch(void);
void eyespyProcessInput(bool allowbuttons);

#endif
