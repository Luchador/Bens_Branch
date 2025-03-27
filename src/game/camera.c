#include <ultra64.h>
#include <math.h>
#include <string.h>
#include "constants.h"
#include "game/camera.h"
#include "game/tex.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

struct coord g_FrustumTopPlane;
f32 g_FrustumTopOffset;
struct coord g_FrustumBottomPlane;
f32 g_FrustumBottomOffset;
struct coord g_FrustumLeftPlane;
f32 g_FrustumLeftOffset;
struct coord g_FrustumRightPlane;
f32 g_FrustumRightOffset;
struct coord g_FrustumNearPlane;
f32 g_FrustumNearOffset;

void camSetScreenSize(f32 width, f32 height)
{
	struct player *player = g_Vars.currentplayer;

	player->c_screenwidth = width;
	player->c_screenheight = height;
	player->c_halfwidth = width * 0.5f;
	player->c_halfheight = height * 0.5f;
}

void camSetScreenPosition(f32 left, f32 top)
{
	struct player *player = g_Vars.currentplayer;

	player->c_screenleft = left;
	player->c_screentop = top;
}

void camSetPerspective(f32 near, f32 fovy, f32 aspect)
{
	struct player *player = g_Vars.currentplayer;

	player->c_perspnear = near;
	player->c_perspfovy = fovy;
	player->c_perspaspect = aspect;
}

f32 camCalculateVerticalAngularSize(f32 arg0)
{
	f32 result = atan2f(g_Vars.currentplayer->c_scalelod60 * arg0 * g_Vars.currentplayer->c_halfheight, 1.0f);
	result *= 114.591552f;

	if (result < 0) {
		result = -result;
	}

	return result;
}

void camSetScale(void)
{
	struct player *player = g_Vars.currentplayer;
	f32 fVar4;
	f32 tmp;
	f32 fVar5;
	f32 fVar2;

	player->c_scaley = sinf(player->c_perspfovy * (M_PI / 360.0f)) / (cosf(player->c_perspfovy * (M_PI / 360.0f)) * player->c_halfheight);
	player->c_scalelod = player->c_scaley;
	player->c_scalex = (player->c_scaley * player->c_perspaspect * player->c_halfheight) / player->c_halfwidth;

	player->c_recipscalex = 1.0f / player->c_scalex;
	player->c_recipscaley = 1.0f / player->c_scaley;

	fVar4 = sinf(0.52359879016876f) / (cosf(0.52359879016876f) * 120.0f);
	player->c_scalelod60 = fVar4;
	player->c_lodscalez = player->c_scalelod / fVar4;
	tmp = player->c_lodscalez * 65536.0f;

	if (tmp > 4294967296.0f) {
		player->c_lodscalezu32 = 0xffffffff;
	} else {
		player->c_lodscalezu32 = tmp;
	}

	fVar2 = player->c_halfheight * player->c_scaley;
	fVar4 = 1.0f / sqrtf(fVar2 * fVar2 + 1.0f);
	player->c_cameratopnorm.x = 0;
	player->c_cameratopnorm.y = fVar4;
	player->c_cameratopnorm.z = fVar2 * fVar4;

	fVar5 = -player->c_halfwidth * player->c_scalex;
	fVar4 = 1.0f / sqrtf(fVar5 * fVar5 + 1.0f);
	player->c_cameraleftnorm.x = -fVar4;
	player->c_cameraleftnorm.y = 0;
	player->c_cameraleftnorm.z = -fVar5 * fVar4;
}

void cam0f0b4c3c(f32 pos2d[2], struct coord *dir2d, f32 arg2)
{
	struct player *player = g_Vars.currentplayer;
	f32 sp20;
	f32 sp1c;
	f32 sp18 = -1.0f;
	f32 f2;

	sp1c = (player->c_halfheight - (pos2d[1] - player->c_screentop)) * player->c_scaley;
	sp20 = (pos2d[0] - player->c_screenleft - player->c_halfwidth) * player->c_scalex;

	f2 = arg2 / sqrtf(sp20 * sp20 + sp1c * sp1c + sp18 * sp18);

	dir2d->x = sp20 * f2;
	dir2d->y = sp1c * f2;
	dir2d->z = sp18 * f2;
}

