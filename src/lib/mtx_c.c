#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "data.h"
#include "lib/mtx.h"
#include "types.h"

float var8005ef10[] = {65536, 65536};

void mtx4LoadIdentity(Mtxf *mtx)
{
	mtx->m[0][0] = 1;
	mtx->m[0][1] = 0;
	mtx->m[0][2] = 0;
	mtx->m[0][3] = 0;

	mtx->m[1][0] = 0;
	mtx->m[1][1] = 1;
	mtx->m[1][2] = 0;
	mtx->m[1][3] = 0;

	mtx->m[2][0] = 0;
	mtx->m[2][1] = 0;
	mtx->m[2][2] = 1;
	mtx->m[2][3] = 0;

	mtx->m[3][0] = 0;
	mtx->m[3][1] = 0;
	mtx->m[3][2] = 0;
	mtx->m[3][3] = 1;
}

void mtx4MultMtx4InPlace(Mtxf *multmtx, Mtxf *subject)
{
	mtx4MultMtx4(multmtx, subject, subject);
}

void mtx4MultMtx4(Mtxf *mtx1, Mtxf *mtx2, Mtxf *dst)
{
	int i;
	float m00 = mtx2->m[0][0];
	float m01 = mtx2->m[0][1];
	float m02 = mtx2->m[0][2];
	float m03 = mtx2->m[0][3];
	float m10 = mtx2->m[1][0];
	float m11 = mtx2->m[1][1];
	float m12 = mtx2->m[1][2];
	float m13 = mtx2->m[1][3];
	float m20 = mtx2->m[2][0];
	float m21 = mtx2->m[2][1];
	float m22 = mtx2->m[2][2];
	float m23 = mtx2->m[2][3];
	float m30 = mtx2->m[3][0];
	float m31 = mtx2->m[3][1];
	float m32 = mtx2->m[3][2];
	float m33 = mtx2->m[3][3];

	for (i = 0; i < 4; i++) {
		dst->m[0][i] = mtx1->m[0][i] * m00 + mtx1->m[1][i] * m01 + mtx1->m[2][i] * m02 + mtx1->m[3][i] * m03;
		dst->m[1][i] = mtx1->m[0][i] * m10 + mtx1->m[1][i] * m11 + mtx1->m[2][i] * m12 + mtx1->m[3][i] * m13;
		dst->m[2][i] = mtx1->m[0][i] * m20 + mtx1->m[1][i] * m21 + mtx1->m[2][i] * m22 + mtx1->m[3][i] * m23;
		dst->m[3][i] = mtx1->m[0][i] * m30 + mtx1->m[1][i] * m31 + mtx1->m[2][i] * m32 + mtx1->m[3][i] * m33;
	}
}

void mtx4RotateVecInPlace(Mtxf *mtx, struct coord *vec)
{
	mtx4RotateVec(mtx, vec, vec);
}

void mtx4RotateVec(Mtxf *mtx, struct coord *vec, struct coord *dst)
{
	float x = vec->x;
	float y = vec->y;
	float z = vec->z;

	dst->x = mtx->m[0][0] * x + mtx->m[1][0] * y + mtx->m[2][0] * z;
	dst->y = mtx->m[0][1] * x + mtx->m[1][1] * y + mtx->m[2][1] * z;
	dst->z = mtx->m[0][2] * x + mtx->m[1][2] * y + mtx->m[2][2] * z;
}

void mtx4TransformVecInPlace(Mtxf *mtx, struct coord *vec)
{
	mtx4TransformVec(mtx, vec, vec);
}

void mtx4TransformVec(Mtxf *mtx, struct coord *vec, struct coord *dst)
{
	float x = vec->x;
	float y = vec->y;
	float z = vec->z;

	dst->x = mtx->m[0][0] * x + mtx->m[1][0] * y + mtx->m[2][0] * z;
	dst->y = mtx->m[0][1] * x + mtx->m[1][1] * y + mtx->m[2][1] * z;
	dst->z = mtx->m[0][2] * x + mtx->m[1][2] * y + mtx->m[2][2] * z;

	dst->x += mtx->m[3][0];
	dst->y += mtx->m[3][1];
	dst->z += mtx->m[3][2];
}

void mtx00015be0(Mtxf *matrix1, Mtxf *matrix2)
{
	mtxApplyAffineTransform(matrix1, matrix2, matrix2);
}

