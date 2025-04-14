#include <math.h>
#include <string.h>
#include "constants.h"
#include "game/objectives.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "game/tex.h"
#include "game/training.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define	FTOFRAC8(x)	((int) MIN(((x) * (128.0f)), 127.0f) & 0xff)

#define EPSILON 0.0000019073486f

void integrateDampedMotion(float *arg0, float *arg1, float arg2)
{
	float tmp = arg1[0] - arg2 * 0.27777779f;
	arg0[0] += arg2 * (arg1[0] + tmp) * 0.5f;
	arg1[0] = tmp;
}

void mtxLoadRandomRotation(Mtx *mtx)
{
	struct coord coord = {0, 0, 0};

	coord.x = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;
	coord.y = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;
	coord.z = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;

	mtx4LoadRotation(&coord, (Mtx*)mtx);
}

void mtxRandomToss(struct coord *coord, Mtx *mtx)
{
	coord->x = RANDOMFRAC() * 1.6666666269302f * 4.0f - 3.3333332538605f;
	coord->y = RANDOMFRAC() * 1.6666666269302f * 4.0f;
	coord->z = RANDOMFRAC() * 1.6666666269302f * 4.0f - 3.3333332538605f;

	mtxLoadRandomRotation(mtx);
}

// Used for spinning falling objects such as grenades and dropped guns
void mtxApplyRotation(Mtx *arg0, Mtx *arg1, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		mtxApplyAffineTransformInPlace(arg1, arg0);
	}
}

void mtxAlign(Mtx *m, float a, float x, float y, float z)
{
	static float dtor = 3.1415926f / 180.0f;
	float s, c, h, hinv;

	utilsNormalizeF(&x, &y, &z);

	a *= dtor;
	s = sinf(a);
	c = cosf(a);
	h = sqrtf(x * x + z * z);

	mtxIdent((Mtx*)*m);

	if (h != 0) {
		hinv = 1 / h;

		(*m)[0][0] = (-z*c - s*y*x) * hinv;
		(*m)[1][0] = (z*s - c*y*x) * hinv;
		(*m)[2][0] = -x;
		(*m)[3][0] = 0;

		(*m)[0][1] = s*h;
		(*m)[1][1] = c*h;
		(*m)[2][1] = -y;
		(*m)[3][1] = 0;

		(*m)[0][2] = (c*x - s*y*z) * hinv;
		(*m)[1][2] = (-s*x - c*y*z) * hinv;
		(*m)[2][2] = -z;
		(*m)[3][2] = 0;

		(*m)[0][3] = 0;
		(*m)[1][3] = 0;
		(*m)[2][3] = 0;
		(*m)[3][3] = 1;
	}
}

/*
* Identity Matrix Functions
*/

void mtxIdent(Mtx *m)
{
	int	i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			(*m)[i][j] = i == j ? 1 : 0;
		}
	}
}

/*
* Translation Matrix Functions
*/

void mtx4TransformVecInPlace(Mtx *mtx, struct coord *vec)
{
	mtx4TransformVec(mtx, vec, vec);
}

void mtx4TransformVec(Mtx *mtx, struct coord *vec, struct coord *dst)
{
	float x = vec->x;
	float y = vec->y;
	float z = vec->z;

	dst->x = (*mtx)[0][0] * x + (*mtx)[1][0] * y + (*mtx)[2][0] * z;
	dst->y = (*mtx)[0][1] * x + (*mtx)[1][1] * y + (*mtx)[2][1] * z;
	dst->z = (*mtx)[0][2] * x + (*mtx)[1][2] * y + (*mtx)[2][2] * z;

	dst->x += (*mtx)[3][0];
	dst->y += (*mtx)[3][1];
	dst->z += (*mtx)[3][2];
}

void mtxApplyAffineTransformInPlace(Mtx *matrix1, Mtx *matrix2)
{
	mtxApplyAffineTransform(matrix1, matrix2, matrix2);
}

void mtxApplyAffineTransform(Mtx *arg0, Mtx *arg1, Mtx *dst)
{
	float m00 = (*arg1)[0][0];
	float m01 = (*arg1)[0][1];
	float m02 = (*arg1)[0][2];
	float m03 = (*arg1)[0][3];
	float m10 = (*arg1)[1][0];
	float m11 = (*arg1)[1][1];
	float m12 = (*arg1)[1][2];
	float m13 = (*arg1)[1][3];
	float m20 = (*arg1)[2][0];
	float m21 = (*arg1)[2][1];
	float m22 = (*arg1)[2][2];
	float m23 = (*arg1)[2][3];
	float m30 = (*arg1)[3][0];
	float m31 = (*arg1)[3][1];
	float m32 = (*arg1)[3][2];
	float m33 = (*arg1)[3][3];

	(*dst)[0][0] = (*arg0)[0][0] * m00 + (*arg0)[1][0] * m01 + (*arg0)[2][0] * m02;
	(*dst)[0][1] = (*arg0)[0][1] * m00 + (*arg0)[1][1] * m01 + (*arg0)[2][1] * m02;
	(*dst)[0][2] = (*arg0)[0][2] * m00 + (*arg0)[1][2] * m01 + (*arg0)[2][2] * m02;
	(*dst)[0][3] = 0;

	(*dst)[1][0] = (*arg0)[0][0] * m10 + (*arg0)[1][0] * m11 + (*arg0)[2][0] * m12;
	(*dst)[1][1] = (*arg0)[0][1] * m10 + (*arg0)[1][1] * m11 + (*arg0)[2][1] * m12;
	(*dst)[1][2] = (*arg0)[0][2] * m10 + (*arg0)[1][2] * m11 + (*arg0)[2][2] * m12;
	(*dst)[1][3] = 0;

	(*dst)[2][0] = (*arg0)[0][0] * m20 + (*arg0)[1][0] * m21 + (*arg0)[2][0] * m22;
	(*dst)[2][1] = (*arg0)[0][1] * m20 + (*arg0)[1][1] * m21 + (*arg0)[2][1] * m22;
	(*dst)[2][2] = (*arg0)[0][2] * m20 + (*arg0)[1][2] * m21 + (*arg0)[2][2] * m22;
	(*dst)[2][3] = 0;

	(*dst)[3][0] = (*arg0)[0][0] * m30 + (*arg0)[1][0] * m31 + (*arg0)[2][0] * m32 + (*arg0)[3][0];
	(*dst)[3][1] = (*arg0)[0][1] * m30 + (*arg0)[1][1] * m31 + (*arg0)[2][1] * m32 + (*arg0)[3][1];
	(*dst)[3][2] = (*arg0)[0][2] * m30 + (*arg0)[1][2] * m31 + (*arg0)[2][2] * m32 + (*arg0)[3][2];
	(*dst)[3][3] = 1;
}