void cam0f0b4d04(struct coord *in, f32 *out)
{
	struct player *player = g_Vars.currentplayer;
	f32 value = 1.0f / in->z;

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

void cam0f0b4d68(struct coord *in, f32 out[2])
{
	struct player *player = g_Vars.currentplayer;
	f32 value;

	if (in->z == 0.0f) {
		value = -100000000000000000000.0f;
	} else {
		value = 1.0f / in->z;
	}

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

void cam0f0b4dec(struct coord *in, f32 out[2])
{
	struct player *player = g_Vars.currentplayer;
	f32 value = 1.0f / in->z;

	if (value < 0) {
		value = -value;
	}

	out[1] = in->y * value * player->c_recipscaley + (player->c_screentop + player->c_halfheight);
	out[0] = (player->c_screenleft + player->c_halfwidth) - in->x * value * player->c_recipscalex;
}

void cam0f0b4e68(f32 in[2], f32 divisor, f32 out[2])
{
	out[1] = in[1] * (1.0f / divisor) * g_Vars.currentplayer->c_recipscaley;
	out[0] = in[0] * (1.0f / divisor) * g_Vars.currentplayer->c_recipscalex;
}

void cam0f0b4eb8(struct coord *arg0, f32 arg1[2], f32 zoom, f32 aspect)
{
	f32 f12;
	f32 f14;
	struct player *player = g_Vars.currentplayer;

	f12 = cosf(zoom * 0.008726646f) * player->c_halfheight / (sinf(zoom * 0.008726646f) * arg0->f[2]);
	f14 = f12 * player->c_halfwidth / (aspect * player->c_halfheight);

	arg1[1] = f12 * arg0->f[1] + (player->c_screentop + player->c_halfheight);
	arg1[0] = player->c_screenleft + player->c_halfwidth - f14 * arg0->f[0];
}

void camSetMtxL1738(Mtx *mtx)
{
	g_Vars.currentplayer->mtxl1738 = mtx;
}

Mtx *camGetMtxL1738(void)
{
	return g_Vars.currentplayer->mtxl1738;
}

void camSetMtxL173c(Mtx *mtx)
{
	g_Vars.currentplayer->mtxl173c = mtx;
}

Mtx *camGetMtxL173c(void)
{
	return g_Vars.currentplayer->mtxl173c;
}

void camSetMtxF006c(Mtxf *mtx)
{
	g_Vars.currentplayer->mtxf006c = mtx;
}

Mtxf *camGetMtxF006c(void)
{
	return g_Vars.currentplayer->mtxf006c;
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

void camSetWorldToScreenMtxf(Mtxf *mtx)
{
	struct player *player = g_Vars.currentplayer;

	player->prevworldtoscreenmtx = player->worldtoscreenmtx;
	player->worldtoscreenmtx = mtx;
	player->c_viewfmdynticknum = g_GfxNumSwaps;
	player->unk0488 = player->unk0484;
	player->unk0484 = g_GfxMemPos;
}

Mtxf *cam0f0b5050(u8 *arg0)
{
	Mtxf *result = NULL;
	s32 i;

	if (arg0 >= g_VtxBuffers[g_GfxActiveBufferIndex] && arg0 < g_VtxBuffers[g_GfxActiveBufferIndex + 1]) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (u8 *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->worldtoscreenmtx;
					}
				}
			}
		}
	} else {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_prevviewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0488 && (u8 *)result < g_Vars.players[i]->unk0488) {
						result = g_Vars.players[i]->prevworldtoscreenmtx;
					}
				}
			} else {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (u8 *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->worldtoscreenmtx;
					}
				}
			}
		}
	}

	return result;
}

Mtxf *cam0f0b53a4(u8 *arg0)
{
	Mtxf *result = NULL;
	s32 i;

	if (arg0 >= g_VtxBuffers[g_GfxActiveBufferIndex] && arg0 < g_VtxBuffers[g_GfxActiveBufferIndex + 1]) {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (u8 *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->projectionmtx;
					}
				}
			}
		}
	} else {
		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.currentplayerindex >= playermgrGetOrderOfPlayer(i)) {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_prevviewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0488 && (u8 *)result < g_Vars.players[i]->unk0488) {
						result = g_Vars.players[i]->prevprojectionmtx;
					}
				}
			} else {
				if (g_GfxNumSwaps == g_Vars.players[i]->c_viewfmdynticknum + 1) {
					if (arg0 >= g_Vars.players[i]->unk0484 && (u8 *)result < g_Vars.players[i]->unk0484) {
						result = g_Vars.players[i]->projectionmtx;
					}
				}
			}
		}
	}

	return result;
}

Mtxf *camGetWorldToScreenMtxf(void)
{
	return g_Vars.currentplayer->worldtoscreenmtx;
}

