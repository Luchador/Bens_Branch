#include <ultra64.h>
#include <stdint.h>
#include <math.h>
#include "constants.h"
#include "game/prop.h"
#include "game/textutils.h"
#include "game/bg.h"
#include "game/room.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/lib_17ce0.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "data.h"
#include "types.h"

#define SURFACE_FLOOR   0
#define SURFACE_CEILING 1

struct debugtri {
	int16_t vertices[3][3];
	uint8_t unk12;
};

union filedataptr g_TileFileData;
int g_TileNumRooms;
int32_t *g_TileRooms;
bool g_HasSlideTimeToEdge;
float g_SlideTimeToEdge;
struct coord g_CdEdgeVtx1;
int var8009a8c4;
struct coord g_CdEdgeVtx2;
struct prop *g_CdObstacleProp;
struct coord g_CdObstaclePos;
float var8009a8f0;
bool g_CdHasSavedPos;
struct coord g_CdPos1;
struct coord g_CdPos2;
struct geoblock g_CdSavedBlock;
struct geo *g_CdObstacleGeo;

bool g_CdHasSavedBlock = false;

float cd00024e40(void)
{
	return var8009a8f0;
}

void cdGetEdge(struct coord *vtx1, struct coord *vtx2)
{
	vtx1->x = g_CdEdgeVtx1.x;
	vtx1->y = g_CdEdgeVtx1.y;
	vtx1->z = g_CdEdgeVtx1.z;

	vtx2->x = g_CdEdgeVtx2.x;
	vtx2->y = g_CdEdgeVtx2.y;
	vtx2->z = g_CdEdgeVtx2.z;
}

float cd00024e98(void)
{
	return g_SlideTimeToEdge;
}

int cd00024ea4(void)
{
	return g_HasSlideTimeToEdge;
}

struct prop *cdGetObstacleProp(void)
{
	return g_CdObstacleProp;
}

void cdGetPos(struct coord *pos)
{
	pos->x = g_CdObstaclePos.x;
	pos->y = g_CdObstaclePos.y;
	pos->z = g_CdObstaclePos.z;
}

void cdGetObstacleNormal(struct coord *normal)
{
	cdGetGeoNormal(g_CdObstacleGeo, normal);
}

int32_t cdGetGeoFlags(void)
{
	int32_t flags = 0;

	switch (g_CdObstacleGeo->type) {
	case GEOTYPE_TILE_I:
		flags = g_CdObstacleGeo->flags;
		break;
	case GEOTYPE_TILE_F:
		flags = g_CdObstacleGeo->flags;
		break;
	case GEOTYPE_BLOCK:
		flags = GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT;
		break;
	case GEOTYPE_CYL:
		flags = g_CdObstacleGeo->flags;
		break;
	}

	return flags;
}

void cdClearResults(void)
{
	g_HasSlideTimeToEdge = false;
	g_CdObstacleProp = NULL;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
}

void cdSetObstacleVtxProp(struct coord *vtx1, struct coord *vtx2, struct prop *prop)
{
	g_CdEdgeVtx1.x = vtx1->x;
	g_CdEdgeVtx1.y = vtx1->y;
	g_CdEdgeVtx1.z = vtx1->z;

	g_CdEdgeVtx2.x = vtx2->x;
	g_CdEdgeVtx2.y = vtx2->y;
	g_CdEdgeVtx2.z = vtx2->z;

	g_HasSlideTimeToEdge = false;
	g_CdObstacleProp = prop;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
}

void cdSetObstacleVtxPropFlt(struct coord *vtx1, struct coord *vtx2, struct prop *prop, float arg3)
{
	g_SlideTimeToEdge = arg3;

	g_CdEdgeVtx1.x = vtx1->x;
	g_CdEdgeVtx1.y = vtx1->y;
	g_CdEdgeVtx1.z = vtx1->z;

	g_CdEdgeVtx2.x = vtx2->x;
	g_CdEdgeVtx2.y = vtx2->y;
	g_CdEdgeVtx2.z = vtx2->z;

	g_HasSlideTimeToEdge = true;
	g_CdObstacleProp = prop;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
}

void cdComputeSlideTimeToEdgeXZ(struct coord *startPos, struct coord *targetPos, float radius)
{
	struct widthxz sp34;
	struct xz sp2c;
	struct xz sp24;
	struct xz sp1c;

	sp34.width = radius;
	sp34.x = startPos->x;
	sp34.z = startPos->z;

	sp1c.x = targetPos->x;
	sp1c.z = targetPos->z;

	sp2c.x = g_CdEdgeVtx1.x;
	sp2c.z = g_CdEdgeVtx1.z;

	sp24.x = g_CdEdgeVtx2.x;
	sp24.z = g_CdEdgeVtx2.z;

	g_SlideTimeToEdge = getSlideTimeToEdgeXZ(&sp34, &sp2c, &sp24, &sp1c);
	g_HasSlideTimeToEdge = true;
}

void cdSetObstacleProp(struct prop *prop)
{
	g_HasSlideTimeToEdge = false;
	g_CdObstacleProp = prop;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
}

void cdSetObstacleVtxColProp(struct coord *vtxpos1, struct coord *vtxpos2, struct coord *collisionpos, struct prop *prop)
{
	g_CdEdgeVtx1.x = vtxpos1->x;
	g_CdEdgeVtx1.y = vtxpos1->y;
	g_CdEdgeVtx1.z = vtxpos1->z;

	g_CdEdgeVtx2.x = vtxpos2->x;
	g_CdEdgeVtx2.y = vtxpos2->y;
	g_CdEdgeVtx2.z = vtxpos2->z;

	g_CdObstaclePos.x = collisionpos->x;
	g_CdObstaclePos.y = collisionpos->y;
	g_CdObstaclePos.z = collisionpos->z;

	g_HasSlideTimeToEdge = false;
	g_CdObstacleProp = prop;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
}

void cdSetObstacleVtxColPropFltGeo(struct coord *vtxpos1, struct coord *vtxpos2, struct coord *collisionpos, struct prop *prop, float arg4, struct geo *geo)
{
	g_CdEdgeVtx1.x = vtxpos1->x;
	g_CdEdgeVtx1.y = vtxpos1->y;
	g_CdEdgeVtx1.z = vtxpos1->z;

	g_CdEdgeVtx2.x = vtxpos2->x;
	g_CdEdgeVtx2.y = vtxpos2->y;
	g_CdEdgeVtx2.z = vtxpos2->z;

	g_CdObstaclePos.x = collisionpos->x;
	g_CdObstaclePos.y = collisionpos->y;
	g_CdObstaclePos.z = collisionpos->z;

	g_HasSlideTimeToEdge = false;
	g_CdObstacleProp = prop;
	var8009a8f0 = arg4;
	g_CdHasSavedPos = false;
	g_CdHasSavedBlock = false;
	g_CdObstacleGeo = geo;
}

void cdSetSavedPos(struct coord *pos1, struct coord *pos2)
{
	g_CdPos1.x = pos1->x;
	g_CdPos1.y = pos1->y;
	g_CdPos1.z = pos1->z;

	g_CdPos2.x = pos2->x;
	g_CdPos2.y = pos2->y;
	g_CdPos2.z = pos2->z;

	g_CdHasSavedPos = true;
}

bool cdGetSavedPos(struct coord *pos1, struct coord *pos2)
{
	if (g_CdHasSavedPos) {
		pos1->x = g_CdPos1.x;
		pos1->y = g_CdPos1.y;
		pos1->z = g_CdPos1.z;

		pos2->x = g_CdPos2.x;
		pos2->y = g_CdPos2.y;
		pos2->z = g_CdPos2.z;
	}

	return g_CdHasSavedPos;
}

void cdSetSavedBlock(struct geoblock *block)
{
	g_CdSavedBlock = *block;
	g_CdHasSavedBlock = true;
}

int cd00025410(float arg0, float arg1, float arg2, float arg3)
{
	float f0 = arg0 * arg3;
	float f2 = arg1 * arg2;

	if (f2 < f0) {
		return 1;
	}

	if (f2 > f0) {
		return -1;
	}

	if (arg0 * arg2 < 0.0f || arg1 * arg3 < 0.0f) {
		return -1;
	}

	if (arg0 * arg0 + arg1 * arg1 < arg2 * arg2 + arg3 * arg3) {
		return 1;
	}

	return 0;
}

int cd000254d8(struct coord *arg0, struct coord *arg1, float arg2, float arg3, float arg4, float arg5, int *arg6)
{
	float sp54;
	float sp50;
	float sp4c;
	float sp48;
	int sp44;
	int sp40;
	int sp3c;
	int sp38;
	int sp34;
	int sp30;
	bool result = false;

	sp54 = arg0->x - arg2;
	sp50 = arg0->z - arg3;

	sp3c = cd00025410(arg4 - arg2, arg5 - arg3, sp54, sp50);
	sp44 = cd00025410(arg4 - arg2, arg5 - arg3, arg1->x - arg2, arg1->z - arg3);
	sp38 = sp3c * sp44;

	if (sp38 <= 0) {
		sp4c = arg1->x - arg0->x;
		sp48 = arg1->z - arg0->z;

		sp34 = cd00025410(sp4c, sp48, -sp54, -sp50);
		sp40 = cd00025410(sp4c, sp48, arg4 - arg0->x, arg5 - arg0->z);
		sp30 = sp34 * sp40;

		if (sp30 <= 0) {
			result = true;
		}
	}

	if (*arg6 && (result || sp3c <= 0)) {
		*arg6 = 0;
	}

	return result;
}

float cd00025654(float x1, float z1, float x2, float z2, float x3, float z3)
{
	float result;

	result = sqrtf((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1));

	if (result == 0.0f) {
		return sqrtf((x3 - x2) * (x3 - x2) + (z3 - z2) * (z3 - z2));
	}

	return ((x3 - x1) * (z2 - z1) + -(x2 - x1) * (z3 - z1)) / result;
}

// Get the distance between two 2D points
float cdGetDistanceXZ(float x1, float z1, float x2, float z2)
{
	x2 -= x1;
	z2 -= z1;

	return sqrtf(x2 * x2 + z2 * z2);
}

bool cdIsPointBetweenXZ(float x1, float z1, float x2, float z2, float x3, float z3)
{
	float xvec;
	float zvec;
	float dot;
	float squaredmagnitude;

	x3 -= x1;
	z3 -= z1;

	xvec = x2 - x1;
	zvec = z2 - z1;

	dot = x3 * xvec + z3 * zvec;
	squaredmagnitude = xvec * xvec + zvec * zvec;

	return (squaredmagnitude < dot && dot < 0) || (dot > 0 && dot < squaredmagnitude);
}

void cd00025848(float tilex, float tilez, float tilewidth, float posx, float posz, float *x1, float *z1, float *x2, float *z2)
{
	posx -= tilex;
	posz -= tilez;

	if (posx != 0 || posz != 0) {
		float dist = sqrtf(posx * posx + posz * posz);

		if (dist > 0) {
			dist = tilewidth / dist;
			posx *= dist;
			posz *= dist;
		}
	}

	*x1 = tilex + posx + posz;
	*z1 = tilez + posz - posx;
	*x2 = tilex + posx - posz;
	*z2 = tilez + posz + posx;
}

void cdGetGeoNormal(struct geo *geo, struct coord *normal)
{
	if (geo->type == GEOTYPE_TILE_I) {
		struct geotilei *tile = (struct geotilei *) geo;
		int sp38[3];
		int sp2c[3];
		int sp20[3];

		sp38[0] = tile->vertices[1][0] - tile->vertices[0][0];
		sp38[1] = tile->vertices[1][1] - tile->vertices[0][1];
		sp38[2] = tile->vertices[1][2] - tile->vertices[0][2];

		sp2c[0] = tile->vertices[2][0] - tile->vertices[0][0];
		sp2c[1] = tile->vertices[2][1] - tile->vertices[0][1];
		sp2c[2] = tile->vertices[2][2] - tile->vertices[0][2];

		sp20[0] = sp38[1] * sp2c[2] - sp38[2] * sp2c[1];
		sp20[1] = sp38[2] * sp2c[0] - sp38[0] * sp2c[2];
		sp20[2] = sp38[0] * sp2c[1] - sp38[1] * sp2c[0];

		normal->x = sp20[0];
		normal->y = sp20[1];
		normal->z = sp20[2];
	} else if (geo->type == GEOTYPE_TILE_F) {
		struct geotilef *tile = (struct geotilef *) geo;
		float sp10[3];
		float sp04[3];

		sp10[0] = tile->vertices[1].x - tile->vertices[0].x;
		sp10[1] = tile->vertices[1].y - tile->vertices[0].y;
		sp10[2] = tile->vertices[1].z - tile->vertices[0].z;

		sp04[0] = tile->vertices[2].x - tile->vertices[0].x;
		sp04[1] = tile->vertices[2].y - tile->vertices[0].y;
		sp04[2] = tile->vertices[2].z - tile->vertices[0].z;

		normal->x = sp10[1] * sp04[2] - sp10[2] * sp04[1];
		normal->y = sp10[2] * sp04[0] - sp10[0] * sp04[2];
		normal->z = sp10[0] * sp04[1] - sp10[1] * sp04[0];
	} else if (geo->type == GEOTYPE_BLOCK) {
		normal->x = 0;
		normal->y = 1;
		normal->z = 0;
	} else if (geo->type == GEOTYPE_CYL) {
		normal->x = 0;
		normal->y = 1;
		normal->z = 0;
	}
}

void cdGetFloorCol(struct geo *geo, int16_t *floorcol)
{
	if (geo == NULL) {
		*floorcol = 0xfff;
		return;
	}

	if (geo->type == GEOTYPE_TILE_I) {
		struct geotilei *tile = (struct geotilei *) geo;
		*floorcol = tile->floorcol;
		return;
	}

	if (geo->type == GEOTYPE_TILE_F) {
		struct geotilef *tile = (struct geotilef *) geo;
		*floorcol = tile->floorcol;
		return;
	}

	if (geo->type == GEOTYPE_BLOCK) {
		*floorcol = 0xfff;
		return;
	}

	if (geo->type == GEOTYPE_CYL) {
		*floorcol = 0xfff;
	}
}

void cdGetFloorType(struct geo *geo, uint8_t *floortype)
{
	bool water = false;

	if (geo && (geo->flags & GEOFLAG_UNDERWATER)) {
		water = true;
	}

	if (geo == NULL) {
		*floortype = 0xff;
		return;
	}

	if (water) {
		*floortype = FLOORTYPE_WATER;
		return;
	}

	if (geo->type == GEOTYPE_TILE_I) {
		struct geotilei *tile0 = (struct geotilei *) geo;
		*floortype = tile0->floortype;
		return;
	}

	if (geo->type == GEOTYPE_TILE_F) {
		struct geotilef *tile1 = (struct geotilef *) geo;
		*floortype = tile1->floortype;
		return;
	}

	if (geo->type == GEOTYPE_BLOCK) {
		*floortype = 0xff;
		return;
	}

	if (geo->type == GEOTYPE_CYL) {
		*floortype = 0xff;
	}
}

