#ifndef IN_GAME_BODY_H
#define IN_GAME_BODY_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void bodiesInit(void);

void bodiesReset(int stagenum);

unsigned int bodyGetRace(int bodynum);
bool bodyLoad(int bodynum);
struct model *body0f02ce8c(int bodynum, int headnum, struct modeldef *bodymodeldef, struct modeldef *headmodeldef, bool sunglasses, struct model *model, bool isplayer, uint8_t varyheight);
struct model *body0f02d338(int bodynum, int headnum, struct modeldef *bodymodeldef, struct modeldef *headmodeldef, bool sunglasses, uint8_t varyheight);
struct model *bodyAllocateModel(int bodynum, int headnum, unsigned int spawnflags);
int bodyGetRandomBond(void);
int bodyChooseHead(int bodynum);
void bodyAllocateChr(int stagenum, struct packedchr *packed, int cmdindex);
struct prop *bodyAllocateEyespy(struct pad *pad, RoomNum room);
void bodyCalculateHeadOffset(struct modeldef *headmodeldef, int headnum, int bodynum);

#endif
