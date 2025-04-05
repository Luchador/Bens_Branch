#pragma once

#include "data.h"
#include "types.h"

uint32_t setupGetCmdLength(uint32_t *cmd);
uint32_t *setupGetCmdByIndex(int cmdindex);
int setupGetCmdIndexByTag(struct tag *tag);
uint32_t setupGetCmdIndexByProp(struct prop *prop);
bool setupLoadModeldef(int modelnum);
bool setupGetObjBbox(struct defaultobj *obj, struct coord *pos, float realrot[3][3], struct coord *arg3, struct coord *arg4);
bool setupGetObjBboxFromMinMax(struct defaultobj *obj, struct coord *min, struct coord *max);
void setupGetObjOverlappedRooms(struct defaultobj *obj, struct coord *pos, float realrot[3][3], RoomNum *rooms);
void setup0f0923d4(struct defaultobj *obj);
struct defaultobj *setupGetObjByCmdIndex(uint32_t cmdindex);
struct defaultobj *setupFindObjForReuse(int type, struct defaultobj **arg1, struct defaultobj **arg2, bool musthaveprop, bool musthavemodel, struct modeldef *modeldef);
