#pragma once

#include "data.h"
#include "types.h"

void mtxLoadRandomRotation(Mtxf *mtx);
void mtxRandomToss(struct coord *arg0, Mtxf *mtx);
void func0f0965e4(float *arg0, float *arg1, float arg2);
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, int count);
void mtxAlignF(float mf[4][4], float a, float x, float y, float z);
void mtxAlign(Mtx *m, float a, float x, float y, float z);
void mtxF2L2(float mf[4][4], Mtx *m);
void mtxF2L(Mtxf *src, Mtxf *dst);
void mtx4MultMtx4InPlace(Mtxf *multmtx, Mtxf *subject);
void mtx4MultMtx4(Mtxf *mtx1, Mtxf *mtx2, Mtxf *dst);
extern void mtxIdent(Mtx *m);
extern void mtxIdentF(float mf[4][4]);
void mtx4LoadIdentity(Mtxf *mtx);
void mtxScaleF(float mf[4][4], float x, float y, float z);
void mtxScale(Mtx *m, float x, float y, float z);
void mtxScaleRow0Full(float arg0, Mtxf *mtx);
void mtxScaleRow0Vec(float mult, Mtxf *mtx);
void mtxScaleRow1Full(float arg0, Mtxf *mtx);
void mtxScaleRow1Vec(float mult, Mtxf *mtx);
void mtxScaleRow2Full(float arg0, Mtxf *mtx);
void mtxScaleRow2Vec(float mult, Mtxf *mtx);
void mtxScaleRotationPart(float scale, Mtxf *arg1);
void mtxRotateF(float mf[4][4], float a, float x, float y, float z);
void mtxRotate(Mtx *m, float a, float x, float y, float z);
void mtx4RotateVecInPlace(Mtxf *mtx, struct coord *vec);
void mtx4RotateVec(Mtxf *mtx, struct coord *vec, struct coord *dst);
void mtx4TransformVecInPlace(Mtxf *mtx, struct coord *vec);
void mtx4TransformVec(Mtxf *mtx, struct coord *vec, struct coord *dst);
void mtx4SetTranslation(struct coord *pos, Mtxf *mtx);
void mtxApplyAffineTransformInPlace(Mtxf *matrix1, Mtxf *matrix2);
void mtxApplyAffineTransform(Mtxf *arg0, Mtxf *arg1, Mtxf *dst);
void mtxFrustumF(float mf[4][4], float l, float r, float b, float t, float n, float f, float scale);
void mtxFrustum(Mtx *m, float l, float r, float b, float t, float n, float f, float scale);
void mtxPerspectiveF(float mf[4][4], float fovy, float aspect, float near, float far, float scale);
void mtxPerspective(Mtx *m, uint16_t *perspNorm, float fovy, float aspect, float near, float far, float scale);
void mtxLookAtReflectF(float mf[4][4], LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp);
void mtxLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp);
void mtx3Copy(float src[3][3], float dst[3][3]);
void mtx4Copy(Mtxf *src, Mtxf *dst);
void mtxScale3x4(float arg0, Mtxf *arg1);
void mtx00015f4c(float scale, Mtxf *arg1);
void mtx3ToMtx4(float src[3][3], Mtxf *dst);
void mtx4ToMtx3(Mtxf *src, float dst[3][3]);