void mtx4SetTranslation(struct coord *pos, Mtx *mtx)
{
	(*mtx)[3][0] = pos->x;
	(*mtx)[3][1] = pos->y;
	(*mtx)[3][2] = pos->z;
}

/*
* Scaling Matrix Functions
*/

void mtxScale(Mtx *m, float x, float y, float z)
{
	mtxIdent(m);

	(*m)[0][0] = x;
	(*m)[1][1] = y;
	(*m)[2][2] = z;
	(*m)[3][3] = 1;
}

void mtxScaleRow0Full(float mult, Mtx *mtx)
{
	(*mtx)[0][0] *= mult;
	(*mtx)[0][1] *= mult;
	(*mtx)[0][2] *= mult;
	(*mtx)[0][3] *= mult;
}

void mtxScaleRow0Vec(float mult, Mtx *mtx)
{
	(*mtx)[0][0] *= mult;
	(*mtx)[0][1] *= mult;
	(*mtx)[0][2] *= mult;
}

void mtxScaleRow1Full(float mult, Mtx *mtx)
{
	(*mtx)[1][0] *= mult;
	(*mtx)[1][1] *= mult;
	(*mtx)[1][2] *= mult;
	(*mtx)[1][3] *= mult;
}

void mtxScaleRow1Vec(float mult, Mtx *mtx)
{
	(*mtx)[1][0] *= mult;
	(*mtx)[1][1] *= mult;
	(*mtx)[1][2] *= mult;
}

void mtxScaleRow2Full(float mult, Mtx *mtx)
{
	(*mtx)[2][0] *= mult;
	(*mtx)[2][1] *= mult;
	(*mtx)[2][2] *= mult;
	(*mtx)[2][3] *= mult;
}

void mtxScaleRow2Vec(float mult, Mtx *mtx)
{
	(*mtx)[2][0] *= mult;
	(*mtx)[2][1] *= mult;
	(*mtx)[2][2] *= mult;
}

void mtxScaleRotationPart(float mult, Mtx *mtx)
{
	(*mtx)[0][0] *= mult;
	(*mtx)[0][1] *= mult;
	(*mtx)[0][2] *= mult;
	(*mtx)[0][3] *= mult;

	(*mtx)[1][0] *= mult;
	(*mtx)[1][1] *= mult;
	(*mtx)[1][2] *= mult;
	(*mtx)[1][3] *= mult;

	(*mtx)[2][0] *= mult;
	(*mtx)[2][1] *= mult;
	(*mtx)[2][2] *= mult;
	(*mtx)[2][3] *= mult;
}

// Multiplies all the 3D rotational and translational components (but not the fourth column)
void mtxScale3x4(float mult, Mtx *mtx)
{
	(*mtx)[0][0] *= mult;
	(*mtx)[0][1] *= mult;
	(*mtx)[0][2] *= mult;

	(*mtx)[1][0] *= mult;
	(*mtx)[1][1] *= mult;
	(*mtx)[1][2] *= mult;

	(*mtx)[2][0] *= mult;
	(*mtx)[2][1] *= mult;
	(*mtx)[2][2] *= mult;

	(*mtx)[3][0] *= mult;
	(*mtx)[3][1] *= mult;
	(*mtx)[3][2] *= mult;
}

/*
* Rotation Matrix Functions
*/

void mtxRotate(Mtx *mtx, float a, float x, float y, float z)
{
	float sine;
	float cosine;
	float ab, bc, ca, t;

	utilsNormalizeF(&x, &y, &z);
	a *= 3.1415926f / 180.0f;
	sine = sinf(a);
	cosine = cosf(a);
	t = 1 - cosine;
	ab = x * y * t;
	bc = y * z * t;
	ca = z * x * t;

	mtxIdent(mtx);

	t = x * x;
	(*mtx)[0][0] = t + cosine * (1 - t);
	(*mtx)[2][1] = bc - x * sine;
	(*mtx)[1][2] = bc + x * sine;

	t = y * y;
	(*mtx)[1][1] = t + cosine * (1 - t);
	(*mtx)[2][0] = ca + y * sine;
	(*mtx)[0][2] = ca - y * sine;

	t = z * z;
	(*mtx)[2][2] = t + cosine * (1 - t);
	(*mtx)[1][0] = ab - z * sine;
	(*mtx)[0][1] = ab + z * sine;
}

void mtx4RotateVecInPlace(Mtx *mtx, struct coord *vec)
{
	mtx4RotateVec((Mtx*)mtx, vec, vec);
}

void mtx4RotateVec(Mtx *mtx, struct coord *vec, struct coord *dst)
{
	float x = vec->x;
	float y = vec->y;
	float z = vec->z;

	dst->x = (*mtx)[0][0] * x + (*mtx)[1][0] * y + (*mtx)[2][0] * z;
	dst->y = (*mtx)[0][1] * x + (*mtx)[1][1] * y + (*mtx)[2][1] * z;
	dst->z = (*mtx)[0][2] * x + (*mtx)[1][2] * y + (*mtx)[2][2] * z;
}

