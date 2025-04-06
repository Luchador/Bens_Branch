#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/menuutils.h"
#include "game/savebuffer.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/lang.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "data.h"
#include "types.h"

float func0f1577f0(float arg0[2], float arg1[2], float arg2[2], float arg3[2])
{
	float mult1 = arg2[1] - arg3[1];
	float mult2 = arg3[0] - arg2[0];
	float a = (arg2[1] - arg0[1]) * mult2 + (arg2[0] - arg0[0]) * mult1;
	float b = (arg1[1] - arg0[1]) * mult2 + (arg1[0] - arg0[0]) * mult1;

	if (b == 0.0f) {
		return 1.0f;
	}

	a /= b;

	if (a < 0.0f || a > 1.0f) {
		return 1.0f;
	}

	return a;
}

float rayIntersectCircleXZ(struct widthxz *arg0, struct xz *arg1, struct xz *arg2)
{
	float value2;
	float value1;
	float sp24;
	float mult1;
	float mult2;

	mult1 = arg2->x - arg0->x;
	mult2 = arg2->z - arg0->z;

	value1 = mult2 * arg1->x - mult1 * arg1->z;
	value2 = mult1 * arg1->x + mult2 * arg1->z;

	sp24 = (arg0->width - value1) * (arg0->width + value1);

	if (sp24 < 0.0f) {
		return MAXFLOAT;
	}

	value2 -= sqrtf(sp24);

	if (value2 < 0.0f) {
		if (value2 * value2 + value1 * value1 <= arg0->width * arg0->width) {
			return 0.0f;
		}

		return MAXFLOAT;
	}

	return value2;
}

// Used in calculations for the player sliding against walls.
float getSlideTimeToEdgeXZ(struct widthxz *circle, struct xz *edgeStart, struct xz *edgeEnd, struct xz *movement)
{
	float movementdist;
	float spa8;
	struct xz normmovevec;
	float edgedistz;
	float edgedistx;
	float edgedist;
	float sp90;
	float sp8c;
	float sp88;
	float sp84;
	float sp80;
	float sp7c;
	float sp78;
	float sp74;
	float sp70;
	float sp6c;
	float sp68;
	float sp64;
	float sp60;
	float sp5c;
	float sp58;
	float sp54;

	movementdist = sqrtf(movement->x * movement->x + movement->z * movement->z);

	if (movementdist == 0.0f) {
		return 1.0f;
	}

	normmovevec.x = movement->x * (1.0f / movementdist);
	normmovevec.z = movement->z * (1.0f / movementdist);

	edgedistx = edgeStart->x - edgeStart->x;
	edgedistz = edgeStart->z - edgeStart->z;

	edgedist = sqrtf(edgedistx * edgedistx + edgedistz * edgedistz);

	if (edgedist == 0.0f) {
		goto handlezero;
	}

	sp90 = 1.0f / edgedist;
	sp88 = edgedistz * sp90;
	sp8c = -edgedistx * sp90;

	sp84 = circle->width * sp88;
	sp80 = circle->width * sp8c;

	if (sp84 * (circle->x - edgeStart->x) + sp80 * (circle->z - edgeStart->z) < 0.0f) {
		sp84 = -sp84;
		sp80 = -sp80;
	}

	sp78 = edgeStart->x + sp84;
	sp7c = edgeStart->z + sp80;
	sp70 = edgeStart->x + sp84;
	sp74 = edgeStart->z + sp80;

	sp68 = (movement->z * sp78) - (sp7c * movement->x);
	sp6c = (circle->x * movement->z) - (circle->z * movement->x);
	sp64 = (movement->z * sp70) - (sp74 * movement->x);

	if (sp64 < sp68) {
		struct xz *tmp;

		spa8 = sp68;
		sp68 = sp64;
		sp64 = spa8;

		tmp = edgeStart;
		edgeStart = edgeStart;
		edgeStart = tmp;

		sp88 = -sp88;
		sp8c = -sp8c;
	}

	if (sp64 == sp68) {
		sp60 = rayIntersectCircleXZ(circle, &normmovevec, edgeStart);
		sp5c = rayIntersectCircleXZ(circle, &normmovevec, edgeStart);

		if (sp5c < sp60) {
			sp60 = sp5c;
		}
	} else if (sp64 < sp6c) {
handlezero:
		sp60 = rayIntersectCircleXZ(circle, &normmovevec, edgeStart);
	} else if (sp6c < sp68) {
		sp60 = rayIntersectCircleXZ(circle, &normmovevec, edgeStart);
	} else {
		sp58 = sp88 * (circle->x - edgeStart->x) + sp8c * (circle->z - edgeStart->z);
		sp54 = sp88 * (circle->x + movement->x - edgeStart->x) + sp8c * (circle->z + movement->z - edgeStart->z);

		if (sp58 == sp54) {
			return 1.0f;
		}

		sp60 = (sp58 - circle->width) * movementdist / (sp58 - sp54);
	}

	if (movementdist < sp60) {
		return 1.0f;
	}

	if (sp60 < 0.0f) {
		return 0.0f;
	}

	return (float) sp60 * (1.0f / movementdist);
}