void mtxApplyAffineTransform(Mtxf *arg0, Mtxf *arg1, Mtxf *dst)
{
	float m00 = arg1->m[0][0];
	float m01 = arg1->m[0][1];
	float m02 = arg1->m[0][2];
	float m03 = arg1->m[0][3];
	float m10 = arg1->m[1][0];
	float m11 = arg1->m[1][1];
	float m12 = arg1->m[1][2];
	float m13 = arg1->m[1][3];
	float m20 = arg1->m[2][0];
	float m21 = arg1->m[2][1];
	float m22 = arg1->m[2][2];
	float m23 = arg1->m[2][3];
	float m30 = arg1->m[3][0];
	float m31 = arg1->m[3][1];
	float m32 = arg1->m[3][2];
	float m33 = arg1->m[3][3];

	dst->m[0][0] = arg0->m[0][0] * m00 + arg0->m[1][0] * m01 + arg0->m[2][0] * m02;
	dst->m[0][1] = arg0->m[0][1] * m00 + arg0->m[1][1] * m01 + arg0->m[2][1] * m02;
	dst->m[0][2] = arg0->m[0][2] * m00 + arg0->m[1][2] * m01 + arg0->m[2][2] * m02;
	dst->m[0][3] = 0;

	dst->m[1][0] = arg0->m[0][0] * m10 + arg0->m[1][0] * m11 + arg0->m[2][0] * m12;
	dst->m[1][1] = arg0->m[0][1] * m10 + arg0->m[1][1] * m11 + arg0->m[2][1] * m12;
	dst->m[1][2] = arg0->m[0][2] * m10 + arg0->m[1][2] * m11 + arg0->m[2][2] * m12;
	dst->m[1][3] = 0;

	dst->m[2][0] = arg0->m[0][0] * m20 + arg0->m[1][0] * m21 + arg0->m[2][0] * m22;
	dst->m[2][1] = arg0->m[0][1] * m20 + arg0->m[1][1] * m21 + arg0->m[2][1] * m22;
	dst->m[2][2] = arg0->m[0][2] * m20 + arg0->m[1][2] * m21 + arg0->m[2][2] * m22;
	dst->m[2][3] = 0;

	dst->m[3][0] = arg0->m[0][0] * m30 + arg0->m[1][0] * m31 + arg0->m[2][0] * m32 + arg0->m[3][0];
	dst->m[3][1] = arg0->m[0][1] * m30 + arg0->m[1][1] * m31 + arg0->m[2][1] * m32 + arg0->m[3][1];
	dst->m[3][2] = arg0->m[0][2] * m30 + arg0->m[1][2] * m31 + arg0->m[2][2] * m32 + arg0->m[3][2];
	dst->m[3][3] = 1;
}

void mtx3Copy(float src[3][3], float dst[3][3])
{
	dst[0][0] = src[0][0];
	dst[0][1] = src[0][1];
	dst[0][2] = src[0][2];

	dst[1][0] = src[1][0];
	dst[1][1] = src[1][1];
	dst[1][2] = src[1][2];

	dst[2][0] = src[2][0];
	dst[2][1] = src[2][1];
	dst[2][2] = src[2][2];
}

void mtx4Copy(Mtxf *src, Mtxf *dst)
{
	*dst = *src;
}

void mtx3ToMtx4(float src[3][3], Mtxf *dst)
{
	dst->m[0][0] = src[0][0];
	dst->m[0][1] = src[0][1];
	dst->m[0][2] = src[0][2];
	dst->m[0][3] = 0;

	dst->m[1][0] = src[1][0];
	dst->m[1][1] = src[1][1];
	dst->m[1][2] = src[1][2];
	dst->m[1][3] = 0;

	dst->m[2][0] = src[2][0];
	dst->m[2][1] = src[2][1];
	dst->m[2][2] = src[2][2];
	dst->m[2][3] = 0;

	dst->m[3][0] = 0;
	dst->m[3][1] = 0;
	dst->m[3][2] = 0;
	dst->m[3][3] = 1;
}

