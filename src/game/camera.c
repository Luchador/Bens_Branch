#include <ultra64.h>
#include <math.h>
#include <string.h>
#include "constants.h"
#include "game/camera.h"
#include "game/tex.h"
#include "game/player.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "data.h"
#include "types.h"

struct coord g_CamFrustumTopNormal;
float g_CamFrustumTopOffset;
struct coord g_CamFrustumBottomNormal;
float g_CamFrustumBottomOffset;
struct coord g_CamFrustumLeftNormal;
float g_CamFrustumLeftOffset;
struct coord g_CamFrustumRightNormal;
float g_CamFrustumRightOffset;
struct coord g_CamFrustumViewOrigin;
float g_CamFrustumViewOffset;

void camSetScreenSize(float width, float height)
{
	struct player *player = g_Vars.currentplayer;

	player->c_screenwidth = width;
	player->c_screenheight = height;
	player->c_halfwidth = width * 0.5f;
	player->c_halfheight = height * 0.5f;
}

void camSetScreenPosition(float left, float top)
{
	struct player *player = g_Vars.currentplayer;

	player->c_screenleft = left;
	player->c_screentop = top;
}

void camSetPerspective(float fovy, float aspect)
{
	struct player *player = g_Vars.currentplayer;

	player->c_perspfovy = fovy;
	player->c_perspaspect = aspect;
}

// Used by the FarSight
float camGetEraserFOV(float autoeraserdist)
{
	float result = atan2f(sinf(0.52359879016876f) / (cosf(0.52359879016876f) * 120.0f) * autoeraserdist * g_Vars.currentplayer->c_halfheight, 1.0f);
	result *= 114.591552f;

	result = fabsf(result);

	return result;
}

void camSetScale(void)
{
	struct player *player = g_Vars.currentplayer;
	float fVar4;

	player->c_scaley = sinf(player->c_perspfovy * (M_PI / 360.0f)) / (cosf(player->c_perspfovy * (M_PI / 360.0f)) * player->c_halfheight);
	player->c_scalex = (player->c_scaley * player->c_perspaspect * player->c_halfheight) / player->c_halfwidth;

	player->c_recipscalex = 1.0f / player->c_scalex;
	player->c_recipscaley = 1.0f / player->c_scaley;

	fVar4 = sinf(0.52359879016876f) / (cosf(0.52359879016876f) * 120.0f);
	player->c_lodscalez = player->c_scaley / fVar4;
}

void camProjectScreenToWorldDir(float pos2d[2], struct coord *dir2d, float desiredLength)
{
	struct player *player = g_Vars.currentplayer;
	float sp20;
	float sp1c;
	float sp18 = -1.0f;
	float f2;

	sp1c = (player->c_halfheight - (pos2d[1] - player->c_screentop)) * player->c_scaley;
	sp20 = (pos2d[0] - player->c_screenleft - player->c_halfwidth) * player->c_scalex;

	f2 = desiredLength / sqrtf(sp20 * sp20 + sp1c * sp1c + sp18 * sp18);

	dir2d->x = sp20 * f2;
	dir2d->y = sp1c * f2;
	dir2d->z = sp18 * f2;
}

