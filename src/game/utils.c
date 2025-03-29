#include <ultra64.h>
#include <math.h>
#include <stdlib.h>
#include "constants.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "system.h"
#include "game/debug.h"

#define COUNTER_NUM (46875ULL)
#define COUNTER_DEN (1000ULL)

/* Time */

void *var800ac0d0;
uint8_t *var800ac0e8[4];

float var800845d0 = 999999;
float g_AlmostZero = 0.00001f;
struct coord g_ZeroVector = {0, 0, 0};
struct coord g_RightVector = {0, 0, 1};
uint8_t *var80084610 = NULL;
uint8_t *var80084614 = NULL;
uint8_t *var80084618 = NULL;

uint64_t utilsGetCount(void)
{
	return (sysGetMicroseconds() * COUNTER_NUM) / COUNTER_DEN;
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

void utilsInit(void)
{
	int i;
	uint32_t slotssize = 0x1900;
	uint32_t allocsize;

	var800ac0d0 = mempAlloc(10000, MEMPOOL_8);

	allocsize = align16(0x3900);
	var800ac0e8[0] = mempAlloc(allocsize, MEMPOOL_8);

	if (var800ac0e8[0] != NULL) {
		for (i = 0; i < ARRAYCOUNT(var800ac0e8); i++) {
			var800ac0e8[i] = var800ac0e8[0] + ((i * 100) << 4);
		}
	} else {
		for (i = 0; i < ARRAYCOUNT(var800ac0e8); i++) {
			var800ac0e8[i] = NULL;
		}
	}

	var80084610 = var800ac0e8[0] + slotssize;
	var80084618 = var800ac0e8[0] + allocsize - 1;
	var80084614 = var80084610;
}

bool normalizeVector(struct coord *invec, struct coord *normalizedvec, uint32_t line, char *file)
{
	float sqdist = invec->x * invec->x + invec->y * invec->y + invec->z * invec->z;
	float mult;

	if (sqdist < g_AlmostZero) {
		invec->x = g_RightVector.x;
		invec->y = g_RightVector.y;
		invec->z = g_RightVector.z;

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

void textureCalcScreenCoords(Gfx **gdlptr, float *screenpos, float *brightness, int width, int height, int arg5, int arg6, int arg7, int arg8)
{
	if (brightness[0] > 0.0f && brightness[1] > 0.0f) {
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

		gDPSetTexturePersp(gdl++, G_TP_NONE);

		xl = (screenpos[0] - brightness[0]) * 4.0f;
		yl = (screenpos[1] - brightness[1]) * 4.0f;
		xh = (screenpos[0] + brightness[0]) * 4.0f;
		yh = (screenpos[1] + brightness[1]) * 4.0f;

		if (xh >= 0 && yh >= 0) {
			if (arg8) {
				width *= 2;
				height *= 2;
				s = -(width * 16);
				t = -(height * 16);
			}

			if (xl < 0) {
				if (arg5) {
					t += ((-xl * height) << 5) / (xh - xl);
				} else {
					s += ((-xl * width) << 5) / (xh - xl);
				}

				xl = 0;
			}

			if (yl < 0) {
				if (arg5) {
					s += ((-yl * width) << 5) / (yh - yl);
				} else {
					t += ((-yl * height) << 5) / (yh - yl);
				}

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

			if (arg5) {
				dsdx = width / (2.0f * brightness[1]) * 1024.0f;
				dtdy = height / (2.0f * brightness[0]) * 1024.0f;
			} else {
				dsdx = width / (2.0f * brightness[0]) * 1024.0f;
				dtdy = height / (2.0f * brightness[1]) * 1024.0f;
			}

			if (arg6) {
				dsdx = 0x10000 - dsdx;

				if (arg8) {
					s = (((width >> 1) - 1) << 5) - s;
				} else {
					s = ((width - 1) << 5) - s;
				}
			}

			if (arg7) {
				dtdy = 0x10000 - dtdy;

				if (arg8) {
					t = (((height >> 1) - 1) << 5) - t;
				} else {
					t = ((height - 1) << 5) - t;
				}
			}

			if (arg5) {
				gSPTextureRectangleFlip(gdl++, xl, yl, xh, yh, 0, s, t, dsdx, dtdy);
			} else {
				gSPTextureRectangle(gdl++, xl, yl, xh, yh, 0, s, t, dsdx, dtdy);
			}
		}

		gDPSetTexturePersp(gdl++, G_TP_PERSP);

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
int utilCompressZeroRuns(uint8_t *input, int numEntries, uint8_t *output, int entrySize) {
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

int untilCompressRoomData(uint8_t *arg0, int *arg1, int *roomnum)
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

void InterpTwoPoints(struct coord *arg0, struct coord *arg1, float standfrac, struct coord *vel)
{
	vel->x = (arg1->x - arg0->x) * standfrac + arg0->x;
	vel->y = (arg1->y - arg0->y) * standfrac + arg0->y;
	vel->z = (arg1->z - arg0->z) * standfrac + arg0->z;
}

void CatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, float arg4, struct coord *arg5)
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