/*
* Frustum Matrix Functions
*/

void mtxFrustum(Mtx *m, float l, float r, float b, float t, float n, float f, float scale)
{
	int i, j;

	mtxIdent(m);

	(*m)[0][0] = 2 * n / (r - l);
	(*m)[1][1] = 2 * n / (t - b);
	(*m)[2][0] = (r + l) / (r - l);
	(*m)[2][1] = (t + b) / (t - b);
	(*m)[2][2] = -(f + n) / (f - n);
	(*m)[2][3] = -1;
	(*m)[3][2] = -2 * f * n / (f - n);
	(*m)[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			(*m)[i][j] *= scale;
		}
	}
}

/*
*  Perspective Matrix Functions
*/

void mtxPerspective(Mtx *mtx, float fovy, float aspect, float near, float far, float scale)
{
	float cot;
	int	i, j;

	mtxIdent((Mtx*)mtx);

	fovy *= 3.1415926f / 180.0f;
	cot = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);

	(*mtx)[0][0] = cot / aspect;
	(*mtx)[1][1] = cot;
	(*mtx)[2][2] = (near + far) / (near - far);
	(*mtx)[2][3] = -1;
	(*mtx)[3][2] = (2.0f * near * far) / (near - far);
	(*mtx)[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			(*mtx)[i][j] *= scale;
		}
	}
}

/*
* Reflection Matrix Functions
*/

void mtxLookAtReflect(Mtx *mtx, LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp)
{
	float len, xLook, yLook, zLook, xRight, yRight, zRight;

	mtxIdent(mtx);

	xLook = xAt - xEye;
	yLook = yAt - yEye;
	zLook = zAt - zEye;

	/* Negate because positive Z is behind us: */
	len = -1.0f / sqrtf (xLook*xLook + yLook*yLook + zLook*zLook);
	xLook *= len;
	yLook *= len;
	zLook *= len;

	/* Right = Up x Look */

	xRight = yUp * zLook - zUp * yLook;
	yRight = zUp * xLook - xUp * zLook;
	zRight = xUp * yLook - yUp * xLook;
	len = 1.0f / sqrtf (xRight*xRight + yRight*yRight + zRight*zRight);
	xRight *= len;
	yRight *= len;
	zRight *= len;

	/* Up = Look x Right */

	xUp = yLook * zRight - zLook * yRight;
	yUp = zLook * xRight - xLook * zRight;
	zUp = xLook * yRight - yLook * xRight;
	len = 1.0f / sqrtf (xUp*xUp + yUp*yUp + zUp*zUp);
	xUp *= len;
	yUp *= len;
	zUp *= len;

	/* reflectance vectors = Up and Right */

	l->l[0].l.dir[0] = FTOFRAC8(xRight);
	l->l[0].l.dir[1] = FTOFRAC8(yRight);
	l->l[0].l.dir[2] = FTOFRAC8(zRight);
	l->l[1].l.dir[0] = FTOFRAC8(xUp);
	l->l[1].l.dir[1] = FTOFRAC8(yUp);
	l->l[1].l.dir[2] = FTOFRAC8(zUp);
	l->l[0].l.col[0] = 0x00;
	l->l[0].l.col[1] = 0x00;
	l->l[0].l.col[2] = 0x00;
	l->l[0].l.pad1 = 0x00;
	l->l[0].l.colc[0] = 0x00;
	l->l[0].l.colc[1] = 0x00;
	l->l[0].l.colc[2] = 0x00;
	l->l[0].l.pad2 = 0x00;
	l->l[1].l.col[0] = 0x00;
	l->l[1].l.col[1] = 0x80;
	l->l[1].l.col[2] = 0x00;
	l->l[1].l.pad1 = 0x00;
	l->l[1].l.colc[0] = 0x00;
	l->l[1].l.colc[1] = 0x80;
	l->l[1].l.colc[2] = 0x00;
	l->l[1].l.pad2 = 0x00;

	(*mtx)[0][0] = xRight;
	(*mtx)[1][0] = yRight;
	(*mtx)[2][0] = zRight;
	(*mtx)[3][0] = -(xEye * xRight + yEye * yRight + zEye * zRight);

	(*mtx)[0][1] = xUp;
	(*mtx)[1][1] = yUp;
	(*mtx)[2][1] = zUp;
	(*mtx)[3][1] = -(xEye * xUp + yEye * yUp + zEye * zUp);

	(*mtx)[0][2] = xLook;
	(*mtx)[1][2] = yLook;
	(*mtx)[2][2] = zLook;
	(*mtx)[3][2] = -(xEye * xLook + yEye * yLook + zEye * zLook);

	(*mtx)[0][3] = 0;
	(*mtx)[1][3] = 0;
	(*mtx)[2][3] = 0;
	(*mtx)[3][3] = 1;
}

/*
* Copy Matrix Functions
*/

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

void mtx4Copy(Mtx *src, Mtx *dst) 
{
	memcpy(dst, src, sizeof(Mtx));
}

void mtxScaleRotationOnly(float mult, Mtx *mtx)
{
	(*mtx)[0][0] *= mult;
	(*mtx)[0][1] *= mult;
	(*mtx)[0][2] *= mult;

	(*mtx)[1][0] *= mult;
	(*mtx)[1][1] *= mult;
	(*mtx)[1][2] *= mult;

	(*mtx)[2][0] *= mult;
	(*mtx)[2][1] *= mult;
	(*mtx)[2][2] *= mult;
}