void mtx4ToMtx3(Mtxf *src, float dst[3][3])
{
	dst[0][0] = src->m[0][0];
	dst[0][1] = src->m[0][1];
	dst[0][2] = src->m[0][2];

	dst[1][0] = src->m[1][0];
	dst[1][1] = src->m[1][1];
	dst[1][2] = src->m[1][2];

	dst[2][0] = src->m[2][0];
	dst[2][1] = src->m[2][1];
	dst[2][2] = src->m[2][2];
}

void mtx4SetTranslation(struct coord *pos, Mtxf *mtx)
{
	mtx->m[3][0] = pos->x;
	mtx->m[3][1] = pos->y;
	mtx->m[3][2] = pos->z;
}

void mtx00015df0(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;
	mtx->m[0][3] *= mult;
}

void mtx00015e24(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;
}

void mtx00015e4c(float mult, Mtxf *mtx)
{
	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;
	mtx->m[1][3] *= mult;
}

void mtx00015e80(float mult, Mtxf *mtx)
{
	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;
}

void mtx00015ea8(float mult, Mtxf *mtx)
{
	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
	mtx->m[2][3] *= mult;
}

void mtx00015edc(float mult, Mtxf *mtx)
{
	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
}

void mtx00015f04(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;
	mtx->m[0][3] *= mult;

	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;
	mtx->m[1][3] *= mult;

	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
	mtx->m[2][3] *= mult;
}

void mtx00015f4c(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;

	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;

	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
}

void mtx00015f88(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;

	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;

	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;

	mtx->m[3][0] *= mult;
	mtx->m[3][1] *= mult;
	mtx->m[3][2] *= mult;
}

void mtxF2L(Mtxf *src, Mtxf *dst)
{
#ifndef GBI_FLOATS
	uint32_t src00 = (int) (src->m[0][0] * var8005ef10[0]);
	uint32_t src01 = (int) (src->m[0][1] * var8005ef10[0]);
	uint32_t src02 = (int) (src->m[0][2] * var8005ef10[0]);
	uint32_t src03 = (int) (src->m[0][3] * var8005ef10[1]);
	uint32_t src10 = (int) (src->m[1][0] * var8005ef10[0]);
	uint32_t src11 = (int) (src->m[1][1] * var8005ef10[0]);
	uint32_t src12 = (int) (src->m[1][2] * var8005ef10[0]);
	uint32_t src13 = (int) (src->m[1][3] * var8005ef10[1]);
	uint32_t src20 = (int) (src->m[2][0] * var8005ef10[0]);
	uint32_t src21 = (int) (src->m[2][1] * var8005ef10[0]);
	uint32_t src22 = (int) (src->m[2][2] * var8005ef10[0]);
	uint32_t src23 = (int) (src->m[2][3] * var8005ef10[1]);
	uint32_t src30 = (int) (src->m[3][0] * var8005ef10[0]);
	uint32_t src31 = (int) (src->m[3][1] * var8005ef10[0]);
	uint32_t src32 = (int) (src->m[3][2] * var8005ef10[0]);
	uint32_t src33 = (int) (src->m[3][3] * var8005ef10[1]);

	dst->l[0][0] = (src00 & 0xffff0000) | src01 >> 16;
	dst->l[0][1] = (src02 & 0xffff0000) | src03 >> 16;
	dst->l[0][2] = (src10 & 0xffff0000) | src11 >> 16;
	dst->l[0][3] = (src12 & 0xffff0000) | src13 >> 16;
	dst->l[1][0] = (src20 & 0xffff0000) | src21 >> 16;
	dst->l[1][1] = (src22 & 0xffff0000) | src23 >> 16;
	dst->l[1][2] = (src30 & 0xffff0000) | src31 >> 16;
	dst->l[1][3] = (src32 & 0xffff0000) | src33 >> 16;

	dst->l[2][0] = src00 << 16 | (src01 & 0xffff);
	dst->l[2][1] = src02 << 16 | (src03 & 0xffff);
	dst->l[2][2] = src10 << 16 | (src11 & 0xffff);
	dst->l[2][3] = src12 << 16 | (src13 & 0xffff);
	dst->l[3][0] = src20 << 16 | (src21 & 0xffff);
	dst->l[3][1] = src22 << 16 | (src23 & 0xffff);
	dst->l[3][2] = src30 << 16 | (src31 & 0xffff);
	dst->l[3][3] = src32 << 16 | (src33 & 0xffff);
#else
	if (src != dst) {
		bcopy(src, dst, sizeof(*dst));
	}
#endif
}
