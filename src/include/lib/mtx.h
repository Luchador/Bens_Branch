#ifndef _IN_LIB_MTX_H
#define _IN_LIB_MTX_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void mtx4LoadIdentity(Mtxf *mtx);
void mtx4MultMtx4InPlace(Mtxf *multmtx, Mtxf *subject);
void mtx4MultMtx4(Mtxf *mtx1, Mtxf *mtx2, Mtxf *dst);
void mtx4RotateVecInPlace(Mtxf *mtx, struct coord *vec);
void mtx4RotateVec(Mtxf *mtx, struct coord *vec, struct coord *dst);
void mtx4TransformVecInPlace(Mtxf *mtx, struct coord *vec);
void mtx4TransformVec(Mtxf *mtx, struct coord *vec, struct coord *dst);
void mtxApplyAffineInPlace(Mtxf *matrix1, Mtxf *matrix2);
void mtxApplyAffineTransform(Mtxf *arg0, Mtxf *arg1, Mtxf *dst);
void mtx3Copy(float src[3][3], float dst[3][3]);
void mtx4Copy(Mtxf *src, Mtxf *dst);
void mtx3ToMtx4(float src[3][3], Mtxf *dst);
void mtx4ToMtx3(Mtxf *src, float dst[3][3]);
void mtx4SetTranslation(struct coord *pos, Mtxf *mtx);
void mtxScaleRowX(float arg0, Mtxf *mtx);
void mtxScaleXAxis(float mult, Mtxf *mtx);
void mtxScaleRowY(float arg0, Mtxf *mtx);
void mtxScaleYAxis(float mult, Mtxf *mtx);
void mtxScaleRowZ(float arg0, Mtxf *mtx);
void mtxScaleZAxis(float mult, Mtxf *mtx);
void mtxScaleRotationAndTranslation(float scale, Mtxf *arg1);
void mtxScaleRotationOnly(float scale, Mtxf *arg1);
void mtxScaleTransform(float arg0, Mtxf *arg1);
void mtxF2L(Mtxf *src, Mtxf *dst);
void mtx00016110(float mtx1[3][3], float mtx2[3][3]);
void mtx00016140(float mtx1[3][3], float mtx2[3][3], float dst[3][3]);
void mtx3LinearTransform(float arg0[3][3], float src[3], float dest[3]);
void mtx00016208(float mtx[3][3], struct coord *coord);
void mtx4LoadYRotationWithTranslation(struct coord *pos, float radians, Mtxf *mtx);
void mtx4LoadXRotation(float radians, Mtxf *mtx);
void mtx4LoadYRotation(float radians, Mtxf *mtx);
void mtx4LoadZRotation(float radians, Mtxf *mtx);
void mtx4LoadRotation(struct coord *rot, Mtxf *mtx);
void mtx4GetRotation(float mtx[4][4], struct coord *dst);
void mtx4LoadRotationAndTranslation(struct coord *pos, struct coord *rot, Mtxf *mtx);
void mtx4LoadTranslation(struct coord *pos, Mtxf *mtx);
void mtx00016710(float mult, float mtx[4][4]);
void mtx00016748(float arg0);
void mtx00016760(void);
void mtx00016784(void);
void mtx00016798(Mtxf *src, Mtxf *dst);
void mtx00016820(Mtx *src, Mtx *dst);
void mtx00016874(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz);
void mtx00016ae4(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz);
void mtx00016b58(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz);
void mtx00016d58(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz);
uint32_t mtx00016dcc(float arg0, float arg1);
void mtx00016e98(float mtx[4][4], float angle, float x, float y, float z);
void mtx4Align(float mtx[4][4], float radians, float x, float y, float z);
void mtx4LoadRotationFrom(float src[4][4], float dst[4][4]);
void mtx000170e4(float src[4][4], float dst[4][4]);
void mtx0001719c(float arg0[4][4], float arg1[4][4]);
void mtxInvertAffineMatrix(float arg0[4][4], float arg1[4][4]);
void mtxFullInverse4x4(float arg0[4][4], float arg1[4][4]);
void mtxInvert4x4Matrix(float arg0[4][4], float arg1[4][4]);
float mtxDeterminant4x4(float arg0[4][4]);
float mtxDeterminant3x3(float arg0, float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8);
float mtxDeterminant2x2(float arg0, float arg1, float arg2, float arg3);

#endif
