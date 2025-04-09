#include <ultra64.h>
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
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define	FTOFIX32(x)	(int)((x) * (float)0x00010000)
#define	FTOFRAC8(x)	((int) MIN(((x) * (128.0f)), 127.0f) & 0xff)

void mtxLoadRandomRotation(Mtxf *mtx)
{
	struct coord coord = {0, 0, 0};

	coord.x = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;
	coord.y = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;
	coord.z = RANDOMFRAC() * M_TAU * 0.0078125f - 0.024539785459638f;

	mtx4LoadRotation(&coord, mtx);
}

void mtxRandomToss(struct coord *coord, Mtxf *mtx)
{
	coord->x = RANDOMFRAC() * 1.6666666269302f * 4.0f - 3.3333332538605f;
	coord->y = RANDOMFRAC() * 1.6666666269302f * 4.0f;
	coord->z = RANDOMFRAC() * 1.6666666269302f * 4.0f - 3.3333332538605f;

	mtxLoadRandomRotation(mtx);
}

void func0f0965e4(float *arg0, float *arg1, float arg2)
{
	float tmp = arg1[0] - arg2 * 0.27777779f;
	arg0[0] += arg2 * (arg1[0] + tmp) * 0.5f;
	arg1[0] = tmp;
}

// Used for spinning falling objects such as grenades and dropped guns
void mtxApplyRotation(Mtxf *arg0, Mtxf *arg1, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		mtxApplyAffineTransformInPlace(arg1, arg0);
	}
}

void mtxF2L(Mtxf *src, Mtxf *dst)
{
	if (src != dst) {
		memcpy(dst, src, sizeof(*dst));
	}
}

void mtxAlignF(float mf[4][4], float a, float x, float y, float z)
{
	static float dtor = 3.1415926f / 180.0f;
	float s, c, h, hinv;

	utilsNormalizeF(&x, &y, &z);

	a *= dtor;
	s = sinf(a);
	c = cosf(a);
	h = sqrtf(x * x + z * z);

	mtxIdentF(mf);

	if (h != 0) {
		hinv = 1 / h;

		mf[0][0] = (-z*c - s*y*x) * hinv;
		mf[1][0] = (z*s - c*y*x) * hinv;
		mf[2][0] = -x;
		mf[3][0] = 0;

		mf[0][1] = s*h;
		mf[1][1] = c*h;
		mf[2][1] = -y;
		mf[3][1] = 0;

		mf[0][2] = (c*x - s*y*z) * hinv;
		mf[1][2] = (-s*x - c*y*z) * hinv;
		mf[2][2] = -z;
		mf[3][2] = 0;

		mf[0][3] = 0;
		mf[1][3] = 0;
		mf[2][3] = 0;
		mf[3][3] = 1;
	}
}

void mtxAlign(Mtx *m, float a, float x, float y, float z)
{
	float mf[4][4];

	mtxAlignF(mf, a, x, y, z);

	mtxF2L2(mf, m);
}


void mtxF2L2(float mf[4][4], Mtx *m)
{
	if ((Mtx *)mf != m) {
		memcpy(m, mf, sizeof(*m));
	}
}

/*
* Identity Matrix Functions
*/

void mtxIdentF(float mf[4][4])
{
	int	i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] = i == j ? 1 : 0;
		}
	}
}

void mtxIdent(Mtx *m)
{
	float mf[4][4];

	mtxIdentF(mf);

	mtxF2L2(mf, m);
}

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

/*
 * Multiplies two 4x4 matrices and stores the result in the destination matrix.
 *
 * This performs a standard matrix multiplication: dst = mtx2 * mtx1.
 * The operation is column-major, which is typical for graphics applications.
 * This is used in transformations like combining translation, rotation, and scale.
 */
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

/*
* Scaling Matrix Functions
*/

void mtxScaleF(float mf[4][4], float x, float y, float z)
{
	mtxIdentF(mf);

	mf[0][0] = x;
	mf[1][1] = y;
	mf[2][2] = z;
	mf[3][3] = 1;
}

void mtxScale(Mtx *m, float x, float y, float z)
{
	float mf[4][4];

	mtxScaleF(mf, x, y, z);

	mtxF2L2(mf, m);
}

void mtxScaleRow0Full(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;
	mtx->m[0][3] *= mult;
}

void mtxScaleRow0Vec(float mult, Mtxf *mtx)
{
	mtx->m[0][0] *= mult;
	mtx->m[0][1] *= mult;
	mtx->m[0][2] *= mult;
}

void mtxScaleRow1Full(float mult, Mtxf *mtx)
{
	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;
	mtx->m[1][3] *= mult;
}

void mtxScaleRow1Vec(float mult, Mtxf *mtx)
{
	mtx->m[1][0] *= mult;
	mtx->m[1][1] *= mult;
	mtx->m[1][2] *= mult;
}

void mtxScaleRow2Full(float mult, Mtxf *mtx)
{
	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
	mtx->m[2][3] *= mult;
}

