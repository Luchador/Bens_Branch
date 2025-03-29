#ifndef IN_GAME_BONDMOVE_H
#define IN_GAME_BONDMOVE_H
#include "data.h"
#include "types.h"

void bmoveSetControlDef(uint32_t controldef);
void bmoveSetAutoMoveCentreEnabled(bool enabled);
void bmoveSetAutoAimY(bool enabled);
bool bmoveIsAutoAimYEnabled(void);
bool bmoveIsAutoAimYEnabledForCurrentWeapon(void);
bool bmoveIsInSightAimMode(void);
void bmoveUpdateAutoAimYProp(struct prop *prop, float autoaimy);
void bmoveSetAutoAimX(bool enabled);
bool bmoveIsAutoAimXEnabled(void);
bool bmoveIsAutoAimXEnabledForCurrentWeapon(void);
void bmoveUpdateAutoAimXProp(struct prop *prop, float autoaimx);
struct prop *bmoveGetHoverbike(void);
struct prop *bmoveGetGrabbedProp(void);
void bmoveGrabProp(struct prop *prop);
void bmoveSetMode(uint32_t movemode);
void bmoveSetModeForAllPlayers(uint32_t movemode);
void bmoveHandleActivate(void);
void bmoveApplyMoveData(struct movedata *data);
void bmoveUpdateSpeedTheta(void);
float bmoveGetSpeedVertaLimit(float value);
void bmoveUpdateSpeedVerta(float value);
float bmoveGetSpeedThetaControlLimit(float value);
void bmoveUpdateSpeedThetaControl(float value);
float bmoveCalculateLookahead(void);
void bmoveResetMoveData(struct movedata *data);
void bmoveProcessInput(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2);
void bmoveFindEnteredRoomsByPos(struct player *player, struct coord *arg1, RoomNum *rooms);
void bmoveFindEnteredRooms(struct player *player, RoomNum *rooms);
void bmoveUpdateRooms(struct player *player);
void bmove0f0cb904(struct coord *coord);
void bmove0f0cba88(float *a, float *b, struct coord *c, float mult1, float mult2);
void bmoveUpdateMoveInitSpeed(struct coord *newpos);
void bmoveTick(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2);
void bmoveUpdateVerta(void);
void bmove0f0cc19c(struct coord *arg);
void bmoveUpdateHead(float arg0, float arg1, float arg2, Mtxf *arg3, float arg4);
void bmove0f0cc654(float arg0, float arg1, float arg2);
int bmoveGetCrouchPos(void);
int bmoveGetCrouchPosByPlayer(int playernum);

#endif