void camProjectViewToScreen(struct coord *in, float *out)
{
	struct player *player = g_Vars.currentplayer;
	float value = 1.0f / in->z;

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

// Same as above function but avoids division by 0
void camProjectViewToScreenSafe(struct coord *in, float out[2])
{
	struct player *player = g_Vars.currentplayer;
	float value;

	if (in->z == 0.0f) {
		value = -100000000000000000000.0f;
	} else {
		value = 1.0f / in->z;
	}

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

void camProjectViewToScreenAbsZ(struct coord *in, float out[2])
{
	struct player *player = g_Vars.currentplayer;
	float value = 1.0f / in->z;

	if (value < 0) {
		value = -value;
	}

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

// Convert a 2D coordinate in view space to screen-relative coordinates
void camScaleViewToScreen(float in[2], float divisor, float out[2])
{
	out[1] = in[1] * (1.0f / divisor) * g_Vars.currentplayer->c_recipscaley;
	out[0] = in[0] * (1.0f / divisor) * g_Vars.currentplayer->c_recipscalex;
}

// Project a world coordinate onto 2D screen space, but with adjustable zoom and aspect ratio instead of relying on the current camera settings
void camProjectWithZoomAndAspect(struct coord *arg0, float arg1[2], float zoom, float aspect)
{
	float f12;
	float f14;
	struct player *player = g_Vars.currentplayer;

	f12 = cosf(zoom * 0.008726646f) * player->c_halfheight / (sinf(zoom * 0.008726646f) * arg0->f[2]);
	f14 = f12 * player->c_halfwidth / (aspect * player->c_halfheight);

	arg1[1] = f12 * arg0->f[1] + (player->c_screentop + player->c_halfheight);
	arg1[0] = player->c_screenleft + player->c_halfwidth - f14 * arg0->f[0];
}

void camSetArtifactMtx(Mtx *mtx)
{
	g_Vars.currentplayer->artifactMtx = mtx;
}

void camSetPerspectiveMtxL(Mtx *mtx)
{
	g_Vars.currentplayer->perspmtxl = mtx;
}

Mtx *camGetPerspectiveMtxL(void)
{
	return g_Vars.currentplayer->perspmtxl;
}

void camSetOrthogonalMtxL(Mtx *mtx)
{
	g_Vars.currentplayer->orthomtxl = mtx;
}

Mtx *camGetOrthogonalMtxL(void)
{
	return g_Vars.currentplayer->orthomtxl;
}

void camSetWorldToScreenMtx(Mtx *mtx)
{
	struct player *player = g_Vars.currentplayer;

	player->prevworldtoscreenmtx = player->worldtoscreenmtx;
	player->worldtoscreenmtx = mtx;
	player->c_viewfmdynticknum = g_GfxNumSwaps;
	player->unk0488 = player->unk0484;
	player->unk0484 = g_GfxMemPos;
}

Mtx *camGetWorldToScreenMtx(uint8_t *arg0)
{
	int i;

	// Check against active VTX buffer range
	if (arg0 >= g_VtxBuffers[g_GfxActiveBufferIndex] && arg0 < g_VtxBuffers[g_GfxActiveBufferIndex + 1]) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum) {
					if (arg0 >= g_Vars.players[i]->unk0484 &&
							(uintptr_t)arg0 < (uintptr_t)g_Vars.players[i]->unk0484) {
						return g_Vars.players[i]->worldtoscreenmtx;
					}
				}
			}
		}
	} else {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_prevviewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0488 &&
							(uintptr_t)arg0 < (uintptr_t)g_Vars.players[i]->unk0488) {
						return g_Vars.players[i]->prevworldtoscreenmtx;
					}
				}
			} else {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0484 &&
							(uintptr_t)arg0 < (uintptr_t)g_Vars.players[i]->unk0484) {
						return g_Vars.players[i]->worldtoscreenmtx;
					}
				}
			}
		}
	}

	return NULL;
}

Mtx *camGetProjectionMtxForPlayers(uint8_t *arg0)
{
	Mtx *result = NULL;
	int i;

	if (arg0 >= g_VtxBuffers[g_GfxActiveBufferIndex] && arg0 < g_VtxBuffers[g_GfxActiveBufferIndex + 1]) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (uint8_t *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->projectionmtx;
					}
				}
			}
		}
	} else {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_prevviewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0488 && (uint8_t *)result < g_Vars.players[i]->unk0488) {
						result = g_Vars.players[i]->prevprojectionmtx;
					}
				}
			} else {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (uint8_t *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->projectionmtx;
					}
				}
			}
		}
	}

	return result;
}

Mtx *camGetPlayerWorldToScreenMtx(void)
{
	return g_Vars.currentplayer->worldtoscreenmtx;
}

void camSetSkyMtx(Mtx *mtx)
{
	g_Vars.currentplayer->skyMtx = mtx;
}

Mtx *camGetSkyMtx(void)
{
	return g_Vars.currentplayer->skyMtx;
}

void camSetProjectionMtx(Mtx *mtx)
{
	struct player *player = g_Vars.currentplayer;

	player->c_prevviewfmdynticknum = player->c_viewfmdynticknum;
	player->prevprojectionmtx = player->projectionmtx;
	player->projectionmtx = mtx;
}

Mtx *camGetProjectionMtx(void)
{
	return g_Vars.currentplayer->projectionmtx;
}

void camSetLookAt(LookAt *lookat)
{
	g_Vars.currentplayer->lookat = lookat;
}

LookAt *camGetLookAt(void)
{
	return g_Vars.currentplayer->lookat;
}

float camGetLodScaleZ(void)
{
	return g_Vars.currentplayer->c_lodscalez;
}

float camGetScreenWidth(void)
{
	return g_Vars.currentplayer->c_screenwidth;
}

float camGetScreenHeight(void)
{
	return g_Vars.currentplayer->c_screenheight;
}

float camGetScreenLeft(void)
{
	return g_Vars.currentplayer->c_screenleft;
}

float camGetScreenTop(void)
{
	return g_Vars.currentplayer->c_screentop;
}

