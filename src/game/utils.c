#include <ultra64.h>
#include "constants.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "game/debug.h"

const char var7f1b7cb0[] = "UM_Make : In\n";
const char var7f1b7cc0[] = "UM_Make : Out\n";
const char var7f1b7cd0[] = "Utils -> ERROR at Line %d of %s\n";
const char var7f1b7cf4[] = "Utils -> UM_fVec3_NormaliseTo - Vec = %s%s (%f,%f,%f)\n";
const char var7f1b7d2c[] = "";
const char var7f1b7d30[] = "";
const char var7f1b7d34[] = "Utils -> Attempt to normalise zeo length vector\n";

void *var800ac0d0;
u8 *var800ac0e8[4];

f32 var800845d0 = 999999;
f32 g_AlmostZero = 0.00001f;
//s32 var800845d8 = 1;
struct coord g_ZeroVector = {0, 0, 0};
struct coord g_RightVector = {0, 0, 1};
u8 *var80084610 = NULL;
u8 *var80084614 = NULL;
u8 *var80084618 = NULL;

// Not used
/*void func0f176d70(s32 arg0)
{
	var800845d8 = arg0;
}*/

// Not used
/*s32 func0f176d7c(void)
{
	return var800845d8;
}*/

u32 align4(u32 arg0)
{
	if (arg0 & 3) {
		arg0 = (arg0 & 0xfffffffc) + 4;
	}

	return arg0;
}

u32 align16(u32 arg0)
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
	s32 i;
	u32 slotssize = 0x1900;
	u32 allocsize;

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

u32 FloatToUInt32(f32 arg0)
{
	if (arg0 > 32767.9f) {
		arg0 = 32767.9f;
	}

	if (arg0 < -32767.9f) {
		arg0 = -32767.9f;
	}

	return (u32)(arg0 * 65536);
}

// Not used
/*s32 func0f176eb0(s32 arg0, s32 arg1)
{
	if (arg0 % arg1 == 0) {
		return arg0;
	}

	return (arg0 / arg1 + 1) * arg1;
}*/

// Not used
/*void func0f176f34(struct coord *a, struct coord *b, struct coord *out)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}*/

// Not used
/*void func0f176f68(struct coord *a, struct coord *b, struct coord *c, struct coord *out)
{
	out->x = a->x + b->x + c->x;
	out->y = a->y + b->y + c->y;
	out->z = a->z + b->z + c->z;
}*/

// Not used
/*void func0f176fb4(struct coord *a, struct coord *b, struct coord *c, struct coord *d, struct coord *out)
{
	out->x = a->x + b->x + c->x + d->x;
	out->y = a->y + b->y + c->y + d->y;
	out->z = a->z + b->z + c->z + d->z;
}*/

// Not used
/*void func0f17701c(struct coord *a, struct coord *b, struct coord *out)
{
	out->x = b->x - a->x;
	out->y = b->y - a->y;
	out->z = b->z - a->z;
}*/

// Not used
/*void func0f177050(struct coord *a, f32 mult, struct coord *out)
{
	out->x = a->x * mult;
	out->y = a->y * mult;
	out->z = a->z * mult;
}*/

// Not used
/*f32 func0f17707c(struct coord *a, struct coord *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}*/

// Calculates in cross product and returns the result in left-handed coordinates. Used in wallhit.c, but nothing is done with the result.
/*void utilsCalcLeftHandedCross(struct coord *a, struct coord *b, struct coord *out)
{
	out->x = a->y * b->z - a->z * b->y;
	out->y = -(a->x * b->z - a->z * b->x);
	out->z = a->x * b->y - a->y * b->x;
}*/

