#ifndef _IN_GAME_MTXUTILS_H
#define _IN_GAME_MTXUTILS_H
#include <ultra64.h>
#include <PR/gbi.h>
#include "data.h"
#include "types.h"

void mtxLoadRandomRotation(Mtxf *mtx);
void mtxRandomToss(struct coord *arg0, Mtxf *mtx);
void func0f0965e4(float *arg0, float *arg1, float arg2);
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, int count);
void mtxPerspective(Mtx *m, float fovy, float aspect, float near, float far);
void mtxPerspectiveF(float mf[4][4], float fovy, float aspect, float near, float far);
void mtxLookAtF(float mf[4][4], float xEye, float yEye, float zEye,
	float xAt,  float yAt,  float zAt,
	float xUp,  float yUp,  float zUp);
void mtxLookAtReflectF(float mf[4][4], LookAt *l,
    float xEye, float yEye, float zEye,
    float xAt,  float yAt,  float zAt,
    float xUp,  float yUp,  float zUp);
void mtxLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye,
        float xAt,  float yAt,  float zAt,
        float xUp,  float yUp,  float zUp);
void mtxLookAt(Mtx *m, float xEye, float yEye, float zEye,
    float xAt,  float yAt,  float zAt,
    float xUp,  float yUp,  float zUp);
void mtxMtxF2L(float mf[4][4], Mtx *m);

#endif