float camGetPerspFovY(void)
{
	return g_Vars.currentplayer->c_perspfovy;
}

float camGetPerspAspect(void)
{
	return g_Vars.currentplayer->c_perspaspect;
}

void camComputeFrustumEdgePlanes(void)
{
	float sp2c;
	float sp28;
	float sp24;
	float sp20;
	struct player *player;
	Mtx *mtx;
	float sp14;
	float sp10;

	player = g_Vars.currentplayer;
	sp24 = player->c_halfheight * player->c_scaley;
	mtx = player->projectionmtx;

	sp2c = 1.0f / sqrtf(sp24 * sp24 + 1.0f);
	sp24 *= sp2c;
	sp20 = -sp2c;

	g_CamFrustumTopNormal.f[0] = -sp20 * (*mtx)[1][0] + (sp24) * (*mtx)[2][0];
	g_CamFrustumTopNormal.f[1] = -sp20 * (*mtx)[1][1] + (sp24) * (*mtx)[2][1];
	g_CamFrustumTopNormal.f[2] = -sp20 * (*mtx)[1][2] + (sp24) * (*mtx)[2][2];

	g_CamFrustumTopOffset = g_CamFrustumTopNormal.f[0] * (*mtx)[3][0] + g_CamFrustumTopNormal.f[1] * (*mtx)[3][1] + g_CamFrustumTopNormal.f[2] * (*mtx)[3][2];

	g_CamFrustumBottomNormal.f[0] = sp20 * (*mtx)[1][0] + (sp24) * (*mtx)[2][0];
	g_CamFrustumBottomNormal.f[1] = sp20 * (*mtx)[1][1] + (sp24) * (*mtx)[2][1];
	g_CamFrustumBottomNormal.f[2] = sp20 * (*mtx)[1][2] + (sp24) * (*mtx)[2][2];

	g_CamFrustumBottomOffset = g_CamFrustumBottomNormal.f[0] * (*mtx)[3][0] + g_CamFrustumBottomNormal.f[1] * (*mtx)[3][1] + g_CamFrustumBottomNormal.f[2] * (*mtx)[3][2];

	sp28 = -player->c_halfwidth * player->c_scalex;

	sp10 = 1.0f / sqrtf(sp28 * sp28 + 1.0f);
	sp28 *= sp10;
	sp14 = -sp10;

	g_CamFrustumLeftNormal.f[0] = sp14 * (*mtx)[0][0] - sp28 * (*mtx)[2][0];
	g_CamFrustumLeftNormal.f[1] = sp14 * (*mtx)[0][1] - sp28 * (*mtx)[2][1];
	g_CamFrustumLeftNormal.f[2] = sp14 * (*mtx)[0][2] - sp28 * (*mtx)[2][2];

	g_CamFrustumLeftOffset = g_CamFrustumLeftNormal.f[0] * (*mtx)[3][0] + g_CamFrustumLeftNormal.f[1] * (*mtx)[3][1] + g_CamFrustumLeftNormal.f[2] * (*mtx)[3][2];

	g_CamFrustumRightNormal.f[0] = -sp14 * (*mtx)[0][0] - sp28 * (*mtx)[2][0];
	g_CamFrustumRightNormal.f[1] = -sp14 * (*mtx)[0][1] - sp28 * (*mtx)[2][1];
	g_CamFrustumRightNormal.f[2] = -sp14 * (*mtx)[0][2] - sp28 * (*mtx)[2][2];

	g_CamFrustumRightOffset = g_CamFrustumRightNormal.f[0] * (*mtx)[3][0] + g_CamFrustumRightNormal.f[1] * (*mtx)[3][1] + g_CamFrustumRightNormal.f[2] * (*mtx)[3][2];

	g_CamFrustumViewOrigin.f[0] = -(*mtx)[3][0];
	g_CamFrustumViewOrigin.f[1] = -(*mtx)[3][1];
	g_CamFrustumViewOrigin.f[2] = -(*mtx)[3][2];

	g_CamFrustumViewOffset = (*mtx)[2][0] * (*mtx)[3][0] + (*mtx)[2][1] * (*mtx)[3][1] + (*mtx)[2][2] * (*mtx)[3][2];
}