void mtxScaleRow2Vec(float mult, Mtxf *mtx)
{
	mtx->m[2][0] *= mult;
	mtx->m[2][1] *= mult;
	mtx->m[2][2] *= mult;
}

void mtxScaleRotationPart(float mult, Mtxf *mtx)
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

/*
* Rotation Matrix Functions
*/

void mtxRotateF(float mf[4][4], float a, float x, float y, float z)
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

	mtxIdentF(mf);

	t = x * x;
	mf[0][0] = t + cosine * (1 - t);
	mf[2][1] = bc - x * sine;
	mf[1][2] = bc + x * sine;

	t = y * y;
	mf[1][1] = t + cosine * (1 - t);
	mf[2][0] = ca + y * sine;
	mf[0][2] = ca - y * sine;

	t = z * z;
	mf[2][2] = t + cosine * (1 - t);
	mf[1][0] = ab - z * sine;
	mf[0][1] = ab + z * sine;
}

void mtxRotate(Mtx *m, float a, float x, float y, float z)
{
	float mf[4][4];

	mtxRotateF(mf, a, x, y, z);

	mtxF2L2(mf, m);
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

/*
* Translation Matrix Functions
*/

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

void mtxApplyAffineTransformInPlace(Mtxf *matrix1, Mtxf *matrix2)
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

void mtx4SetTranslation(struct coord *pos, Mtxf *mtx)
{
	mtx->m[3][0] = pos->x;
	mtx->m[3][1] = pos->y;
	mtx->m[3][2] = pos->z;
}

/*
* Frustum Matrix Functions
*/

void mtxFrustumF(float mf[4][4], float l, float r, float b, float t, float n, float f, float scale)
{
	int i, j;

	mtxIdentF(mf);

	mf[0][0] = 2 * n / (r - l);
	mf[1][1] = 2 * n / (t - b);
	mf[2][0] = (r + l) / (r - l);
	mf[2][1] = (t + b) / (t - b);
	mf[2][2] = -(f + n) / (f - n);
	mf[2][3] = -1;
	mf[3][2] = -2 * f * n / (f - n);
	mf[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] *= scale;
		}
	}
}

void mtxFrustum(Mtx *m, float l, float r, float b, float t, float n, float f, float scale)
{
	float mf[4][4];

	mtxFrustumF(mf, l, r, b, t, n, f, scale);

	mtxF2L2(mf, m);
}

/*
*  Perspective Matrix Functions
*/

void mtxPerspectiveF(float mf[4][4], float fovy, float aspect, float near, float far, float scale)
{
	float cot;
	int	i, j;

	mtxIdentF(mf);

	fovy *= 3.1415926f / 180.0f;
	cot = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);

	mf[0][0] = cot / aspect;
	mf[1][1] = cot;
	mf[2][2] = (near + far) / (near - far);
	mf[2][3] = -1;
	mf[3][2] = (2.0f * near * far) / (near - far);
	mf[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] *= scale;
		}
	}
}

void mtxPerspective(Mtx *m, uint16_t *perspNorm, float fovy, float aspect, float near, float far, float scale)
{
	float mf[4][4];

	mtxPerspectiveF(mf, fovy, aspect, near, far, scale);

	mtxF2L2(mf, m);
}

/*
* Reflection Matrix Functions
*/

void mtxLookAtReflectF(float mf[4][4], LookAt *l,
	float xEye, float yEye, float zEye,
	float xAt,  float yAt,  float zAt,
	float xUp,  float yUp,  float zUp)
{
float len, xLook, yLook, zLook, xRight, yRight, zRight;

mtxIdentF(mf);

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

mf[0][0] = xRight;
mf[1][0] = yRight;
mf[2][0] = zRight;
mf[3][0] = -(xEye * xRight + yEye * yRight + zEye * zRight);

mf[0][1] = xUp;
mf[1][1] = yUp;
mf[2][1] = zUp;
mf[3][1] = -(xEye * xUp + yEye * yUp + zEye * zUp);

mf[0][2] = xLook;
mf[1][2] = yLook;
mf[2][2] = zLook;
mf[3][2] = -(xEye * xLook + yEye * yLook + zEye * zLook);

mf[0][3] = 0;
mf[1][3] = 0;
mf[2][3] = 0;
mf[3][3] = 1;
}

void mtxLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye,
	float xAt,  float yAt,  float zAt,
	float xUp,  float yUp,  float zUp)
{
float mf[4][4];

mtxLookAtReflectF(mf, l, xEye, yEye, zEye, xAt, yAt, zAt,
		xUp, yUp, zUp);

mtxF2L2(mf, m);
}

// Multiplies all the 3D rotational and translational components (but not the fourth column)
void mtxScale3x4(float mult, Mtxf *mtx)
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

void mtx4Copy(Mtxf *src, Mtxf *dst)
{
	*dst = *src;
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