float cdFindGroundInIntTileAtVertex(struct geotilei *tile, float x, float z, int vertexindex)
{
	struct coord sp7c;
	struct coord sp70;
	int64_t sp68;
	int64_t sp60;
	int64_t sp58;
	int64_t tmp;
	float ground;
	int next;

	if (vertexindex == 0) {
		vertexindex = 1;
	}

	next = (vertexindex + 1) % tile->header.numvertices;

	if (next == 0) {
		next = 1;
	}

	sp7c.x = tile->vertices[vertexindex][0] - tile->vertices[0][0];
	sp7c.y = tile->vertices[vertexindex][1] - tile->vertices[0][1];
	sp7c.z = tile->vertices[vertexindex][2] - tile->vertices[0][2];

	sp70.x = tile->vertices[next][0] - tile->vertices[0][0];
	sp70.y = tile->vertices[next][1] - tile->vertices[0][1];
	sp70.z = tile->vertices[next][2] - tile->vertices[0][2];

	sp58 = sp7c.f[1] * sp70.f[2] - sp7c.f[2] * sp70.f[1];
	sp60 = sp7c.f[2] * sp70.f[0] - sp7c.f[0] * sp70.f[2];
	sp68 = sp7c.f[0] * sp70.f[1] - sp7c.f[1] * sp70.f[0];

	tmp = sp58 * tile->vertices[0][0]
		+ sp60 * tile->vertices[0][1]
		+ sp68 * tile->vertices[0][2];

	if (sp60 == 0) {
		return *(int16_t *)(tile->ymax + (uintptr_t)tile);
	}

	ground = (tmp - (double)x * sp58 - (double)z * sp68) / sp60;

	if (ground > *(int16_t *)(tile->ymax + (uintptr_t)tile)) {
		ground = *(int16_t *)(tile->ymax + (uintptr_t)tile);
	} else if (ground < *(int16_t *)(tile->ymin + (uintptr_t)tile)) {
		ground = *(int16_t *)(tile->ymin + (uintptr_t)tile);
	}

	return ground;
}

float cdFindGroundInIntTile(struct geotilei *tile, float x, float z)
{
	int i = 1;
	int ival = -1;
	struct geotilei *tile2 = tile;

	if (tile->header.numvertices >= 4) {
		while (i < tile->header.numvertices) {
			float tmpz = tile2->vertices[i][2];
			float tmpx = tile2->vertices[i][0];

			float fval = ((tile->vertices[0][2] - tmpz) * (x - tmpx))
				- ((tile2->vertices[0][0] - tmpx) * (0, z - tmpz));

			if (fval != 0) {
				if (ival < 0) {
					ival = (fval > 0);
				} else if (ival != 0 && fval < 0) {
					i--;
					break;
				} else if (ival == 0 && fval > 0) {
					i--;
					break;
				}
			}

			i++;
		}
	}

	return cdFindGroundInIntTileAtVertex(tile, x, z, i);
}

float cdFindGroundInFltTile(struct geotilef *tile, float x, float z)
{
	struct coord sp24;
	struct coord sp18;
	struct coord sp0c;
	float tmp;
	float ground;

	sp24.x = tile->vertices[1].x - tile->vertices[0].x;
	sp24.y = tile->vertices[1].y - tile->vertices[0].y;
	sp24.z = tile->vertices[1].z - tile->vertices[0].z;

	sp18.x = tile->vertices[2].x - tile->vertices[0].x;
	sp18.y = tile->vertices[2].y - tile->vertices[0].y;
	sp18.z = tile->vertices[2].z - tile->vertices[0].z;

	sp0c.x = sp24.f[1] * sp18.f[2] - sp24.f[2] * sp18.f[1];
	sp0c.y = sp24.f[2] * sp18.f[0] - sp24.f[0] * sp18.f[2];
	sp0c.z = sp24.f[0] * sp18.f[1] - sp24.f[1] * sp18.f[0];

	tmp = sp0c.f[0] * tile->vertices[0].f[0]
		+ sp0c.f[1] * tile->vertices[0].f[1]
		+ sp0c.f[2] * tile->vertices[0].f[2];

	if (sp0c.f[1] == 0) {
		return tile->vertices[tile->ymax].y;
	}

	ground = (tmp - (double)x * (double)sp0c.f[0] - (double)z * (double)sp0c.f[2]) / (double)sp0c.f[1];

	if (ground > tile->vertices[tile->ymax].y) {
		ground = tile->vertices[tile->ymax].y;
	} else if (ground < tile->vertices[tile->ymin].y) {
		ground = tile->vertices[tile->ymin].y;
	}

	return ground;
}

bool cdIs2dPointInIntTile(struct geotilei *tile, float x, float z)
{
	int result = -1;
	int numvertices = tile->header.numvertices;
	int i;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		float value = ((float)tile->vertices[next][2] - (float)tile->vertices[i][2]) * (x - tile->vertices[i][0])
			- ((float)tile->vertices[next][0] - (float)tile->vertices[i][0]) * (z - tile->vertices[i][2]);

		if (value != 0) {
			if (i == 0 || result < 0) {
				result = (value > 0);
			} else {
				if (result != 0 && value < 0) {
					return false;
				}

				if (result == 0 && value > 0) {
					return false;
				}
			}
		}
	}

	if (result < 0) {
		return false;
	}

	return true;
}

bool cdIs2dPointInFltTile(struct geotilef *tile, float x, float z)
{
	int result = -1;
	int numvertices = tile->header.numvertices;
	int i;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		float value = (tile->vertices[next].z - tile->vertices[i].z) * (x - tile->vertices[i].x)
			- (tile->vertices[next].x - tile->vertices[i].x) * (z - tile->vertices[i].z);

		if (value != 0) {
			if (i == 0 || result < 0) {
				result = (value > 0);
			} else {
				if (result != 0 && value < 0) {
					return false;
				}

				if (result == 0 && value > 0) {
					return false;
				}
			}
		}
	}

	if (result < 0) {
		return false;
	}

	return true;
}

bool cdIs2dPointInBlock(struct geoblock *tile, float x, float z)
{
	int result = -1;
	int numvertices = tile->header.numvertices;
	int i;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		float value = (tile->vertices[next][1] - tile->vertices[i][1]) * (x - tile->vertices[i][0])
			- (tile->vertices[next][0] - tile->vertices[i][0]) * (z - tile->vertices[i][1]);

		if (value != 0) {
			if (i == 0 || result < 0) {
				result = (value > 0);
			} else {
				if (result != 0 && value < 0) {
					return false;
				}

				if (result == 0 && value > 0) {
					return false;
				}
			}
		}
	}

	if (result < 0) {
		return false;
	}

	return true;
}

bool cdIs2dPointInCyl(struct geocyl *cyl, float x, float z)
{
	float xdiff = x - cyl->x;
	float zdiff = z - cyl->z;

	return xdiff * xdiff + zdiff * zdiff <= cyl->radius * cyl->radius;
}

bool cd000266a4(float x, float z, struct geo *geo)
{
	if (geo == NULL) {
		return false;
	}

	if (geo->type == GEOTYPE_BLOCK) {
		return cdIs2dPointInBlock((struct geoblock *) geo, x, z);
	}

	if (geo->type == GEOTYPE_CYL) {
		return cdIs2dPointInCyl((struct geocyl *) geo, x, z);
	}

	return false;
}

/**
 * For a lift or escalator step, find the props which are riding on it.
 */
void cdGetPropsOnPlatform(struct prop *platform, int16_t *propnums, int maxlen)
{
	uint8_t *start;
	uint8_t *end;
	int16_t roompropnums[257];
	struct prop *prop;
	int16_t *roompropnumptr;
	struct geo *geo;
	int len = 0;

	if (propUpdateGeometry(platform, &start, &end)) {
		roomGetProps(platform->rooms, roompropnums, 256);
		roompropnumptr = roompropnums;

		while (*roompropnumptr >= 0) {
			prop = &g_Vars.props[*roompropnumptr];

			if (prop != platform) {
				geo = (struct geo *) start;

				while (geo < (struct geo *) end) {
					if (geo->type == GEOTYPE_TILE_I) {
						struct geotilei *tile = (struct geotilei *) geo;
						geo = (struct geo *)((uintptr_t)geo + sizeof(struct geotilei) + sizeof(tile->vertices[0]) * (tile->header.numvertices - ARRAYCOUNT(tile->vertices)));
					} else if (geo->type == GEOTYPE_TILE_F) {
						struct geotilef *tile = (struct geotilef *) geo;
						struct coord *pos = &prop->pos;

						if ((geo->flags & (GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2))
								&& pos->x >= tile->vertices[tile->xmin].x
								&& pos->x <= tile->vertices[tile->xmax].x
								&& pos->z >= tile->vertices[tile->zmin].z
								&& pos->z <= tile->vertices[tile->zmax].z
								&& pos->y >= tile->vertices[tile->ymin].y
								&& cdIs2dPointInFltTile(tile, pos->x, pos->z)
								&& pos->y >= cdFindGroundInFltTile(tile, pos->x, pos->z)) {
							break;
						}

						geo = (struct geo *)((uintptr_t)geo + sizeof(struct geotilef) + sizeof(struct coord) * (tile->header.numvertices - ARRAYCOUNT(tile->vertices)));
					} else if (geo->type == GEOTYPE_BLOCK) {
						geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
					} else if (geo->type == GEOTYPE_CYL) {
						geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
					}
				}

				if (geo < (struct geo *) end) {
					if (len < maxlen - 2) {
						propnums[len] = *roompropnumptr;
						len++;
					} else {
						break;
					}
				}
			}

			roompropnumptr++;
		}
	}

	propnums[len] = -1;
}