void mtx3ToMtx4(float src[3][3], Mtx *dst)
{
	(*dst)[0][0] = src[0][0];
	(*dst)[0][1] = src[0][1];
	(*dst)[0][2] = src[0][2];
	(*dst)[0][3] = 0;

	(*dst)[1][0] = src[1][0];
	(*dst)[1][1] = src[1][1];
	(*dst)[1][2] = src[1][2];
	(*dst)[1][3] = 0;

	(*dst)[2][0] = src[2][0];
	(*dst)[2][1] = src[2][1];
	(*dst)[2][2] = src[2][2];
	(*dst)[2][3] = 0;

	(*dst)[3][0] = 0;
	(*dst)[3][1] = 0;
	(*dst)[3][2] = 0;
	(*dst)[3][3] = 1;
}

void mtx4ToMtx3(Mtx *src, float dst[3][3])
{
	dst[0][0] = (*src)[0][0];
	dst[0][1] = (*src)[0][1];
	dst[0][2] = (*src)[0][2];

	dst[1][0] = (*src)[1][0];
	dst[1][1] = (*src)[1][1];
	dst[1][2] = (*src)[1][2];

	dst[2][0] = (*src)[2][0];
	dst[2][1] = (*src)[2][1];
	dst[2][2] = (*src)[2][2];
}

void mtx4MultMtx4InPlace(Mtx *multmtx, Mtx *subject)
{
	mtx4MultMtx4(multmtx, subject, subject);
}

/*
 * Multiplies two 4x4 matrices and stores the result in the destination matrix.
 *
 * This performs a standard matrix multiplication: dst = mtx2 * mtx1.
 * The operation is column-major, which is typical for graphics applications.
 * This is used in transformations like combining translation, rotation, and scale.
 */
void mtx4MultMtx4(Mtx *mtx1, Mtx *mtx2, Mtx *dst)
{
	int i;
	float m00 = (*mtx2)[0][0];
	float m01 = (*mtx2)[0][1];
	float m02 = (*mtx2)[0][2];
	float m03 = (*mtx2)[0][3];
	float m10 = (*mtx2)[1][0];
	float m11 = (*mtx2)[1][1];
	float m12 = (*mtx2)[1][2];
	float m13 = (*mtx2)[1][3];
	float m20 = (*mtx2)[2][0];
	float m21 = (*mtx2)[2][1];
	float m22 = (*mtx2)[2][2];
	float m23 = (*mtx2)[2][3];
	float m30 = (*mtx2)[3][0];
	float m31 = (*mtx2)[3][1];
	float m32 = (*mtx2)[3][2];
	float m33 = (*mtx2)[3][3];

	for (i = 0; i < 4; i++) {
		(*dst)[0][i] = (*mtx1)[0][i] * m00 + (*mtx1)[1][i] * m01 + (*mtx1)[2][i] * m02 + (*mtx1)[3][i] * m03;
		(*dst)[1][i] = (*mtx1)[0][i] * m10 + (*mtx1)[1][i] * m11 + (*mtx1)[2][i] * m12 + (*mtx1)[3][i] * m13;
		(*dst)[2][i] = (*mtx1)[0][i] * m20 + (*mtx1)[1][i] * m21 + (*mtx1)[2][i] * m22 + (*mtx1)[3][i] * m23;
		(*dst)[3][i] = (*mtx1)[0][i] * m30 + (*mtx1)[1][i] * m31 + (*mtx1)[2][i] * m32 + (*mtx1)[3][i] * m33;
	}
}

void mtx3x3TransposeMulInPlace(float mtx1[3][3], float mtx2[3][3])
{
	float mtx3[3][3];

	mtx3x3TransposeMul(mtx1, mtx2, mtx3);
	mtx3Copy(mtx3, mtx2);
}

void mtx3x3TransposeMul(float mtx1[3][3], float mtx2[3][3], float dst[3][3])
{
	int i;
	int j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[j][i] = mtx1[0][i] * mtx2[j][0] + mtx1[1][i] * mtx2[j][1] + mtx1[2][i] * mtx2[j][2];
		}
	}
}

void mtx3LinearTransform(float mtx[3][3], float src[3], float dest[3])
{
	int i;

	for (i = 0; i < 3; i++) {
		dest[i] = mtx[0][i] * src[0] + mtx[1][i] * src[1] + mtx[2][i] * src[2];
	}
}

void mtx00016208(float mtx[3][3], struct coord *coord)
{
	float tmp[3];

	mtx3LinearTransform(mtx, (float *)coord, tmp);

	coord->x = tmp[0];
	coord->y = tmp[1];
	coord->z = tmp[2];
}

void mtx4LoadXRotation(float angle, Mtx *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	(*mtx)[0][0] = 1;
	(*mtx)[0][1] = 0;
	(*mtx)[0][2] = 0;
	(*mtx)[0][3] = 0;

	(*mtx)[1][0] = 0;
	(*mtx)[1][1] = cos;
	(*mtx)[1][2] = sin;
	(*mtx)[1][3] = 0;

	(*mtx)[2][0] = 0;
	(*mtx)[2][1] = -sin;
	(*mtx)[2][2] = cos;
	(*mtx)[2][3] = 0;

	(*mtx)[3][0] = 0;
	(*mtx)[3][1] = 0;
	(*mtx)[3][2] = 0;
	(*mtx)[3][3] = 1;
}

void mtx4LoadYRotation(float angle, Mtx *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	(*mtx)[0][0] = cos;
	(*mtx)[0][1] = 0;
	(*mtx)[0][2] = -sin;
	(*mtx)[0][3] = 0;

	(*mtx)[1][0] = 0;
	(*mtx)[1][1] = 1;
	(*mtx)[1][2] = 0;
	(*mtx)[1][3] = 0;

	(*mtx)[2][0] = sin;
	(*mtx)[2][1] = 0;
	(*mtx)[2][2] = cos;
	(*mtx)[2][3] = 0;

	(*mtx)[3][0] = 0;
	(*mtx)[3][1] = 0;
	(*mtx)[3][2] = 0;
	(*mtx)[3][3] = 1;
}

