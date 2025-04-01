#include <math.h>
#include "constants.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "game/debug.h"
#include "system.h"

float g_AlmostZero = 0.00001f;
struct coord g_RightVector = {0, 0, 1};

#define COUNTER_NUM (46875ULL)
#define COUNTER_DEN (1000ULL)

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

bool utilsNormalizeVector(struct coord *invec, struct coord *normalizedvec, uint32_t line, char *file)
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

float utilsClampF(float f, float min, float max)
{
	const float t = f < min ? min : f;
	return t > max ? max : t;
}

int utilsClamp(int d, int min, int max)
{
	const int t = d < min ? min : d;
	return t > max ? max : t;
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

void utilsCalcScreenCoords(Gfx **gdlptr, float *screenpos, float *brightness, int width, int height, int arg5, int arg6, int arg7, int arg8)
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

// Möller–Trumbore ray-triangle intersection algorithm
bool utilsRayIntersectsTriangle(
	float ax, float ay, float az,                // Vertex A of triangle
	float bx, float by, float bz,                // Vertex B of triangle
	float cx, float cy, float cz,                // Vertex C of triangle
	struct coord *offset,                        // Optional offset to apply to triangle
	struct coord *rayOrigin,
	struct coord *rayEnd,
	struct coord *rayDir,
	struct coord *outIntersection,               // Optional output: intersection point
	struct coord *outNormal)                     // Optional output: triangle normal
{
	// Apply offset to triangle vertex A if provided
	if (offset != NULL) {
		ax += offset->x;
		ay += offset->y;
		az += offset->z;
	}

	// Triangle edges
	float edge1x = bx - ax;
	float edge1y = by - ay;
	float edge1z = bz - az;

	float edge2x = cx - bx;
	float edge2y = cy - by;
	float edge2z = cz - bz;

	// Normal = cross(edge1, edge2)
	float normalX = edge1y * edge2z - edge1z * edge2y;
	float normalY = edge1z * edge2x - edge1x * edge2z;
	float normalZ = edge1x * edge2y - edge1y * edge2x;

	// Plane D = dot(normal, vertex A)
	float planeD = -(normalX * ax + normalY * ay + normalZ * az);

	// Ray direction and origin
	float dirX = rayDir->x;
	float dirY = rayDir->y;
	float dirZ = rayDir->z;

	float originX = rayOrigin->x;
	float originY = rayOrigin->y;
	float originZ = rayOrigin->z;

	// Dot product of normal and ray direction
	float denom = normalX * dirX + normalY * dirY + normalZ * dirZ;

	if (denom == 0.0f) {
		// Ray is parallel to triangle
		return false;
	}

	// Calculate distance along ray to intersection point
	float t = -(normalX * originX + normalY * originY + normalZ * originZ + planeD) / denom;

	// Compute intersection point: P = origin + t * dir
	float px = originX + t * dirX;
	float py = originY + t * dirY;
	float pz = originZ + t * dirZ;

	// Test if point lies between ray origin and ray end
	float endX = rayEnd->x;
	float endY = rayEnd->y;
	float endZ = rayEnd->z;

	float vecToPointX = px - originX;
	float vecToPointY = py - originY;
	float vecToPointZ = pz - originZ;

	float segmentX = endX - originX;
	float segmentY = endY - originY;
	float segmentZ = endZ - originZ;

	// If dot(vecToPoint, segment) < 0, point is behind origin
	float forwardTest = vecToPointX * segmentX + vecToPointY * segmentY + vecToPointZ * segmentZ;

	if (forwardTest < 0) {
		return false;
	}

	// Check if point lies within ray segment
	float toEndX = px - endX;
	float toEndY = py - endY;
	float toEndZ = pz - endZ;

	float backTest = toEndX * segmentX + toEndY * segmentY + toEndZ * segmentZ;

	if (backTest > 0) {
		return false;
	}

	// Barycentric coordinate test
	float edge0x = bx - ax;
	float edge0y = by - ay;
	float edge0z = bz - az;

	float edge1x2 = cx - ax;
	float edge1y2 = cy - ay;
	float edge1z2 = cz - az;

	float dx = px - ax;
	float dy = py - ay;
	float dz = pz - az;

	float dot00 = edge0x * edge0x + edge0y * edge0y + edge0z * edge0z;
	float dot01 = edge0x * edge1x2 + edge0y * edge1y2 + edge0z * edge1z2;
	float dot02 = edge0x * dx + edge0y * dy + edge0z * dz;
	float dot11 = edge1x2 * edge1x2 + edge1y2 * edge1y2 + edge1z2 * edge1z2;
	float dot12 = edge1x2 * dx + edge1y2 * dy + edge1z2 * dz;

	float denomBary = dot00 * dot11 - dot01 * dot01;

	if (denomBary == 0.0f) {
		return false;
	}

	float u = (dot11 * dot02 - dot01 * dot12) / denomBary;
	float v = (dot00 * dot12 - dot01 * dot02) / denomBary;

	if (u < 0.0f || v < 0.0f || u + v > 1.0f) {
		// Outside triangle
		return false;
	}

	// Passed all tests: intersection is valid
	if (outIntersection) {
		outIntersection->x = px;
		outIntersection->y = py;
		outIntersection->z = pz;
	}

	if (outNormal) {
		outNormal->x = normalX;
		outNormal->y = normalY;
		outNormal->z = normalZ;
	}

return true;
}

bool utilsRayIntersectsTriangleS16(struct vec3s16 *arg0, struct vec3s16 *arg1, struct vec3s16 *arg2,
		struct coord *arg3, struct coord *t0, struct coord *t1,
		struct coord *t2, struct coord *t3, struct coord *t4)
{
	// Casting arg0/arg1/arg2 properties from s16 to float
	return utilsRayIntersectsTriangle(
			arg0->x, arg0->y, arg0->z,
			arg1->x, arg1->y, arg1->z,
			arg2->x, arg2->y, arg2->z,
			arg3, t0, t1, t2, t3, t4);
}

bool utilsRayIntersectsTriangleF(struct coord *arg0, struct coord *arg1, struct coord *arg2,
		struct coord *arg3, struct coord *t0, struct coord *t1,
		struct coord *t2, struct coord *t3, struct coord *t4)
{
	// No casting (arg0/arg1/arg2 properties are already floats)
	return utilsRayIntersectsTriangle(
			arg0->x, arg0->y, arg0->z,
			arg1->x, arg1->y, arg1->z,
			arg2->x, arg2->y, arg2->z,
			arg3, t0, t1, t2, t3, t4);
}