bool cd00026a04(struct coord *pos, uint8_t *start, uint8_t *end, int16_t geoflags, int room, struct geo **tileptr, int *roomptr, float *groundptr, bool ceiling)
{
	bool result = false;
	struct geo *geo = (struct geo *) start;

	if (room);

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;

			if ((geo->flags & geoflags)
					&& pos->x >= *(int16_t *)(tile->xmin + (uintptr_t)tile)
					&& pos->x <= *(int16_t *)(tile->xmax + (uintptr_t)tile)
					&& pos->z >= *(int16_t *)(tile->zmin + (uintptr_t)tile)
					&& pos->z <= *(int16_t *)(tile->zmax + (uintptr_t)tile)) {
				if ((!ceiling && pos->y >= *(int16_t *)(tile->ymin + (uintptr_t)tile))
						|| (ceiling && pos->y <= *(int16_t *)(tile->ymax + (uintptr_t)tile))) {
					if (cdIs2dPointInIntTile(tile, pos->x, pos->z)) {
						float ground = cdFindGroundInIntTile(tile, pos->x, pos->z);

						if ((!ceiling && ground <= pos->y && ground > *groundptr)
								|| (ceiling && ground >= pos->y && ground < *groundptr)) {
							*groundptr = ground;
							*tileptr = geo;
							*roomptr = room;
							result = true;
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geotilei) + sizeof(tile->vertices[0]) * (tile->header.numvertices - ARRAYCOUNT(tile->vertices)));
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;

			if ((geo->flags & geoflags)
					&& pos->x >= tile->vertices[tile->xmin].x
					&& pos->x <= tile->vertices[tile->xmax].x
					&& pos->z >= tile->vertices[tile->zmin].z
					&& pos->z <= tile->vertices[tile->zmax].z) {
				if ((!ceiling && pos->y >= tile->vertices[tile->ymin].y)
						|| (ceiling && pos->y <= tile->vertices[tile->ymax].y)) {
					if (cdIs2dPointInFltTile(tile, pos->x, pos->z)) {
						float ground = cdFindGroundInFltTile(tile, pos->x, pos->z);

						if ((!ceiling && pos->y >= ground && ground > *groundptr)
								|| (ceiling && pos->y <= ground && ground < *groundptr)) {
							*groundptr = ground;
							*tileptr = geo;
							*roomptr = room;
							result = true;
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geotilef) + sizeof(struct coord) * (tile->header.numvertices - ARRAYCOUNT(tile->vertices)));
		} else if (geo->type == GEOTYPE_BLOCK) {
			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
		}
	}

	return result;
}

void cdFindClosestVertical(struct coord *pos, RoomNum *rooms, int16_t geoflags, struct geo **geoptr, RoomNum *roomptr, float *groundptr, struct prop **propptr, bool ceiling)
{
	RoomNum *roomptr2;
	int roomnum;
	uint8_t *start;
	uint8_t *end;
	float closesty;
	struct geo *geo = NULL;
	int room = 0;
	struct prop *bestprop = NULL;
	int16_t *propnumptr;
	int16_t propnums[256];

	if (ceiling) {
		closesty = 4294967296;
	} else {
		closesty = -4294967296;
	}

	roomptr2 = rooms;
	roomnum = rooms[0];

	while (roomnum != -1) {
		if (roomnum < g_TileNumRooms) {
			start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
			end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

			cd00026a04(pos, start, end, geoflags, roomnum, &geo, &room, &closesty, ceiling);
		}

		roomptr2++;
		roomnum = *roomptr2;
	}

	roomGetProps(rooms, propnums, 256);
	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (propUpdateGeometry(prop, &start, &end)
				&& cd00026a04(pos, start, end, geoflags, prop->rooms[0], &geo, &room, &closesty, ceiling)) {
			bestprop = prop;
		}

		propnumptr++;
	}

	*geoptr = geo;
	*roomptr = room;
	*groundptr = closesty;

	if (propptr != NULL) {
		*propptr = bestprop;
	}
}

bool cd0002709cIntTile(struct geotilei *tile, float x, float z, float radius, struct prop *prop, struct collision *collision)
{
	bool result = false;

	if (cdIs2dPointInIntTile(tile, x, z)) {
		collision->geo = &tile->header;
		collision->vertexindex = 0;
		collision->prop = prop;
		result = true;
	} else {
		int numvertices = tile->header.numvertices;
		int i;

		for (i = 0; i < numvertices; i++) {
			int next = (i + 1) % numvertices;
			float value = cd00025654(tile->vertices[i][0], tile->vertices[i][2], tile->vertices[next][0], tile->vertices[next][2], x, z);

			if (value < 0) {
				value = -value;
			}

			if (value <= radius
					&& (cdGetDistanceXZ(tile->vertices[i][0], tile->vertices[i][2], x, z) <= radius
						|| cdGetDistanceXZ(tile->vertices[next][0], tile->vertices[next][2], x, z) <= radius
						|| cdIsPointBetweenXZ(tile->vertices[i][0], tile->vertices[i][2], tile->vertices[next][0], tile->vertices[next][2], x, z))) {
				collision->geo = &tile->header;
				collision->vertexindex = i;
				collision->prop = prop;
				result = true;
				break;
			}
		}
	}

	return result;
}

bool cd000272f8FltTile(struct geotilef *tile, float x, float z, float radius, struct prop *prop, struct collision *collision)
{
	bool result = false;

	if (cdIs2dPointInFltTile(tile, x, z)) {
		collision->geo = &tile->header;
		collision->vertexindex = 0;
		collision->prop = prop;
		result = true;
	} else {
		int numvertices = tile->header.numvertices;
		int i;

		for (i = 0; i < numvertices; i++) {
			int next = (i + 1) % numvertices;
			float value = cd00025654(tile->vertices[i].x, tile->vertices[i].z, tile->vertices[next].x, tile->vertices[next].z, x, z);

			if (value < 0) {
				value = -value;
			}

			if (value <= radius
					&& (cdGetDistanceXZ(tile->vertices[i].x, tile->vertices[i].z, x, z) <= radius
						|| cdGetDistanceXZ(tile->vertices[next].x, tile->vertices[next].z, x, z) <= radius
						|| cdIsPointBetweenXZ(tile->vertices[i].x, tile->vertices[i].z, tile->vertices[next].x, tile->vertices[next].z, x, z))) {
				collision->geo = &tile->header;
				collision->vertexindex = i;
				collision->prop = prop;
				result = true;
				break;
			}
		}
	}

	return result;
}

int cd000274e0Block(struct geoblock *tile, float x, float z, float radius, struct prop *prop, struct collision *collision)
{
	bool result = false;

	if (cdIs2dPointInBlock(tile, x, z)) {
		if (collision) {
			collision->geo = &tile->header;
			collision->vertexindex = 0;
			collision->prop = prop;
		}

		result = true;
	} else {
		int numvertices = tile->header.numvertices;
		int i;

		for (i = 0; i < numvertices; i++) {
			int next = (i + 1) % numvertices;
			float value = cd00025654(tile->vertices[i][0], tile->vertices[i][1],
					tile->vertices[next][0], tile->vertices[next][1],
					x, z);

			if (value < 0) {
				value = -value;
			}

			if (value <= radius
					&& (cdGetDistanceXZ(tile->vertices[i][0], tile->vertices[i][1], x, z) <= radius
						|| cdGetDistanceXZ(tile->vertices[next][0], tile->vertices[next][1], x, z) <= radius
						|| cdIsPointBetweenXZ(tile->vertices[i][0], tile->vertices[i][1], tile->vertices[next][0], tile->vertices[next][1], x, z))) {
				if (collision) {
					collision->geo = &tile->header;
					collision->vertexindex = i;
					collision->prop = prop;
				}

				result = true;
				break;
			}
		}
	}

	return result;
}

bool cd000276c8Cyl(struct geocyl *cyl, float x, float z, float radius, struct prop *prop, struct collision *collision)
{
	bool result = false;

	float sumx = x - cyl->x;
	float sumz = z - cyl->z;
	float sumwidth = cyl->radius + radius;

	if (sumx * sumx + sumz * sumz <= sumwidth * sumwidth) {
		result = true;

		if (collision) {
			collision->geo = &cyl->header;
			collision->vertexindex = 0;
			collision->prop = prop;
		}
	}

	return result;
}

int cdTestRampWall(struct geotilei *tile, struct coord *pos, float width, float y1, float y2);

void cdCollectGeoForCylFromList(struct coord *pos, float radius, uint8_t *start, uint8_t *end, int16_t geoflags,
		bool checkvertical, float arg6, float arg7, struct prop *prop,
		struct collision *collisions, int maxcollisions, int *numcollisions, int roomnum)
{
	struct geo *geo = (struct geo *) start;
	int result;

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;

			if ((geo->flags & geoflags)
					&& pos->x >= *(int16_t *)(tile->xmin + (uintptr_t)tile) - radius
					&& pos->x <= *(int16_t *)(tile->xmax + (uintptr_t)tile) + radius
					&& pos->z >= *(int16_t *)(tile->zmin + (uintptr_t)tile) - radius
					&& pos->z <= *(int16_t *)(tile->zmax + (uintptr_t)tile) + radius
					&& (!checkvertical || (pos->y + arg6 >= *(int16_t *)(tile->ymin + (uintptr_t)tile)
							&& pos->y + arg7 <= *(int16_t *)(tile->ymax + (uintptr_t)tile)))) {
				if (geo->flags & GEOFLAG_RAMPWALL) {
					result = cdTestRampWall(tile, pos, radius, pos->y + arg7, pos->y + arg6);
				} else {
					result = 1;
				}

				if (result != 0) {
					if (cd0002709cIntTile(tile, pos->x, pos->z, radius, prop, &collisions[*numcollisions])) {
						collisions[*numcollisions].room = roomnum;
						*numcollisions = *numcollisions + 1;

						if (*numcollisions >= maxcollisions) {
							break;
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 6 + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;
			int tmp = 0x40;

			if ((geo->flags & geoflags)
					&& pos->x >= *(float *)((uintptr_t)tile + tile->xmin * 0xc + 0x10) - radius
					&& pos->x <= *(float *)((uintptr_t)tile + tile->xmax * 0xc + 0x10) + radius
					&& pos->z >= *(float *)((uintptr_t)tile + tile->zmin * 0xc + 0x18) - radius
					&& pos->z <= *(float *)((uintptr_t)tile + tile->zmax * 0xc + 0x18) + radius
					&& (!checkvertical || (pos->y + arg6 >= *(float*)((uintptr_t)tile + tile->ymin * 0xc + 0x14)
							&& pos->y + arg7 <= *(float *)((uintptr_t)tile + tile->ymax * 0xc + 0x14)))) {
				result = cd000272f8FltTile(tile, pos->x, pos->z, radius, prop, &collisions[*numcollisions]);

				if (result != 0) {
					collisions[*numcollisions].room = roomnum;
					*numcollisions = *numcollisions + 1;

					if (*numcollisions >= maxcollisions) {
						break;
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + (tile->header.numvertices - tmp) * 0xc + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *block = (struct geoblock *) geo;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& (!checkvertical || (pos->y + arg6 >= block->ymin && pos->y + arg7 <= block->ymax))) {
				result = cd000274e0Block(block, pos->x, pos->z, radius, prop, &collisions[*numcollisions]);

				if (result) {
					collisions[*numcollisions].room = roomnum;
					*numcollisions = *numcollisions + 1;

					if (*numcollisions >= maxcollisions) {
						break;
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + 0x4c);
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) geo;

			if ((geoflags & geo->flags)
					&& (!checkvertical || (pos->y + arg6 >= cyl->ymin && pos->y + arg7 <= cyl->ymax))) {
				result = cd000276c8Cyl(cyl, pos->x, pos->z, radius, prop, &collisions[*numcollisions]);

				if (result) {
					collisions[*numcollisions].room = roomnum;
					*numcollisions = *numcollisions + 1;

					if (*numcollisions >= maxcollisions) {
						break;
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + 0x18);
		}
	}
}

void cdCollectGeoForCyl(struct coord *pos, float radius, RoomNum *rooms, int32_t types, int16_t geoflags, bool checkvertical, float ymax, float ymin, struct collision *collisions, int maxcollisions)
{
	RoomNum *roomptr;
	int roomnum;
	uint8_t *start;
	uint8_t *end;
	int numcollisions = 0;
	int16_t *propnumptr;
	int16_t propnums[256];

	// Check BG
	if (types & CDTYPE_BG) {
		roomptr = rooms;
		roomnum = rooms[0];

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
				end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

				cdCollectGeoForCylFromList(pos, radius, start, end, geoflags, checkvertical, ymax, ymin, NULL, collisions, maxcollisions, &numcollisions, roomnum);

				if (numcollisions >= maxcollisions) {
					goto end;
				}
			}

			roomptr++;
			roomnum = *roomptr;
		}
	}

	// Check props
	roomGetProps(rooms, propnums, 256);
	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (propIsOfCdType(prop, types) && propUpdateGeometry(prop, &start, &end)) {
			cdCollectGeoForCylFromList(pos, radius, start, end, geoflags, checkvertical, ymax, ymin, prop, collisions, maxcollisions, &numcollisions, prop->rooms[0]);

			if (numcollisions >= maxcollisions) {
				break;
			}
		}

		propnumptr++;
	}

end:
	collisions[numcollisions].geo = NULL;
}

void cd00027f78(struct geotilei *tile, float arg1, float arg2, float arg3, struct prop *prop, struct collision *collisions, int maxcollisions, int *numcollisions)
{
	int i;
	int numvertices = tile->header.numvertices;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		if (tile->vertices[i][0] != tile->vertices[next][0] || tile->vertices[i][2] != tile->vertices[next][2]) {
			float f0 = cd00025654(tile->vertices[i][0], tile->vertices[i][2], tile->vertices[next][0], tile->vertices[next][2], arg1, arg2);

			if (f0 < 0.0f) {
				f0 = -f0;
			}

			if (f0 <= arg3
					&& (cdGetDistanceXZ(tile->vertices[i][0], tile->vertices[i][2], arg1, arg2) <= arg3
						|| cdGetDistanceXZ(tile->vertices[next][0], tile->vertices[next][2], arg1, arg2) <= arg3
						|| cdIsPointBetweenXZ(tile->vertices[i][0], tile->vertices[i][2], tile->vertices[next][0], tile->vertices[next][2], arg1, arg2))) {
				if (*numcollisions < maxcollisions) {
					collisions[*numcollisions].geo = &tile->header;
					collisions[*numcollisions].vertexindex = i;
					collisions[*numcollisions].prop = prop;
					*numcollisions += 1;
				} else {
					break;
				}
			}
		}
	}
}

/**
 * Triangular wall tiles are generally implemented with just a bounding box check,
 * but this means if the player gets on top of one they can walk across it.
 * This is a problem for ramps that the player can jump off at any height
 * such as the Air Base staircase near the baggage terminal.
 *
 * To handle this, the ramp wall tiles are flagged with GEOFLAG_RAMPWALL.
 * When collision checks are being done, tiles with this flag are passed to
 * this function which does a more extensive check.
 */
int cdTestRampWall(struct geotilei *tile, struct coord *pos, float width, float y1, float y2)
{
	int count;
	int i;
	int y1count;
	int y2count;
	int numverts;

	if (!g_Vars.enableslopes && (tile->header.flags & GEOFLAG_SLOPE)) {
		return 0;
	}

	numverts = tile->header.numvertices;
	y2count = 0;
	y1count = 0;
	count = 0;

	for (i = 0; i < numverts; i++) {
		int next = i + 1;
		int last = numverts - 1;
		float posval;
		float thisvals[2];
		float nextvals[2];
		float somefloat;
		float somefloat2;
		int xdiff;
		int zdiff;

		if (i == last) {
			next = 0;
		}

		xdiff = tile->vertices[next][0] - tile->vertices[i][0];
		zdiff = tile->vertices[next][2] - tile->vertices[i][2];

		if (xdiff < 0) {
			xdiff = -xdiff;
		}

		if (zdiff < 0) {
			zdiff = -zdiff;
		}

		if (xdiff || zdiff) {
			thisvals[1] = tile->vertices[i][1];
			nextvals[1] = tile->vertices[next][1];

			if (zdiff < xdiff) {
				thisvals[0] = tile->vertices[i][0];
				nextvals[0] = tile->vertices[next][0];
				posval = pos->x;
			} else {
				thisvals[0] = tile->vertices[i][2];
				nextvals[0] = tile->vertices[next][2];
				posval = pos->z;
			}

			somefloat = (posval - thisvals[0]) / (nextvals[0] - thisvals[0]);

			if (somefloat <= 1.0f && somefloat >= 0.0f) {
				somefloat2 = thisvals[1] + (nextvals[1] - thisvals[1]) * somefloat;

				if (somefloat2 >= y2 - 1.0f) {
					y2count++;

					if (y1count != 0) {
						count++;
					}
				} else if (somefloat2 <= y1 + 1.0f) {
					y1count++;

					if (y2count != 0) {
						count++;
					}
				} else {
					count++;
				}
			}
		}
	}

	return count;
}

void cd0002840c(struct geotilef *tile, float arg1, float arg2, float arg3, struct prop *prop, struct collision *collisions, int maxcollisions, int *numcollisions)
{
	int i;
	int numvertices = tile->header.numvertices;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		if (tile->vertices[i].x != tile->vertices[next].x || tile->vertices[i].z != tile->vertices[next].z) {
			float f0 = cd00025654(tile->vertices[i].x, tile->vertices[i].z, tile->vertices[next].x, tile->vertices[next].z, arg1, arg2);

			if (f0 < 0.0f) {
				f0 = -f0;
			}

			if (f0 <= arg3
					&& (cdGetDistanceXZ(tile->vertices[i].x, tile->vertices[i].z, arg1, arg2) <= arg3
						|| cdGetDistanceXZ(tile->vertices[next].x, tile->vertices[next].z, arg1, arg2) <= arg3
						|| cdIsPointBetweenXZ(tile->vertices[i].x, tile->vertices[i].z, tile->vertices[next].x, tile->vertices[next].z, arg1, arg2))) {
				if (*numcollisions < maxcollisions) {
					collisions[*numcollisions].geo = &tile->header;
					collisions[*numcollisions].vertexindex = i;
					collisions[*numcollisions].prop = prop;
					*numcollisions += 1;
				} else {
					break;
				}
			}
		}
	}
}

void cd00028638(struct geoblock *block, float arg1, float arg2, float arg3, struct prop *prop, struct collision *collisions, int maxcollisions, int *numcollisions)
{
	int i;
	int numvertices = block->header.numvertices;

	for (i = 0; i < numvertices; i++) {
		int next = (i + 1) % numvertices;

		if (block->vertices[i][0] != block->vertices[next][0] || block->vertices[i][1] != block->vertices[next][1]) {
			float f0 = cd00025654(block->vertices[i][0], block->vertices[i][1], block->vertices[next][0], block->vertices[next][1], arg1, arg2);

			if (f0 < 0.0f) {
				f0 = -f0;
			}

			if (f0 <= arg3
					&& (cdGetDistanceXZ(block->vertices[i][0], block->vertices[i][1], arg1, arg2) <= arg3
						|| cdGetDistanceXZ(block->vertices[next][0], block->vertices[next][1], arg1, arg2) <= arg3
						|| cdIsPointBetweenXZ(block->vertices[i][0], block->vertices[i][1], block->vertices[next][0], block->vertices[next][1], arg1, arg2))) {
				if (*numcollisions < maxcollisions) {
					collisions[*numcollisions].geo = &block->header;
					collisions[*numcollisions].vertexindex = i;
					collisions[*numcollisions].prop = prop;
					*numcollisions += 1;
				} else {
					break;
				}
			}
		}
	}
}

void cd0002885c(struct geocyl *cyl, float x, float z, float arg3, struct prop *prop, struct collision *collisions, int maxcollisions, int *numcollisions)
{
	float xdiff = x - cyl->x;
	float zdiff = z - cyl->z;
	float f16 = arg3 + cyl->radius;

	if (xdiff * xdiff + zdiff * zdiff <= f16 * f16) {
		if (*numcollisions < maxcollisions) {
			collisions[*numcollisions].geo = &cyl->header;
			collisions[*numcollisions].vertexindex = 0;
			collisions[*numcollisions].prop = prop;
			*numcollisions += 1;
		}
	}
}

void cdCollectGeoForCylMoveFromList(uint8_t *start, uint8_t *end, struct coord *pos, float radius, int16_t geoflags,
		bool checkvertical, float arg6, float arg7, struct prop *prop,
		struct collision *collisions, int maxcollisions, int *numcollisions)
{
	struct geo *geo = (struct geo *) start;

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;

			if (geo->flags & geoflags) {
				if (pos->x >= *(int16_t *)(tile->xmin + (uintptr_t)tile) - radius
						&& pos->x <= *(int16_t *)(tile->xmax + (uintptr_t)tile) + radius
						&& pos->z >= *(int16_t *)(tile->zmin + (uintptr_t)tile) - radius
						&& pos->z <= *(int16_t *)(tile->zmax + (uintptr_t)tile) + radius
						&& (!checkvertical || (pos->y + arg6 >= *(int16_t *)(tile->ymin + (uintptr_t)tile)
								&& pos->y + arg7 <= *(int16_t *)(tile->ymax + (uintptr_t)tile)))) {
					bool pass;

					if (geo->flags & GEOFLAG_RAMPWALL) {
						pass = cdTestRampWall(tile, pos, radius, pos->y + arg7, pos->y + arg6);
					} else {
						pass = true;
					}

					if (pass) {
						cd00027f78(tile, pos->x, pos->z, radius, prop, collisions, maxcollisions, numcollisions);
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 6 + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;

			if ((geo->flags & geoflags)
					&& pos->x >= tile->vertices[tile->xmin].x - radius
					&& pos->x <= tile->vertices[tile->xmax].x + radius
					&& pos->z >= tile->vertices[tile->zmin].z - radius
					&& pos->z <= tile->vertices[tile->zmax].z + radius
					&& (!checkvertical || (pos->y + arg6 >= tile->vertices[tile->ymin].y
							&& pos->y + arg7 <= tile->vertices[tile->ymax].y))) {
				cd0002840c(tile, pos->x, pos->z, radius, prop, collisions, maxcollisions, numcollisions);
			}

			geo = (struct geo *)((uintptr_t)geo + (uintptr_t)(tile->header.numvertices - 0x40) * 0xc + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *block = (struct geoblock *) geo;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& (!checkvertical || (pos->y + arg6 >= block->ymin && pos->y + arg7 <= block->ymax))) {
				cd00028638(block, pos->x, pos->z, radius, prop, collisions, maxcollisions, numcollisions);
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) geo;

			if ((geoflags & geo->flags)
					&& (!checkvertical || (pos->y + arg6 >= cyl->ymin && pos->y + arg7 <= cyl->ymax))) {
				cd0002885c(cyl, pos->x, pos->z, radius, prop, collisions, maxcollisions, numcollisions);
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
		}
	}
}

void cdCollectGeoForCylMove(struct coord *pos, float width, RoomNum *rooms, int32_t types, int16_t geoflags, bool checkvertical, float ymax, float ymin, struct collision *collisions, int maxcollisions)
{
	RoomNum *roomptr;
	int roomnum;
	uint8_t *start;
	uint8_t *end;
	int numcollisions = 0;
	int16_t *propnumptr;
	int16_t propnums[256];

	// Check BG
	if (types & CDTYPE_BG) {
		roomptr = rooms;
		roomnum = rooms[0];

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
				end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

				cdCollectGeoForCylMoveFromList(start, end, pos, width, geoflags, checkvertical, ymax, ymin, NULL, collisions, maxcollisions, &numcollisions);
			}

			roomptr++;
			roomnum = *roomptr;
		}
	}

	// Check props
	roomGetProps(rooms, propnums, 256);
	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (propIsOfCdType(prop, types) && propUpdateGeometry(prop, &start, &end)) {
			cdCollectGeoForCylMoveFromList(start, end, pos, width, geoflags, checkvertical, ymax, ymin, prop, collisions, maxcollisions, &numcollisions);
		}

		propnumptr++;
	}

	collisions[numcollisions].geo = NULL;
}

void cd0002901c(struct coord *pos, struct coord *dist, float width, struct collision *collisions)
{
	int i;
	struct widthxz spf8;
	struct xz spf0;
	struct xz spe8;
	struct xz spe0;
	float bestvalue = 0.0f;
	int bestindex = -1;
	int32_t stack;
	float value;
	int curr;
	int next;
	struct coord vtx1;
	struct coord vtx2;
	struct geo *geo;

	for (i = 0; (geo = collisions[i].geo) != NULL; i++) {
		if (1);
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;

			spf8.width = width;
			spf8.x = pos->x;
			spf8.z = pos->z;

			spe0.x = dist->x;
			spe0.z = dist->z;

			curr = collisions[i].vertexindex;
			next = (curr + 1) % tile->header.numvertices;

			spf0.x = tile->vertices[curr][0];
			spf0.z = tile->vertices[curr][2];

			spe8.x = tile->vertices[next][0];
			spe8.z = tile->vertices[next][2];

			value = getSlideTimeToEdgeXZ(&spf8, &spf0, &spe8, &spe0);

			if (bestindex < 0 || value < bestvalue) {
				bestvalue = value;
				bestindex = i;
			}
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;

			spf8.width = width;
			spf8.x = pos->x;
			spf8.z = pos->z;

			spe0.x = dist->x;
			spe0.z = dist->z;

			curr = collisions[i].vertexindex;
			next = (curr + 1) % tile->header.numvertices;

			spf0.x = tile->vertices[curr].x;
			spf0.z = tile->vertices[curr].z;

			spe8.x = tile->vertices[next].x;
			spe8.z = tile->vertices[next].z;

			value = getSlideTimeToEdgeXZ(&spf8, &spf0, &spe8, &spe0);

			if (bestindex < 0 || value < bestvalue) {
				bestvalue = value;
				bestindex = i;
			}
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *block = (struct geoblock *) geo;

			spf8.width = width;
			spf8.x = pos->x;
			spf8.z = pos->z;

			spe0.x = dist->x;
			spe0.z = dist->z;

			curr = collisions[i].vertexindex;
			next = (curr + 1) % block->header.numvertices;

			spf0.x = block->vertices[curr][0];
			spf0.z = block->vertices[curr][1];

			spe8.x = block->vertices[next][0];
			spe8.z = block->vertices[next][1];

			value = getSlideTimeToEdgeXZ(&spf8, &spf0, &spe8, &spe0);

			if (bestindex < 0 || value < bestvalue) {
				bestvalue = value;
				bestindex = i;
			}
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) geo;

			spf8.width = cyl->radius + width;
			spf8.x = pos->x;
			spf8.z = pos->z;

			spe0.x = dist->x;
			spe0.z = dist->z;

			spf0.x = cyl->x;
			spf0.z = cyl->z;

			spe8.x = cyl->x;
			spe8.z = cyl->z;

			value = getSlideTimeToEdgeXZ(&spf8, &spf0, &spe8, &spe0);

			if (bestindex < 0 || value < bestvalue) {
				bestvalue = value;
				bestindex = i;
			}
		}
	}

	if (collisions[bestindex].geo->type == GEOTYPE_TILE_I) {
		struct geotilei *tile = (struct geotilei *) collisions[bestindex].geo;
		int curr = collisions[bestindex].vertexindex;
		int next = (curr + 1) % tile->header.numvertices;

		vtx1.x = tile->vertices[curr][0];
		vtx1.y = tile->vertices[curr][1];
		vtx1.z = tile->vertices[curr][2];

		vtx2.x = tile->vertices[next][0];
		vtx2.y = tile->vertices[next][1];
		vtx2.z = tile->vertices[next][2];
	} else if (collisions[bestindex].geo->type == GEOTYPE_TILE_F) {
		struct geotilef *tile = (struct geotilef *) collisions[bestindex].geo;
		int curr = collisions[bestindex].vertexindex;
		int next = (curr + 1) % tile->header.numvertices;

		vtx1.x = tile->vertices[curr].x;
		vtx1.y = tile->vertices[curr].y;
		vtx1.z = tile->vertices[curr].z;

		vtx2.x = tile->vertices[next].x;
		vtx2.y = tile->vertices[next].y;
		vtx2.z = tile->vertices[next].z;
	} else if (collisions[bestindex].geo->type == GEOTYPE_BLOCK) {
		struct geoblock *block = (struct geoblock *) collisions[bestindex].geo;
		int curr = collisions[bestindex].vertexindex;
		int next = (curr + 1) % block->header.numvertices;

		vtx1.x = block->vertices[curr][0];
		vtx1.y = pos->y;
		vtx1.z = block->vertices[curr][1];

		vtx2.x = block->vertices[next][0];
		vtx2.y = pos->y;
		vtx2.z = block->vertices[next][1];
	} else if (collisions[bestindex].geo->type == GEOTYPE_CYL) {
		struct geocyl *cyl = (struct geocyl *) collisions[bestindex].geo;

		cd00025848(cyl->x, cyl->z, cyl->radius, pos->x, pos->z, &vtx1.x, &vtx1.z, &vtx2.x, &vtx2.z);

		vtx1.y = pos->y;
		vtx2.y = pos->y;
	}

	cdSetObstacleVtxPropFlt(&vtx1, &vtx2, collisions[bestindex].prop, bestvalue);
}

