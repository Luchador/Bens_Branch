#ifndef IN_GAME_BODY_H
#define IN_GAME_BODY_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void bodiesInit(void);

void bodiesReset(int stagenum);

unsigned int bodyGetRace(int bodynum);
bool bodyLoad(int bodynum);
struct model *body0f02ce8c(s32 bodynum, s32 headnum, struct modeldef *bodymodeldef, struct modeldef *headmodeldef, bool sunglasses, struct model *model, bool isplayer, u8 varyheight);
struct model *body0f02d338(s32 bodynum, s32 headnum, struct modeldef *bodymodeldef, struct modeldef *headmodeldef, bool sunglasses, u8 varyheight);
struct model *bodyAllocateModel(s32 bodynum, s32 headnum, u32 spawnflags);
s32 bodyGetRandomBond(void);
s32 bodyChooseHead(s32 bodynum);
void bodyAllocateChr(s32 stagenum, struct packedchr *packed, s32 cmdindex);
struct prop *bodyAllocateEyespy(struct pad *pad, RoomNum room);
void bodyCalculateHeadOffset(struct modeldef *headmodeldef, s32 headnum, s32 bodynum);

#endif