void camSetMtxF1754(Mtxf *mtx)
{
	g_Vars.currentplayer->mtxf1754 = mtx;
}

Mtxf *camGetMtxF1754(void)
{
	return g_Vars.currentplayer->mtxf1754;
}

Mtxf *camGetPrevWorldToScreenMtxf(void)
{
	return g_Vars.currentplayer->prevworldtoscreenmtx;
}

void camSetProjectionMtxF(Mtxf *mtx)
{
	struct player *player = g_Vars.currentplayer;

	player->c_prevviewfmdynticknum = player->c_viewfmdynticknum;
	player->prevprojectionmtx = player->projectionmtx;
	player->projectionmtx = mtx;
}

Mtxf *camGetProjectionMtxF(void)
{
	return g_Vars.currentplayer->projectionmtx;
}

Mtxf *camGetPrevProjectionMtxF(void)
{
	return g_Vars.currentplayer->prevprojectionmtx;
}

void camSetLookAt(LookAt *lookat)
{
	g_Vars.currentplayer->lookat = lookat;
}

LookAt *camGetLookAt(void)
{
	return g_Vars.currentplayer->lookat;
}

f32 camGetLodScaleZ(void)
{
	return g_Vars.currentplayer->c_lodscalez;
}

u32 camGetLodScaleZU32(void)
{
	return g_Vars.currentplayer->c_lodscalezu32;
}

f32 camGetScreenWidth(void)
{
	return g_Vars.currentplayer->c_screenwidth;
}

f32 camGetScreenHeight(void)
{
	return g_Vars.currentplayer->c_screenheight;
}

f32 camGetScreenLeft(void)
{
	return g_Vars.currentplayer->c_screenleft;
}

f32 camGetScreenTop(void)
{
	return g_Vars.currentplayer->c_screentop;
}

f32 camGetPerspFovY(void)
{
	return g_Vars.currentplayer->c_perspfovy;
}

f32 camGetPerspAspect(void)
{
	return g_Vars.currentplayer->c_perspaspect;
}

void camComputeFrustumPlanes(void)
{
	f32 topNorm;
	f32 topProj;
	f32 leftNorm;
	f32 leftProj;
	struct player *player = g_Vars.currentplayer;
	Mtxf *projMtx = player->projectionmtx;

	// Vertical (top and bottom) frustum planes
	f32 halfHeight = player->c_halfheight * player->c_scaley;
	f32 verticalInvLen = 1.0f / sqrtf(halfHeight * halfHeight + 1.0f);
	halfHeight *= verticalInvLen;

	f32 planeYFactor = -verticalInvLen;

	// Top plane normal
	g_FrustumTopPlane.x = -planeYFactor * projMtx->m[1][0] + halfHeight * projMtx->m[2][0];
	g_FrustumTopPlane.y = -planeYFactor * projMtx->m[1][1] + halfHeight * projMtx->m[2][1];
	g_FrustumTopPlane.z = -planeYFactor * projMtx->m[1][2] + halfHeight * projMtx->m[2][2];
	g_FrustumTopOffset = g_FrustumTopPlane.x * projMtx->m[3][0] +
	                     g_FrustumTopPlane.y * projMtx->m[3][1] +
	                     g_FrustumTopPlane.z * projMtx->m[3][2];

	// Bottom plane normal
	g_FrustumBottomPlane.x = planeYFactor * projMtx->m[1][0] + halfHeight * projMtx->m[2][0];
	g_FrustumBottomPlane.y = planeYFactor * projMtx->m[1][1] + halfHeight * projMtx->m[2][1];
	g_FrustumBottomPlane.z = planeYFactor * projMtx->m[1][2] + halfHeight * projMtx->m[2][2];
	g_FrustumBottomOffset = g_FrustumBottomPlane.x * projMtx->m[3][0] +
	                        g_FrustumBottomPlane.y * projMtx->m[3][1] +
	                        g_FrustumBottomPlane.z * projMtx->m[3][2];

	// Horizontal (left and right) frustum planes
	f32 halfWidth = -player->c_halfwidth * player->c_scalex;
	f32 horizontalInvLen = 1.0f / sqrtf(halfWidth * halfWidth + 1.0f);
	halfWidth *= horizontalInvLen;

	f32 planeXFactor = -horizontalInvLen;

	// Left plane normal
	g_FrustumLeftPlane.x = planeXFactor * projMtx->m[0][0] - halfWidth * projMtx->m[2][0];
	g_FrustumLeftPlane.y = planeXFactor * projMtx->m[0][1] - halfWidth * projMtx->m[2][1];
	g_FrustumLeftPlane.z = planeXFactor * projMtx->m[0][2] - halfWidth * projMtx->m[2][2];
	g_FrustumLeftOffset = g_FrustumLeftPlane.x * projMtx->m[3][0] +
	                      g_FrustumLeftPlane.y * projMtx->m[3][1] +
	                      g_FrustumLeftPlane.z * projMtx->m[3][2];

	// Right plane normal
	g_FrustumRightPlane.x = -planeXFactor * projMtx->m[0][0] - halfWidth * projMtx->m[2][0];
	g_FrustumRightPlane.y = -planeXFactor * projMtx->m[0][1] - halfWidth * projMtx->m[2][1];
	g_FrustumRightPlane.z = -planeXFactor * projMtx->m[0][2] - halfWidth * projMtx->m[2][2];
	g_FrustumRightOffset = g_FrustumRightPlane.x * projMtx->m[3][0] +
	                       g_FrustumRightPlane.y * projMtx->m[3][1] +
	                       g_FrustumRightPlane.z * projMtx->m[3][2];

	// Near plane direction
	g_FrustumNearPlane.x = -projMtx->m[3][0];
	g_FrustumNearPlane.y = -projMtx->m[3][1];
	g_FrustumNearPlane.z = -projMtx->m[3][2];

	// Near plane offset (dot product with forward direction)
	g_FrustumNearOffset = projMtx->m[2][0] * projMtx->m[3][0] +
	                      projMtx->m[2][1] * projMtx->m[3][1] +
	                      projMtx->m[2][2] * projMtx->m[3][2];
}