void mtx4LoadYRotationWithTranslation(struct coord *coord, float angle, Mtx *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	(*mtx)[0][0] = cos;
	(*mtx)[0][1] = 0;
	(*mtx)[0][2] = -sin;
	(*mtx)[0][3] = 0;

	(*mtx)[1][0] = 0;
	(*mtx)[1][1] = 1;
	(*mtx)[1][2] = 0;
	(*mtx)[1][3] = 0;

	(*mtx)[2][0] = sin;
	(*mtx)[2][1] = 0;
	(*mtx)[2][2] = cos;
	(*mtx)[2][3] = 0;

	(*mtx)[3][0] = coord->x;
	(*mtx)[3][1] = coord->y;
	(*mtx)[3][2] = coord->z;
	(*mtx)[3][3] = 1;
}

void mtx4LoadZRotation(float angle, Mtx *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	(*mtx)[0][0] = cos;
	(*mtx)[0][1] = sin;
	(*mtx)[0][2] = 0;
	(*mtx)[0][3] = 0;

	(*mtx)[1][0] = -sin;
	(*mtx)[1][1] = cos;
	(*mtx)[1][2] = 0;
	(*mtx)[1][3] = 0;

	(*mtx)[2][0] = 0;
	(*mtx)[2][1] = 0;
	(*mtx)[2][2] = 1;
	(*mtx)[2][3] = 0;

	(*mtx)[3][0] = 0;
	(*mtx)[3][1] = 0;
	(*mtx)[3][2] = 0;
	(*mtx)[3][3] = 1;
}

void mtx4LoadRotation(struct coord *src, Mtx *dest)
{
	float xcos = cosf(src->x);
	float xsin = sinf(src->x);
	float ycos = cosf(src->y);
	float ysin = sinf(src->y);
	float zcos = cosf(src->z);
	float zsin = sinf(src->z);
	float a = xsin * zsin;
	float b = xcos * zsin;
	float c = xsin * zcos;
	float d = xcos * zcos;

	(*dest)[0][0] = ycos * zcos;
	(*dest)[0][1] = ycos * zsin;
	(*dest)[0][2] = -ysin;
	(*dest)[0][3] = 0;

	(*dest)[1][0] = c * ysin - xcos * zsin;
	(*dest)[1][1] = a * ysin + xcos * zcos;
	(*dest)[1][2] = xsin * ycos;
	(*dest)[1][3] = 0;

	(*dest)[2][0] = d * ysin + xsin * zsin;
	(*dest)[2][1] = b * ysin - xsin * zcos;
	(*dest)[2][2] = xcos * ycos;
	(*dest)[2][3] = 0;

	(*dest)[3][0] = 0;
	(*dest)[3][1] = 0;
	(*dest)[3][2] = 0;
	(*dest)[3][3] = 1;
}

void mtx4GetRotation(Mtx *mtx, struct coord *dst)
{
	float norm;
	float sin_x_cos_y = (*mtx)[1][2];
	float cos_x_cos_y = (*mtx)[2][2];

	norm = sqrtf(sin_x_cos_y * sin_x_cos_y + cos_x_cos_y * cos_x_cos_y);

	if (EPSILON < norm) {
		dst->x = atan2f((*mtx)[1][2], (*mtx)[2][2]);
		dst->y = atan2f(-(*mtx)[0][2], norm);
		dst->z = atan2f((*mtx)[0][1], (*mtx)[0][0]);
	} else {
		dst->x = 0;
		dst->y = atan2f(-(*mtx)[0][2], norm);
		dst->z = atan2f(-(*mtx)[1][0], (*mtx)[1][1]);
	}
}

void mtx4LoadRotationAndTranslation(struct coord *pos, struct coord *rot, Mtx *mtx)
{
	mtx4LoadRotation(rot, mtx);
	mtx4SetTranslation(pos, mtx);
}

void mtx4LoadTranslation(struct coord *pos, Mtx *mtx)
{
	mtxIdent(mtx);
	mtx4SetTranslation(pos, (Mtx*)mtx);
}

void mtx00016710(float mult, Mtx *mtx)
{
	(*mtx)[0][2] *= mult;
	(*mtx)[1][2] *= mult;
	(*mtx)[2][2] *= mult;
	(*mtx)[3][2] *= mult;
}

/**
 * Constructs a view matrix (camera transform) using position, look direction, and up vector.
 *
 * - pos(x, y, z): The position of the camera in world space.
 * - look(x, y, z): The direction the camera is looking (not a target point).
 * - up(x, y, z): The camera's up direction.
 *
 * Output matrix transforms world coordinates into camera (view) space.
 * Equivalent to gluLookAt().
 */
void mtxBuildCameraMatrix(Mtx *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	float a;
	float b;
	float c;
	float tmp;

	tmp = -1 / sqrtf(lookx * lookx + looky * looky + lookz * lookz);
	lookx *= tmp;
	looky *= tmp;
	lookz *= tmp;

	a = upy * lookz - upz * looky;
	b = upz * lookx - upx * lookz;
	c = upx * looky - upy * lookx;

	tmp = 1 / sqrtf(a * a + b * b + c * c);
	a *= tmp;
	b *= tmp;
	c *= tmp;

	upx = looky * c - lookz * b;
	upy = lookz * a - lookx * c;
	upz = lookx * b - looky * a;

	tmp = 1 / sqrtf(upx * upx + upy * upy + upz * upz);
	upx *= tmp;
	upy *= tmp;
	upz *= tmp;

	(*mtx)[0][0] = a;
	(*mtx)[1][0] = b;
	(*mtx)[2][0] = c;
	(*mtx)[3][0] = -(posx * a + posy * b + posz * c);

	(*mtx)[0][1] = upx;
	(*mtx)[1][1] = upy;
	(*mtx)[2][1] = upz;
	(*mtx)[3][1] = -(posx * upx + posy * upy + posz * upz);

	(*mtx)[0][2] = lookx;
	(*mtx)[1][2] = looky;
	(*mtx)[2][2] = lookz;
	(*mtx)[3][2] = -(posx * lookx + posy * looky + posz * lookz);

	(*mtx)[0][3] = 0;
	(*mtx)[1][3] = 0;
	(*mtx)[2][3] = 0;
	(*mtx)[3][3] = 1;
}

