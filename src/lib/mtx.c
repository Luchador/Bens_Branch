#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/padhalllv.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

void mtx00016110(float mtx1[3][3], float mtx2[3][3])
{
	float mtx3[3][3];

	mtx00016140(mtx1, mtx2, mtx3);
	mtx3Copy(mtx3, mtx2);
}

void mtx00016140(float mtx1[3][3], float mtx2[3][3], float dst[3][3])
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

void mtx4LoadYRotationWithTranslation(struct coord *coord, float angle, Mtxf *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	mtx->m[0][0] = cos;
	mtx->m[0][1] = 0;
	mtx->m[0][2] = -sin;
	mtx->m[0][3] = 0;

	mtx->m[1][0] = 0;
	mtx->m[1][1] = 1;
	mtx->m[1][2] = 0;
	mtx->m[1][3] = 0;

	mtx->m[2][0] = sin;
	mtx->m[2][1] = 0;
	mtx->m[2][2] = cos;
	mtx->m[2][3] = 0;

	mtx->m[3][0] = coord->x;
	mtx->m[3][1] = coord->y;
	mtx->m[3][2] = coord->z;
	mtx->m[3][3] = 1;
}

void mtx4LoadXRotation(float angle, Mtxf *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	mtx->m[0][0] = 1;
	mtx->m[0][1] = 0;
	mtx->m[0][2] = 0;
	mtx->m[0][3] = 0;

	mtx->m[1][0] = 0;
	mtx->m[1][1] = cos;
	mtx->m[1][2] = sin;
	mtx->m[1][3] = 0;

	mtx->m[2][0] = 0;
	mtx->m[2][1] = -sin;
	mtx->m[2][2] = cos;
	mtx->m[2][3] = 0;

	mtx->m[3][0] = 0;
	mtx->m[3][1] = 0;
	mtx->m[3][2] = 0;
	mtx->m[3][3] = 1;
}

void mtx4LoadYRotation(float angle, Mtxf *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	mtx->m[0][0] = cos;
	mtx->m[0][1] = 0;
	mtx->m[0][2] = -sin;
	mtx->m[0][3] = 0;

	mtx->m[1][0] = 0;
	mtx->m[1][1] = 1;
	mtx->m[1][2] = 0;
	mtx->m[1][3] = 0;

	mtx->m[2][0] = sin;
	mtx->m[2][1] = 0;
	mtx->m[2][2] = cos;
	mtx->m[2][3] = 0;

	mtx->m[3][0] = 0;
	mtx->m[3][1] = 0;
	mtx->m[3][2] = 0;
	mtx->m[3][3] = 1;
}

void mtx4LoadZRotation(float angle, Mtxf *mtx)
{
	float cos = cosf(angle);
	float sin = sinf(angle);

	mtx->m[0][0] = cos;
	mtx->m[0][1] = sin;
	mtx->m[0][2] = 0;
	mtx->m[0][3] = 0;

	mtx->m[1][0] = -sin;
	mtx->m[1][1] = cos;
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

void mtx4LoadRotation(struct coord *src, Mtxf *dest)
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

	dest->m[0][0] = ycos * zcos;
	dest->m[0][1] = ycos * zsin;
	dest->m[0][2] = -ysin;
	dest->m[0][3] = 0;

	dest->m[1][0] = c * ysin - xcos * zsin;
	dest->m[1][1] = a * ysin + xcos * zcos;
	dest->m[1][2] = xsin * ycos;
	dest->m[1][3] = 0;

	dest->m[2][0] = d * ysin + xsin * zsin;
	dest->m[2][1] = b * ysin - xsin * zcos;
	dest->m[2][2] = xcos * ycos;
	dest->m[2][3] = 0;

	dest->m[3][0] = 0;
	dest->m[3][1] = 0;
	dest->m[3][2] = 0;
	dest->m[3][3] = 1;
}

#define EPSILON 0.0000019073486f

