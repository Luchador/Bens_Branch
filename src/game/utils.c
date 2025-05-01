#include <ultra64.h>
#include <math.h>
#include <stdlib.h>
#include "constants.h"
#include "game/debug.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "bss.h"
#include "types.h"
#include "system.h"
#include "video.h"

float g_AlmostZero = 0.00001f;

#define COUNTER_NUM (46875ULL)
#define COUNTER_DEN (1000ULL)

uint64_t utilsGetCount(void)
{
	return (sysGetMicroseconds() * COUNTER_NUM) / COUNTER_DEN;
}

int utilsClamp(int value, int min, int max) 
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

float utilsClampF(float value, float min, float max) 
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

// Converts a hardcoded N64 X coordinate to scaled PC screen X
int utilsScaleX(int x)
{
    return (int)(x * (videoGetWidth() / SCREEN_320));
}

// Converts a hardcoded N64 Y coordinate to scaled PC screen Y
int utilsScaleY(int y)
{
    return (int)(y * (videoGetHeight() / SCREEN_240));
}

int utilsScaleW(int w)
{
    return (int)(w * (videoGetWidth() / SCREEN_320));
}

int utilsScaleH(int h)
{
    return (int)(h * (videoGetHeight() / SCREEN_240));
}

uint32_t align4(uint32_t arg0)
{
	if (arg0 & 3) {
		arg0 = (arg0 & 0xfffffffc) + 4;
	}

	return arg0;
}

uint32_t align16(uint32_t arg0)
{
	if (arg0 & 0xf) {
		arg0 = (arg0 & 0xfffffff0) + 0x10;
	}

	return arg0;
}

uintptr_t align32(uintptr_t arg0)
{
	if (arg0 & 0x1f) {
		arg0 = (arg0 & ((intptr_t)~0x1f)) + 0x20;
	}

	return arg0;
}

bool utilsNormalizeVec(struct coord *invec, struct coord *normalizedvec)
{
	float sqdist = invec->x * invec->x + invec->y * invec->y + invec->z * invec->z;
	float mult;

	if (sqdist < g_AlmostZero) { // Set invec to standard right vector (0, 0, 1)
		invec->x = 0;
		invec->y = 0;
		invec->z = 1;

		return false;
	}

	mult = 1.0f / sqrtf(sqdist);

	normalizedvec->x = invec->x * mult;
	normalizedvec->y = invec->y * mult;
	normalizedvec->z = invec->z * mult;

	return true;
}

void utilsNormalizeF(float *x, float *y, float *z)
{
	float hyp = sqrtf(*x * *x + *y * *y + *z * *z);

	if (hyp > 0.0f) {
		float hyp2 = 1.0f / hyp;
		*x = *x * hyp2;
		*y = *y * hyp2;
		*z = *z * hyp2;
	} else {
		*x = 0.0f;
		*y = 0.0f;
		*z = 1.0f;
	}
}

void utilsRenderScreenTexture(Gfx **gdlptr, float *screenpos, float *screensize, int width, int height, bool flipU, bool flipV, bool arg8)
{
	if (screensize[0] > 0.0f && screensize[1] > 0.0f) {
		Gfx *gdl = *gdlptr;
		int xl;
		int yl;
		int xh;
		int yh;
		int s = 0;
		int t = 0;
		int dsdx;
		int dtdy;
		int widthx4;
		int heightx4;
		int sp20 = 0;
		int sp1c = 0;

		gfx_Set_Texture_Persp(gdl++, G_TP_NONE);

		xl = (screenpos[0] - screensize[0]) * 4.0f;
		yl = (screenpos[1] - screensize[1]) * 4.0f;
		xh = (screenpos[0] + screensize[0]) * 4.0f;
		yh = (screenpos[1] + screensize[1]) * 4.0f;

		if (xh >= 0 && yh >= 0) {
			if (arg8) {
				width *= 2;
				height *= 2;
				s = -(width * 16);
				t = -(height * 16);
			}

			if (xl < 0) {
				s += ((-xl * width) * 32) / (xh - xl);
				xl = 0;
			}

			if (yl < 0) {
				t += ((-yl * height) * 32) / (yh - yl);
				yl = 0;
			}

			widthx4 = viGetWidth() * 4;
			heightx4 = viGetHeight() * 4;

			if (widthx4 < xh) {
				xh = widthx4;
			}

			if (heightx4 < yh) {
				yh = heightx4;
			}

			dsdx = width / (2.0f * screensize[0]) * 1024.0f;
			dtdy = height / (2.0f * screensize[1]) * 1024.0f;

			if (flipU) {
				dsdx = 0x10000 - dsdx;

				if (arg8) {
					s = (((width >> 1) - 1) * 32) - s;
				} else {
					s = ((width - 1) * 32) - s;
				}
			}

			if (flipV) {
				dtdy = 0x10000 - dtdy;

				if (arg8) {
					t = (((height >> 1) - 1) * 32) - t;
				} else {
					t = ((height - 1) * 32) - t;
				}
			}

			gdl += gfx_Texture_Rectangle(gdl, xl, yl, xh, yh, 0, s, t, dsdx, dtdy);
		}

		gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

		*gdlptr = gdl;
	}
}