bool camIsPointInFrustum(struct coord *point, f32 radius)
{
	Mtxf *mtx = g_Vars.currentplayer->projectionmtx;

	if (g_FrustumNearOffset + radius < mtx->m[2][0] * point->f[0] + mtx->m[2][1] * point->f[1] + mtx->m[2][2] * point->f[2]) {
		return false;
	}

	if (g_FrustumLeftOffset + radius < g_FrustumLeftPlane.f[0] * point->f[0] + g_FrustumLeftPlane.f[1] * point->f[1] + g_FrustumLeftPlane.f[2] * point->f[2]) {
		return false;
	}

	if (g_FrustumRightOffset + radius < g_FrustumRightPlane.f[0] * point->f[0] + g_FrustumRightPlane.f[1] * point->f[1] + g_FrustumRightPlane.f[2] * point->f[2]) {
		return false;
	}

	if (g_FrustumTopOffset + radius < g_FrustumTopPlane.f[0] * point->f[0] + g_FrustumTopPlane.f[1] * point->f[1] + g_FrustumTopPlane.f[2] * point->f[2]) {
		return false;
	}

	if (g_FrustumBottomOffset + radius < g_FrustumBottomPlane.f[0] * point->f[0] + g_FrustumBottomPlane.f[1] * point->f[1] + g_FrustumBottomPlane.f[2] * point->f[2]) {
		return false;
	}

	return true;
}