void mtxBuildLookAtMatrixF(Mtx *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	mtxBuildCameraMatrix(mtx, posx, posy, posz, lookx - posx, looky - posy, lookz - posz, upx, upy, upz);
}

void mtxBuildLookAtMatrix2F(Mtx *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	float a;
	float b;
	float c;
	float tmp;

	tmp = -1 / sqrtf(lookx * lookx + looky * looky + lookz * lookz);
	lookx *= tmp;
	looky *= tmp;
	lookz *= tmp;

	a = upy * lookz - upz * looky;
	b = upz * lookx - upx * lookz;
	c = upx * looky - upy * lookx;

	tmp = 1 / sqrtf(a * a + b * b + c * c);
	a *= tmp;
	b *= tmp;
	c *= tmp;

	upx = looky * c - lookz * b;
	upy = lookz * a - lookx * c;
	upz = lookx * b - looky * a;

	tmp = 1 / sqrtf(upx * upx + upy * upy + upz * upz);
	upx *= tmp;
	upy *= tmp;
	upz *= tmp;

	(*mtx)[0][0] = a;
	(*mtx)[1][0] = upx;
	(*mtx)[2][0] = lookx;
	(*mtx)[3][0] = posx;

	(*mtx)[0][1] = b;
	(*mtx)[1][1] = upy;
	(*mtx)[2][1] = looky;
	(*mtx)[3][1] = posy;

	(*mtx)[0][2] = c;
	(*mtx)[1][2] = upz;
	(*mtx)[2][2] = lookz;
	(*mtx)[3][2] = posz;

	(*mtx)[0][3] = 0;
	(*mtx)[1][3] = 0;
	(*mtx)[2][3] = 0;
	(*mtx)[3][3] = 1;
}

void mtxBuildLookAtFromTarget(Mtx *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	mtxBuildLookAtMatrix2F(mtx, posx, posy, posz, lookx - posx, looky - posy, lookz - posz, upx, upy, upz);
}

/**
 * Builds a "look-at"-style rotation matrix that aligns to the given vector (x, y, z),
 * applying a twist around the vector by the given angle.
 * Used for aligning muzzle flashes.
 */
void mtxBuildFacingMatrix(Mtx *mtx, float angle, float x, float y, float z)
{
	float sine;
	float cosine;
	float norm;
	float invnorm;
	float cos_x;
	float sin_x;
	float cos_z;
	float sin_z;

	utilsNormalizeF(&x, &y, &z);
	sine = sinf(angle);
	cosine = cosf(angle);
	norm = sqrtf(x * x + z * z);

	if (norm != 0) {
		cos_x = x * cosine;
		sin_x = x * sine;
		cos_z = z * cosine;
		sin_z = z * sine;
		invnorm = 1 / norm;

		(*mtx)[0][0] = (-cos_z - y * sin_x) * invnorm;
		(*mtx)[1][0] = (sine * norm);
		(*mtx)[2][0] = (cos_x - y * sin_z) * invnorm;
		(*mtx)[3][0] = 0;
		(*mtx)[0][1] = (sin_z - y * cos_x) * invnorm;
		(*mtx)[1][1] = (cosine * norm);
		(*mtx)[2][1] = (-sin_x - y * cos_z) * invnorm;
		(*mtx)[3][1] = 0;
		(*mtx)[0][2] = -x;
		(*mtx)[1][2] = -y;
		(*mtx)[2][2] = -z;
		(*mtx)[3][2] = 0;

		(*mtx)[0][3] = 0;
		(*mtx)[1][3] = 0;
		(*mtx)[2][3] = 0;
		(*mtx)[3][3] = 1;
		return;
	}

	mtxIdent(mtx);
}

void mtx4Align(Mtx *mtx, float angle, float x, float y, float z)
{
	angle = RAD2DEG(angle);
	mtxAlign(mtx, angle, x, y, z);
}

void mtx4LoadRotationFrom(Mtx *src, Mtx *dst)
{
	(*dst)[0][0] = (*src)[0][0];
	(*dst)[0][1] = (*src)[1][0];
	(*dst)[0][2] = (*src)[2][0];

	(*dst)[1][0] = (*src)[0][1];
	(*dst)[1][1] = (*src)[1][1];
	(*dst)[1][2] = (*src)[2][1];

	(*dst)[2][0] = (*src)[0][2];
	(*dst)[2][1] = (*src)[1][2];
	(*dst)[2][2] = (*src)[2][2];

	(*dst)[3][0] = 0;
	(*dst)[3][1] = 0;
	(*dst)[3][2] = 0;

	(*dst)[0][3] = 0;
	(*dst)[1][3] = 0;
	(*dst)[2][3] = 0;
	(*dst)[3][3] = 1;
}

void mtxNormalizeRotationMatrix(Mtx *src, Mtx *dst)
{
	float tmp = ((*src)[0][0] * (*src)[0][0] + (*src)[1][0] * (*src)[1][0] + (*src)[2][0] * (*src)[2][0]);
	tmp = 1 / tmp;

	(*dst)[0][0] = (*src)[0][0] * tmp;
	(*dst)[0][1] = (*src)[1][0] * tmp;
	(*dst)[0][2] = (*src)[2][0] * tmp;

	(*dst)[1][0] = (*src)[0][1] * tmp;
	(*dst)[1][1] = (*src)[1][1] * tmp;
	(*dst)[1][2] = (*src)[2][1] * tmp;

	(*dst)[2][0] = (*src)[0][2] * tmp;
	(*dst)[2][1] = (*src)[1][2] * tmp;
	(*dst)[2][2] = (*src)[2][2] * tmp;

	(*dst)[3][0] = 0;
	(*dst)[3][1] = 0;
	(*dst)[3][2] = 0;

	(*dst)[0][3] = 0;
	(*dst)[1][3] = 0;
	(*dst)[2][3] = 0;
	(*dst)[3][3] = 1;
}