// Determines if a point or sphere (point + radius) is in the camera's view
bool camIsPointInFrustum(struct coord *point, float radius)
{
	Mtx *mtx = (Mtx*)g_Vars.currentplayer->projectionmtx;

	if (g_CamFrustumViewOffset + radius < (*mtx)[2][0] * point->f[0] + (*mtx)[2][1] * point->f[1] + (*mtx)[2][2] * point->f[2]) {
		return false;
	}

	if (g_CamFrustumLeftOffset + radius < g_CamFrustumLeftNormal.f[0] * point->f[0] + g_CamFrustumLeftNormal.f[1] * point->f[1] + g_CamFrustumLeftNormal.f[2] * point->f[2]) {
		return false;
	}

	if (g_CamFrustumRightOffset + radius < g_CamFrustumRightNormal.f[0] * point->f[0] + g_CamFrustumRightNormal.f[1] * point->f[1] + g_CamFrustumRightNormal.f[2] * point->f[2]) {
		return false;
	}

	if (g_CamFrustumTopOffset + radius < g_CamFrustumTopNormal.f[0] * point->f[0] + g_CamFrustumTopNormal.f[1] * point->f[1] + g_CamFrustumTopNormal.f[2] * point->f[2]) {
		return false;
	}

	if (g_CamFrustumBottomOffset + radius < g_CamFrustumBottomNormal.f[0] * point->f[0] + g_CamFrustumBottomNormal.f[1] * point->f[1] + g_CamFrustumBottomNormal.f[2] * point->f[2]) {
		return false;
	}

	return true;
}

bool camIsPosInScreenBox(struct coord *pos, float radius, struct drawslot *drawslot)
{
	struct coord planeNormal;
	float planeOffset;

	// Back-face culling: camera facing direction vs. point
	float cameraDepth = (*g_Vars.currentplayer->projectionmtx)[2][0] * pos->x +
	                    (*g_Vars.currentplayer->projectionmtx)[2][1] * pos->y +
	                    (*g_Vars.currentplayer->projectionmtx)[2][2] * pos->z;

	if (g_CamFrustumViewOffset + radius < cameraDepth) {
		return false;
	}

	// --- Left plane ---
	float leftOffset = (drawslot->box.xmin - g_Vars.currentplayer->c_screenleft - g_Vars.currentplayer->c_halfwidth) * g_Vars.currentplayer->c_scalex;
	float leftLenInv = 1.0f / sqrtf(leftOffset * leftOffset + 1.0f);
	float leftX = leftOffset * leftLenInv;
	float leftY = -leftLenInv;

	planeNormal.x = leftY * (*g_Vars.currentplayer->projectionmtx)[0][0] - leftX * (*g_Vars.currentplayer->projectionmtx)[2][0];
	planeNormal.y = leftY * (*g_Vars.currentplayer->projectionmtx)[0][1] - leftX * (*g_Vars.currentplayer->projectionmtx)[2][1];
	planeNormal.z = leftY * (*g_Vars.currentplayer->projectionmtx)[0][2] - leftX * (*g_Vars.currentplayer->projectionmtx)[2][2];
	planeOffset = planeNormal.x * (*g_Vars.currentplayer->projectionmtx)[3][0] +
	              planeNormal.y * (*g_Vars.currentplayer->projectionmtx)[3][1] +
	              planeNormal.z * (*g_Vars.currentplayer->projectionmtx)[3][2];

	if (planeOffset + radius < planeNormal.x * pos->x + planeNormal.y * pos->y + planeNormal.z * pos->z) {
		return false;
	}

	// --- Right plane ---
	float rightOffset = -(drawslot->box.xmax - g_Vars.currentplayer->c_screenleft - g_Vars.currentplayer->c_halfwidth) * g_Vars.currentplayer->c_scalex;
	float rightLenInv = 1.0f / sqrtf(rightOffset * rightOffset + 1.0f);
	float rightX = rightOffset * rightLenInv;
	float rightY = -rightLenInv;

	planeNormal.x = -rightY * (*g_Vars.currentplayer->projectionmtx)[0][0] - rightX * (*g_Vars.currentplayer->projectionmtx)[2][0];
	planeNormal.y = -rightY * (*g_Vars.currentplayer->projectionmtx)[0][1] - rightX * (*g_Vars.currentplayer->projectionmtx)[2][1];
	planeNormal.z = -rightY * (*g_Vars.currentplayer->projectionmtx)[0][2] - rightX * (*g_Vars.currentplayer->projectionmtx)[2][2];
	planeOffset = planeNormal.x * (*g_Vars.currentplayer->projectionmtx)[3][0] +
	              planeNormal.y * (*g_Vars.currentplayer->projectionmtx)[3][1] +
	              planeNormal.z * (*g_Vars.currentplayer->projectionmtx)[3][2];

	if (planeOffset + radius < planeNormal.x * pos->x + planeNormal.y * pos->y + planeNormal.z * pos->z) {
		return false;
	}

	// --- Top plane ---
	float topOffset = (g_Vars.currentplayer->c_halfheight - (drawslot->box.ymin - g_Vars.currentplayer->c_screentop)) * g_Vars.currentplayer->c_scaley;
	float topLenInv = 1.0f / sqrtf(topOffset * topOffset + 1.0f);
	float topY = topOffset * topLenInv;
	float topZ = -topLenInv;

	planeNormal.x = -topZ * (*g_Vars.currentplayer->projectionmtx)[1][0] + topY * (*g_Vars.currentplayer->projectionmtx)[2][0];
	planeNormal.y = -topZ * (*g_Vars.currentplayer->projectionmtx)[1][1] + topY * (*g_Vars.currentplayer->projectionmtx)[2][1];
	planeNormal.z = -topZ * (*g_Vars.currentplayer->projectionmtx)[1][2] + topY * (*g_Vars.currentplayer->projectionmtx)[2][2];
	planeOffset = planeNormal.x * (*g_Vars.currentplayer->projectionmtx)[3][0] +
	              planeNormal.y * (*g_Vars.currentplayer->projectionmtx)[3][1] +
	              planeNormal.z * (*g_Vars.currentplayer->projectionmtx)[3][2];

	if (planeOffset + radius < planeNormal.x * pos->x + planeNormal.y * pos->y + planeNormal.z * pos->z) {
		return false;
	}

	// --- Bottom plane ---
	float bottomOffset = -(g_Vars.currentplayer->c_halfheight - (drawslot->box.ymax - g_Vars.currentplayer->c_screentop)) * g_Vars.currentplayer->c_scaley;
	float bottomLenInv = 1.0f / sqrtf(bottomOffset * bottomOffset + 1.0f);
	float bottomY = bottomOffset * bottomLenInv;
	float bottomZ = -bottomLenInv;

	planeNormal.x = bottomZ * (*g_Vars.currentplayer->projectionmtx)[1][0] + bottomY * (*g_Vars.currentplayer->projectionmtx)[2][0];
	planeNormal.y = bottomZ * (*g_Vars.currentplayer->projectionmtx)[1][1] + bottomY * (*g_Vars.currentplayer->projectionmtx)[2][1];
	planeNormal.z = bottomZ * (*g_Vars.currentplayer->projectionmtx)[1][2] + bottomY * (*g_Vars.currentplayer->projectionmtx)[2][2];
	planeOffset = planeNormal.x * (*g_Vars.currentplayer->projectionmtx)[3][0] +
	              planeNormal.y * (*g_Vars.currentplayer->projectionmtx)[3][1] +
	              planeNormal.z * (*g_Vars.currentplayer->projectionmtx)[3][2];

	if (planeOffset + radius < planeNormal.x * pos->x + planeNormal.y * pos->y + planeNormal.z * pos->z) {
		return false;
	}

	return true;
}


