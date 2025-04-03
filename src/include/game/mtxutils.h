#ifndef _IN_GAME_MTXUTILS_H
#define _IN_GAME_MTXUTILS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void mtxLoadRandomRotation(Mtxf *mtx);
void mtxRandomToss(struct coord *arg0, Mtxf *mtx);
void func0f0965e4(float *arg0, float *arg1, float arg2);
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, int count);
void mtxF2LBulk(Mtxf *matrices, int count);

#endif
