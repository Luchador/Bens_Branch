#include <ultra64.h>
#include <math.h>
#include <string.h>
#include "constants.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#define	FTOFIX32(x)	(int)((x) * (float)0x00010000)
#define	FTOFRAC8(x)	((int) MIN(((x) * (128.0f)), 127.0f) & 0xff)

void mtxF2LBulk(Mtxf *mtx, int count)
{
#ifndef GBI_FLOATS
	do {
		uint32_t m00 = (int) (mtx->m[0][0] * var8005ef10[0]);
		uint32_t m01 = (int) (mtx->m[0][1] * var8005ef10[0]);
		uint32_t m02 = (int) (mtx->m[0][2] * var8005ef10[0]);
		uint32_t m03 = (int) (mtx->m[0][3] * var8005ef10[1]);
		uint32_t m10 = (int) (mtx->m[1][0] * var8005ef10[0]);
		uint32_t m11 = (int) (mtx->m[1][1] * var8005ef10[0]);
		uint32_t m12 = (int) (mtx->m[1][2] * var8005ef10[0]);
		uint32_t m13 = (int) (mtx->m[1][3] * var8005ef10[1]);
		uint32_t m20 = (int) (mtx->m[2][0] * var8005ef10[0]);
		uint32_t m21 = (int) (mtx->m[2][1] * var8005ef10[0]);
		uint32_t m22 = (int) (mtx->m[2][2] * var8005ef10[0]);
		uint32_t m23 = (int) (mtx->m[2][3] * var8005ef10[1]);
		uint32_t m30 = (int) (mtx->m[3][0] * var8005ef10[0]);
		uint32_t m31 = (int) (mtx->m[3][1] * var8005ef10[0]);
		uint32_t m32 = (int) (mtx->m[3][2] * var8005ef10[0]);
		uint32_t m33 = (int) (mtx->m[3][3] * var8005ef10[1]);

		mtx->l[0][0] = (m00 & 0xffff0000) | m01 >> 16;
		mtx->l[0][1] = (m02 & 0xffff0000) | m03 >> 16;
		mtx->l[0][2] = (m10 & 0xffff0000) | m11 >> 16;
		mtx->l[0][3] = (m12 & 0xffff0000) | m13 >> 16;
		mtx->l[1][0] = (m20 & 0xffff0000) | m21 >> 16;
		mtx->l[1][1] = (m22 & 0xffff0000) | m23 >> 16;
		mtx->l[1][2] = (m30 & 0xffff0000) | m31 >> 16;
		mtx->l[1][3] = (m32 & 0xffff0000) | m33 >> 16;
		mtx->l[2][0] = m00 << 16 | (m01 & 0xffff);
		mtx->l[2][1] = m02 << 16 | (m03 & 0xffff);
		mtx->l[2][2] = m10 << 16 | (m11 & 0xffff);
		mtx->l[2][3] = m12 << 16 | (m13 & 0xffff);
		mtx->l[3][0] = m20 << 16 | (m21 & 0xffff);
		mtx->l[3][1] = m22 << 16 | (m23 & 0xffff);
		mtx->l[3][2] = m30 << 16 | (m31 & 0xffff);
		mtx->l[3][3] = m32 << 16 | (m33 & 0xffff);

		mtx++;

		count--;
	} while (count);
#endif
}

void mtxLoadRandomRotation(Mtxf *mtx)
{
	struct coord coord = {0, 0, 0};

	coord.x = RANDOMFRAC() * PALUPF(M_TAU) * 0.0078125f - PALUPF(0.024539785459638f);
	coord.y = RANDOMFRAC() * PALUPF(M_TAU) * 0.0078125f - PALUPF(0.024539785459638f);
	coord.z = RANDOMFRAC() * PALUPF(M_TAU) * 0.0078125f - PALUPF(0.024539785459638f);

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
		mtx00015be0(arg1, arg0);
	}
}

void mtxF2L2(float mf[4][4], Mtx *m)
{
#ifdef GBI_FLOATS
	if ((Mtx *)mf != m) {
		memcpy(m, mf, sizeof(*m));
	}
#else
	int	i, j;
	int	e1, e2;
	int	*ai, *af;

	ai = (int *) &m->m[0][0];
	af = (int *) &m->m[2][0];

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 2; j++) {
			e1 = FTOFIX32(mf[i][j * 2]);
			e2 = FTOFIX32(mf[i][j * 2 + 1]);

			*(ai++) = (e1 & 0xffff0000) | ((e2 >> 16) & 0xffff);
			*(af++) = ((e1 << 16) & 0xffff0000) | (e2 & 0xffff);
		}
	}
#endif
}

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

//--------------------------------------------//
//
//--------Alignment Matrix Functions----------//
//
//--------------------------------------------//

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

//--------------------------------------------//
//
//-----------Scale Matrix Functions-----------//
//
//--------------------------------------------//

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

//--------------------------------------------//
//
//----------Frustum Matrix Functions----------//
//
//--------------------------------------------//

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

//--------------------------------------------//
//
//--------Perspective Matrix Functions--------//
//
//--------------------------------------------//

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

void mtxPerspective(Mtx *m, float fovy, float aspect, float near, float far, float scale)
{
	float mf[4][4];

	mtxPerspectiveF(mf, fovy, aspect, near, far, scale);

	mtxF2L2(mf, m);
}

//--------------------------------------------//
//
//-----------Rotate Matrix Functions----------//
//
//--------------------------------------------//

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

//--------------------------------------------//
//
//---------Reflection Matrix Functions--------//
//
//--------------------------------------------//

void mtxLookAtReflectF(float mf[4][4], LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp)
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

void mtxLookAtReflect(Mtx *m, LookAt *l, float xEye, float yEye, float zEye, float xAt,  float yAt,  float zAt, float xUp,  float yUp,  float zUp)
{
	float mf[4][4];

	mtxLookAtReflectF(mf, l, xEye, yEye, zEye, xAt, yAt, zAt, xUp, yUp, zUp);

	mtxF2L2(mf, m);
}