/*
 *   Inverts a transformation matrix that contains only rotation and translation.
 *   Assumes the rotation part is orthonormal.
 */
void mtxInvertRigidBodyMatrix(Mtx *arg0, Mtx *arg1)
{
	float tmp = (*arg0)[0][0] * (*arg0)[0][0] + (*arg0)[1][0] * (*arg0)[1][0] + (*arg0)[2][0] * (*arg0)[2][0];
	tmp = 1 / tmp;

	(*arg1)[0][0] = (*arg0)[0][0] * tmp;
	(*arg1)[0][1] = (*arg0)[1][0] * tmp;
	(*arg1)[0][2] = (*arg0)[2][0] * tmp;
	(*arg1)[1][0] = (*arg0)[0][1] * tmp;
	(*arg1)[1][1] = (*arg0)[1][1] * tmp;
	(*arg1)[1][2] = (*arg0)[2][1] * tmp;
	(*arg1)[2][0] = (*arg0)[0][2] * tmp;
	(*arg1)[2][1] = (*arg0)[1][2] * tmp;
	(*arg1)[2][2] = (*arg0)[2][2] * tmp;
	(*arg1)[3][0] = -((*arg1)[0][0] * (*arg0)[3][0] + (*arg1)[1][0] * (*arg0)[3][1] + (*arg1)[2][0] * (*arg0)[3][2]);
	(*arg1)[3][1] = -((*arg1)[0][1] * (*arg0)[3][0] + (*arg1)[1][1] * (*arg0)[3][1] + (*arg1)[2][1] * (*arg0)[3][2]);
	(*arg1)[3][2] = -((*arg1)[0][2] * (*arg0)[3][0] + (*arg1)[1][2] * (*arg0)[3][1] + (*arg1)[2][2] * (*arg0)[3][2]);
	(*arg1)[0][3] = 0;
	(*arg1)[1][3] = 0;
	(*arg1)[2][3] = 0;
	(*arg1)[3][3] = 1;
}

void mtxInvertAffine(Mtx *arg0, Mtx *arg1)
{
	float f0 = 0.0f;
	f0 += (*arg0)[0][0] * (*arg0)[1][1] * (*arg0)[2][2];
	f0 += (*arg0)[0][1] * (*arg0)[1][2] * (*arg0)[2][0];
	f0 += (*arg0)[0][2] * (*arg0)[1][0] * (*arg0)[2][1];
	f0 -= (*arg0)[0][2] * (*arg0)[1][1] * (*arg0)[2][0];
	f0 -= (*arg0)[0][1] * (*arg0)[1][0] * (*arg0)[2][2];
	f0 -= (*arg0)[0][0] * (*arg0)[1][2] * (*arg0)[2][1];
	f0 = 1.0f / f0;

	(*arg1)[0][0] = ((*arg0)[1][1] * (*arg0)[2][2] - (*arg0)[1][2] * (*arg0)[2][1]) * f0;
	(*arg1)[1][0] = ((*arg0)[1][2] * (*arg0)[2][0] - (*arg0)[1][0] * (*arg0)[2][2]) * f0;
	(*arg1)[2][0] = ((*arg0)[1][0] * (*arg0)[2][1] - (*arg0)[1][1] * (*arg0)[2][0]) * f0;
	(*arg1)[0][1] = ((*arg0)[0][2] * (*arg0)[2][1] - (*arg0)[0][1] * (*arg0)[2][2]) * f0;
	(*arg1)[1][1] = ((*arg0)[0][0] * (*arg0)[2][2] - (*arg0)[0][2] * (*arg0)[2][0]) * f0;
	(*arg1)[2][1] = ((*arg0)[0][1] * (*arg0)[2][0] - (*arg0)[0][0] * (*arg0)[2][1]) * f0;
	(*arg1)[0][2] = ((*arg0)[0][1] * (*arg0)[1][2] - (*arg0)[0][2] * (*arg0)[1][1]) * f0;
	(*arg1)[1][2] = ((*arg0)[0][2] * (*arg0)[1][0] - (*arg0)[0][0] * (*arg0)[1][2]) * f0;
	(*arg1)[2][2] = ((*arg0)[0][0] * (*arg0)[1][1] - (*arg0)[0][1] * (*arg0)[1][0]) * f0;
	(*arg1)[3][0] = -((*arg0)[3][0] * (*arg1)[0][0] + (*arg0)[3][1] * (*arg1)[1][0] + (*arg0)[3][2] * (*arg1)[2][0]);
	(*arg1)[3][1] = -((*arg0)[3][0] * (*arg1)[0][1] + (*arg0)[3][1] * (*arg1)[1][1] + (*arg0)[3][2] * (*arg1)[2][1]);
	(*arg1)[3][2] = -((*arg0)[3][0] * (*arg1)[0][2] + (*arg0)[3][1] * (*arg1)[1][2] + (*arg0)[3][2] * (*arg1)[2][2]);
	(*arg1)[0][3] = 0.0f;
	(*arg1)[1][3] = 0.0f;
	(*arg1)[2][3] = 0.0f;
	(*arg1)[3][3] = 1.0f;
}

void mtxFullInverse4x4(float arg0[4][4], float arg1[4][4])
{
	int i;
	int j;
	float tmp;

	mtxAdjugate4x4(arg0, arg1);

	tmp = 1.0f / mtxDet4x4(arg0);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			arg1[i][j] *= tmp;
		}
	}
}