bool isPointInBBox(struct coord *point, struct coord *bbox)
{
	if (point->x < bbox->x && -bbox->x < point->x
			&& point->y < bbox->y && -bbox->y < point->y
			&& point->z < bbox->z && -bbox->z < point->z) {
		return true;
	}
	return false;
}

float coordsGetDistance(struct coord *a, struct coord *b)
{
	float xdiff = b->x - a->x;
	float ydiff = b->y - a->y;
	float zdiff = b->z - a->z;

	return sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
}

// RLE-ish compression algorithm
int utilCompressRoomData(uint8_t *input, int numEntries, uint8_t *output, int entrySize) {
    int outputIndex = 0;
    int zeroRunLength = 0;

    for (int i = 0; i < numEntries; i++) {
        int index = i * entrySize;
        uint8_t firstByte = input[index];

        if (firstByte != 0) {
            if (i != 0 && input[index - entrySize] == 0) {
                // End of a zero run
                output[outputIndex++] = 0;

                if (zeroRunLength == 255) {
                    output[outputIndex++] = 200;
                    output[outputIndex++] = 0;
                    zeroRunLength -= 200;
                }

                while (zeroRunLength > 255) {
                    output[outputIndex++] = 255;
                    zeroRunLength -= 255;
                }

                output[outputIndex++] = zeroRunLength;
                zeroRunLength = 0;
            }

            output[outputIndex++] = firstByte;
        } else {
            zeroRunLength++;
        }
    }

    // Terminator
    output[outputIndex++] = 0;
    output[outputIndex++] = 0;

    return outputIndex;
}

int utilsDecompressRoomData(uint8_t *arg0, int *arg1, int *roomnum)
{
	int result;

	if (*arg1 == 0) {
		*roomnum = -1;
	}

	while (arg0[*arg1] == 0) {
		*arg1 += 1;

		if (arg0[*arg1]) {
			while (arg0[*arg1] == 255) {
				*roomnum += 255;
				*arg1 += 1;
			}

			*roomnum += arg0[*arg1];
			*arg1 += 1;
		} else {
			return -1;
		}
	}

	*roomnum += 1;

	result = arg0[*arg1];

	*arg1 += 1;

	return result;
}

void utilsInterpTwoPoints(struct coord *arg0, struct coord *arg1, float standfrac, struct coord *vel)
{
	vel->x = (arg1->x - arg0->x) * standfrac + arg0->x;
	vel->y = (arg1->y - arg0->y) * standfrac + arg0->y;
	vel->z = (arg1->z - arg0->z) * standfrac + arg0->z;
}

void utilsCatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, float arg4, struct coord *arg5)
{
	float mult0;
	float mult1;
	float mult2;
	float mult3;

	float squared = arg4 * arg4;
	float cubed = arg4 * arg4 * arg4;

	mult0 = squared - 0.5f * (arg4 + cubed);
	mult1 = 1.5f * cubed - 2.5f * squared + 1.0f;
	mult2 = -1.5f * cubed + 2.0f * squared + 0.5f * arg4;
	mult3 = 0.5f * (cubed - squared);

	arg5->x = mult0 * arg0->f[0] + mult1 * arg1->f[0] + mult2 * arg2->f[0] + mult3 * arg3->f[0];
	arg5->y = mult0 * arg0->f[1] + mult1 * arg1->f[1] + mult2 * arg2->f[1] + mult3 * arg3->f[1];
	arg5->z = mult0 * arg0->f[2] + mult1 * arg1->f[2] + mult2 * arg2->f[2] + mult3 * arg3->f[2];
}