bool normalizeVector(struct coord *invec, struct coord *normalizedvec, u32 line, char *file)
{
	f32 sqdist = invec->x * invec->x + invec->y * invec->y + invec->z * invec->z;
	f32 mult;

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

void textureCalcScreenCoords(Gfx **gdlptr, f32 *screenpos, f32 *brightness, s32 width, s32 height, s32 arg5, s32 arg6, s32 arg7, s32 arg8)
{
	if (brightness[0] > 0.0f && brightness[1] > 0.0f) {
		Gfx *gdl = *gdlptr;
		s32 xl;
		s32 yl;
		s32 xh;
		s32 yh;
		s32 s = 0;
		s32 t = 0;
		s32 dsdx;
		s32 dtdy;
		s32 widthx4;
		s32 heightx4;
		s32 sp20 = 0;
		s32 sp1c = 0;

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

// Not used
/*bool func0f177230(struct coord *a, struct coord *b, struct coord *c)
{
	struct coord diff;
	diff.x = a->x - b->x;
	diff.y = a->y - b->y;
	diff.z = a->z - b->z;

	return diff.x * c->x + diff.y * c->y + diff.z * c->z > 0;
}*/

// Not used
/*bool func0f177298(struct coord *a, struct coord *b, struct coord *c)
{
	struct coord diff;
	diff.x = a->x - b->x;
	diff.y = a->y - b->y;
	diff.z = a->z - b->z;

	return diff.x * c->x + diff.y * c->y + diff.z * c->z < 0;
}*/

// Not used
/*bool func0f177300(struct coord *a, struct coord *b)
{
	f32 diff = a->x - b->x;

	if (ABS(diff) < g_AlmostZero) {
		diff = a->y - b->y;

		if (ABS(diff) < g_AlmostZero) {
			diff = a->z - b->z;

			if (ABS(diff) < g_AlmostZero) {
				return true;
			}
		}
	}

	return false;
}*/

bool isPointInBBox(struct coord *point, struct coord *bbox)
{
	if (point->x < bbox->x && -bbox->x < point->x
			&& point->y < bbox->y && -bbox->y < point->y
			&& point->z < bbox->z && -bbox->z < point->z) {
		return true;
	}
	return false;
}

f32 coordsGetDistance(struct coord *a, struct coord *b)
{
	f32 xdiff = b->x - a->x;
	f32 ydiff = b->y - a->y;
	f32 zdiff = b->z - a->z;

	return sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
}

// Not used
/*bool func0f1774b4(struct coord *arg0, struct coord *arg1, struct coord *out)
{
	struct coord a;
	struct coord b;
	struct coord c;
	f32 mult;

	if (!normalizeVector(arg1, &a, 702, "utils.c")) {
		osSyncPrintf("UTILS -> DEBUG ERROR - UM_fVec3_MakeNormalTo - Cant normalise\n");
		return false;
	}

	if (!normalizeVector(arg0, &b, 710, "utils.c")) {
		osSyncPrintf("UTILS -> DEBUG ERROR - UM_fVec3_MakeNormalTo - Cant normalise\n");
		return false;
	}

	mult = -(a.x * b.x + a.y * b.y + a.z * b.z);

	c.x = mult * a.x;
	c.y = mult * a.y;
	c.z = mult * a.z;

	out->x = b.x + c.x;
	out->y = b.y + c.y;
	out->z = b.z + c.z;

	return true;
}*/

// Not used
/*void func0f17758c(f32 *arg0, f32 *arg1, struct coord *arg2, f32 *arg3)
{
	f32 a = arg0[0] - arg1[0];
	f32 b = arg0[1] - arg1[1];
	f32 c = arg0[2] - arg1[2];
	f32 sum = a * arg2->x + b * arg2->y + c * arg2->z;

	a = -sum * arg2->x;
	b = -sum * arg2->y;
	c = -sum * arg2->z;

	arg3[0] = a + arg0[0];
	arg3[1] = b + arg0[1];
	arg3[2] = c + arg0[2];
}*/

// Not used
/*void func0f177624(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3)
{
	f32 dist;
	f32 tmpx;
	f32 tmpz;

	normalizeVector(arg0, arg1, 771, "utils.c");

	dist = sqrtf(arg1->x * arg1->x + arg1->z * arg1->z);

	tmpx = (arg1->x / dist);
	tmpz = arg1->z / dist;

	arg2->x = tmpz;
	arg2->y = 0.0f;
	arg2->z = -tmpx;

	arg3->x = arg1->y * tmpz;
	arg3->y = -dist;
	arg3->z = arg1->y * arg2->z;
}*/

// Not used
/*f32 func0f1776cc(struct coord *a, struct coord *b, struct coord *c)
{
	f32 xdiff = c->x - a->x;
	f32 ydiff = c->y - a->y;
	f32 zdiff = c->z - a->z;

	f32 sqdist = xdiff * b->x + ydiff * b->y + zdiff * b->z;

	if (sqdist < g_AlmostZero && sqdist > -g_AlmostZero) {
		return var800845d0;
	}

	return (xdiff * xdiff + ydiff * ydiff + zdiff * zdiff) / sqdist;
}*/

// Not used
/*bool func0f17776c(struct coord *a, struct coord *b, f32 mult, struct coord *out)
{
	struct coord tmp;
	tmp.x = b->x * mult;
	tmp.y = b->y * mult;
	tmp.z = b->z * mult;

	out->x = a->x + tmp.x;
	out->y = a->y + tmp.y;
	out->z = a->z + tmp.z;

	return true;
}*/

// Not used
/*bool func0f1777b8(struct coord *a, struct coord *b, struct coord *c, struct coord *out)
{
	f32 mult = func0f1776cc(a, b, c);
	func0f17776c(a, b, mult, out);

	return true;
}*/

s32 func0f177a54(u8 *arg0, s32 arg1, u8 *arg2, s32 arg3)
{
	s32 i = 0;
	s32 v1 = 0;
	s32 t0 = 0;

	for (; i < arg1; i++) {
		s32 index = i * arg3;

		if (arg0[index] != 0) {
			u8 *ptr = &arg0[index];

			if (i != 0 && ptr[-arg3] == 0) {
				arg2[v1++] = 0;

				if (t0 == 255) {
					arg2[v1++] = 200;
					arg2[v1++] = 0;
					t0 -= 200;
				} else {
					while (t0 > 255) {
						arg2[v1++] = 255;
						t0 -= 255;
					}
				}

				arg2[v1++] = t0;
				t0 = 0;
			}

			arg2[v1++] = arg0[index];
		} else {
			t0++;
		}
	}

	arg2[v1++] = 0;
	arg2[v1++] = 0;

	return v1;
}

// Not used
/*u8 func0f177b44(u8 *arg0, s32 *arg1)
{
	static s32 var800ac108;

	if (*arg1 == -1) {
		var800ac108 = 0;
	}

	if (arg0[var800ac108] == 0) {
		var800ac108++;

		if (arg0[var800ac108] != 0) {
			*arg1 += arg0[var800ac108];
			var800ac108++;
		}
	}

	*arg1 += 1;

	return arg0[var800ac108++];
}*/

//const char var7f1b7e50[] = "UM_ZeroRunVerify_U8 - FAILED on item %d\n";

/*s32 func0f177bb4(u8 *arg0, s32 *arg1, s32 *arg2)
{
	s32 result = 0;
	s32 value;
	s32 tmp = 255;

	static s32 var80084624 = 0;

	if (arg0 == NULL) {
		return -1;
	}

	if (*arg1 == 0) {
		*arg2 = -1;
	}

	var80084624--;

	if (var80084624 <= 0) {
		if (arg0[*arg1] != 0) {
			result = arg0[*arg1];
		} else {
			*arg1 += 1;

			if (arg0[*arg1] != 0) {
				var80084624 = 0;
				value = arg0[*arg1];

				while (tmp == value) {
					var80084624 += tmp;
					*arg1 += 1;
					value = arg0[*arg1];
				}

				var80084624 += value;
			} else {
				return -1;
			}
		}

		*arg1 += 1;
	}

	*arg2 += 1;

	return result;
}*/

s32 func0f177c8c(u8 *arg0, s32 *arg1, s32 *arg2)
{
	s32 result;

	if (*arg1 == 0) {
		*arg2 = -1;
	}

	while (arg0[*arg1] == 0) {
		*arg1 += 1;

		if (arg0[*arg1]) {
			while (arg0[*arg1] == 255) {
				*arg2 += 255;
				*arg1 += 1;
			}

			*arg2 += arg0[*arg1];
			*arg1 += 1;
		} else {
			return -1;
		}
	}

	*arg2 += 1;

	result = arg0[*arg1];

	*arg1 += 1;

	return result;
}

void InterpTwoPoints(struct coord *arg0, struct coord *arg1, f32 standfrac, struct coord *vel)
{
	vel->x = (arg1->x - arg0->x) * standfrac + arg0->x;
	vel->y = (arg1->y - arg0->y) * standfrac + arg0->y;
	vel->z = (arg1->z - arg0->z) * standfrac + arg0->z;
}

void CatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, f32 arg4, struct coord *arg5)
{
	f32 stack;
	f32 mult0;
	f32 mult1;
	f32 mult2;
	f32 mult3;

	f32 squared = arg4 * arg4;
	f32 cubed = arg4 * arg4 * arg4;

	mult0 = squared - 0.5f * (arg4 + cubed);
	mult1 = 1.5f * cubed - 2.5f * squared + 1.0f;
	mult2 = -1.5f * cubed + 2.0f * squared + 0.5f * arg4;
	mult3 = 0.5f * (cubed - squared);

	arg5->x = mult0 * arg0->f[0] + mult1 * arg1->f[0] + mult2 * arg2->f[0] + mult3 * arg3->f[0];
	arg5->y = mult0 * arg0->f[1] + mult1 * arg1->f[1] + mult2 * arg2->f[1] + mult3 * arg3->f[1];
	arg5->z = mult0 * arg0->f[2] + mult1 * arg1->f[2] + mult2 * arg2->f[2] + mult3 * arg3->f[2];
}