float cdFindGroundFromList(struct collision *collisions, struct coord *pos, struct collision **collisionptr, float width)
{
	struct collision *collision;
	int i;
	float curground = -4294967296;
	bool hasground = false;
	bool anyintile = false;
	bool hasflag0100 = false;
	bool hasdie = false;
	bool hasgroundfromearlier;
	bool isdie;
	float nextvalue;
	float spe4;
	float f30;
	float x;
	float z;
	float spd4;
	float f14;
	float spb4;
	float thisvalue;
	int32_t unused2;
	int next;
	int numvertices;
	float spb8;
	int32_t unused3[8];
	float sp94;
	int32_t unused4[6];
	float sp78;
	float sp74;
	float nextx;
	float nextz;
	float ground;
	int32_t unused5[5];
	float thisx;
	float thisz;

	*collisionptr = NULL;

	for (collision = collisions; collision->geo != NULL; collision++) {
		if (collision->geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) collision->geo;

			if (tile->header.flags & GEOFLAG_DIE) {
				collision->intile = false;
			} else {
				if (tile->header.flags & GEOFLAG_SLOPE) {
					hasflag0100 = true;
				}

				collision->intile = cdIs2dPointInIntTile(tile, pos->x, pos->z);

				if (collision->intile) {
					anyintile = true;
				}
			}
		} else if (collision->geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) collision->geo;

			collision->intile = cdIs2dPointInFltTile(tile, pos->x, pos->z);

			if (collision->intile) {
				anyintile = true;
			}
		}
	}

	if (anyintile) {
		for (collision = collisions; collision->geo != NULL; collision++) {
			if (collision->intile) {
				if (collision->geo->type == GEOTYPE_TILE_I) {
					struct geotilei *tile = (struct geotilei *) collision->geo;

					if ((tile->header.flags & GEOFLAG_STEP) == 0) {
						ground = cdFindGroundInIntTile((void *)collision->geo, pos->x, pos->z);

						if (ground >= curground && ground < pos->y) {
							curground = ground;
							*collisionptr = collision;
							hasground = true;
						}
					}
				} else if (collision->geo->type == GEOTYPE_TILE_F) {
					struct geotilef *tile = (struct geotilef *) collision->geo;

					ground = cdFindGroundInFltTile((void *)collision->geo, pos->x, pos->z);

					if (ground >= curground && ground < pos->y) {
						curground = ground;
						*collisionptr = collision;
						hasground = true;
					}
				}
			}
		}

		for (collision = collisions; collision->geo != NULL; collision++) {
			if (collision->intile) {
				if (collision->geo->type == GEOTYPE_TILE_I) {
					struct geotilei *tile = (struct geotilei *) collision->geo;

					if (tile->header.flags & GEOFLAG_STEP) {
						ground = cdFindGroundInIntTile((void *)collision->geo, pos->x, pos->z);

						if (ground >= curground && (ground < pos->y || !hasground)) {
							curground = ground;
							*collisionptr = collision;
							hasground = true;
						}
					}
				}
			}
		}
	}

	hasgroundfromearlier = hasground;

	if (!hasground || hasflag0100) {
		spe4 = 4294967296.0f;

		for (collision = collisions; collision->geo != NULL; collision++) {
			if (collision->intile == false
					&& (!hasgroundfromearlier || (collision->geo->type == GEOTYPE_TILE_I && (collision->geo->flags & GEOFLAG_SLOPE))))
			{
				if (collision->geo->type == GEOTYPE_TILE_I) {
					struct geotilei *tile = (struct geotilei *) collision->geo;
					numvertices = tile->header.numvertices;
					isdie = (tile->header.flags & GEOFLAG_DIE) != 0;

					if (!isdie || !hasground)
					{
						for (i = 0; i < numvertices; i++) {
							thisx = tile->vertices[i][0];
							thisz = tile->vertices[i][2];

							next = (i + 1) % numvertices;

							nextx = tile->vertices[next][0];
							nextz = tile->vertices[next][2];

							spd4 = cd00025654(thisx, thisz, nextx, nextz, pos->x, pos->z);
							f30 = spd4;

							if (f30 < 0.0f) {
								f30 = -f30;
							}

							if (f30 < spe4 || hasdie)
							{
								if (cdIsPointBetweenXZ(thisx, thisz, nextx, nextz, pos->x, pos->z)) {
									spb8 = nextx - thisx;
									spb4 = nextz - thisz;
									f14 = spd4 / sqrtf(spb8 * spb8 + spb4 * spb4);
									x = pos->x + f14 * -spb4;
									z = pos->z + f14 * spb8;

									ground = cdFindGroundInIntTileAtVertex(tile, x, z, i);

									if (ground < pos->y || (collision->geo->flags & GEOFLAG_STEP)) {
										curground = ground;
										*collisionptr = collision;
										spe4 = f30;
										hasground = true;
										hasdie = isdie;
									}
								} else {
									thisvalue = cdGetDistanceXZ(thisx, thisz, pos->x, pos->z);
									nextvalue = cdGetDistanceXZ(nextx, nextz, pos->x, pos->z);

									if (thisvalue < nextvalue) {
										if (thisvalue < spe4 || hasdie)
										{
											x = tile->vertices[i][0];
											z = tile->vertices[i][2];
											ground = cdFindGroundInIntTileAtVertex(tile, x, z, i);

											if (ground < pos->y || (collision->geo->flags & GEOFLAG_STEP)) {
												curground = ground;
												*collisionptr = collision;
												spe4 = thisvalue;
												hasground = true;
												hasdie = isdie;
											}
										}
									} else {
										if (nextvalue < spe4 || hasdie)
										{
											x = tile->vertices[next][0];
											z = tile->vertices[next][2];
											ground = cdFindGroundInIntTileAtVertex(tile, x, z, i);

											if (ground < pos->y || (collision->geo->flags & GEOFLAG_STEP)) {
												curground = ground;
												*collisionptr = collision;
												spe4 = nextvalue;
												hasground = true;
												hasdie = isdie;
											}
										}
									}
								}
							}
						}
					}
				} else if (collision->geo->type == GEOTYPE_TILE_F) {
					struct geotilef *tile = (struct geotilef *) collision->geo;
					int numvertices = tile->header.numvertices;
					int i;

					for (i = 0; i < numvertices; i++) {
						thisx = tile->vertices[i].x;
						thisz = tile->vertices[i].z;

						next = (i + 1) % numvertices;

						nextx = tile->vertices[next].x;
						nextz = tile->vertices[next].z;

						sp94 = cd00025654(thisx, thisz, nextx, nextz, pos->x, pos->z);
						f30 = sp94;

						if (f30 < 0.0f) {
							f30 = -f30;
						}

						if (f30 < spe4) {
							if (cdIsPointBetweenXZ(thisx, thisz, nextx, nextz, pos->x, pos->z)) {
								sp78 = nextx - thisx;
								sp74 = nextz - thisz;
								f14 = sp94 / sqrtf(sp78 * sp78 + sp74 * sp74);
								x = pos->x + f14 * -sp74;
								z = pos->z + f14 * sp78;

								ground = cdFindGroundInFltTile(tile, x, z);

								if (ground < pos->y) {
									curground = ground;
									*collisionptr = collision;
									spe4 = f30;
									hasground = true;
									hasdie = false;
								}
							} else {
								thisvalue = cdGetDistanceXZ(thisx, thisz, pos->x, pos->z);
								nextvalue = cdGetDistanceXZ(nextx, nextz, pos->x, pos->z);

								if (thisvalue < nextvalue) {
									if (thisvalue < spe4) {
										x = tile->vertices[i].x;
										z = tile->vertices[i].z;
										ground = cdFindGroundInFltTile(tile, x, z);

										if (ground < pos->y) {
											curground = ground;
											*collisionptr = collision;
											spe4 = thisvalue;
											hasground = true;
											hasdie = false;
										}
									}
								} else {
									if (nextvalue < spe4) {
										x = tile->vertices[next].x;
										z = tile->vertices[next].z;
										ground = cdFindGroundInFltTile(tile, x, z);

										if (ground < pos->y) {
											curground = ground;
											*collisionptr = collision;
											spe4 = nextvalue;
											hasground = true;
											hasdie = false;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return curground;
}

/**
 * Test if the given cylinder is intersecting a tile with the given geoflags.
 * If so, populate the laddernormal argument and return true.
 *
 * There is nothing specific to ladders in this function, but it's only used
 * for finding ladders.
 */
bool cdFindLadder(struct coord *pos, float width, float ymax, float ymin, RoomNum *rooms, int16_t geoflags, struct coord *laddernormal)
{
	struct collision collisions[2];

	cdCollectGeoForCyl(pos, width, rooms, CDTYPE_BG, geoflags, CHECKVERTICAL_YES, ymax, ymin, collisions, 1);

	if (collisions[0].geo) {
		struct geotilei *tile = (struct geotilei *) collisions[0].geo;
		struct coord dist;

		cdGetGeoNormal(collisions[0].geo, laddernormal);

		dist.x = pos->x - tile->vertices[0][0];
		dist.y = pos->y - tile->vertices[0][1];
		dist.z = pos->z - tile->vertices[0][2];

		if (dist.f[0] * laddernormal->f[0] + dist.f[1] * laddernormal->f[1] + dist.f[2] * laddernormal->f[2] < 0) {
			laddernormal->x = -laddernormal->x;
			laddernormal->y = -laddernormal->y;
			laddernormal->z = -laddernormal->z;
		}

		return true;
	}

	return false;
}

bool cd0002a13c(struct coord *pos, float radius, float ymax, float ymin, RoomNum *rooms, int16_t geoflags)
{
	struct collision collisions[2];

	cdCollectGeoForCyl(pos, radius, rooms, CDTYPE_BG, geoflags, CHECKVERTICAL_YES, ymax, ymin, collisions, 1);

	if (collisions[0].geo) {
		return true;
	}

	return false;
}

float cdFindGroundInfoAtCyl(struct coord *pos, float radius, RoomNum *rooms, int16_t *floorcol,
		uint8_t *floortype, int16_t *floorflags, RoomNum *floorroom, bool *inlift, struct prop **lift)
{
	struct collision collisions[21];
	struct collision *sp72 = NULL;
	float ground;
	struct geo *geo = NULL;

	cdCollectGeoForCyl(pos, radius, rooms, CDTYPE_ALL, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, CHECKVERTICAL_NO, 0, 0, collisions, 20);
	ground = cdFindGroundFromList(collisions, pos, &sp72, radius);

	if (sp72) {
		geo = sp72->geo;
	}

	if (floorcol) {
		cdGetFloorCol(geo, floorcol);
	}

	if (floortype) {
		cdGetFloorType(geo, floortype);
	}

	if (floorflags && geo) {
		*floorflags = geo->flags;
	}

	if (floorroom) {
		if (sp72) {
			*floorroom = sp72->room;
		} else {
			*floorroom = -1;
		}
	}

	if (inlift) {
		if (geo && geo->type == GEOTYPE_TILE_F && (geo->flags & GEOFLAG_LIFTFLOOR)) {
			*inlift = true;
			*lift = sp72->prop;

			if (*lift && (*lift)->obj->modelnum == MODEL_ESCA_STEP && floortype) {
				*floortype = FLOORTYPE_METAL;
			}
		} else {
			*inlift = false;
			*lift = NULL;
		}
	}

	return ground;
}

float cdFindGroundAtCyl(struct coord *pos, float radius, RoomNum *rooms, int16_t *floorcol, uint8_t *floortype)
{
	return cdFindGroundInfoAtCyl(pos, radius, rooms, floorcol, floortype, NULL, NULL, false, NULL);
}

float cdFindFloorYColourTypeAtPos(struct coord *pos, RoomNum *rooms, int16_t *floorcol, uint8_t *floortype)
{
	struct geo *geo;
	RoomNum sp30[2];
	float sp2c;
	float result = -4294967296;

	cdFindClosestVertical(pos, rooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &sp30[1], &sp2c, NULL, SURFACE_FLOOR);

	if (geo) {
		result = sp2c;
	}

	if (floorcol) {
		cdGetFloorCol(geo, floorcol);
	}

	if (floortype) {
		cdGetFloorType(geo, floortype);
	}

	return result;
}

int cdFindFloorRoomAtPos(struct coord *pos, RoomNum *nearrooms)
{
	struct geo *geo;
	RoomNum room;
	float sp2c;

	cdFindClosestVertical(pos, nearrooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &room, &sp2c, 0, SURFACE_FLOOR);

	return room;
}

RoomNum cdFindFloorRoomYColourFlagsAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcolptr, int16_t *flagsptr)
{
	struct geo *geo;
	RoomNum room;
	float sp2c;

	cdFindClosestVertical(pos, rooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &room, &sp2c, NULL, SURFACE_FLOOR);

	if (geo != NULL) {
		*arg2 = sp2c;
	}

	if (floorcolptr != NULL) {
		cdGetFloorCol(geo, floorcolptr);
	}

	if (flagsptr != NULL && geo != NULL) {
		*flagsptr = geo->flags;
	}

	return room;
}

RoomNum cdFindCeilingRoomYColourFlagsAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcolptr, int16_t *flagsptr)
{
	struct geo *geo;
	RoomNum sp32;
	float sp2c;

	cdFindClosestVertical(pos, rooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &sp32, &sp2c, NULL, SURFACE_CEILING);

	if (geo != NULL) {
		*arg2 = sp2c;
	}

	if (floorcolptr != NULL) {
		cdGetFloorCol(geo, floorcolptr);
	}

	if (flagsptr != NULL && geo != NULL) {
		*flagsptr = geo->flags;
	}

	return sp32;
}

RoomNum cdFindFloorRoomYColourNormalPropAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcol, struct coord *normal, struct prop **propptr)
{
	struct geo *geo;
	RoomNum room;
	float sp2c;

	cdFindClosestVertical(pos, rooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &room, &sp2c, propptr, SURFACE_FLOOR);

	if (geo) {
		*arg2 = sp2c;
		cdGetGeoNormal(geo, normal);
	}

	if (floorcol) {
		cdGetFloorCol(geo, floorcol);
	}

	return room;
}

RoomNum cdFindCeilingRoomYColourFlagsNormalAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcol, int16_t *flagsptr, struct coord *normal)
{
	struct geo *geo;
	RoomNum sp32;
	float sp2c;

	cdFindClosestVertical(pos, rooms, GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2, &geo, &sp32, &sp2c, NULL, SURFACE_CEILING);

	if (geo) {
		*arg2 = sp2c;
		cdGetGeoNormal(geo, normal);
	}

	if (floorcol) {
		cdGetFloorCol(geo, floorcol);
	}

	if (flagsptr != NULL && geo != NULL) {
		*flagsptr = geo->flags;
	}

	return sp32;
}

/**
 * Tests if a cylinder volume fits in the given position.
 */
int cdTestVolume(struct coord *pos, float width, RoomNum *rooms, int types, bool checkvertical, float ymax, float ymin)
{
	struct collision collisions[2];
	bool result = true;

	cdCollectGeoForCyl(pos, width, rooms, types, GEOFLAG_WALL, checkvertical, ymax, ymin, collisions, 1);

	if (collisions[0].geo) {
		result = false;
		cdSetObstacleProp(collisions[0].prop);
	}

	return result;
}

int cdExamCylMove01(struct coord *pos, struct coord *pos2, float radius, RoomNum *rooms, int types, bool checkvertical, float ymax, float ymin)
{
	struct collision collisions[2];
	int cdresult;
	struct coord sp70;
	struct coord sp64;

	cdresult = CDRESULT_NOCOLLISION;

	cdCollectGeoForCyl(pos2, radius, rooms, types, GEOFLAG_WALL, checkvertical, ymax, ymin, collisions, 1);

	if (collisions[0].geo != NULL) {
		cdresult = CDRESULT_COLLISION;

		if (collisions[0].geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) collisions[0].geo;
			int this = collisions[0].vertexindex;
			int next = (this + 1) % tile->header.numvertices;

			sp70.x = tile->vertices[this][0];
			sp70.y = tile->vertices[this][1];
			sp70.z = tile->vertices[this][2];

			sp64.x = tile->vertices[next][0];
			sp64.y = tile->vertices[next][1];
			sp64.z = tile->vertices[next][2];
		} else if (collisions[0].geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) collisions[0].geo;
			int this = collisions[0].vertexindex;
			int next = (this + 1) % tile->header.numvertices;

			sp70.x = tile->vertices[this].x;
			sp70.y = tile->vertices[this].y;
			sp70.z = tile->vertices[this].z;

			sp64.x = tile->vertices[next].x;
			sp64.y = tile->vertices[next].y;
			sp64.z = tile->vertices[next].z;
		} else if (collisions[0].geo->type == GEOTYPE_BLOCK) {
			struct geoblock *block = (struct geoblock *) collisions[0].geo;
			int this = collisions[0].vertexindex;
			int next = (this + 1) % block->header.numvertices;

			sp70.x = block->vertices[this][0];
			sp70.y = pos->y;
			sp70.z = block->vertices[this][1];

			sp64.x = block->vertices[next][0];
			sp64.y = pos->y;
			sp64.z = block->vertices[next][1];
		} else if (collisions[0].geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) collisions[0].geo;

			cd00025848(cyl->x, cyl->z, cyl->radius, pos->x, pos->z, &sp70.x, &sp70.z, &sp64.x, &sp64.z);

			sp70.y = pos->y;
			sp64.y = pos->y;
		}

		cdSetObstacleVtxProp(&sp70, &sp64, collisions[0].prop);
	}

	return cdresult;
}

int cdExamCylMove02(struct coord *origpos, struct coord *dstpos, float width, RoomNum *dstrooms, int types, bool checkvertical, float ymax, float ymin)
{
	struct collision collisions[21];
	struct coord dist;
	int result = CDRESULT_NOCOLLISION;

	cdCollectGeoForCylMove(dstpos, width, dstrooms, types, GEOFLAG_WALL, checkvertical, ymax, ymin, collisions, 20);

	if (collisions[0].geo) {
		result = CDRESULT_COLLISION;

		dist.x = dstpos->x - origpos->x;
		dist.y = dstpos->y - origpos->y;
		dist.z = dstpos->z - origpos->z;

		cd0002901c(origpos, &dist, width, collisions);
	}

	return result;
}

bool cd0002aac0IntTile(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geotilei *tile, struct coord *arg4, struct coord *arg5)
{
	int i;
	uint8_t numvertices = tile->header.numvertices;

	for (i = 2; i < numvertices; i++) {
		if (utilsIntersectTest1((struct vec3s16 *)&tile->vertices[0][0],
					(struct vec3s16 *)&tile->vertices[i - 1][0],
					(struct vec3s16 *)&tile->vertices[i][0],
					NULL, arg0, arg1, arg2, arg4, arg5)) {
			return true;
		}
	}

	return false;
}

bool cd0002ab98FltTile(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geotilef *tile, struct coord *arg4, struct coord *arg5)
{
	int i;
	uint8_t numvertices = tile->header.numvertices;

	for (i = 2; i < numvertices; i++) {
		if (utilsIntersectTest2(&tile->vertices[0], &tile->vertices[i - 1], &tile->vertices[i],
					NULL, arg0, arg1, arg2, arg4, arg5)) {
			return true;
		}
	}

	return false;
}

bool cd0002ac70IntTile(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geotilei *tile,
		struct coord *arg4, struct coord *arg5, struct coord *arg6, bool arg7, float arg8, float arg9)
{
	bool result = false;
	int i;
	float f0;
	int numvertices = tile->header.numvertices;
	int next;
	int spb8 = 1;
	float f22 = 1.0f;
	int spb0;
	float spac;
	float spa8;
	float ymax = *(int16_t *)(tile->ymax + (uintptr_t)tile);
	float ymin = *(int16_t *)(tile->ymin + (uintptr_t)tile);
	float spa0[2];
	float sp98[2];
	float sp90[2];
	float sp88[2];

	if (!arg7
			|| (arg0->y + arg8 >= ymin && arg1->y + arg9 <= ymax)
			|| (arg0->y + arg9 <= ymax && arg1->y + arg8 >= ymin)) {
		for (i = 0; i < numvertices; i++) {
			next = (i + 1) % numvertices;

			if (cd000254d8(arg0, arg1, tile->vertices[i][0], tile->vertices[i][2], tile->vertices[next][0], tile->vertices[next][2], &spb8)) {
				spa0[0] = arg0->x;
				spa0[1] = arg0->z;
				sp98[0] = arg1->x;
				sp98[1] = arg1->z;
				sp90[0] = tile->vertices[i][0];
				sp90[1] = tile->vertices[i][2];
				sp88[0] = tile->vertices[next][0];
				sp88[1] = tile->vertices[next][2];

				f0 = func0f1577f0(spa0, sp98, sp90, sp88);

				if (f0 < f22) {
					if (arg7) {
						spa8 = (arg1->y - arg0->y) * f0 + arg0->y;
						spac = spa8 + arg8;
						spa8 = spa8 + arg9;
					}

					if (!arg7 || (!(spa8 >= ymax) && !(spac <= ymin))) {
						result = true;
						f22 = f0;
						spb0 = i;
					}
				}
			}
		}

		if (result) {
			arg4->x = arg0->x + arg2->f[0] * f22;
			arg4->y = arg0->y + arg2->f[1] * f22;
			arg4->z = arg0->z + arg2->f[2] * f22;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = tile->vertices[spb0][0];
				arg5->y = arg4->y;
				arg5->z = tile->vertices[spb0][2];

				arg6->x = tile->vertices[(spb0 + 1) % numvertices][0];
				arg6->y = arg4->y;
				arg6->z = tile->vertices[(spb0 + 1) % numvertices][2];
			}
		} else if (!result && spb8) {
			result = true;

			arg4->x = arg0->x;
			arg4->y = arg0->y;
			arg4->z = arg0->z;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = arg0->x;
				arg5->y = arg0->y;
				arg5->z = arg0->z;

				arg6->x = arg0->x;
				arg6->y = arg0->y;
				arg6->z = arg0->z;
			}
		}
	}

	return result;
}