bool utilsIsPointInCone(struct coord *arg0, struct coord *arg1, struct coord *arg2, float arg3)
{
	struct coord sp0c; // vector from arg0 to arg2
	float value;

	sp0c.x = arg2->x - arg0->x;
	sp0c.y = arg2->y - arg0->y;
	sp0c.z = arg2->z - arg0->z;

	value = arg1->f[0] * sp0c.f[0] + arg1->f[1] * sp0c.f[1] + arg1->f[2] * sp0c.f[2]; // dot product of arg1 and sp0c

	if (value > 0) { // sp0c points in the same general direction as arg1
		float a = arg1->f[0] * arg1->f[0] + arg1->f[1] * arg1->f[1] + arg1->f[2] * arg1->f[2];
		float b = sp0c.f[0] * sp0c.f[0] + sp0c.f[1] * sp0c.f[1] + sp0c.f[2] * sp0c.f[2];

		if ((b - arg3 * arg3) * a <= value * value) {
			return true;
		}
	}

	return false;
}

bool utilsSphereIntersectsOrientedBbox(struct coord *sphereCenter, float radius, struct modelrodata_bbox *bbox, Mtx *mtx)
{
	Mtx sp58;
	struct coord sp4c;
	struct coord sp40;
	struct coord sp34;
	struct coord sp28;

	sp34.x = sp34.y = sp34.z = radius;

	sp4c.x = sphereCenter->x - (*mtx)[3][0];
	sp4c.y = sphereCenter->y - (*mtx)[3][1];
	sp4c.z = sphereCenter->z - (*mtx)[3][2];

	mtxNormalizeRotationMatrix(mtx, &sp58);
	mtx4RotateVec(&sp58, &sp4c, &sp40);
	mtx4RotateVec(&sp58, &sp34, &sp28);

	if (sp28.x < 0.0f) {
		sp28.x = -sp28.x;
	}

	if (sp28.y < 0.0f) {
		sp28.y = -sp28.y;
	}

	if (sp28.z < 0.0f) {
		sp28.z = -sp28.z;
	}

	return sp40.x - sp28.x <= bbox->xmax && sp28.x + sp40.x >= bbox->xmin
		&& sp40.y - sp28.y <= bbox->ymax && sp28.y + sp40.y >= bbox->ymin
		&& sp40.z - sp28.z <= bbox->zmax && sp28.z + sp40.z >= bbox->zmin;
}

