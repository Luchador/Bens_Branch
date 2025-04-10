#pragma once

#include "data.h"
#include "types.h"

void quaternionEulerToQuat(struct coord *arg0, float quat[4]);
void quaternionToMtxF(float arg0[4], Mtxf *arg1);
void quaternion3x3MtxToQuatF(Mtxf *matrix, float arg1[4]);
void quaternion3x3MtxToQuat(Mtx *mtx, float arg1[4]);
void quaternionToTransformMtx(struct coord *pos, float rot[4], Mtxf *matrix);
void quaternionSlerp(float q1[4], float q2[4], float t, float result[4]);
void quaternionSlerpFromIdentity(float target[4], float t, float result[4]);
void quaternionAvoidFlips(float q1[4], float q2[4]);
void quaternionMultQuaternion(float a[4], float b[4], float result[4]);