bool cd0002b128FltTile(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geotilef *tile,
		struct coord *arg4, struct coord *arg5, struct coord *arg6, bool arg7, float arg8, float arg9)
{
	bool result = false;
	int i;
	float f0;
	int numvertices = tile->header.numvertices;
	int next;
	int spb8 = 1;
	float f22 = 1.0f;
	int spb0;
	float spac;
	float spa8;
	float ymax = tile->vertices[tile->ymax].y;
	float ymin = tile->vertices[tile->ymin].y;
	float spa0[2];
	float sp98[2];
	float sp90[2];
	float sp88[2];

	if (!arg7
			|| (arg0->y + arg8 >= ymin && arg1->y + arg9 <= ymax)
			|| (arg0->y + arg9 <= ymax && arg1->y + arg8 >= ymin)) {
		for (i = 0; i < numvertices; i++) {
			next = (i + 1) % numvertices;

			if (cd000254d8(arg0, arg1, tile->vertices[i].x, tile->vertices[i].z, tile->vertices[next].x, tile->vertices[next].z, &spb8)) {
				spa0[0] = arg0->x;
				spa0[1] = arg0->z;
				sp98[0] = arg1->x;
				sp98[1] = arg1->z;
				sp90[0] = tile->vertices[i].x;
				sp90[1] = tile->vertices[i].z;
				sp88[0] = tile->vertices[next].x;
				sp88[1] = tile->vertices[next].z;

				f0 = func0f1577f0(spa0, sp98, sp90, sp88);

				if (f0 < f22) {
					if (arg7) {
						spa8 = (arg1->y - arg0->y) * f0 + arg0->y;
						spac = spa8 + arg8;
						spa8 = spa8 + arg9;
					}

					if (!arg7 || (!(spa8 >= ymax) && !(spac <= ymin))) {
						result = true;
						f22 = f0;
						spb0 = i;
					}
				}
			}
		}

		if (result) {
			arg4->x = arg0->x + arg2->f[0] * f22;
			arg4->y = arg0->y + arg2->f[1] * f22;
			arg4->z = arg0->z + arg2->f[2] * f22;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = tile->vertices[spb0].x;
				arg5->y = arg4->y;
				arg5->z = tile->vertices[spb0].z;

				arg6->x = tile->vertices[(spb0 + 1) % numvertices].x;
				arg6->y = arg4->y;
				arg6->z = tile->vertices[(spb0 + 1) % numvertices].z;
			}
		} else if (!result && spb8) {
			result = true;

			arg4->x = arg0->x;
			arg4->y = arg0->y;
			arg4->z = arg0->z;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = arg0->x;
				arg5->y = arg0->y;
				arg5->z = arg0->z;

				arg6->x = arg0->x;
				arg6->y = arg0->y;
				arg6->z = arg0->z;
			}
		}
	}

	return result;
}