// Möller–Trumbore algorithm
static bool utilsTriRayIntersectionTest(
	float f0, float f1, float f2,
	float f12, float f13, float f14,
	float f15, float f16, float f17,
	struct coord *arg3, struct coord *t0, struct coord *t1,
	struct coord *t2, struct coord *t3, struct coord *t4)
{
float f3;
float f4;
float f5;
float f6;
float f7;
float f8;
float f9;
float f10;
float f11;
float f18;
float f19;
float f20;
float f21;
float f22;
float f23;
float f24;
float f25;
float f26;
float f27;
float f28;

f3 = f12 - f0;
f4 = f13 - f1;
f5 = f14 - f2;
f6 = f15 - f12;
f7 = f16 - f13;
f8 = f17 - f14;
f9 = f15 - f0;
f10 = f16 - f1;
f11 = f17 - f2;

if (arg3 != NULL) {
	f0 += arg3->x;
	f1 += arg3->y;
	f2 += arg3->z;
}

f12 = f4 * f8;
f13 = f7 * f5;
f14 = f5 * f6;
f12 = f12 - f13;
f15 = f8 * f3;
f16 = f3 * f7;
f13 = f14 - f15;
f14 = f6 * f4;
f14 = f16 - f14;
f15 = f12 * f0;
f16 = f13 * f1;
f17 = f14 * f2;
f15 = f15 + f16;
f15 = f15 + f17;

f16 = t2->x;
f17 = t2->y;
f18 = t2->z;

f19 = f12 * f16;
f20 = f13 * f17;
f19 = f19 + f20;
f20 = f14 * f18;
f19 = f19 + f20;

if (f19 == 0.0f) {
	return false;
}

f20 = t1->x;
f21 = t1->y;
f22 = t1->z;
f23 = f12 * f20;
f24 = f13 * f21;
f23 = f15 - f23;
f25 = f14 * f22;
f23 = f23 - f24;
f23 = f23 - f25;
f19 = f23 / f19;
f23 = f19 * f16;
f24 = f19 * f17;
f23 = f20 + f23;
f25 = f19 * f18;
f24 = f21 + f24;
f25 = f22 + f25;
f26 = f23 - f20;
f26 = f16 * f26;
f27 = f24 - f21;
f27 = f17 * f27;
f28 = f25 - f22;
f26 = f26 + f27;
f27 = f18 * f28;
f26 = f26 + f27;

if (f26 > 0) {
	return false;
}

f20 = t0->x;
f21 = t0->y;
f22 = t0->z;
f26 = f23 - f20;
f26 = f16 * f26;
f27 = f24 - f21;
f27 = f17 * f27;
f28 = f25 - f22;
f26 = f26 + f27;
f27 = f18 * f28;
f26 = f26 + f27;

if (f26 < 0) {
	return false;
}

f0 = f23 - f0;
f1 = f24 - f1;
f2 = f25 - f2;
f26 = f6 * f4;
f27 = f3 * f7;
f26 = f26 - f27;

if (f26 != 0.0f) {
	f27 = f0 * f4;
	f28 = f1 * f3;
} else {
	f26 = f7 * f5;
	f27 = f4 * f8;
	f26 = f26 - f27;

	if (f26 != 0.0f) {
		f27 = f1 * f5;
		f28 = f2 * f4;
	} else {
		f26 = f8 * f3;
		f27 = f5 * f6;
		f26 = f26 - f27;
		f27 = f2 * f3;
		f28 = f0 * f5;
	}
}

f27 = f27 - f28;
f27 = f27 / f26;

if (f27 < 0.0f) {
	return false;
}

if (f3 != 0.0f) {
	f28 = f27 * f9;
	f28 = f0 - f28;
	f28 = f28 / f3;
} else if (f4 != 0.0f) {
	f28 = f27 * f10;
	f28 = f1 - f28;
	f28 = f28 / f4;
} else {
	f28 = f27 * f11;
	f28 = f2 - f28;
	f28 = f28 / f5;
}

if (f28 >= 0.0f && f28 + f27 <= 1.0f) {
	if (t3 != NULL) {
		t3->x = f23;
		t3->y = f24;
		t3->z = f25;
	}

	if (t4 != NULL) {
		t4->x = f12;
		t4->y = f13;
		t4->z = f14;
	}

	return true;
}

return false;
}

bool utilsIntersectTest1(struct vec3s16 *arg0, struct vec3s16 *arg1, struct vec3s16 *arg2,
	struct coord *arg3, struct coord *t0, struct coord *t1,
	struct coord *t2, struct coord *t3, struct coord *t4)
{
// Casting arg0/arg1/arg2 properties from s16 to float
return utilsTriRayIntersectionTest(
		arg0->x, arg0->y, arg0->z,
		arg1->x, arg1->y, arg1->z,
		arg2->x, arg2->y, arg2->z,
		arg3, t0, t1, t2, t3, t4);
}

bool utilsIntersectTest2(struct coord *arg0, struct coord *arg1, struct coord *arg2,
	struct coord *arg3, struct coord *t0, struct coord *t1,
	struct coord *t2, struct coord *t3, struct coord *t4)
{
// No casting (arg0/arg1/arg2 properties are already floats)
return utilsTriRayIntersectionTest(
		arg0->x, arg0->y, arg0->z,
		arg1->x, arg1->y, arg1->z,
		arg2->x, arg2->y, arg2->z,
		arg3, t0, t1, t2, t3, t4);
}

struct RGBA utilsUnpackColorRGBA(uint32_t color)
{
    struct RGBA result;
    result.r = (color >> 24) & 0xFF;
    result.g = (color >> 16) & 0xFF;
    result.b = (color >> 8)  & 0xFF;
    result.a = (color >> 0)  & 0xFF;
    return result;
}