void mtx4GetRotation(float mtx[4][4], struct coord *dst)
{
	float norm;
	float sin_x_cos_y = mtx[1][2];
	float cos_x_cos_y = mtx[2][2];

	norm = sqrtf(sin_x_cos_y * sin_x_cos_y + cos_x_cos_y * cos_x_cos_y);

	if (EPSILON < norm) {
		dst->x = atan2f(mtx[1][2], mtx[2][2]);
		dst->y = atan2f(-mtx[0][2], norm);
		dst->z = atan2f(mtx[0][1], mtx[0][0]);
	} else {
		dst->x = 0;
		dst->y = atan2f(-mtx[0][2], norm);
		dst->z = atan2f(-mtx[1][0], mtx[1][1]);
	}
}

void mtx4LoadRotationAndTranslation(struct coord *pos, struct coord *rot, Mtxf *mtx)
{
	mtx4LoadRotation(rot, mtx);
	mtx4SetTranslation(pos, mtx);
}

void mtx4LoadTranslation(struct coord *pos, Mtxf *mtx)
{
	mtx4LoadIdentity(mtx);
	mtx4SetTranslation(pos, mtx);
}

void mtx00016710(float mult, float mtx[4][4])
{
	mtx[0][2] *= mult;
	mtx[1][2] *= mult;
	mtx[2][2] *= mult;
	mtx[3][2] *= mult;
}

void mtx00016748(float arg0)
{
	var8005ef10[0] = 65536 * arg0;
}

void mtx00016760(void)
{
	g_Vars.unk000510 = var8005ef10[0];
	var8005ef10[0] = 65536;
}

void mtx00016784(void)
{
	var8005ef10[0] = g_Vars.unk000510;
}

/**
 * src is passed as an Mtxf but it's read as words rather than floats.
 * It might be an Mtx rather than Mtxf.
 *
 * @TODO: Investigate
 */
void mtx00016798(Mtxf *src, Mtxf *dst)
{
	uint32_t *srcwords = (uint32_t *) src;
	float *dstfloats = (float *) dst;
	int i;

	for (i = 0; i < 8; i++) {
		uint32_t word1 = srcwords[i + 0];
		uint32_t word2 = srcwords[i + 8];

		dstfloats[(i << 1) + 0] = (int) ((word1 & 0xffff0000) | (word2 >> 16)) / var8005ef10[0];
		dstfloats[(i << 1) + 1] = (int) ((word1 << 16) | (word2 & 0xffff)) / var8005ef10[i & 1];
	}
}

void mtx00016820(Mtx *src, Mtx *dst)
{
	uint32_t *srcwords = (uint32_t *) src;
	uint32_t *dstwords = (uint32_t *) dst;
	int i;

	for (i = 0; i < 8; i++) {
		uint32_t word1 = srcwords[i + 0];
		uint32_t word2 = srcwords[i + 8];

		dstwords[(i << 1) + 0] = (word1 & 0xffff0000) | (word2 >> 16);
		dstwords[(i << 1) + 1] = (word1 << 16) | (word2 & 0xffff);
	}
}

void mtx00016874(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
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

	mtx->m[0][0] = a;
	mtx->m[1][0] = b;
	mtx->m[2][0] = c;
	mtx->m[3][0] = -(posx * a + posy * b + posz * c);

	mtx->m[0][1] = upx;
	mtx->m[1][1] = upy;
	mtx->m[2][1] = upz;
	mtx->m[3][1] = -(posx * upx + posy * upy + posz * upz);

	mtx->m[0][2] = lookx;
	mtx->m[1][2] = looky;
	mtx->m[2][2] = lookz;
	mtx->m[3][2] = -(posx * lookx + posy * looky + posz * lookz);

	mtx->m[0][3] = 0;
	mtx->m[1][3] = 0;
	mtx->m[2][3] = 0;
	mtx->m[3][3] = 1;
}

void mtx00016ae4(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	mtx00016874(mtx, posx, posy, posz, lookx - posx, looky - posy, lookz - posz, upx, upy, upz);
}