bool cd0002b560Block(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geoblock *block,
		struct coord *arg4, struct coord *arg5, struct coord *arg6, bool arg7, float arg8, float arg9)
{
	bool result = false;
	int i;
	float f0;
	int numvertices = block->header.numvertices;
	int next;
	int spb8 = 1;
	float f22 = 1.0f;
	int spb0;
	float spac;
	float spa8;
	float spa0[2];
	float sp98[2];
	float sp90[2];
	float sp88[2];

	if (!arg7
			|| (arg0->y + arg8 >= block->ymin && arg1->y + arg9 <= block->ymax)
			|| (arg0->y + arg9 <= block->ymax && arg1->y + arg8 >= block->ymin)) {
		for (i = 0; i < numvertices; i++) {
			next = (i + 1) % numvertices;

			if (cd000254d8(arg0, arg1, block->vertices[i][0], block->vertices[i][1], block->vertices[next][0], block->vertices[next][1], &spb8)) {
				spa0[0] = arg0->x;
				spa0[1] = arg0->z;
				sp98[0] = arg1->x;
				sp98[1] = arg1->z;
				sp90[0] = block->vertices[i][0];
				sp90[1] = block->vertices[i][1];
				sp88[0] = block->vertices[next][0];
				sp88[1] = block->vertices[next][1];

				f0 = func0f1577f0(spa0, sp98, sp90, sp88);

				if (f0 < f22) {
					if (arg7) {
						spa8 = (arg1->y - arg0->y) * f0 + arg0->y;
						spac = spa8 + arg8;
						spa8 = spa8 + arg9;
					}

					if (!arg7 || (!(spa8 >= block->ymax) && !(spac <= block->ymin))) {
						result = true;
						f22 = f0;
						spb0 = i;
					}
				}
			}
		}

		if (result) {
			arg4->x = arg0->x + arg2->f[0] * f22;
			arg4->y = arg0->y + arg2->f[1] * f22;
			arg4->z = arg0->z + arg2->f[2] * f22;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = block->vertices[spb0][0];
				arg5->y = arg4->y;
				arg5->z = block->vertices[spb0][1];

				arg6->x = block->vertices[(spb0 + 1) % numvertices][0];
				arg6->y = arg4->y;
				arg6->z = block->vertices[(spb0 + 1) % numvertices][1];
			}
		} else if (!result && spb8) {
			result = true;

			arg4->x = arg0->x;
			arg4->y = arg0->y;
			arg4->z = arg0->z;

			if (arg5 != NULL && arg6 != NULL) {
				arg5->x = arg0->x;
				arg5->y = arg0->y;
				arg5->z = arg0->z;

				arg6->x = arg0->x;
				arg6->y = arg0->y;
				arg6->z = arg0->z;
			}
		}
	}

	return result;
}

bool cd0002b954Cyl(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct geocyl *cyl,
		struct coord *arg4, struct coord *arg5, struct coord *arg6, bool arg7, float arg8, float arg9)
{
	bool result = false;
	float mult;
	float sp74;
	float x = cyl->x;
	float z = cyl->z;
	float radius = cyl->radius;

	if (!arg7
			|| (arg0->y + arg8 >= cyl->ymin && arg1->y + arg9 <= cyl->ymax)
			|| (arg0->y + arg9 <= cyl->ymax && arg1->y + arg8 >= cyl->ymin)) {
		sp74 = cd00025654(arg0->x, arg0->z, arg1->x, arg1->z, x, z);

		if (sp74 < 0.0f) {
			sp74 = -sp74;
		}

		if (sp74 < radius
				&& (cdGetDistanceXZ(arg0->x, arg0->z, x, z) < radius
					|| cdGetDistanceXZ(arg1->x, arg1->z, x, z) < radius
					|| cdIsPointBetweenXZ(arg0->x, arg0->z, arg1->x, arg1->z, x, z))) {
			float xdiff = arg1->x - arg0->x;
			float zdiff = arg1->z - arg0->z;
			float sqdist;
			float dist;
			float sp50;
			float sp4c;
			float sp48;

			sp50 = sqrtf(xdiff * xdiff + zdiff * zdiff);

			if (sp50 > 0.0f) {
				xdiff = x - arg0->x;
				zdiff = z - arg0->z;

				sqdist = xdiff * xdiff + zdiff * zdiff;

				if (sp74 * sp74 <= sqdist) {
					dist = sqrtf(sqdist - sp74 * sp74) - sqrtf(radius * radius - sp74 * sp74);
				} else {
					dist = 0.0f;
				}

				mult = dist / sp50;
			} else {
				mult = 0.0f;
			}

			if (mult < 1.0f) {
				if (arg7) {
					sp48 = (arg1->y - arg0->y) * mult + arg0->y;
					sp4c = sp48 + arg8;
					sp48 = sp48 + arg9;
				}

				if (!arg7 || (!(sp48 >= cyl->ymax) && !(sp4c <= cyl->ymin))) {
					result = true;

					arg4->x = arg0->x + arg2->f[0] * mult;
					arg4->y = arg0->y + arg2->f[1] * mult;
					arg4->z = arg0->z + arg2->f[2] * mult;

					if (arg5 != NULL && arg6 != NULL) {
						cd00025848(x, z, radius, arg0->x, arg0->z, &arg5->x, &arg5->z, &arg6->x, &arg6->z);

						arg5->y = arg4->y;
						arg6->y = arg4->y;
					}
				}
			}
		}
	}

	return result;
}

