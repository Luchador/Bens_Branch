#include <ultra64.h>
#include <math.h>
#include <PR/gbi.h>
#include "constants.h"
#include "game/objectives.h"
#include "game/mtxutils.h"
#include "game/tex.h"
#include "game/training.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

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

void mtxPerspectiveF(float mf[4][4], float fovy, float aspect, float near, float far)
{
	float cot;
	int	i, j;

	guMtxIdentF(mf);

	fovy *= 3.1415926f / 180.0f;
	cot = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);

	mf[0][0] = cot / aspect;
	mf[1][1] = cot;
	mf[2][2] = (near + far) / (near - far);
	mf[2][3] = -1;
	mf[3][2] = (2.0f * near * far) / (near - far);
	mf[3][3] = 0;
}

void mtxPerspective(Mtx *m, float fovy, float aspect, float near, float far)
{
	float mf[4][4];

	mtxPerspectiveF(mf, fovy, aspect, near, far);

	guMtxF2L(mf, m);
}

void mtxMtxIdentF(float mf[4][4])
{
	int	i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] = i == j ? 1 : 0;
		}
	}
}

void mtxLookAtReflectF(float mf[4][4], LookAt *l,
	float xEye, float yEye, float zEye,
	float xAt,  float yAt,  float zAt,
	float xUp,  float yUp,  float zUp)
{
float len, xLook, yLook, zLook, xRight, yRight, zRight;

mtxMtxIdentF(mf);

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

mtxMtxF2L(mf, m);
}


void mtxMtxF2L(float mf[4][4], Mtx *m)
{
#ifdef GBI_FLOATS
	if ((Mtx *)mf != m) {
		bcopy(mf, m, sizeof(*m));
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