#ifndef _IN_GAME_MTXUTILS_H
#define _IN_GAME_MTXUTILS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void mtxLoadRandomRotation(Mtxf *mtx);
void mtxRandomToss(struct coord *arg0, Mtxf *mtx);
void func0f0965e4(f32 *arg0, f32 *arg1, f32 arg2);
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, s32 count);

#endif