bool cdTestAToBGeolist(uint8_t *start, uint8_t *end, struct coord *arg2, struct coord *arg3, struct coord *arg4, int16_t geoflags, bool checkvertical, int arg7, float arg8, float arg9)
{
	struct geo *geo = (struct geo *) start;

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;
			struct coord min;
			struct coord max;
			struct coord spc4;
			struct coord spb8;

			if (tile->header.flags & geoflags) {
				min.x = *(int16_t *)(tile->xmin + (uintptr_t)tile);

				if ((!(arg2->x < min.x)) || !(arg3->x < min.x)) {
					max.x = *(int16_t *)(tile->xmax + (uintptr_t)tile);

					if ((!(arg2->x > max.x)) || !(arg3->x > max.x)) {
						min.z = *(int16_t *)(tile->zmin + (uintptr_t)tile);

						if ((!(arg2->z < min.z)) || !(arg3->z < min.z)) {
							max.z = *(int16_t *)(tile->zmax + (uintptr_t)tile);

							if ((!(arg2->z > max.z)) || !(arg3->z > max.z)) {
								if (1);
								if (checkvertical) {
									min.y = *(int16_t *)(tile->ymin + (uintptr_t)tile);
									max.y = *(int16_t *)(tile->ymax + (uintptr_t)tile);

									if ((!(arg2->y < min.y) || !(arg3->y < min.y))
											&& (!(arg2->y > max.y) || !(arg3->y > max.y))
											&& bgTestLineIntersectsBbox(arg2, arg4, &min, &max)
											&& cd0002aac0IntTile(arg2, arg3, arg4, tile, &spc4, &spb8)) {
										return false;
									}
								} else if (cd0002ac70IntTile(arg2, arg3, arg4, tile, &spc4, 0, 0, arg7, arg8, arg9)) {
									return false;
								}
							}
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 6 + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;
			struct coord min;
			struct coord max;
			struct coord sp90;
			struct coord sp84;

			if (tile->header.flags & geoflags) {
				min.x = tile->vertices[tile->xmin].x;
				max.x = tile->vertices[tile->xmax].x;
				min.z = tile->vertices[tile->zmin].z;
				max.z = tile->vertices[tile->zmax].z;

				if (((!(arg2->x < min.x)) || !(arg3->x < min.x))
						&& (!(arg2->x > max.x) || !(arg3->x > max.x))
						&& ((!(arg2->z < min.z)) || !(arg3->z < min.z))
						&& (!(arg2->z > max.z) || !(arg3->z > max.z))) {
					if (checkvertical) {
						min.y = tile->vertices[tile->ymin].y;
						max.y = tile->vertices[tile->ymax].y;

						if ((!(arg2->y < min.y) || !(arg3->y < min.y))
								&& (!(arg2->y > max.y) || !(arg3->y > max.y))
								&& bgTestLineIntersectsBbox(arg2, arg4, &min, &max)
								&& cd0002ab98FltTile(arg2, arg3, arg4, tile, &sp90, &sp84)) {
							return false;
						}
					} else if (cd0002b128FltTile(arg2, arg3, arg4, tile, &sp90, 0, 0, arg7, arg8, arg9)) {
						return false;
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + (uintptr_t)(tile->header.numvertices - 0x40) * 0xc + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct coord sp78;
			struct geoblock *block = (struct geoblock *) geo;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& cd0002b560Block(arg2, arg3, arg4, block, &sp78, 0, 0, arg7, arg8, arg9)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			struct coord sp68;
			struct geocyl *cyl = (struct geocyl *) geo;

			if ((geoflags & cyl->header.flags)
					&& cd0002b954Cyl(arg2, arg3, arg4, cyl, &sp68, 0, 0, arg7, arg8, arg9)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
		}
	}

	return true;
}

void cd0002c328IntTile(struct geotilei *tile, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4)
{
	struct coord sp3c;
	int32_t stack[2];
	uint8_t numvertices;
	float max = 0.0f;
	float min = 0.0f;
	float dist;
	int i;

	if (arg2->x != 0.0f || arg2->z != 0.0f) {
		sp3c.x = arg2->z;
		sp3c.y = 0.0f;
		sp3c.z = -arg2->x;

		dist = sqrtf(sp3c.f[0] * sp3c.f[0] + sp3c.f[2] * sp3c.f[2]);

		if (dist > 0.0f) {
			sp3c.x *= 1.0f / dist;
			sp3c.z *= 1.0f / dist;
		} else {
			sp3c.x = 0.0f;
			sp3c.y = 0.0f;
			sp3c.z = 1.0f;
		}
	} else {
		sp3c.x = 0.0f;
		sp3c.y = 0.0f;
		sp3c.z = 1.0f;
	}

	numvertices = tile->header.numvertices;

	for (i = 0; i < numvertices; i++) {
		float xdiff = tile->vertices[i][0] - arg1->x;
		float zdiff = tile->vertices[i][2] - arg1->z;
		float f0 = xdiff * sp3c.f[0] + zdiff * sp3c.f[2];

		if (f0 > max) {
			max = f0;
		} else if (f0 < min) {
			min = f0;
		}
	}

	arg3->x = arg1->x + sp3c.f[0] * max;
	arg3->y = arg1->y;
	arg3->z = arg1->z + sp3c.f[2] * max;

	arg4->x = arg1->x + sp3c.f[0] * min;
	arg4->y = arg1->y;
	arg4->z = arg1->z + sp3c.f[2] * min;
}

void cd0002c528FltTile(struct geotilef *tile, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4)
{
	struct coord sp3c;
	int32_t stack[2];
	uint8_t numvertices;
	float max = 0.0f;
	float min = 0.0f;
	float dist;
	int i;

	if (arg2->x != 0.0f || arg2->z != 0.0f) {
		sp3c.x = arg2->z;
		sp3c.y = 0.0f;
		sp3c.z = -arg2->x;

		dist = sqrtf(sp3c.f[0] * sp3c.f[0] + sp3c.f[2] * sp3c.f[2]);

		if (dist > 0.0f) {
			sp3c.x *= 1.0f / dist;
			sp3c.z *= 1.0f / dist;
		} else {
			sp3c.x = 0.0f;
			sp3c.y = 0.0f;
			sp3c.z = 1.0f;
		}
	} else {
		sp3c.x = 0.0f;
		sp3c.y = 0.0f;
		sp3c.z = 1.0f;
	}

	numvertices = tile->header.numvertices;

	for (i = 0; i < numvertices; i++) {
		float xdiff = tile->vertices[i].x - arg1->x;
		float zdiff = tile->vertices[i].z - arg1->z;
		float f0 = xdiff * sp3c.f[0] + zdiff * sp3c.f[2];

		if (f0 > max) {
			max = f0;
		} else if (f0 < min) {
			min = f0;
		}
	}

	arg3->x = arg1->x + sp3c.f[0] * max;
	arg3->y = arg1->y;
	arg3->z = arg1->z + sp3c.f[2] * max;

	arg4->x = arg1->x + sp3c.f[0] * min;
	arg4->y = arg1->y;
	arg4->z = arg1->z + sp3c.f[2] * min;
}

bool cdExamAToBGeolist(uint8_t *start, uint8_t *end, struct coord *arg2, struct coord *arg3, struct coord *arg4,
		int16_t geoflags, bool checkvertical, int arg7, float ymax, float ymin, float *arg10, struct coord *arg11,
		struct coord *arg12, struct coord *arg13, struct geo **geoptr, int roomnum)
{
	struct geo *geo;
	float x;
	float y;
	float z;
	float sum;
	bool ok;
	bool result = false;

	geo = (struct geo *) start;

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;
			struct coord min;
			struct coord max;
			struct coord sp12c;
			struct coord sp120;
			struct coord sp114;
			struct coord sp108;

			if (geo->flags & GEOFLAG_RAMPWALL) {
				ok = cdTestRampWall(tile, arg2, 0, arg2->y + ymin, arg2->y + ymax);
			} else {
				ok = true;
			}

			if (ok && (geo->flags & geoflags)) {
				min.x = *(int16_t *)(tile->xmin + (uintptr_t)tile);

				if (!(arg2->x < min.x) || !(arg3->x < min.x)) {
					max.x = *(int16_t *)(tile->xmax + (uintptr_t)tile);

					if (!(arg2->x > max.x) || !(arg3->x > max.x)) {
						min.z = *(int16_t *)(tile->zmin + (uintptr_t)tile);

						if (!(arg2->z < min.z) || !(arg3->z < min.z)) {
							max.z = *(int16_t *)(tile->zmax + (uintptr_t)tile);

							if (!(arg2->z > max.z) || !(arg3->z > max.z)) {
								if (checkvertical) {
									min.y = *(int16_t *)(tile->ymin + (uintptr_t)tile);
									max.y = *(int16_t *)(tile->ymax + (uintptr_t)tile);

									if ((!(arg2->y < min.y) || !(arg3->y < min.y))
											&& (!(arg2->y > max.y) || !(arg3->y > max.y))
											&& bgTestLineIntersectsBbox(arg2, arg4, &min, &max)
											&& cd0002aac0IntTile(arg2, arg3, arg4, tile, &sp12c, &sp120)) {
										x = sp12c.x - arg2->x;
										y = sp12c.y - arg2->y;
										z = sp12c.z - arg2->z;

										sum = x * x + y * y + z * z;

										if (sum < *arg10) {
											result = true;
											*arg10 = sum;

											arg11->x = sp12c.x;
											arg11->y = sp12c.y;
											arg11->z = sp12c.z;

											cd0002c328IntTile(tile, &sp12c, &sp120, arg12, arg13);

											*geoptr = geo;
										}
									}
								} else if (cd0002ac70IntTile(arg2, arg3, arg4, tile, &sp12c, &sp114, &sp108, arg7, ymax, ymin)) {
									x = sp12c.x - arg2->x;
									y = sp12c.y - arg2->y;
									z = sp12c.z - arg2->z;

									sum = x * x + y * y + z * z;

									if (sum < *arg10) {
										result = true;
										*arg10 = sum;

										arg11->x = sp12c.x;
										arg11->y = sp12c.y;
										arg11->z = sp12c.z;

										arg12->x = sp114.x;
										arg12->y = sp114.y;
										arg12->z = sp114.z;

										arg13->x = sp108.x;
										arg13->y = sp108.y;
										arg13->z = sp108.z;

										*geoptr = geo;
									}
								}
							}
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + (uintptr_t)(tile->header.numvertices * 6) + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;
			struct coord min;
			struct coord max;
			struct coord spe0;
			struct coord spd4;
			struct coord spc8;
			struct coord spbc;

			if (geo->flags & geoflags) {
				min.x = tile->vertices[tile->xmin].x;
				max.x = tile->vertices[tile->xmax].x;
				min.z = tile->vertices[tile->zmin].z;
				max.z = tile->vertices[tile->zmax].z;

				if ((!(arg2->x < min.x) || !(arg3->x < min.x))
						&& (!(arg2->x > max.x) || !(arg3->x > max.x))
						&& (!(arg2->z < min.z) || !(arg3->z < min.z))
						&& (!(arg2->z > max.z) || !(arg3->z > max.z))) {
					if (checkvertical) {
						min.y = tile->vertices[tile->ymin].y;
						max.y = tile->vertices[tile->ymax].y;

						if ((!(arg2->y < min.y) || !(arg3->y < min.y))
								&& (!(arg2->y > max.y) || !(arg3->y > max.y))
								&& bgTestLineIntersectsBbox(arg2, arg4, &min, &max)
								&& cd0002ab98FltTile(arg2, arg3, arg4, tile, &spe0, &spd4)) {
							x = spe0.x - arg2->x;
							y = spe0.y - arg2->y;
							z = spe0.z - arg2->z;

							sum = x * x + y * y + z * z;

							if (sum < *arg10) {
								result = true;
								*arg10 = sum;

								arg11->x = spe0.x;
								arg11->y = spe0.y;
								arg11->z = spe0.z;

								cd0002c528FltTile(tile, &spe0, &spd4, arg12, arg13);

								*geoptr = geo;
							}
						}
					} else if (cd0002b128FltTile(arg2, arg3, arg4, tile, &spe0, &spc8, &spbc, arg7, ymax, ymin)) {
						x = spe0.x - arg2->x;
						y = spe0.y - arg2->y;
						z = spe0.z - arg2->z;

						sum = x * x + y * y + z * z;

						if (sum < *arg10) {
							result = true;
							*arg10 = sum;

							arg11->x = spe0.x;
							arg11->y = spe0.y;
							arg11->z = spe0.z;

							arg12->x = spc8.x;
							arg12->y = spc8.y;
							arg12->z = spc8.z;

							arg13->x = spbc.x;
							arg13->y = spbc.y;
							arg13->z = spbc.z;

							*geoptr = geo;
						}
					}
				}
			}

			geo = (struct geo *)((uintptr_t)geo + (uintptr_t)(tile->header.numvertices - 0x40) * 0xc + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct coord spb0;
			struct coord spa4;
			struct coord sp98;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& (cd0002b560Block(arg2, arg3, arg4, (struct geoblock *)geo, &spb0, &spa4, &sp98, arg7, ymax, ymin))) {
				x = spb0.x - arg2->x;
				y = spb0.y - arg2->y;
				z = spb0.z - arg2->z;

				sum = x * x + y * y + z * z;

				if (sum < *arg10) {
					result = true;
					*arg10 = sum;

					arg11->x = spb0.x;
					arg11->y = spb0.y;
					arg11->z = spb0.z;

					arg12->x = spa4.x;
					arg12->y = spa4.y;
					arg12->z = spa4.z;

					arg13->x = sp98.x;
					arg13->y = sp98.y;
					arg13->z = sp98.z;

					*geoptr = geo;
				}
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) geo;
			struct coord sp88;
			struct coord sp7c;
			struct coord sp70;

			if ((geoflags & geo->flags)
					&& cd0002b954Cyl(arg2, arg3, arg4, cyl, &sp88, &sp7c, &sp70, arg7, ymax, ymin)) {
				x = sp88.x - arg2->x;
				y = sp88.y - arg2->y;
				z = sp88.z - arg2->z;

				sum = x * x + y * y + z * z;

				if (sum < *arg10) {
					result = true;
					*arg10 = sum;

					arg11->x = sp88.x;
					arg11->y = sp88.y;
					arg11->z = sp88.z;

					arg12->x = sp7c.x;
					arg12->y = sp7c.y;
					arg12->z = sp7c.z;

					arg13->x = sp70.x;
					arg13->y = sp70.y;
					arg13->z = sp70.z;

					*geoptr = geo;
				}
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
		}
	}

	return !result;
}

bool cdTestAToB(struct coord *pos, struct coord *coord2, RoomNum *rooms, int32_t types, int16_t geoflags, bool checkvertical, int arg6, float ymax, float ymin)
{
	int roomnum;
	RoomNum *roomptr;
	uint8_t *start;
	uint8_t *end;
	struct coord sp27c;
	int16_t *propnumptr;
	int16_t propnums[256];

	sp27c.x = coord2->x - pos->x;
	sp27c.y = coord2->y - pos->y;
	sp27c.z = coord2->z - pos->z;

	if (types & CDTYPE_BG) {
		roomptr = rooms;
		roomnum = rooms[0];

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
				end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

				if (cdTestAToBGeolist(start, end, pos, coord2, &sp27c, geoflags, checkvertical, arg6, ymax, ymin) == 0) {
					cdSetObstacleProp(NULL);
					return false;
				}
			}

			roomptr++;
			roomnum = *roomptr;
		}
	}

	roomGetProps(rooms, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (propIsOfCdType(prop, types)
				&& propUpdateGeometry(prop, &start, &end)
				&& cdTestAToBGeolist(start, end, pos, coord2, &sp27c, geoflags, checkvertical, arg6, ymax, ymin) == 0) {
			cdSetObstacleProp(prop);
			return false;
		}

		propnumptr++;
	}

	return true;
}

int cdExamAToB(struct coord *from, struct coord *to, RoomNum *rooms, int types,
               int16_t geoflags, bool checkvertical, int flags, float ymax, float ymin)
{
	RoomNum roomnum;
	RoomNum *roomiter;
	uint8_t *geolistStart;
	uint8_t *geolistEnd;
	struct coord dir;                 // Direction vector from A to B
	bool collided = false;
	struct coord collisionNormal;
	struct coord hitPos;
	struct coord edgeDir;
	float closestDist = 4294967296.0f;
	struct geo *hitGeo = NULL;
	int16_t *propiter;
	int16_t propnums[256];

	// Compute direction vector from A to B
	dir.x = to->x - from->x;
	dir.y = to->y - from->y;
	dir.z = to->z - from->z;

	// --- Check against background geometry in each room ---
	if (types & CDTYPE_BG) {
		roomiter = rooms;
		roomnum = rooms[0];

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				int32_t *roomOffsets = &g_TileRooms[roomnum];
				geolistStart = g_TileFileData.uint8_t + roomOffsets[0];
				geolistEnd = g_TileFileData.uint8_t + roomOffsets[1];

				if (!cdExamAToBGeolist(geolistStart, geolistEnd, from, to, &dir,
						geoflags, checkvertical, flags, ymax, ymin,
						&closestDist, &collisionNormal, &hitPos, &edgeDir, &hitGeo, roomnum)) {
					collided = true;
					cdSetObstacleVtxColPropFltGeo(&hitPos, &edgeDir, &collisionNormal, NULL, closestDist, hitGeo);
				}
			}

			roomiter++;
			roomnum = *roomiter;
		}
	}

	// --- Check against dynamic props in the same rooms ---
	roomGetProps(rooms, propnums, ARRAYCOUNT(propnums));
	propiter = propnums;

	while (*propiter >= 0) {
		struct prop *prop = &g_Vars.props[*propiter];

		if (propIsOfCdType(prop, types)
				&& propUpdateGeometry(prop, &geolistStart, &geolistEnd)
				&& !cdExamAToBGeolist(geolistStart, geolistEnd, from, to, &dir,
					geoflags, checkvertical, flags, ymax, ymin,
					&closestDist, &collisionNormal, &hitPos, &edgeDir, &hitGeo, -999)) {
			collided = true;
			cdSetObstacleVtxColPropFltGeo(&hitPos, &edgeDir, &collisionNormal, prop, closestDist, hitGeo);
		}

		propiter++;
	}

	return !collided;
}


bool cdTestCylMove01(struct coord *pos, RoomNum *rooms, struct coord *targetpos, int32_t types, int32_t arg4, float ymax, float ymin)
{
	RoomNum sp44[21];
	RoomNum sp34[8];

	portalTraceLineThroughRooms(pos, targetpos, rooms, sp34, sp44, 20);

	return cdTestAToB(pos, targetpos, sp44, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg4, ymax, ymin);
}

int cdTestCylMove02(struct coord *pos, RoomNum *rooms, struct coord *coord2, RoomNum *rooms2, int32_t types, bool arg5, float ymax, float ymin)
{
	int result;
	RoomNum sp44[20];
	RoomNum sp34[8];

	propFindRoomsContainingNewPos(pos, rooms, coord2, sp34, sp44, 20);

	if (roomArrayIntersects(sp34, rooms2)) {
		result = cdTestAToB(pos, coord2, sp44, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg5, ymax, ymin);
	} else {
		result = CDRESULT_COLLISION;
	}

	return result;
}

int cdExamCylMove03(struct coord *pos, RoomNum *rooms, struct coord *arg2, int32_t types, int32_t arg4, float ymax, float ymin)
{
	RoomNum sp44[21];
	RoomNum sp34[8];

	portalTraceLineThroughRooms(pos, arg2, rooms, sp34, sp44, 20);

	return cdExamAToB(pos, arg2, sp44, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg4, ymax, ymin);
}

int cdTestCylMove04(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int arg5, float ymax, float ymin)
{
	RoomNum rooms[21];

	portalTraceLineThroughRooms(arg0, arg2, arg1, arg3, rooms, 20);

	return cdTestAToB(arg0, arg2, rooms, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg5, ymax, ymin);
}

int cdExamCylMove05(struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2, int types, bool arg5, float ymax, float ymin)
{
	RoomNum sp44[21];
	RoomNum sp34[8];
	int result;

	propFindRoomsContainingNewPos(pos, rooms, pos2, sp34, sp44, 20);

	result = cdExamAToB(pos, pos2, sp44, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg5, ymax, ymin);

	if (result != CDRESULT_COLLISION && !roomArrayIntersects(sp34, rooms2)) {
		cdClearResults();
		result = CDRESULT_ERROR;
	}

	return result;
}

int cdExamCylMove06(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, float width, int types, int arg6, float ymax, float ymin)
{
	RoomNum sp5c[21];
	RoomNum sp4c[8];
	struct coord sp40;
	int result;

	propFindRoomsContainingNewPos(arg0, arg1, arg2, sp4c, sp5c, 20);

	result = cdExamAToB(arg0, arg2, sp5c, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg6, ymax, ymin);

	if (result == CDRESULT_COLLISION) {
		sp40.x = arg2->x - arg0->x;
		sp40.y = arg2->y - arg0->y;
		sp40.z = arg2->z - arg0->z;

		cdComputeSlideTimeToEdgeXZ(arg0, &sp40, width);
	} else if (!roomArrayIntersects(sp4c, arg3)) {
		cdClearResults();
		result = CDRESULT_ERROR;
	}

	return result;
}

int cdExamCylMove07(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int arg5, float ymax, float ymin)
{
	RoomNum rooms[21];

	portalTraceLineThroughRooms(arg0, arg2, arg1, arg3, rooms, 20);

	return cdExamAToB(arg0, arg2, rooms, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg5, ymax, ymin);
}

int cdExamCylMove08(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, float width, int32_t types, int arg6, float ymax, float ymin)
{
	RoomNum rooms[21];
	struct coord sp40;
	int result;

	portalTraceLineThroughRooms(arg0, arg2, arg1, arg3, rooms, 20);

	result = cdExamAToB(arg0, arg2, rooms, types, GEOFLAG_WALL, CHECKVERTICAL_NO, arg6, ymax, ymin);

	if (result == CDRESULT_COLLISION) {
		sp40.x = arg2->x - arg0->x;
		sp40.y = arg2->y - arg0->y;
		sp40.z = arg2->z - arg0->z;

		cdComputeSlideTimeToEdgeXZ(arg0, &sp40, width);
	}

	return result;
}

bool cdTestLos03(struct coord *viewpos, RoomNum *rooms, struct coord *targetpos, int32_t types, int16_t geoflags)
{
	RoomNum sp44[21];
	RoomNum sp34[8];

	portalTraceLineThroughRooms(viewpos, targetpos, rooms, sp34, sp44, 20);

	return cdTestAToB(viewpos, targetpos, sp44, types, geoflags, CHECKVERTICAL_YES, 1, 0, 0);
}

bool cdTestLos04(struct coord *frompos, RoomNum *fromrooms, struct coord *topos, int types)
{
	return cdTestLos03(frompos, fromrooms, topos, types, GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT);
}

bool cdTestLos05(struct coord *coord, RoomNum *rooms, struct coord *coord2, RoomNum *rooms2, int types, int16_t geoflags)
{
	bool result;
	RoomNum sp44[20];
	RoomNum sp34[8];

	propFindRoomsContainingNewPos(coord, rooms, coord2, sp34, sp44, 20);

	if (roomArrayIntersects(sp34, rooms2)) {
		result = cdTestAToB(coord, coord2, sp44, types, geoflags, CHECKVERTICAL_YES, 1, 0, 0);
	} else {
		result = false;
	}

	return result;
}

bool cdTestLos06(struct coord *arg0, RoomNum *rooms1, struct coord *arg2, RoomNum *rooms2, int32_t types)
{
	return cdTestLos05(arg0, rooms1, arg2, rooms2, types, GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT);
}

bool cdTestLos07(struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2, RoomNum *rooms3, int32_t types, int16_t geoflags)
{
	bool result;
	RoomNum sp34[20];

	propFindRoomsContainingNewPos(pos, rooms, pos2, rooms3, sp34, 20);

	if (roomArrayIntersects(rooms3, rooms2)) {
		result = cdTestAToB(pos, pos2, sp34, types, geoflags, CHECKVERTICAL_YES, 1, 0, 0);
	} else {
		result = false;
	}

	return result;
}

int cdExamLos08(struct coord *pos, RoomNum *rooms, struct coord *pos2, int32_t types, int16_t geoflags)
{
	RoomNum sp44[21];
	RoomNum sp34[8];

	portalTraceLineThroughRooms(pos, pos2, rooms, sp34, sp44, 20);

	return cdExamAToB(pos, pos2, sp44, types, geoflags, CHECKVERTICAL_YES, 1, 0, 0);
}

int cdExamLos09(struct coord *pos, RoomNum *rooms, struct coord *pos2, int32_t types)
{
	return cdExamLos08(pos, rooms, pos2, types, GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT);
}

int cdTestLos10(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int16_t geoflags)
{
	RoomNum rooms[21];

	portalTraceLineThroughRooms(arg0, arg2, arg1, arg3, rooms, 20);

	return cdTestAToB(arg0, arg2, rooms, types, geoflags, CHECKVERTICAL_YES, 1, 0, 0);
}

int cdTestLos11(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types)
{
	return cdTestLos10(arg0, arg1, arg2, arg3, types, GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT);
}

bool cd0002ded8(struct coord *arg0, struct coord *arg1, struct prop *prop)
{
	uint8_t *start;
	uint8_t *end;
	struct coord sp7c;
	bool result = false;
	struct coord sp6c;
	struct coord sp60;
	struct coord sp54;
	float sp50 = 4294967296;
	struct geo *geo;

	sp7c.x = arg1->x - arg0->x;
	sp7c.y = arg1->y - arg0->y;
	sp7c.z = arg1->z - arg0->z;

	if (propUpdateGeometry(prop, &start, &end)) {
		if (!cdExamAToBGeolist(start, end, arg0, arg1, &sp7c,
					GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT,
					CHECKVERTICAL_YES, 1, 0, 0, &sp50, &sp6c, &sp60, &sp54, &geo, -999)) {
			result = true;
			cdSetObstacleVtxColPropFltGeo(&sp60, &sp54, &sp6c, prop, sp50, geo);
		}
	}

	return !result;
}

/**
 * Return true if both blocks are not intersecting on the X/Z plane.
 */
bool cdBlockExcludesBlockLaterally(struct geoblock *block1, struct geoblock *block2)
{
	int32_t stack[4];
	float zero = 0.0f;
	int numvertices0 = block1->header.numvertices;
	int numvertices1 = block2->header.numvertices;
	int i;

	for (i = 0; i < numvertices0; i++) {
		int next = (i + 1) % numvertices0;
		double diff1;
		double diff2;

		diff1 = block1->vertices[next][1] - (double)block1->vertices[i][1];
		diff2 = block1->vertices[i][0] - (double)block1->vertices[next][0];

		if (diff1 == zero && diff2 == zero) {
			if (cdIs2dPointInBlock(block2, block1->vertices[i][0], block1->vertices[i][1])) {
				return false;
			}
		} else {
			double sum1 = block1->vertices[i][0] * diff1 + block1->vertices[i][1] * diff2;
			double sum2;
			int j = (next + 1) % numvertices0;
			int k;

			while (j != i) {
				sum2 = block1->vertices[j][0] * diff1 + block1->vertices[j][1] * diff2;

				if (1);
				if (1);
				if (1);

				if (sum2 != sum1) {
					break;
				}

				j = (j + 1) % numvertices0;
			}

			for (k = 0; k < numvertices1; k++) {
				double sum3 = block2->vertices[k][0] * diff1 + block2->vertices[k][1] * diff2;

				if (sum2 == sum1) {
					sum2 = sum1 - sum3 + sum1;
				}

				if ((sum3 < sum1 && sum2 < sum1) || (sum3 > sum1 && sum2 > sum1)) {
					break;
				}
			}

			if (k == numvertices1) {
				return true;
			}
		}
	}

	return false;
}

int cdTestBlockOverlapsGeolist(uint8_t *start, uint8_t *end, struct geoblock *block, int16_t geoflags)
{
	struct geo *geo = (struct geo *) start;

	while (geo < (struct geo *) end) {
		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *) geo;
			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 6 + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *) geo;
			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 0xc + 0x10);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *thisblock = (struct geoblock *) geo;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& thisblock->ymax >= block->ymin
					&& thisblock->ymin <= block->ymax) {
				// Tiles are overlapping vertically
				int i;

				for (i = 0; i < block->header.numvertices; i++) {
					if (cdIs2dPointInBlock(thisblock, block->vertices[i][0], block->vertices[i][1])) {
						return CDRESULT_COLLISION;
					}
				}

				for (i = 0; i < thisblock->header.numvertices; i++) {
					if (cdIs2dPointInBlock(block, thisblock->vertices[i][0], thisblock->vertices[i][1])) {
						return CDRESULT_COLLISION;
					}
				}

				// This is a bit wasteful...
				// If A excludes B, there's no point checking if B excludes A.
				if (!cdBlockExcludesBlockLaterally(block, thisblock) && !cdBlockExcludesBlockLaterally(thisblock, block)) {
					return CDRESULT_COLLISION;
				}
			}

			geo = (struct geo *)((uintptr_t)geo + 0x4c);
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *) geo;

			if ((geoflags & geo->flags)
					&& cyl->ymax >= block->ymin
					&& cyl->ymin <= block->ymax
					&& cd000274e0Block(block, cyl->x, cyl->z, cyl->radius, NULL, NULL)) {
				return CDRESULT_COLLISION;
			}

			geo = (struct geo *)((uintptr_t)geo + 0x18);
		}
	}

	return CDRESULT_NOCOLLISION;
}