void mtx00016b58(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
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

	mtx->m[0][0] = a;
	mtx->m[1][0] = upx;
	mtx->m[2][0] = lookx;
	mtx->m[3][0] = posx;

	mtx->m[0][1] = b;
	mtx->m[1][1] = upy;
	mtx->m[2][1] = looky;
	mtx->m[3][1] = posy;

	mtx->m[0][2] = c;
	mtx->m[1][2] = upz;
	mtx->m[2][2] = lookz;
	mtx->m[3][2] = posz;

	mtx->m[0][3] = 0;
	mtx->m[1][3] = 0;
	mtx->m[2][3] = 0;
	mtx->m[3][3] = 1;
}

void mtx00016d58(Mtxf *mtx, float posx, float posy, float posz, float lookx, float looky, float lookz, float upx, float upy, float upz)
{
	mtx00016b58(mtx, posx, posy, posz, lookx - posx, looky - posy, lookz - posz, upx, upy, upz);
}

uint32_t mtx00016dcc(float arg0, float arg1)
{
	float sum = arg0 + arg1;
	uint16_t result;

	if (sum <= 2) {
		result = 0xffff;
	} else {
		result = 0x20000 / sum;

		if (result <= 0) {
			result = 1;
		}
	}

	return result;
}

void mtx00016e98(float mtx[4][4], float angle, float x, float y, float z)
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

		mtx[0][0] = (-cos_z - y * sin_x) * invnorm;
		mtx[1][0] = (sine * norm);
		mtx[2][0] = (cos_x - y * sin_z) * invnorm;
		mtx[3][0] = 0;
		mtx[0][1] = (sin_z - y * cos_x) * invnorm;
		mtx[1][1] = (cosine * norm);
		mtx[2][1] = (-sin_x - y * cos_z) * invnorm;
		mtx[3][1] = 0;
		mtx[0][2] = -x;
		mtx[1][2] = -y;
		mtx[2][2] = -z;
		mtx[3][2] = 0;

		mtx[0][3] = 0;
		mtx[1][3] = 0;
		mtx[2][3] = 0;
		mtx[3][3] = 1;
		return;
	}

	mtx4LoadIdentity((Mtxf *)mtx);
}

void mtx4Align(float mtx[4][4], float angle, float x, float y, float z)
{
	angle = RAD2DEG(angle);
	mtxAlignF(mtx, angle, x, y, z);
}

void mtx4LoadRotationFrom(float src[4][4], float dst[4][4])
{
	dst[0][0] = src[0][0];
	dst[0][1] = src[1][0];
	dst[0][2] = src[2][0];

	dst[1][0] = src[0][1];
	dst[1][1] = src[1][1];
	dst[1][2] = src[2][1];

	dst[2][0] = src[0][2];
	dst[2][1] = src[1][2];
	dst[2][2] = src[2][2];

	dst[3][0] = 0;
	dst[3][1] = 0;
	dst[3][2] = 0;

	dst[0][3] = 0;
	dst[1][3] = 0;
	dst[2][3] = 0;
	dst[3][3] = 1;
}

void mtx000170e4(float src[4][4], float dst[4][4])
{
	float tmp = (src[0][0] * src[0][0] + src[1][0] * src[1][0] + src[2][0] * src[2][0]);
	tmp = 1 / tmp;

	dst[0][0] = src[0][0] * tmp;
	dst[0][1] = src[1][0] * tmp;
	dst[0][2] = src[2][0] * tmp;

	dst[1][0] = src[0][1] * tmp;
	dst[1][1] = src[1][1] * tmp;
	dst[1][2] = src[2][1] * tmp;

	dst[2][0] = src[0][2] * tmp;
	dst[2][1] = src[1][2] * tmp;
	dst[2][2] = src[2][2] * tmp;

	dst[3][0] = 0;
	dst[3][1] = 0;
	dst[3][2] = 0;

	dst[0][3] = 0;
	dst[1][3] = 0;
	dst[2][3] = 0;
	dst[3][3] = 1;
}