bool camIsPosInScreenBox(struct coord *pos, f32 arg1, struct drawslot *drawslot)
{
	struct coord sp74;
	f32 sp70;
	struct coord sp64;
	f32 sp60;
	struct coord sp54;
	f32 sp50;
	struct coord sp44;
	f32 sp40;
	f32 sp3c;
	f32 sp38;
	f32 sp34;
	f32 sp30;
	f32 sp2c;
	f32 sp28;
	f32 sp24;
	f32 sp20;
	f32 sp1c;
	f32 sp18;

	if (g_FrustumNearOffset + arg1 < g_Vars.currentplayer->projectionmtx->m[2][0] * pos->f[0] + g_Vars.currentplayer->projectionmtx->m[2][1] * pos->f[1] + g_Vars.currentplayer->projectionmtx->m[2][2] * pos->f[2]) {
		return false;
	}

	sp38 = (drawslot->box.xmin - g_Vars.currentplayer->c_screenleft - g_Vars.currentplayer->c_halfwidth) * g_Vars.currentplayer->c_scalex;

	sp3c = 1.0f / sqrtf(sp38 * sp38 + 1.0f);
	sp38 *= sp3c;
	sp24 = -sp3c;

	sp54.f[0] = sp24 * g_Vars.currentplayer->projectionmtx->m[0][0] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][0];
	sp54.f[1] = sp24 * g_Vars.currentplayer->projectionmtx->m[0][1] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][1];
	sp54.f[2] = sp24 * g_Vars.currentplayer->projectionmtx->m[0][2] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][2];

	sp50 = sp54.f[0] * g_Vars.currentplayer->projectionmtx->m[3][0] + sp54.f[1] * g_Vars.currentplayer->projectionmtx->m[3][1] + sp54.f[2] * g_Vars.currentplayer->projectionmtx->m[3][2];

	if (sp50 + arg1 < sp54.f[0] * pos->f[0] + sp54.f[1] * pos->f[1] + sp54.f[2] * pos->f[2]) {
		return false;
	}

	sp38 = -(drawslot->box.xmax - g_Vars.currentplayer->c_screenleft - g_Vars.currentplayer->c_halfwidth) * g_Vars.currentplayer->c_scalex;
	sp30 = 1.0f / sqrtf(sp38 * sp38 + 1.0f);
	sp38 *= sp30;
	sp20 = -sp30;

	sp44.f[0] = -sp20 * g_Vars.currentplayer->projectionmtx->m[0][0] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][0];
	sp44.f[1] = -sp20 * g_Vars.currentplayer->projectionmtx->m[0][1] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][1];
	sp44.f[2] = -sp20 * g_Vars.currentplayer->projectionmtx->m[0][2] - sp38 * g_Vars.currentplayer->projectionmtx->m[2][2];

	sp40 = sp44.f[0] * g_Vars.currentplayer->projectionmtx->m[3][0] + sp44.f[1] * g_Vars.currentplayer->projectionmtx->m[3][1] + sp44.f[2] * g_Vars.currentplayer->projectionmtx->m[3][2];

	if (sp40 + arg1 < sp44.f[0] * pos->f[0] + sp44.f[1] * pos->f[1] + sp44.f[2] * pos->f[2]) {
		return false;
	}

	sp34 = (g_Vars.currentplayer->c_halfheight - (drawslot->box.ymin - g_Vars.currentplayer->c_screentop)) * g_Vars.currentplayer->c_scaley;
	sp2c = 1.0f / sqrtf(sp34 * sp34 + 1.0f);
	sp34 *= sp2c;
	sp1c = -sp2c;

	sp74.f[0] = -sp1c * g_Vars.currentplayer->projectionmtx->m[1][0] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][0];
	sp74.f[1] = -sp1c * g_Vars.currentplayer->projectionmtx->m[1][1] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][1];
	sp74.f[2] = -sp1c * g_Vars.currentplayer->projectionmtx->m[1][2] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][2];

	sp70 = sp74.f[0] * g_Vars.currentplayer->projectionmtx->m[3][0] + sp74.f[1] * g_Vars.currentplayer->projectionmtx->m[3][1] + sp74.f[2] * g_Vars.currentplayer->projectionmtx->m[3][2];

	if (sp70 + arg1 < sp74.f[0] * pos->f[0] + sp74.f[1] * pos->f[1] + sp74.f[2] * pos->f[2]) {
		return false;
	}

	sp34 = -(g_Vars.currentplayer->c_halfheight - (drawslot->box.ymax - g_Vars.currentplayer->c_screentop)) * g_Vars.currentplayer->c_scaley;
	sp28 = 1.0f / sqrtf(sp34 * sp34 + 1.0f);
	sp34 *= sp28;
	sp18 = -sp28;

	sp64.f[0] = sp18 * g_Vars.currentplayer->projectionmtx->m[1][0] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][0];
	sp64.f[1] = sp18 * g_Vars.currentplayer->projectionmtx->m[1][1] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][1];
	sp64.f[2] = sp18 * g_Vars.currentplayer->projectionmtx->m[1][2] + sp34 * g_Vars.currentplayer->projectionmtx->m[2][2];

	sp60 = sp64.f[0] * g_Vars.currentplayer->projectionmtx->m[3][0] + sp64.f[1] * g_Vars.currentplayer->projectionmtx->m[3][1] + sp64.f[2] * g_Vars.currentplayer->projectionmtx->m[3][2];

	if (sp60 + arg1 < sp64.f[0] * pos->f[0] + sp64.f[1] * pos->f[1] + sp64.f[2] * pos->f[2]) {
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
bool camIsPosInFovAndVisibleRoom(RoomNum *rooms, struct coord *pos, f32 arg2)
{
	s32 i;
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