/**
 * Test if the given block overlaps any prop. Set the saved obstacle prop if so.
 *
 * The BG tests are pointless and not used, as cdTestBlockOverlapsGeolist only
 * tests blocks and cylinders.
 *
 * The function is used to check if a door is being blocked by another prop,
 * and is also used in a sanity check to make sure a moved object hasn't moved
 * into the player's position.
 */
int cdTestBlockOverlapsAnyProp(struct geoblock *geo, RoomNum *rooms, int32_t types)
{
	int result = CDRESULT_NOCOLLISION;
	int roomnum;
	uint8_t *start;
	uint8_t *end;
	RoomNum *roomptr;
	int16_t propnums[256];
	int16_t *propnumptr;

	// Check BG
	if (types & CDTYPE_BG) {
		roomptr = rooms;
		roomnum = rooms[0];

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
				end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

				result = cdTestBlockOverlapsGeolist(start, end, geo, GEOFLAG_WALL);

				if (result == CDRESULT_COLLISION) {
					cdSetObstacleProp(NULL);
					break;
				}
			}

			roomptr++;
			roomnum = *roomptr;
		}
	}

	// Check props
	if (result != CDRESULT_COLLISION) {
		roomGetProps(rooms, propnums, 256);
		propnumptr = propnums;

		while (*propnumptr >= 0) {
			struct prop *prop = &g_Vars.props[*propnumptr];

			if (propIsOfCdType(prop, types) && propUpdateGeometry(prop, &start, &end)) {
				result = cdTestBlockOverlapsGeolist(start, end, geo, GEOFLAG_WALL);

				if (result == CDRESULT_COLLISION) {
					cdSetObstacleProp(prop);
					break;
				}
			}

			propnumptr++;
		}
	}

	return result;
}

bool cd0002e680IntTile(struct geotilei *tile, int numvertices, struct coord *verts, struct coord *diffs, struct prop *prop, struct geoblock *block)
{
	bool result = false;
	int i;
	int next;
	int curr;
	struct coord sp84;
	struct coord sp78;
	struct coord sp6c;

	for (i = 0; i < numvertices; i++) {
		next = (i + 1) % numvertices;
		curr = i;

		if (cd0002ac70IntTile((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)),
					(struct coord *)((uintptr_t)verts + next * sizeof(struct coord)),
					(struct coord *)((uintptr_t)diffs + curr * sizeof(struct coord)),
					tile, &sp6c, &sp84, &sp78, 0, 0.0f, 0.0f)) {
			cdSetObstacleVtxColProp(&sp84, &sp78, &sp6c, prop);
			cdSetSavedPos((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)), (struct coord *)((uintptr_t)verts + next * sizeof(struct coord)));
			cdSetSavedBlock(block);
			result = true;
			break;
		}
	}

	return result;
}

bool cd0002e82cIntTile(struct geotilef *tile, int numvertices, struct coord *verts, struct coord *diffs, struct prop *prop, struct geoblock *block)
{
	bool result = false;
	int i;
	int next;
	int curr;
	struct coord sp84;
	struct coord sp78;
	struct coord sp6c;

	for (i = 0; i < numvertices; i++) {
		int remaining = numvertices - i;
		next = (remaining + numvertices - 2) % numvertices;
		curr = remaining - 1;

		if (cd0002b128FltTile((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)),
					(struct coord *)((uintptr_t)verts + next * sizeof(struct coord)),
					(struct coord *)((uintptr_t)diffs + curr * sizeof(struct coord)),
					tile, &sp6c, &sp84, &sp78, 0, 0.0f, 0.0f)) {
			cdSetObstacleVtxColProp(&sp84, &sp78, &sp6c, prop);
			cdSetSavedPos((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)), (struct coord *)((uintptr_t)verts + next * sizeof(struct coord)));
			cdSetSavedBlock(block);
			result = true;
			break;
		}
	}

	return result;
}

bool cd0002e9d8Block(struct geoblock *thisblock, int numvertices, struct coord *verts, struct coord *diffs, struct prop *prop, struct geoblock *block)
{
	bool result = false;
	int i;
	int next;
	int curr;
	struct coord sp84;
	struct coord sp78;
	struct coord sp6c;

	for (i = 0; i < numvertices; i++) {
		next = (i + 1) % numvertices;
		curr = i;

		if (cd0002b560Block((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)),
					(struct coord *)((uintptr_t)verts + next * sizeof(struct coord)),
					(struct coord *)((uintptr_t)diffs + curr * sizeof(struct coord)),
					thisblock, &sp6c, &sp84, &sp78, 0, 0.0f, 0.0f)) {
			cdSetObstacleVtxColProp(&sp84, &sp78, &sp6c, prop);
			cdSetSavedPos((struct coord *)((uintptr_t)verts + curr * sizeof(struct coord)), (struct coord *)((uintptr_t)verts + next * sizeof(struct coord)));
			cdSetSavedBlock(block);
			result = true;
			break;
		}
	}

	return result;
}

bool cd0002eb84Cyl(struct geocyl *cyl, int numvertices, struct coord *arg2, struct coord *arg3, struct prop *prop, struct geoblock *block)
{
	bool result = false;
	int i;
	int next;
	int curr;
	struct coord sp84;
	struct coord sp78;
	struct coord sp6c;

	for (i = 0; i < numvertices; i++) {
		int remaining = numvertices - i;
		next = (remaining + numvertices - 2) % numvertices;
		curr = remaining - 1;

		if (cd0002b954Cyl((struct coord *)((uintptr_t)arg2 + curr * sizeof(struct coord)),
					(struct coord *)((uintptr_t)arg2 + next * sizeof(struct coord)),
					(struct coord *)((uintptr_t)arg3 + curr * sizeof(struct coord)),
					cyl, &sp6c, &sp84, &sp78, 0, 0.0f, 0.0f)) {
			cdSetObstacleVtxColProp(&sp84, &sp78, &sp6c, prop);
			cdSetSavedPos((struct coord *)((uintptr_t)arg2 + curr * sizeof(struct coord)), (struct coord *)((uintptr_t)arg2 + next * sizeof(struct coord)));
			cdSetSavedBlock(block);
			result = true;
			break;
		}
	}

	return result;
}

bool cd0002ed30(uint8_t *start, uint8_t *end, struct geoblock *block, int numvertices, struct coord *verts, struct coord *diffs, int16_t geoflags, struct prop *prop)
{
	struct geo *geo = (struct geo *) start;

	while (geo < (struct geo *) end) {
		if (1);

		if (geo->type == GEOTYPE_TILE_I) {
			struct geotilei *tile = (struct geotilei *)geo;

			if ((geoflags & geo->flags)
					&& *(int16_t *)(tile->ymax + (uintptr_t)tile) >= block->ymin
					&& *(int16_t *)(tile->ymin + (uintptr_t)tile) <= block->ymax
					&& cd0002e680IntTile(tile, numvertices, verts, diffs, prop, block)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + tile->header.numvertices * 6 + 0xe);
		} else if (geo->type == GEOTYPE_TILE_F) {
			struct geotilef *tile = (struct geotilef *)geo;

			if ((geoflags & geo->flags)
					&& tile->vertices[tile->ymax].y >= block->ymin
					&& tile->vertices[tile->ymin].y <= block->ymax
					&& cd0002e82cIntTile(tile, numvertices, verts, diffs, prop, block)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + (uintptr_t)(tile->header.numvertices - 0x40) * 0xc + 0x310);
		} else if (geo->type == GEOTYPE_BLOCK) {
			struct geoblock *block2 = (struct geoblock *)geo;

			if ((geoflags & (GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT | GEOFLAG_BLOCK_SHOOT))
					&& block2->ymax >= block->ymin
					&& block2->ymin <= block->ymax
					&& cd0002e9d8Block(block2, numvertices, verts, diffs, prop, block)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geoblock));
		} else if (geo->type == GEOTYPE_CYL) {
			struct geocyl *cyl = (struct geocyl *)geo;

			if ((geoflags & geo->flags)
					&& cyl->ymax >= block->ymin
					&& cyl->ymin <= block->ymax
					&& cd0002eb84Cyl(cyl, numvertices, verts, diffs, prop, block)) {
				return false;
			}

			geo = (struct geo *)((uintptr_t)geo + sizeof(struct geocyl));
		}
	}

	return true;
}

bool cd0002f02c(struct geoblock *block, RoomNum *rooms, int types)
{
	int numvertices = block->header.numvertices;
	int i;
	uint8_t *start;
	uint8_t *end;
	int next;
	int16_t propnums[256];
	int16_t *propnumptr;
	bool result = true;
	struct coord verts[8];
	struct coord diffs[8];

	for (i = 0; i < numvertices; i++) {
		verts[i].x = block->vertices[i][0];
		verts[i].y = block->ymax;
		verts[i].z = block->vertices[i][1];
	}

	for (i = 0; i < numvertices; i++) {
		next = (i + 1) % numvertices;

		diffs[i].x = verts[next].x - verts[i].x;
		diffs[i].y = verts[next].y - verts[i].y;
		diffs[i].z = verts[next].z - verts[i].z;
	}

	if (types & CDTYPE_BG) {
		RoomNum *roomsptr = rooms;
		int roomnum = *roomsptr;

		while (roomnum != -1) {
			if (roomnum < g_TileNumRooms) {
				start = g_TileFileData.uint8_t + g_TileRooms[roomnum];
				end = g_TileFileData.uint8_t + g_TileRooms[roomnum + 1];

				result = cd0002ed30(start, end, block, numvertices, verts, diffs, GEOFLAG_WALL, NULL);

				if (!result) {
					break;
				}
			}

			roomsptr++;
			roomnum = *roomsptr;
		}
	}

	if (result) {
		roomGetProps(rooms, propnums, 256);

		propnumptr = propnums;

		while (*propnumptr >= 0) {
			struct prop *prop = &g_Vars.props[*propnumptr];

			if (propIsOfCdType(prop, types)) {
				if (propUpdateGeometry(prop, &start, &end)) {
					result = cd0002ed30(start, end, block, numvertices, verts, diffs, GEOFLAG_WALL, prop);

					if (!result) {
						break;
					}
				}
			}

			propnumptr++;
		}
	}

	return result;
}


Gfx *cdRender(Gfx *gdl, int32_t arg1, int32_t arg2, int32_t arg3)
{
	return gdl;
}

void cd0002f2fc(int32_t arg0, int32_t arg1)
{
	// empty
}

bool cdIsNearlyInSightWithFlags(struct coord *viewpos, RoomNum *rooms, struct coord *targetpos, float distance, int types, int16_t geoflags)
{
	struct coord diff;
	float x;
	float z;
	struct coord vector;

	if (cdTestLos03(viewpos, rooms, targetpos, types, geoflags)) {
		return true;
	}

	vector.x = targetpos->x - viewpos->x;
	vector.y = 0;
	vector.z = targetpos->z - viewpos->z;

	utilsNormalizeF(&vector.x, &vector.y, &vector.z);

	x = vector.f[0] * distance;
	z = vector.f[2] * distance;

	diff.x = targetpos->x - z;
	diff.y = targetpos->y;
	diff.z = targetpos->z + x;

	if (cdTestLos03(viewpos, rooms, &diff, types, geoflags)) {
		return true;
	}

	diff.x = targetpos->x + z;
	diff.y = targetpos->y;
	diff.z = targetpos->z - x;

	if (cdTestLos03(viewpos, rooms, &diff, types, geoflags)) {
		return true;
	}

	return false;
}

bool cdIsNearlyInSight(struct coord *viewpos, RoomNum *rooms, struct coord *targetpos, float distance, int types)
{
	return cdIsNearlyInSightWithFlags(viewpos, rooms, targetpos, distance, types, GEOFLAG_BLOCK_SIGHT);
}
