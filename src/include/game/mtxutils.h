#ifndef _IN_GAME_MTXUTILS_H
#define _IN_GAME_MTXUTILS_H
#include "data.h"
#include "types.h"

void mtxF2LBulk(Mtxf *mtx, int count);
void mtxLoadRandomRotation(Mtxf *mtx);
void mtxRandomToss(struct coord *arg0, Mtxf *mtx);
void func0f0965e4(float *arg0, float *arg1, float arg2);
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, int count);
void mtxAlignF(float mf[4][4], float a, float x, float y, float z);
void mtxAlign(Mtx *m, float a, float x, float y, float z);
void mtxIdent(Mtx *m);
void mtxIdentF(float mf[4][4]);
void mtxF2L2(float mf[4][4], Mtx *m);
void mtxScaleF(float mf[4][4], float x, float y, float z);
void mtxScale(Mtx *m, float x, float y, float z);
void mtxFrustumF(float mf[4][4], float l, float r, float b, float t, float n, float f, float scale);
void mtxFrustum(Mtx *m, float l, float r, float b, float t, float n, float f, float scale);
void mtxPerspectiveF(float mf[4][4], float fovy, float aspect, float near, float far, float scale);
void mtxPerspective(Mtx *m, float fovy, float aspect, float near, float far, float scale);
void mtxRotateF(float mf[4][4], float a, float x, float y, float z);
void mtxRotate(Mtx *m, float a, float x, float y, float z);
void mtxLookAtReflectF(float mf[4][4], LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp);
void mtxLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp);

#endif