void mtx0001719c(float arg0[4][4], float arg1[4][4])
{
	float tmp = arg0[0][0] * arg0[0][0] + arg0[1][0] * arg0[1][0] + arg0[2][0] * arg0[2][0];
	tmp = 1 / tmp;

	arg1[0][0] = arg0[0][0] * tmp;
	arg1[0][1] = arg0[1][0] * tmp;
	arg1[0][2] = arg0[2][0] * tmp;
	arg1[1][0] = arg0[0][1] * tmp;
	arg1[1][1] = arg0[1][1] * tmp;
	arg1[1][2] = arg0[2][1] * tmp;
	arg1[2][0] = arg0[0][2] * tmp;
	arg1[2][1] = arg0[1][2] * tmp;
	arg1[2][2] = arg0[2][2] * tmp;
	arg1[3][0] = -(arg1[0][0] * arg0[3][0] + arg1[1][0] * arg0[3][1] + arg1[2][0] * arg0[3][2]);
	arg1[3][1] = -(arg1[0][1] * arg0[3][0] + arg1[1][1] * arg0[3][1] + arg1[2][1] * arg0[3][2]);
	arg1[3][2] = -(arg1[0][2] * arg0[3][0] + arg1[1][2] * arg0[3][1] + arg1[2][2] * arg0[3][2]);
	arg1[0][3] = 0;
	arg1[1][3] = 0;
	arg1[2][3] = 0;
	arg1[3][3] = 1;
}

void mtx000172f0(float arg0[4][4], float arg1[4][4])
{
	float f0 = 0.0f;
	f0 += arg0[0][0] * arg0[1][1] * arg0[2][2];
	f0 += arg0[0][1] * arg0[1][2] * arg0[2][0];
	f0 += arg0[0][2] * arg0[1][0] * arg0[2][1];
	f0 -= arg0[0][2] * arg0[1][1] * arg0[2][0];
	f0 -= arg0[0][1] * arg0[1][0] * arg0[2][2];
	f0 -= arg0[0][0] * arg0[1][2] * arg0[2][1];
	f0 = 1.0f / f0;

	arg1[0][0] = (arg0[1][1] * arg0[2][2] - arg0[1][2] * arg0[2][1]) * f0;
	arg1[1][0] = (arg0[1][2] * arg0[2][0] - arg0[1][0] * arg0[2][2]) * f0;
	arg1[2][0] = (arg0[1][0] * arg0[2][1] - arg0[1][1] * arg0[2][0]) * f0;
	arg1[0][1] = (arg0[0][2] * arg0[2][1] - arg0[0][1] * arg0[2][2]) * f0;
	arg1[1][1] = (arg0[0][0] * arg0[2][2] - arg0[0][2] * arg0[2][0]) * f0;
	arg1[2][1] = (arg0[0][1] * arg0[2][0] - arg0[0][0] * arg0[2][1]) * f0;
	arg1[0][2] = (arg0[0][1] * arg0[1][2] - arg0[0][2] * arg0[1][1]) * f0;
	arg1[1][2] = (arg0[0][2] * arg0[1][0] - arg0[0][0] * arg0[1][2]) * f0;
	arg1[2][2] = (arg0[0][0] * arg0[1][1] - arg0[0][1] * arg0[1][0]) * f0;
	arg1[3][0] = -(arg0[3][0] * arg1[0][0] + arg0[3][1] * arg1[1][0] + arg0[3][2] * arg1[2][0]);
	arg1[3][1] = -(arg0[3][0] * arg1[0][1] + arg0[3][1] * arg1[1][1] + arg0[3][2] * arg1[2][1]);
	arg1[3][2] = -(arg0[3][0] * arg1[0][2] + arg0[3][1] * arg1[1][2] + arg0[3][2] * arg1[2][2]);
	arg1[0][3] = 0.0f;
	arg1[1][3] = 0.0f;
	arg1[2][3] = 0.0f;
	arg1[3][3] = 1.0f;
}

void mtx00017588(float arg0[4][4], float arg1[4][4])
{
	int i;
	int j;
	float tmp;

	mtx00017614(arg0, arg1);

	tmp = 1.0f / mtx00017a78(arg0);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			arg1[i][j] *= tmp;
		}
	}
}