/**
 * This function is building a drawslot on the stack so it can pass it to
 * camIsPosInScreenBox, however if we allocate this struct then it uses too much
 * stack and creates a mismatch.
 *
 * We resolve this by allocating a screenbox instead, which is a substruct of
 * drawslot and is all we need in this function. screenbox isn't at the
 * start of drawslot though, so we use a negative array index to pass the
 * correct address to camIsPosInScreenBox so it can interpret the pointer as a
 * drawslot.
 */
bool camIsPosInFovAndVisibleRoom(RoomNum *rooms, struct coord *pos, float arg2)
{
	int i;
	RoomNum room;
	bool hasdata = false;
	struct drawslot *thisthing;
	static struct drawslot dslot;
	struct screenbox box;

	for (i = 0, room = rooms[i]; room != -1; i++, room = rooms[i]) {
		if (g_Rooms[room].flags & ROOMFLAG_ONSCREEN) {
			thisthing = bgGetRoomDrawSlot(room);

			if (hasdata == false) {
				box.xmin = thisthing->box.xmin;
				box.ymin = thisthing->box.ymin;
				box.xmax = thisthing->box.xmax;
				box.ymax = thisthing->box.ymax;
			} else {
				if (thisthing->box.xmin < box.xmin) {
					box.xmin = thisthing->box.xmin;
				}

				if (thisthing->box.ymin < box.ymin) {
					box.ymin = thisthing->box.ymin;
				}

				if (thisthing->box.xmax > box.xmax) {
					box.xmax = thisthing->box.xmax;
				}

				if (thisthing->box.ymax > box.ymax) {
					box.ymax = thisthing->box.ymax;
				}
			}

			hasdata = true;
		}
	}

	if (!hasdata) {
		return false;
	}

	memcpy(&dslot.box, &box, sizeof(box));
	return camIsPosInScreenBox(pos, arg2, &dslot);
}