void mtxAdjugate4x4(float arg0[4][4], float arg1[4][4])
{
	float mtx00, mtx10, mtx20, mtx30;
	float mtx04, mtx14, mtx24, mtx34;
	float mtx08, mtx18, mtx28, mtx38;
	float mtx0c, mtx1c, mtx2c, mtx3c;

	mtx00 = arg0[0][0]; mtx04 = arg0[0][1];
	mtx08 = arg0[0][2]; mtx0c = arg0[0][3];
	mtx10 = arg0[1][0]; mtx14 = arg0[1][1];
	mtx18 = arg0[1][2]; mtx1c = arg0[1][3];
	mtx20 = arg0[2][0]; mtx24 = arg0[2][1];
	mtx28 = arg0[2][2]; mtx2c = arg0[2][3];
	mtx30 = arg0[3][0]; mtx34 = arg0[3][1];
	mtx38 = arg0[3][2]; mtx3c = arg0[3][3];

	arg1[0][0] =  mtxDet3x3(mtx14, mtx24, mtx34, mtx18, mtx28, mtx38, mtx1c, mtx2c, mtx3c);
	arg1[1][0] = -mtxDet3x3(mtx10, mtx20, mtx30, mtx18, mtx28, mtx38, mtx1c, mtx2c, mtx3c);
	arg1[2][0] =  mtxDet3x3(mtx10, mtx20, mtx30, mtx14, mtx24, mtx34, mtx1c, mtx2c, mtx3c);
	arg1[3][0] = -mtxDet3x3(mtx10, mtx20, mtx30, mtx14, mtx24, mtx34, mtx18, mtx28, mtx38);
	arg1[0][1] = -mtxDet3x3(mtx04, mtx24, mtx34, mtx08, mtx28, mtx38, mtx0c, mtx2c, mtx3c);
	arg1[1][1] =  mtxDet3x3(mtx00, mtx20, mtx30, mtx08, mtx28, mtx38, mtx0c, mtx2c, mtx3c);
	arg1[2][1] = -mtxDet3x3(mtx00, mtx20, mtx30, mtx04, mtx24, mtx34, mtx0c, mtx2c, mtx3c);
	arg1[3][1] =  mtxDet3x3(mtx00, mtx20, mtx30, mtx04, mtx24, mtx34, mtx08, mtx28, mtx38);
	arg1[0][2] =  mtxDet3x3(mtx04, mtx14, mtx34, mtx08, mtx18, mtx38, mtx0c, mtx1c, mtx3c);
	arg1[1][2] = -mtxDet3x3(mtx00, mtx10, mtx30, mtx08, mtx18, mtx38, mtx0c, mtx1c, mtx3c);
	arg1[2][2] =  mtxDet3x3(mtx00, mtx10, mtx30, mtx04, mtx14, mtx34, mtx0c, mtx1c, mtx3c);
	arg1[3][2] = -mtxDet3x3(mtx00, mtx10, mtx30, mtx04, mtx14, mtx34, mtx08, mtx18, mtx38);
	arg1[0][3] = -mtxDet3x3(mtx04, mtx14, mtx24, mtx08, mtx18, mtx28, mtx0c, mtx1c, mtx2c);
	arg1[1][3] =  mtxDet3x3(mtx00, mtx10, mtx20, mtx08, mtx18, mtx28, mtx0c, mtx1c, mtx2c);
	arg1[2][3] = -mtxDet3x3(mtx00, mtx10, mtx20, mtx04, mtx14, mtx24, mtx0c, mtx1c, mtx2c);
	arg1[3][3] =  mtxDet3x3(mtx00, mtx10, mtx20, mtx04, mtx14, mtx24, mtx08, mtx18, mtx28);
}

float mtxDet4x4(float arg0[4][4])
{
	float tmp;
	float sp78, sp74, sp70, sp6c;
	float sp68, sp64, sp60, sp5c;
	float sp58, sp54, sp50, sp4c;
	float sp48, sp44, sp40, sp3c;
	float sp38;
	float sp34;
	float sp30;

	sp78 = arg0[0][0]; sp68 = arg0[0][1];
	sp58 = arg0[0][2]; sp48 = arg0[0][3];
	sp74 = arg0[1][0]; sp64 = arg0[1][1];
	sp54 = arg0[1][2]; sp44 = arg0[1][3];
	sp70 = arg0[2][0]; sp60 = arg0[2][1];
	sp50 = arg0[2][2]; sp40 = arg0[2][3];
	sp6c = arg0[3][0]; sp5c = arg0[3][1];
	sp4c = arg0[3][2]; sp3c = arg0[3][3];

	sp30 = mtxDet3x3(sp74, sp70, sp6c, sp64, sp60, sp5c, sp44, sp40, sp3c);
	sp34 = mtxDet3x3(sp74, sp70, sp6c, sp54, sp50, sp4c, sp44, sp40, sp3c);
	sp38 = mtxDet3x3(sp64, sp60, sp5c, sp54, sp50, sp4c, sp44, sp40, sp3c);

	tmp = mtxDet3x3(sp74, sp70, sp6c, sp64, sp60, sp5c, sp54, sp50, sp4c);

	return (sp78 * sp38 - sp68 * sp34 + sp58 * sp30) - tmp * sp48;
}

float mtxDet3x3(float arg0, float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8)
{
	float sp24;
	float sp20;
	float sp1c;

	sp1c = mtxDet2x2(arg1, arg2, arg7, arg8);
	sp20 = mtxDet2x2(arg4, arg5, arg7, arg8);
	sp24 = mtxDet2x2(arg1, arg2, arg4, arg5);

	return sp24 * arg6 + (arg0 * sp20 - arg3 * sp1c);
}

float mtxDet2x2(float arg0, float arg1, float arg2, float arg3)
{
	return arg0 * arg3 - arg1 * arg2;
}