void mtx00017614(float arg0[4][4], float arg1[4][4])
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

	arg1[0][0] =  mtx00017c2c(mtx14, mtx24, mtx34, mtx18, mtx28, mtx38, mtx1c, mtx2c, mtx3c);
	arg1[1][0] = -mtx00017c2c(mtx10, mtx20, mtx30, mtx18, mtx28, mtx38, mtx1c, mtx2c, mtx3c);
	arg1[2][0] =  mtx00017c2c(mtx10, mtx20, mtx30, mtx14, mtx24, mtx34, mtx1c, mtx2c, mtx3c);
	arg1[3][0] = -mtx00017c2c(mtx10, mtx20, mtx30, mtx14, mtx24, mtx34, mtx18, mtx28, mtx38);
	arg1[0][1] = -mtx00017c2c(mtx04, mtx24, mtx34, mtx08, mtx28, mtx38, mtx0c, mtx2c, mtx3c);
	arg1[1][1] =  mtx00017c2c(mtx00, mtx20, mtx30, mtx08, mtx28, mtx38, mtx0c, mtx2c, mtx3c);
	arg1[2][1] = -mtx00017c2c(mtx00, mtx20, mtx30, mtx04, mtx24, mtx34, mtx0c, mtx2c, mtx3c);
	arg1[3][1] =  mtx00017c2c(mtx00, mtx20, mtx30, mtx04, mtx24, mtx34, mtx08, mtx28, mtx38);
	arg1[0][2] =  mtx00017c2c(mtx04, mtx14, mtx34, mtx08, mtx18, mtx38, mtx0c, mtx1c, mtx3c);
	arg1[1][2] = -mtx00017c2c(mtx00, mtx10, mtx30, mtx08, mtx18, mtx38, mtx0c, mtx1c, mtx3c);
	arg1[2][2] =  mtx00017c2c(mtx00, mtx10, mtx30, mtx04, mtx14, mtx34, mtx0c, mtx1c, mtx3c);
	arg1[3][2] = -mtx00017c2c(mtx00, mtx10, mtx30, mtx04, mtx14, mtx34, mtx08, mtx18, mtx38);
	arg1[0][3] = -mtx00017c2c(mtx04, mtx14, mtx24, mtx08, mtx18, mtx28, mtx0c, mtx1c, mtx2c);
	arg1[1][3] =  mtx00017c2c(mtx00, mtx10, mtx20, mtx08, mtx18, mtx28, mtx0c, mtx1c, mtx2c);
	arg1[2][3] = -mtx00017c2c(mtx00, mtx10, mtx20, mtx04, mtx14, mtx24, mtx0c, mtx1c, mtx2c);
	arg1[3][3] =  mtx00017c2c(mtx00, mtx10, mtx20, mtx04, mtx14, mtx24, mtx08, mtx18, mtx28);
}

float mtx00017a78(float arg0[4][4])
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

	sp30 = mtx00017c2c(sp74, sp70, sp6c, sp64, sp60, sp5c, sp44, sp40, sp3c);
	sp34 = mtx00017c2c(sp74, sp70, sp6c, sp54, sp50, sp4c, sp44, sp40, sp3c);
	sp38 = mtx00017c2c(sp64, sp60, sp5c, sp54, sp50, sp4c, sp44, sp40, sp3c);

	tmp = mtx00017c2c(sp74, sp70, sp6c, sp64, sp60, sp5c, sp54, sp50, sp4c);

	return (sp78 * sp38 - sp68 * sp34 + sp58 * sp30) - tmp * sp48;
}

float mtx00017c2c(float arg0, float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8)
{
	float sp24;
	float sp20;
	float sp1c;

	sp1c = mtx00017cbc(arg1, arg2, arg7, arg8);
	sp20 = mtx00017cbc(arg4, arg5, arg7, arg8);
	sp24 = mtx00017cbc(arg1, arg2, arg4, arg5);

	return sp24 * arg6 + (arg0 * sp20 - arg3 * sp1c);
}

float mtx00017cbc(float arg0, float arg1, float arg2, float arg3)
{
	return arg0 * arg3 - arg1 * arg2;
}
