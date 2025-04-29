#include <ultra64.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "constants.h"
#include "game/quaternion.h"
#include "game/mtxutils.h"
#include "game/utils.h"
#include "game/camera.h"
#include "game/sky.h"
#include "game/env.h"
#include "game/pad.h"
#include "game/tex.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/sched.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "video.h"
#include "game/gfxmemory.h"
#include "game/artifacts.h"
#include "game/player.h"

#define CORNERSTATE_NONE     0x0
#define CORNERSTATE_BR       0x1
#define CORNERSTATE_BL       0x2
#define CORNERSTATE_BOTTOM   0x3
#define CORNERSTATE_TR       0x4
#define CORNERSTATE_RIGHT    0x5
#define CORNERSTATE_TR_BL_BR 0x7
#define CORNERSTATE_TL       0x8
#define CORNERSTATE_LEFT     0xa
#define CORNERSTATE_TL_BL_BR 0xb
#define CORNERSTATE_TOP      0xc
#define CORNERSTATE_TL_TR_BR 0xd
#define CORNERSTATE_TL_TR_BL 0xe
#define CORNERSTATE_FULL     0xf

uint32_t g_SkyStageNum;
bool g_SkyLightningActive;
Mtx g_SkyMtx;
struct coord g_SunPositions[3]; // relative to centre screen, with a huge scale
float g_SunScreenXPositions[4];
float g_SunScreenYPositions[4];

float g_SkyCloudOffset = 0;
float g_SkyWindSpeed = 1;
float g_SunAlphaFracs[3] = {0};
int g_SunFlareTimers240[3] = {0};

uint32_t FloatToUInt32(float arg0)
{
	if (arg0 > 32767.9f) {
		arg0 = 32767.9f;
	}

	if (arg0 < -32767.9f) {
		arg0 = -32767.9f;
	}

	return (uint32_t)(arg0 * 65536);
}

void skyGetWorldPosFromScreenPos(float left, float top, struct coord *dst)
{
	Mtx *mtx = camGetProjectionMtx();
	float pos[2];

	pos[0] = left + camGetScreenLeft();
	pos[1] = top + camGetScreenTop() + envGetCurrent()->clouds_height;

	camProjectScreenToWorldDir(pos, dst, 100);
	mtx4RotateVecInPlace(mtx, dst);
}

bool skyIsScreenCornerInSky(struct coord *corner3dpos, struct coord *dstpos, float *dstfrac)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	float f12 = 2.0f * corner3dpos->y / sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2] + 0.0001f);
	float sp2c;
	float f12_2;
	float sp24;

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*dstfrac = 1.0f - f12;

	if (corner3dpos->y == 0.0f) {
		sp24 = 0.01f;
	} else {
		sp24 = corner3dpos->y;
	}

	if (sp24 > 0.0f) {
		sp2c = (envGetCurrent()->clouds_scale - campos->y) / sp24;
		f12_2 = sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		dstpos->x = campos->x + sp2c * corner3dpos->f[0];
		dstpos->y = campos->y + sp2c * sp24;
		dstpos->z = campos->z + sp2c * corner3dpos->f[2];

		return true;
	}

	return false;
}

bool skyIsCornerInWater(struct coord *corner3dpos, struct coord *dstpos, float *dstfrac)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	float f12 = -2.0f * corner3dpos->y / sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2] + 0.0001f);
	float sp2c;
	float f12_2;
	float sp24;

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*dstfrac = 1.0f - f12;

	if (corner3dpos->y == 0.0f) {
		sp24 = -0.01f;
	} else {
		sp24 = corner3dpos->y;
	}

	if (sp24 < 0.0f) {
		sp2c = (envGetCurrent()->water_scale - campos->y) / sp24;
		f12_2 = sqrtf(corner3dpos->f[0] * corner3dpos->f[0] + corner3dpos->f[2] * corner3dpos->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		dstpos->x = campos->x + sp2c * corner3dpos->f[0];
		dstpos->y = campos->y + sp2c * sp24;
		dstpos->z = campos->z + sp2c * corner3dpos->f[2];

		return true;
	}

	return false;
}

/**
 * Scale base based on the height percentage between base and ref...
 * except the new y is zero.
 */
void skyCalculateEdgeVertex(struct coord *base, struct coord *ref, struct coord *out)
{
	float mult = base->y / (base->y - ref->y);

	out->x = (ref->x - base->x) * mult + base->x;
	out->y = 0;
	out->z = (ref->z - base->z) * mult + base->z;
}

float skyClamp(float value, float min, float max)
{
	if (value < min) {
		return min;
	}

	if (value > max) {
		return max;
	}

	return value;
}

float skyRound(float value)
{
	return (int)(value + 0.5f);
}

void skyChooseCloudVtxColour(struct skyvtx3d *arg0, float arg1)
{
	struct environment *env = envGetCurrent();
	float scale = 1.0f - arg1;
	float r = env->sky_r;
	float g = env->sky_g;
	float b = env->sky_b;

	arg0->r = r + env->clouds_r * (1.0f - r * (1.0f / 255.0f)) * scale;
	arg0->g = g + env->clouds_g * (1.0f - g * (1.0f / 255.0f)) * scale;
	arg0->b = b + env->clouds_b * (1.0f - b * (1.0f / 255.0f)) * scale;

	if (g_SkyLightningActive) {
		arg0->r = arg0->g = arg0->b = 0xff;
	}

	arg0->a = 0xff;
}

void skyChooseWaterVtxColour(struct skyvtx3d *arg0, float arg1)
{
	struct environment *env = envGetCurrent();
	float scale = 1.0f - arg1;
	float r = env->sky_r;
	float g = env->sky_g;
	float b = env->sky_b;

	arg0->r = r + env->water_r * (1.0f - r * (1.0f / 255.0f)) * scale;
	arg0->g = g + env->water_g * (1.0f - g * (1.0f / 255.0f)) * scale;
	arg0->b = b + env->water_b * (1.0f - b * (1.0f / 255.0f)) * scale;
	arg0->a = 0xff;
}

Gfx *skyRender(Gfx *gdl)
{
	struct coord tl3dpos;
	struct coord tr3dpos;
	struct coord bl3dpos;
	struct coord br3dpos;
	struct coord sp674;
	struct coord sp668;
	struct coord sp65c;
	struct coord sp650;
	struct coord sp644;
	struct coord sp638;
	struct coord sp62c;
	struct coord sp620;
	struct coord sp614;
	struct coord sp608;
	struct coord sp5fc;
	struct coord sp5f0;
	struct coord sp5e4;
	struct coord sp5d8;
	struct coord sp5cc;
	struct coord sp5c0;
	struct coord sp5b4;
	struct coord sp5a8;
	struct coord sp59c;
	struct coord sp590;
	float sp58c;
	float sp588;
	float sp584;
	float sp580;
	float sp57c;
	float sp578;
	float sp574;
	float sp570;
	float sp56c;
	float sp568;
	float sp564;
	float sp560;
	float sp55c;
	float sp558;
	float sp554;
	float sp550;
	float sp54c;
	float sp548;
	int numvertices;
	int j;
	int cornerstate;
	int tlcornerissky;
	int trcornerissky;
	int blcornerissky;
	int brcornerissky;
	struct skyvtx3d skyvertices3d[5];
	struct skyvtx3d watervertices3d[5];
	float tmp;
	float scale;
	bool sp430;
	struct environment *env;

	sp430 = false;
	env = envGetCurrent();

	if (!env->clouds_enabled || g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		if (PLAYERCOUNT() == 1) {
			gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);

			if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
				gdl = viSetFillColour(gdl, 0, 0, 0);
			} else {
				gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
			}

			gfx_Fill_Rectangle(gdl++, viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth() - 1,
					viGetViewTop() + viGetViewHeight() - 1);

			return gdl;
		}

		gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);

		if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
			gdl = viSetFillColour(gdl, 0, 0, 0);
		} else {
			gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
		}

		gfx_Set_Render_Mode(gdl++, G_RM_NOOP, G_RM_NOOP2);

		gfx_Fill_Rectangle(gdl++,
				g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop,
				g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth - 1,
				g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight - 1);

		return gdl;
	}

	gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);

	skyGetWorldPosFromScreenPos(-4.0f, -4.0f, &tl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() + 4.0f, -4.0f, &tr3dpos);
	skyGetWorldPosFromScreenPos(-4.0f, camGetScreenHeight() + 4.0f, &bl3dpos);
	skyGetWorldPosFromScreenPos(camGetScreenWidth() + 4.0f, camGetScreenHeight() + 4.0f, &br3dpos);

	tlcornerissky = skyIsScreenCornerInSky(&tl3dpos, &sp644, &sp58c);
	trcornerissky = skyIsScreenCornerInSky(&tr3dpos, &sp638, &sp588);
	blcornerissky = skyIsScreenCornerInSky(&bl3dpos, &sp62c, &sp584);
	brcornerissky = skyIsScreenCornerInSky(&br3dpos, &sp620, &sp580);

	skyIsCornerInWater(&tl3dpos, &sp5e4, &sp56c);
	skyIsCornerInWater(&tr3dpos, &sp5d8, &sp568);
	skyIsCornerInWater(&bl3dpos, &sp5cc, &sp564);
	skyIsCornerInWater(&br3dpos, &sp5c0, &sp560);

	// For each screen edge, check if one vertex is off-sky and the other is on-sky.
	// If so, calculate where along the the edge the sky starts/ends.
	if (tlcornerissky != blcornerissky) {
		sp54c = camGetScreenTop() + camGetScreenHeight() * (tl3dpos.f[1] / (tl3dpos.f[1] - bl3dpos.f[1]));

		skyGetWorldPosFromScreenPos(0.0f, sp54c, &sp65c);
		skyCalculateEdgeVertex(&tl3dpos, &bl3dpos, &sp65c);
		skyIsScreenCornerInSky(&sp65c, &sp5fc, &sp574);
		skyIsCornerInWater(&sp65c, &sp59c, &sp554);
	} else {
		sp54c = 0.0f;
	}

	if (trcornerissky != brcornerissky) {
		sp548 = camGetScreenTop() + camGetScreenHeight() * (tr3dpos.f[1] / (tr3dpos.f[1] - br3dpos.f[1]));

		skyGetWorldPosFromScreenPos(camGetScreenWidth() - 0.1f, sp548, &sp650);
		skyCalculateEdgeVertex(&tr3dpos, &br3dpos, &sp650);
		skyIsScreenCornerInSky(&sp650, &sp5f0, &sp570);
		skyIsCornerInWater(&sp650, &sp590, &sp550);
	} else {
		sp548 = 0.0f;
	}

	if (tlcornerissky != trcornerissky) {
		skyGetWorldPosFromScreenPos(camGetScreenLeft() + camGetScreenWidth() * (tl3dpos.f[1] / (tl3dpos.f[1] - tr3dpos.f[1])), 0.0f, &sp674);
		skyCalculateEdgeVertex(&tl3dpos, &tr3dpos, &sp674);
		skyIsScreenCornerInSky(&sp674, &sp614, &sp57c);
		skyIsCornerInWater(&sp674, &sp5b4, &sp55c);
	}

	if (blcornerissky != brcornerissky) {
		tmp = camGetScreenLeft() + camGetScreenWidth() * (bl3dpos.f[1] / (bl3dpos.f[1] - br3dpos.f[1]));

		skyGetWorldPosFromScreenPos(tmp, camGetScreenHeight() - 0.1f, &sp668);
		skyCalculateEdgeVertex(&bl3dpos, &br3dpos, &sp668);
		skyIsScreenCornerInSky(&sp668, &sp608, &sp578);
		skyIsCornerInWater(&sp668, &sp5a8, &sp558);
	}

	cornerstate = (tlcornerissky << 3) | (trcornerissky << 2) | (blcornerissky << 1) | brcornerissky;

	/**
	 * Do maths stuff for the ground/water/below-horizon plane.
	 *
	 * The CORNERSTATE constants denote which corners are in the sky,
	 * which is why these cases appear to be inverted.
	 */
	switch (cornerstate) {
	case CORNERSTATE_FULL:
		// All four screen corners are in the sky.
		numvertices = 0;
		scale = 1.0f / 30.0f;
		break;
	case CORNERSTATE_NONE:
		// All four screen corners are on the ground.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp5cc.f[0] * scale;
		watervertices3d[2].y = sp5cc.f[1] * scale;
		watervertices3d[2].z = sp5cc.f[2] * scale;
		watervertices3d[3].x = sp5c0.f[0] * scale;
		watervertices3d[3].y = sp5c0.f[1] * scale;
		watervertices3d[3].z = sp5c0.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5cc.f[0];
		watervertices3d[2].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5c0.f[0];
		watervertices3d[3].t = sp5c0.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp564);
		skyChooseWaterVtxColour(&watervertices3d[3], sp560);
		break;
	case CORNERSTATE_BOTTOM:
		// The bottom corners are in the sky.
		// This is possible by turning the drugspy upside down in Air Base.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp59c.f[0] * scale;
		watervertices3d[2].y = sp59c.f[1] * scale;
		watervertices3d[2].z = sp59c.f[2] * scale;
		watervertices3d[3].x = sp590.f[0] * scale;
		watervertices3d[3].y = sp590.f[1] * scale;
		watervertices3d[3].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp59c.f[0];
		watervertices3d[2].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp590.f[0];
		watervertices3d[3].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp554);
		skyChooseWaterVtxColour(&watervertices3d[3], sp550);
		break;
	case CORNERSTATE_TOP:
		// The top corners are in the sky. A common occurrence.
		numvertices = 4;
		sp430 = true;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5cc.f[0] * scale;
		watervertices3d[1].y = sp5cc.f[1] * scale;
		watervertices3d[1].z = sp5cc.f[2] * scale;
		watervertices3d[2].x = sp590.f[0] * scale;
		watervertices3d[2].y = sp590.f[1] * scale;
		watervertices3d[2].z = sp590.f[2] * scale;
		watervertices3d[3].x = sp59c.f[0] * scale;
		watervertices3d[3].y = sp59c.f[1] * scale;
		watervertices3d[3].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5cc.f[0];
		watervertices3d[1].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp590.f[0];
		watervertices3d[2].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp59c.f[0];
		watervertices3d[3].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp564);
		skyChooseWaterVtxColour(&watervertices3d[2], sp550);
		skyChooseWaterVtxColour(&watervertices3d[3], sp554);
		break;
	case CORNERSTATE_LEFT:
		// The left side corners are in the sky.
		// This would happen if the camera rolls significantly.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp5c0.f[0] * scale;
		watervertices3d[1].y = sp5c0.f[1] * scale;
		watervertices3d[1].z = sp5c0.f[2] * scale;
		watervertices3d[2].x = sp5b4.f[0] * scale;
		watervertices3d[2].y = sp5b4.f[1] * scale;
		watervertices3d[2].z = sp5b4.f[2] * scale;
		watervertices3d[3].x = sp5a8.f[0] * scale;
		watervertices3d[3].y = sp5a8.f[1] * scale;
		watervertices3d[3].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5c0.f[0];
		watervertices3d[1].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5b4.f[0];
		watervertices3d[2].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5a8.f[0];
		watervertices3d[3].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp560);
		skyChooseWaterVtxColour(&watervertices3d[2], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[3], sp558);
		break;
	case CORNERSTATE_RIGHT:
		// The right side corners are in the sky.
		// This would happen if the camera rolls significantly.
		numvertices = 4;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp5e4.f[0] * scale;
		watervertices3d[1].y = sp5e4.f[1] * scale;
		watervertices3d[1].z = sp5e4.f[2] * scale;
		watervertices3d[2].x = sp5a8.f[0] * scale;
		watervertices3d[2].y = sp5a8.f[1] * scale;
		watervertices3d[2].z = sp5a8.f[2] * scale;
		watervertices3d[3].x = sp5b4.f[0] * scale;
		watervertices3d[3].y = sp5b4.f[1] * scale;
		watervertices3d[3].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5e4.f[0];
		watervertices3d[1].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5a8.f[0];
		watervertices3d[2].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5b4.f[0];
		watervertices3d[3].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp558);
		skyChooseWaterVtxColour(&watervertices3d[3], sp55c);
		break;
	case CORNERSTATE_TL_TR_BL:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5a8.f[0] * scale;
		watervertices3d[1].y = sp5a8.f[1] * scale;
		watervertices3d[1].z = sp5a8.f[2] * scale;
		watervertices3d[2].x = sp590.f[0] * scale;
		watervertices3d[2].y = sp590.f[1] * scale;
		watervertices3d[2].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5a8.f[0];
		watervertices3d[1].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp590.f[0];
		watervertices3d[2].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp558);
		skyChooseWaterVtxColour(&watervertices3d[2], sp550);
		break;
	case CORNERSTATE_TL_TR_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp59c.f[0] * scale;
		watervertices3d[1].y = sp59c.f[1] * scale;
		watervertices3d[1].z = sp59c.f[2] * scale;
		watervertices3d[2].x = sp5a8.f[0] * scale;
		watervertices3d[2].y = sp5a8.f[1] * scale;
		watervertices3d[2].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp59c.f[0];
		watervertices3d[1].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5a8.f[0];
		watervertices3d[2].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp554);
		skyChooseWaterVtxColour(&watervertices3d[2], sp558);
		break;
	case CORNERSTATE_TL_BL_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp590.f[0] * scale;
		watervertices3d[1].y = sp590.f[1] * scale;
		watervertices3d[1].z = sp590.f[2] * scale;
		watervertices3d[2].x = sp5b4.f[0] * scale;
		watervertices3d[2].y = sp5b4.f[1] * scale;
		watervertices3d[2].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp590.f[0];
		watervertices3d[1].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5b4.f[0];
		watervertices3d[2].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp550);
		skyChooseWaterVtxColour(&watervertices3d[2], sp55c);
		break;
	case CORNERSTATE_TR_BL_BR:
		numvertices = 3;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5b4.f[0] * scale;
		watervertices3d[1].y = sp5b4.f[1] * scale;
		watervertices3d[1].z = sp5b4.f[2] * scale;
		watervertices3d[2].x = sp59c.f[0] * scale;
		watervertices3d[2].y = sp59c.f[1] * scale;
		watervertices3d[2].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5b4.f[0];
		watervertices3d[1].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp59c.f[0];
		watervertices3d[2].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp554);
		break;
	case CORNERSTATE_BR:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5cc.f[0] * scale;
		watervertices3d[0].y = sp5cc.f[1] * scale;
		watervertices3d[0].z = sp5cc.f[2] * scale;
		watervertices3d[1].x = sp5e4.f[0] * scale;
		watervertices3d[1].y = sp5e4.f[1] * scale;
		watervertices3d[1].z = sp5e4.f[2] * scale;
		watervertices3d[2].x = sp5d8.f[0] * scale;
		watervertices3d[2].y = sp5d8.f[1] * scale;
		watervertices3d[2].z = sp5d8.f[2] * scale;
		watervertices3d[3].x = sp590.f[0] * scale;
		watervertices3d[3].y = sp590.f[1] * scale;
		watervertices3d[3].z = sp590.f[2] * scale;
		watervertices3d[4].x = sp5a8.f[0] * scale;
		watervertices3d[4].y = sp5a8.f[1] * scale;
		watervertices3d[4].z = sp5a8.f[2] * scale;
		watervertices3d[0].s = sp5cc.f[0];
		watervertices3d[0].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5e4.f[0];
		watervertices3d[1].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5d8.f[0];
		watervertices3d[2].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp590.f[0];
		watervertices3d[3].t = sp590.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp5a8.f[0];
		watervertices3d[4].t = sp5a8.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp564);
		skyChooseWaterVtxColour(&watervertices3d[1], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[2], sp568);
		skyChooseWaterVtxColour(&watervertices3d[3], sp550);
		skyChooseWaterVtxColour(&watervertices3d[4], sp558);
		break;
	case CORNERSTATE_BL:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5e4.f[0] * scale;
		watervertices3d[0].y = sp5e4.f[1] * scale;
		watervertices3d[0].z = sp5e4.f[2] * scale;
		watervertices3d[1].x = sp5d8.f[0] * scale;
		watervertices3d[1].y = sp5d8.f[1] * scale;
		watervertices3d[1].z = sp5d8.f[2] * scale;
		watervertices3d[2].x = sp5c0.f[0] * scale;
		watervertices3d[2].y = sp5c0.f[1] * scale;
		watervertices3d[2].z = sp5c0.f[2] * scale;
		watervertices3d[3].x = sp5a8.f[0] * scale;
		watervertices3d[3].y = sp5a8.f[1] * scale;
		watervertices3d[3].z = sp5a8.f[2] * scale;
		watervertices3d[4].x = sp59c.f[0] * scale;
		watervertices3d[4].y = sp59c.f[1] * scale;
		watervertices3d[4].z = sp59c.f[2] * scale;
		watervertices3d[0].s = sp5e4.f[0];
		watervertices3d[0].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5d8.f[0];
		watervertices3d[1].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5c0.f[0];
		watervertices3d[2].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5a8.f[0];
		watervertices3d[3].t = sp5a8.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp59c.f[0];
		watervertices3d[4].t = sp59c.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[1], sp568);
		skyChooseWaterVtxColour(&watervertices3d[2], sp560);
		skyChooseWaterVtxColour(&watervertices3d[3], sp558);
		skyChooseWaterVtxColour(&watervertices3d[4], sp554);
		break;
	case CORNERSTATE_TR:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5c0.f[0] * scale;
		watervertices3d[0].y = sp5c0.f[1] * scale;
		watervertices3d[0].z = sp5c0.f[2] * scale;
		watervertices3d[1].x = sp5cc.f[0] * scale;
		watervertices3d[1].y = sp5cc.f[1] * scale;
		watervertices3d[1].z = sp5cc.f[2] * scale;
		watervertices3d[2].x = sp5e4.f[0] * scale;
		watervertices3d[2].y = sp5e4.f[1] * scale;
		watervertices3d[2].z = sp5e4.f[2] * scale;
		watervertices3d[3].x = sp5b4.f[0] * scale;
		watervertices3d[3].y = sp5b4.f[1] * scale;
		watervertices3d[3].z = sp5b4.f[2] * scale;
		watervertices3d[4].x = sp590.f[0] * scale;
		watervertices3d[4].y = sp590.f[1] * scale;
		watervertices3d[4].z = sp590.f[2] * scale;
		watervertices3d[0].s = sp5c0.f[0];
		watervertices3d[0].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5cc.f[0];
		watervertices3d[1].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5e4.f[0];
		watervertices3d[2].t = sp5e4.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp5b4.f[0];
		watervertices3d[3].t = sp5b4.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp590.f[0];
		watervertices3d[4].t = sp590.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp560);
		skyChooseWaterVtxColour(&watervertices3d[1], sp564);
		skyChooseWaterVtxColour(&watervertices3d[2], sp56c);
		skyChooseWaterVtxColour(&watervertices3d[3], sp55c);
		skyChooseWaterVtxColour(&watervertices3d[4], sp550);
		break;
	case CORNERSTATE_TL:
		numvertices = 5;
		scale = 1.0f / 30.0f;
		watervertices3d[0].x = sp5d8.f[0] * scale;
		watervertices3d[0].y = sp5d8.f[1] * scale;
		watervertices3d[0].z = sp5d8.f[2] * scale;
		watervertices3d[1].x = sp5c0.f[0] * scale;
		watervertices3d[1].y = sp5c0.f[1] * scale;
		watervertices3d[1].z = sp5c0.f[2] * scale;
		watervertices3d[2].x = sp5cc.f[0] * scale;
		watervertices3d[2].y = sp5cc.f[1] * scale;
		watervertices3d[2].z = sp5cc.f[2] * scale;
		watervertices3d[3].x = sp59c.f[0] * scale;
		watervertices3d[3].y = sp59c.f[1] * scale;
		watervertices3d[3].z = sp59c.f[2] * scale;
		watervertices3d[4].x = sp5b4.f[0] * scale;
		watervertices3d[4].y = sp5b4.f[1] * scale;
		watervertices3d[4].z = sp5b4.f[2] * scale;
		watervertices3d[0].s = sp5d8.f[0];
		watervertices3d[0].t = sp5d8.f[2] + g_SkyCloudOffset;
		watervertices3d[1].s = sp5c0.f[0];
		watervertices3d[1].t = sp5c0.f[2] + g_SkyCloudOffset;
		watervertices3d[2].s = sp5cc.f[0];
		watervertices3d[2].t = sp5cc.f[2] + g_SkyCloudOffset;
		watervertices3d[3].s = sp59c.f[0];
		watervertices3d[3].t = sp59c.f[2] + g_SkyCloudOffset;
		watervertices3d[4].s = sp5b4.f[0];
		watervertices3d[4].t = sp5b4.f[2] + g_SkyCloudOffset;

		skyChooseWaterVtxColour(&watervertices3d[0], sp568);
		skyChooseWaterVtxColour(&watervertices3d[1], sp560);
		skyChooseWaterVtxColour(&watervertices3d[2], sp564);
		skyChooseWaterVtxColour(&watervertices3d[3], sp554);
		skyChooseWaterVtxColour(&watervertices3d[4], sp55c);
		break;
	default:
		return gdl;
	}

	if (numvertices > 0) {
		// Some corners are not in the sky, so consider water
		Mtx sp3cc;
		Mtx sp38c;
		struct skyvtx2d watervertices2d[5];
		int i;

		mtx4MultMtx4(camGetSkyMtx(), camGetPlayerWorldToScreenMtx(), &sp3cc);
		mtxScale(&g_SkyMtx, 1.0f / scale, 1.0f / scale, 1.0f / scale);
		mtx4MultMtx4(&sp3cc, &g_SkyMtx, &sp38c);

		for (i = 0; i < numvertices; i++) {
			skyConvertVertex(&watervertices3d[i], &sp38c, 130, 65535.0f, 65535.0f, &watervertices2d[i]);

			watervertices2d[i].x = skyClamp(watervertices2d[i].x, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
			watervertices2d[i].y = skyClamp(watervertices2d[i].y, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);

			if (watervertices2d[i].y > camGetScreenTop() * 4.0f + 4.0f
					&& watervertices2d[i].y < (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 4.0f) {
				watervertices2d[i].y -= 4.0f;
			}
		}

		if (!env->water_enabled) {
			float x1 = 1279.0f;
			float y1 = 959.0f;
			float x2 = 0.0f;
			float y2 = 0.0f;

			for (j = 0; j < numvertices; j++) {
				if (watervertices2d[j].x < x1) {
					x1 = watervertices2d[j].x;
				}

				if (watervertices2d[j].x > x2) {
					x2 = watervertices2d[j].x;
				}

				if (watervertices2d[j].y < y1) {
					y1 = watervertices2d[j].y;
				}

				if (watervertices2d[j].y > y2) {
					y2 = watervertices2d[j].y;
				}
			}

			gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);
			gfx_Set_Render_Mode(gdl++, G_RM_NOOP, G_RM_NOOP2);
			gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
			gfx_Fill_Rectangle(gdl++, (int)(x1 * 0.25f), (int)(y1 * 0.25f), (int)(x2 * 0.25f), (int)(y2 * 0.25f));
			gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
		} else {

			texSelect(&gdl, &g_TexSkyWaterConfigs[env->water_type], 1, 0, 2, 1, NULL);

			gfx_Set_Render_Mode(gdl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

			Vtx *verts = gfxAllocateVertices(numvertices);
			Col *cols = gfxAllocateColours(numvertices);
			Mtx *mtx = gfxAllocateMatrix();
			mtx4MultMtx4(camGetPlayerWorldToScreenMtx(), &g_SkyMtx, mtx);

			gSPSetExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
			gfx_Matrix(gdl++, mtx, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
			gfx_Color(gdl++, cols, numvertices);
			gSPVertex(gdl++, (uintptr_t)(verts), numvertices, 0);

			for (int i = 0; i < numvertices; ++i) {
				verts[i].x = watervertices3d[i].x;
				verts[i].y = watervertices3d[i].y;
				verts[i].z = watervertices3d[i].z;
				verts[i].s = skyClamp(watervertices3d[i].s * 0.1f + g_SkyCloudOffset, -32768.f, 32767.f);
				verts[i].t = skyClamp((watervertices3d[i].t  - g_SkyCloudOffset) * 0.1f + g_SkyCloudOffset, -32768.f, 32767.f);
				verts[i].colour = i * 4;
				cols[i].r = watervertices3d[i].r;
				cols[i].g = watervertices3d[i].g;
				cols[i].b = watervertices3d[i].b;
				cols[i].a = watervertices3d[i].a;
			}

			if (numvertices == 4) {
				gfx_Tri2(gdl++, 0, 1, 3, 3, 2, 0);
			} else if (numvertices == 5) {
				gfx_Tri3(gdl++, 0, 1, 2, 0, 2, 3, 0, 3, 4);
			} else if (numvertices == 3) {
				gfx_Tri1(gdl++, 0, 1, 2);
			}

			gfx_Pop_Matrix(gdl++, G_MTX_MODELVIEW);
			gSPClearExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
		}
	}

	/**
	 * Maths for the upper half of the skydome.
	 */
	switch (cornerstate) {
	case CORNERSTATE_NONE:
		// All four screen corners are on the ground.
		return gdl;
	case CORNERSTATE_FULL:
		// All four screen corners are in the sky.
		numvertices = 4;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp62c.f[0] * scale;
		skyvertices3d[2].y = sp62c.f[1] * scale;
		skyvertices3d[2].z = sp62c.f[2] * scale;
		skyvertices3d[3].x = sp620.f[0] * scale;
		skyvertices3d[3].y = sp620.f[1] * scale;
		skyvertices3d[3].z = sp620.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp62c.f[0] * 0.1f;
		skyvertices3d[2].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp620.f[0] * 0.1f;
		skyvertices3d[3].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp580);
		break;
	case CORNERSTATE_TOP:
		numvertices = 4;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp5fc.f[0] * scale;
		skyvertices3d[2].y = sp5fc.f[1] * scale;
		skyvertices3d[2].z = sp5fc.f[2] * scale;
		skyvertices3d[3].x = sp5f0.f[0] * scale;
		skyvertices3d[3].y = sp5f0.f[1] * scale;
		skyvertices3d[3].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[2].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[3].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp570);
		break;
	case CORNERSTATE_BOTTOM:
		numvertices = 4;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp62c.f[0] * scale;
		skyvertices3d[1].y = sp62c.f[1] * scale;
		skyvertices3d[1].z = sp62c.f[2] * scale;
		skyvertices3d[2].x = sp5f0.f[0] * scale;
		skyvertices3d[2].y = sp5f0.f[1] * scale;
		skyvertices3d[2].z = sp5f0.f[2] * scale;
		skyvertices3d[3].x = sp5fc.f[0] * scale;
		skyvertices3d[3].y = sp5fc.f[1] * scale;
		skyvertices3d[3].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp62c.f[0] * 0.1f;
		skyvertices3d[1].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[2].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[3].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp574);
		break;
	case CORNERSTATE_RIGHT:
		numvertices = 4;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp620.f[0] * scale;
		skyvertices3d[1].y = sp620.f[1] * scale;
		skyvertices3d[1].z = sp620.f[2] * scale;
		skyvertices3d[2].x = sp614.f[0] * scale;
		skyvertices3d[2].y = sp614.f[1] * scale;
		skyvertices3d[2].z = sp614.f[2] * scale;
		skyvertices3d[3].x = sp608.f[0] * scale;
		skyvertices3d[3].y = sp608.f[1] * scale;
		skyvertices3d[3].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp620.f[0] * 0.1f;
		skyvertices3d[1].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp614.f[0] * 0.1f;
		skyvertices3d[2].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp608.f[0] * 0.1f;
		skyvertices3d[3].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp578);
		break;
	case CORNERSTATE_LEFT:
		numvertices = 4;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp644.f[0] * scale;
		skyvertices3d[1].y = sp644.f[1] * scale;
		skyvertices3d[1].z = sp644.f[2] * scale;
		skyvertices3d[2].x = sp608.f[0] * scale;
		skyvertices3d[2].y = sp608.f[1] * scale;
		skyvertices3d[2].z = sp608.f[2] * scale;
		skyvertices3d[3].x = sp614.f[0] * scale;
		skyvertices3d[3].y = sp614.f[1] * scale;
		skyvertices3d[3].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp644.f[0] * 0.1f;
		skyvertices3d[1].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp608.f[0] * 0.1f;
		skyvertices3d[2].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp614.f[0] * 0.1f;
		skyvertices3d[3].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp57c);
		break;
	case CORNERSTATE_BR:
		numvertices = 3;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp608.f[0] * scale;
		skyvertices3d[1].y = sp608.f[1] * scale;
		skyvertices3d[1].z = sp608.f[2] * scale;
		skyvertices3d[2].x = sp5f0.f[0] * scale;
		skyvertices3d[2].y = sp5f0.f[1] * scale;
		skyvertices3d[2].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp608.f[0] * 0.1f;
		skyvertices3d[1].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[2].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp570);
		break;
	case CORNERSTATE_BL:
		numvertices = 3;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp5fc.f[0] * scale;
		skyvertices3d[1].y = sp5fc.f[1] * scale;
		skyvertices3d[1].z = sp5fc.f[2] * scale;
		skyvertices3d[2].x = sp608.f[0] * scale;
		skyvertices3d[2].y = sp608.f[1] * scale;
		skyvertices3d[2].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[1].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp608.f[0] * 0.1f;
		skyvertices3d[2].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp578);
		break;
	case CORNERSTATE_TR:
		numvertices = 3;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp5f0.f[0] * scale;
		skyvertices3d[1].y = sp5f0.f[1] * scale;
		skyvertices3d[1].z = sp5f0.f[2] * scale;
		skyvertices3d[2].x = sp614.f[0] * scale;
		skyvertices3d[2].y = sp614.f[1] * scale;
		skyvertices3d[2].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[1].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp614.f[0] * 0.1f;
		skyvertices3d[2].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp57c);
		break;
	case CORNERSTATE_TL:
		numvertices = 3;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp614.f[0] * scale;
		skyvertices3d[1].y = sp614.f[1] * scale;
		skyvertices3d[1].z = sp614.f[2] * scale;
		skyvertices3d[2].x = sp5fc.f[0] * scale;
		skyvertices3d[2].y = sp5fc.f[1] * scale;
		skyvertices3d[2].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp614.f[0] * 0.1f;
		skyvertices3d[1].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[2].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp574);
		break;
	case CORNERSTATE_TL_TR_BL:
		numvertices = 5;
		skyvertices3d[0].x = sp62c.f[0] * scale;
		skyvertices3d[0].y = sp62c.f[1] * scale;
		skyvertices3d[0].z = sp62c.f[2] * scale;
		skyvertices3d[1].x = sp644.f[0] * scale;
		skyvertices3d[1].y = sp644.f[1] * scale;
		skyvertices3d[1].z = sp644.f[2] * scale;
		skyvertices3d[2].x = sp638.f[0] * scale;
		skyvertices3d[2].y = sp638.f[1] * scale;
		skyvertices3d[2].z = sp638.f[2] * scale;
		skyvertices3d[3].x = sp5f0.f[0] * scale;
		skyvertices3d[3].y = sp5f0.f[1] * scale;
		skyvertices3d[3].z = sp5f0.f[2] * scale;
		skyvertices3d[4].x = sp608.f[0] * scale;
		skyvertices3d[4].y = sp608.f[1] * scale;
		skyvertices3d[4].z = sp608.f[2] * scale;
		skyvertices3d[0].s = sp62c.f[0] * 0.1f;
		skyvertices3d[0].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp644.f[0] * 0.1f;
		skyvertices3d[1].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp638.f[0] * 0.1f;
		skyvertices3d[2].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[3].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp608.f[0] * 0.1f;
		skyvertices3d[4].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp570);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp578);
		break;
	case CORNERSTATE_TL_TR_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp644.f[0] * scale;
		skyvertices3d[0].y = sp644.f[1] * scale;
		skyvertices3d[0].z = sp644.f[2] * scale;
		skyvertices3d[1].x = sp638.f[0] * scale;
		skyvertices3d[1].y = sp638.f[1] * scale;
		skyvertices3d[1].z = sp638.f[2] * scale;
		skyvertices3d[2].x = sp620.f[0] * scale;
		skyvertices3d[2].y = sp620.f[1] * scale;
		skyvertices3d[2].z = sp620.f[2] * scale;
		skyvertices3d[3].x = sp608.f[0] * scale;
		skyvertices3d[3].y = sp608.f[1] * scale;
		skyvertices3d[3].z = sp608.f[2] * scale;
		skyvertices3d[4].x = sp5fc.f[0] * scale;
		skyvertices3d[4].y = sp5fc.f[1] * scale;
		skyvertices3d[4].z = sp5fc.f[2] * scale;
		skyvertices3d[0].s = sp644.f[0] * 0.1f;
		skyvertices3d[0].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp638.f[0] * 0.1f;
		skyvertices3d[1].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp620.f[0] * 0.1f;
		skyvertices3d[2].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp608.f[0] * 0.1f;
		skyvertices3d[3].t = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[4].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp578);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp574);
		break;
	case CORNERSTATE_TL_BL_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp620.f[0] * scale;
		skyvertices3d[0].y = sp620.f[1] * scale;
		skyvertices3d[0].z = sp620.f[2] * scale;
		skyvertices3d[1].x = sp62c.f[0] * scale;
		skyvertices3d[1].y = sp62c.f[1] * scale;
		skyvertices3d[1].z = sp62c.f[2] * scale;
		skyvertices3d[2].x = sp644.f[0] * scale;
		skyvertices3d[2].y = sp644.f[1] * scale;
		skyvertices3d[2].z = sp644.f[2] * scale;
		skyvertices3d[3].x = sp614.f[0] * scale;
		skyvertices3d[3].y = sp614.f[1] * scale;
		skyvertices3d[3].z = sp614.f[2] * scale;
		skyvertices3d[4].x = sp5f0.f[0] * scale;
		skyvertices3d[4].y = sp5f0.f[1] * scale;
		skyvertices3d[4].z = sp5f0.f[2] * scale;
		skyvertices3d[0].s = sp620.f[0] * 0.1f;
		skyvertices3d[0].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp62c.f[0] * 0.1f;
		skyvertices3d[1].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp644.f[0] * 0.1f;
		skyvertices3d[2].t = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp614.f[0] * 0.1f;
		skyvertices3d[3].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp5f0.f[0] * 0.1f;
		skyvertices3d[4].t = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp58c);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp57c);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp570);
		break;
	case CORNERSTATE_TR_BL_BR:
		numvertices = 5;
		skyvertices3d[0].x = sp638.f[0] * scale;
		skyvertices3d[0].y = sp638.f[1] * scale;
		skyvertices3d[0].z = sp638.f[2] * scale;
		skyvertices3d[1].x = sp620.f[0] * scale;
		skyvertices3d[1].y = sp620.f[1] * scale;
		skyvertices3d[1].z = sp620.f[2] * scale;
		skyvertices3d[2].x = sp62c.f[0] * scale;
		skyvertices3d[2].y = sp62c.f[1] * scale;
		skyvertices3d[2].z = sp62c.f[2] * scale;
		skyvertices3d[3].x = sp5fc.f[0] * scale;
		skyvertices3d[3].y = sp5fc.f[1] * scale;
		skyvertices3d[3].z = sp5fc.f[2] * scale;
		skyvertices3d[4].x = sp614.f[0] * scale;
		skyvertices3d[4].y = sp614.f[1] * scale;
		skyvertices3d[4].z = sp614.f[2] * scale;
		skyvertices3d[0].s = sp638.f[0] * 0.1f;
		skyvertices3d[0].t = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[1].s = sp620.f[0] * 0.1f;
		skyvertices3d[1].t = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[2].s = sp62c.f[0] * 0.1f;
		skyvertices3d[2].t = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[3].s = sp5fc.f[0] * 0.1f;
		skyvertices3d[3].t = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		skyvertices3d[4].s = sp614.f[0] * 0.1f;
		skyvertices3d[4].t = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		skyChooseCloudVtxColour(&skyvertices3d[0], sp588);
		skyChooseCloudVtxColour(&skyvertices3d[1], sp580);
		skyChooseCloudVtxColour(&skyvertices3d[2], sp584);
		skyChooseCloudVtxColour(&skyvertices3d[3], sp574);
		skyChooseCloudVtxColour(&skyvertices3d[4], sp57c);
		break;
	default:
		return gdl;
	}

	texSelect(&gdl, &g_TexSkyWaterConfigs[env->clouds_type], 1, 0, 2, 1, NULL);

	RGBA skyColor = {env->sky_r, env->sky_g, env->sky_b, 255};
	gfx_Set_Env_Color(gdl++, skyColor);
	gDPSetCombineLERP(gdl++,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE);

	Mtx sp1ec;
	Mtx sp1ac;
	struct skyvtx2d skyvertices2d[5];
	int i;

	mtx4MultMtx4(camGetSkyMtx(), camGetPlayerWorldToScreenMtx(), &sp1ec);
	mtxScale(&g_SkyMtx, 1.0f / scale, 1.0f / scale, 1.0f / scale);
	mtx4MultMtx4(&sp1ec, &g_SkyMtx, &sp1ac);

	for (i = 0; i < numvertices; i++) {
		skyConvertVertex(&skyvertices3d[i], &sp1ac, 130, 65535.0f, 65535.0f, &skyvertices2d[i]);

		skyvertices2d[i].x = skyClamp(skyvertices2d[i].x, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
		skyvertices2d[i].y = skyClamp(skyvertices2d[i].y, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);
	}

	Vtx *verts = gfxAllocateVertices(numvertices);
	Col *cols = gfxAllocateColours(numvertices);
	Mtx *mtx = gfxAllocateMatrix();
	mtx4MultMtx4(camGetPlayerWorldToScreenMtx(), &g_SkyMtx, mtx);

	gSPSetExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);
	gfx_Matrix(gdl++, mtx, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_PUSH);
	gfx_Color(gdl++, cols, numvertices);
	gSPVertex(gdl++, (uintptr_t)(verts), numvertices, 0);

	for (int i = 0; i < numvertices; ++i) {
		verts[i].x = skyvertices3d[i].x;
		verts[i].y = skyvertices3d[i].y;
		verts[i].z = skyvertices3d[i].z;
		verts[i].s = skyClamp(skyvertices3d[i].s, -32768.f, 32767.f);
		verts[i].t = skyClamp(skyvertices3d[i].t, -32768.f, 32767.f);
		verts[i].colour = i * 4;
		cols[i].r = skyvertices3d[i].r;
		cols[i].g = skyvertices3d[i].g;
		cols[i].b = skyvertices3d[i].b;
		cols[i].a = skyvertices3d[i].a;
	}

	if (numvertices == 4) {
		gfx_Tri2(gdl++, 0, 1, 3, 3, 2, 0);
	} else if (numvertices == 5) {
		gfx_Tri3(gdl++, 0, 1, 2, 0, 2, 3, 0, 3, 4);
	} else if (numvertices == 3) {
		gfx_Tri1(gdl++, 0, 1, 2);
	}

	gfx_Pop_Matrix(gdl++, G_MTX_MODELVIEW);
	gSPClearExtraGeometryModeEXT(gdl++, G_NO_CLIPPING_EXT);

	return gdl;
}

/**
 * Convert a 3D vertex to 2D.
 */
void skyConvertVertex(struct skyvtx3d *srcvtx, Mtx *mtx, uint16_t arg2, float arg3, float arg4, struct skyvtx2d *dstvtx)
{
	float sp68[4];
	float t;
	float s;
	float f22;
	float f0;
	float sp48[4];
	float sp38[4];
	float sp34;
	float sp30;
	float mult;

	mult = arg2 / 65536.0f;

	sp68[0] = (srcvtx->x * (*mtx)[0][0] + srcvtx->y * (*mtx)[1][0] + srcvtx->z * (*mtx)[2][0]) + (*mtx)[3][0];
	sp68[1] = (srcvtx->x * (*mtx)[0][1] + srcvtx->y * (*mtx)[1][1] + srcvtx->z * (*mtx)[2][1]) + (*mtx)[3][1];
	sp68[2] = (srcvtx->x * (*mtx)[0][2] + srcvtx->y * (*mtx)[1][2] + srcvtx->z * (*mtx)[2][2]) + (*mtx)[3][2];
	sp68[3] = (srcvtx->x * (*mtx)[0][3] + srcvtx->y * (*mtx)[1][3] + srcvtx->z * (*mtx)[2][3]) + (*mtx)[3][3];

	s = srcvtx->s * (arg3 * (1.0f / 65536.0f));
	t = srcvtx->t * (arg4 * (1.0f / 65536.0f));

	if (sp68[3] == 0.0f) {
		f22 = 32767.0f;
	} else {
		f22 = 1.0f / (sp68[3] * mult);
	}

	f0 = f22;

	if (f0 < 0.0f) {
		f0 = 32767.0f;
	}

	sp48[0] = sp68[0] * f0 * mult;
	sp48[1] = sp68[1] * f0 * mult;
	sp48[2] = sp68[2] * f0 * mult;
	sp48[3] = sp68[3] * f0 * mult;

	sp34 = camGetScreenWidth();
	sp30 = camGetScreenWidth();
	sp38[0] = sp48[0] * (sp34 + sp34) + (sp30 + sp30 + camGetScreenLeft() * 4);

	sp34 = camGetScreenHeight();
	sp30 = camGetScreenHeight();
	sp38[1] = -sp48[1] * (sp34 + sp34) + (sp30 + sp30 + camGetScreenTop() * 4);

	sp34 = 511.0f;
	sp30 = 511.0f;
	sp38[2] = sp48[2] * sp34 + sp30;

	sp34 = 0;
	sp30 = 0;
	sp38[3] = sp48[3] * sp34 + sp30;

	sp38[0] = skyClamp(sp38[0], -4090.0f, 4090.0f);
	sp38[1] = skyClamp(sp38[1], -4090.0f, 4090.0f);
	sp38[2] = skyClamp(sp38[2], 0.0f, 32767.0f);
	sp38[3] = skyClamp(sp38[3], 0.0f, 32767.0f);

	dstvtx->unk00 = sp68[0];
	dstvtx->unk04 = sp68[1];
	dstvtx->unk08 = sp68[2];
	dstvtx->unk0c = sp68[3];
	dstvtx->s = s;
	dstvtx->t = t;
	dstvtx->x = sp38[0];
	dstvtx->y = sp38[1] - envGetCurrent()->clouds_height * 4.0f;
	dstvtx->unk30 = sp38[2];
	dstvtx->unk34 = f22;

	dstvtx->r = srcvtx->r;
	dstvtx->g = srcvtx->g;
	dstvtx->b = srcvtx->b;
	dstvtx->a = srcvtx->a;
}

bool skyVerticesAreSame(struct skyvtx2d *vtx0, struct skyvtx2d *vtx1)
{
	float xdiff = vtx0->x - vtx1->x;
	float ydiff = vtx0->y - vtx1->y;

	return sqrtf(xdiff * xdiff + ydiff * ydiff) < 1.0f ? true : false;
}

void skyCreateSunArtifact(struct artifact *artifact, int x, int y)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();

	if (x >= viewleft && x < viewleft + viewwidth && y >= viewtop && y < viewtop + viewheight) {
		const int i = (artifact - schedGetWriteArtifacts()) >> 3;
		struct coord zero = { 0.f };
		struct environment *env = envGetCurrent();
		struct coord sunpos;
		sunpos.x = env->suns[i].pos[0];
		sunpos.y = env->suns[i].pos[1];
		sunpos.z = env->suns[i].pos[2];
		artifact->losCheckResult = artifactTestLos(&sunpos, &zero, x, y) * 0xfffc;
		artifact->zbufferPixelPtr = &g_ZbufPtr1[(int)camGetScreenWidth() * y + x];
		artifact->screenPos.screenX = x;
		artifact->screenPos.screenY = y;
		artifact->type = ARTIFACTTYPE_CIRCLE;
	}
}

float skyGetArtifactGroupIntensityFrac(struct artifact *artifacts)
{
	float sum = 0;

	for (int i = 0; i < 8; i++) {
		if (artifacts[i].type == ARTIFACTTYPE_CIRCLE && artifacts[i].losCheckResult == 0xfffc) {
			sum += 0.125f;
		}
	}

	return sum;
}

Gfx *skyRenderSuns(Gfx *gdl, bool xray)
{
	struct artifact *artifacts;
	uint8_t colour[3];
	struct environment *env;
	bool onscreen;
	float radius;

	Mtx *worldToScreenMtx = camGetPlayerWorldToScreenMtx();
	Mtx* skyTransformMtx = camGetSkyMtx();
	env = envGetCurrent();

	if (env->numsuns <= 0 || !g_ZbufPtr1 || g_Vars.mplayerisrunning) {
		return gdl;
	}

	int16_t viewleft = viGetViewLeft();
	int16_t viewtop = viGetViewTop();
	int16_t viewwidth = viGetViewWidth();
	int16_t viewheight = viGetViewHeight();

	float viewleftf = viewleft;
	float viewtopf = viewtop;
	float viewwidthf = viewwidth;
	float viewheightf = viewheight;

	struct sun *sun = env->suns;

	for (int i = 0; i < env->numsuns; i++) {
		g_SunPositions[i].f[0] = sun->pos[0];
		g_SunPositions[i].f[1] = sun->pos[1];
		g_SunPositions[i].f[2] = sun->pos[2];

		colour[0] = sun->red;
		colour[1] = sun->green;
		colour[2] = sun->blue;

		if (!xray) {
			mtx4TransformVecInPlace(worldToScreenMtx, &g_SunPositions[i]);
			mtx4TransformVecInPlace(skyTransformMtx, &g_SunPositions[i]);

			if (g_SunPositions[i].f[2] > 1.0f) {
				g_SunScreenXPositions[i] = (g_SunPositions[i].f[0] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewwidthf + viewleftf;
				g_SunScreenYPositions[i] = (-g_SunPositions[i].f[1] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewheightf + viewtopf;
				radius = 60.0f / viGetFovY() * sun->texture_size;
				onscreen = false;

				if (g_SunScreenXPositions[i] >= viewleftf - radius
						&& g_SunScreenXPositions[i] < viewleftf + viewwidth + radius
						&& g_SunScreenYPositions[i] >= viewtopf - radius
						&& g_SunScreenYPositions[i] < viewtopf + viewheightf + radius) {
					// Sun is at least partially on screen
					if (g_SunScreenXPositions[i] >= viewleftf
							&& g_SunScreenXPositions[i] < viewleftf + viewwidthf
							&& g_SunScreenYPositions[i] >= viewtopf
							&& g_SunScreenYPositions[i] < viewtopf + viewheightf) {
						// Sun's centre point is on-screen
						float distfromedge;
						float mindistfromedge;
						artifacts = schedGetWriteArtifacts();
						onscreen = true;
						mindistfromedge = 1000;

						if ((int)g_SunScreenXPositions[i] < viewleft + 15) {
							distfromedge = g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((int)g_SunScreenYPositions[i] < viewtop + 15) {
							distfromedge = g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((int)g_SunScreenXPositions[i] > viewleft + viewwidth - 16) {
							distfromedge = viewleft + viewwidth - 1 - g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((int)g_SunScreenYPositions[i] > viewtop + viewheight - 16) {
							distfromedge = viewtop + viewheight - 1 - g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						mindistfromedge -= 1.0f;

						if (mindistfromedge < 0.0f) {
							mindistfromedge = 0.0f;
						}

						g_SunAlphaFracs[i] = mindistfromedge * (1.0f / 15.0f);

						if (g_SunAlphaFracs[i] > 1.0f) {
							g_SunAlphaFracs[i] = 1.0f;
						}

						const bool prevperim = g_Vars.currentplayer->bondperimenabled;
						playerSetPerimEnabled(g_Vars.currentplayer->prop, false);

						skyCreateSunArtifact(&artifacts[i * 8 + 0], (int)g_SunScreenXPositions[i] - 7, (int)g_SunScreenYPositions[i] + 1);
						skyCreateSunArtifact(&artifacts[i * 8 + 1], (int)g_SunScreenXPositions[i] - 5, (int)g_SunScreenYPositions[i] - 3);
						skyCreateSunArtifact(&artifacts[i * 8 + 2], (int)g_SunScreenXPositions[i] - 3, (int)g_SunScreenYPositions[i] + 5);
						skyCreateSunArtifact(&artifacts[i * 8 + 3], (int)g_SunScreenXPositions[i] - 1, (int)g_SunScreenYPositions[i] - 7);
						skyCreateSunArtifact(&artifacts[i * 8 + 4], (int)g_SunScreenXPositions[i] + 1, (int)g_SunScreenYPositions[i] + 7);
						skyCreateSunArtifact(&artifacts[i * 8 + 5], (int)g_SunScreenXPositions[i] + 3, (int)g_SunScreenYPositions[i] - 5);
						skyCreateSunArtifact(&artifacts[i * 8 + 6], (int)g_SunScreenXPositions[i] + 5, (int)g_SunScreenYPositions[i] + 3);
						skyCreateSunArtifact(&artifacts[i * 8 + 7], (int)g_SunScreenXPositions[i] + 7, (int)g_SunScreenYPositions[i] - 1);

						playerSetPerimEnabled(g_Vars.currentplayer->prop, prevperim);
					}

					g_SunFlareTimers240[i] += g_Vars.lvupdate240;

					texSelect(&gdl, &g_TexLightGlareConfigs[5], 4, 0, 2, 1, NULL);

					gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
					gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
					gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
					gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
					gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
					gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
					gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
					gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
					gDPSetCombineLERP(gdl++,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0);
					RGBA envColor = {colour[0], colour[1], colour[2], (int)(g_SunAlphaFracs[i] * 255.0f)};
					gfx_Set_Env_Color(gdl++, envColor);

					
					float texCenter[2] = { g_SunScreenXPositions[i], g_SunScreenYPositions[i] };
					float texRadius[2] = {
						(radius * 0.5f) * (SCREEN_ASPECT / videoGetAspect()),
						radius * 0.5f
					};

					utilsRenderScreenTexture(&gdl, texCenter, texRadius, g_TexLightGlareConfigs[5].width, g_TexLightGlareConfigs[5].height, 0, 1, 1, true);

					gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
					gfx_Set_Texture_LOD(gdl++, G_TL_LOD);
				}

				float artifactIntensity = skyGetArtifactGroupIntensityFrac(&schedGetFrontArtifacts()[i * 8]);

				if (onscreen && artifactIntensity > 0.0f) {
					g_SunFlareTimers240[i] += g_Vars.lvupdate240;
				} else {
					g_SunFlareTimers240[i] = 0;
				}
			}
		}

		sun++;
	}

	return gdl;
}

/**
 * Render a lens flare.
 *
 * Used for the sun and the Deep Sea teleports.
 */
Gfx *skyRenderFlare(Gfx *gdl, float x, float y, float intensityfrac, float size, int flaretimer240, float alphafrac)
{
	int i;
	float f2;
	float f12;
	float sp17c[2];
	float sp174[2];
	int sp15c[] = { 16, 32, 12, 32, 24, 64 }; // diameters?
	int sp144[] = { 60, 80, 225, 275, 470, 570 }; // distances from the source?

	uint32_t colours[] = {
		0xff99ffff, // pinkish/purple
		0x9999ffff, // blue
		0x99ffffff, // very light blue
		0x99ff99ff, // green
		0xffff99ff, // yellow
		0xff9999ff, // red
	};

	float xdist;
	float ydist;
	float fovy;

	xdist = (x - viGetViewWidth() / 2.0f) * 0.01f;
	ydist = (y - viGetViewHeight() / 2.0f) * 0.01f;

	// Render the source artifact (eg. the artifact that is on top of the sun)
	texSelect(&gdl, &g_TexLightGlareConfigs[6], 4, 0, 2, 1, NULL);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);

	fovy = viGetFovY();

	RGBA envColor = {255, 255, 255, (int)(alphafrac * intensityfrac * 255.0f)};
	gfx_Set_Env_Color(gdl++, envColor);
	f2 = ((int) ((60.0f / fovy) * (size * (0.5f + (0.5f * intensityfrac)))));

	sp17c[0] = x;
	sp17c[1] = y;
	sp174[1] = f2 * 0.5f;
	sp174[0] = f2 * 0.5f;

	sp174[0] *=  SCREEN_ASPECT / videoGetAspect();

	utilsRenderScreenTexture(&gdl, sp17c, sp174, g_TexLightGlareConfigs[6].width, g_TexLightGlareConfigs[6].height, 0, 1, 1, true);

	// Render the other artifacts
	texSelect(&gdl, &g_TexLightGlareConfigs[1], 4, 0, 2, 1, NULL);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);

	for (i = 0; i < 6; i++) {
		float f12;
		float f14;
		float tmp;

		if (flaretimer240 < TICKS(90)) {
			if (flaretimer240 < TICKS(30)) {
				f2 = flaretimer240 * (1.0f / TICKS(30.0f));
			} else {
				f2 = 1.0f;
			}
		} else {
			f2 = (TICKS(180.0f) - (flaretimer240 - TICKS(90))) * (1.0f / TICKS(180.0f)) * 0.5f;

			if (f2 < 0.0f) {
				f2 =  0.0f;
			}

			f2 += 0.5f;
		}

		f12 = x - sp144[i] * xdist;
		f14 = y - sp144[i] * ydist;

		tmp = sp15c[i];

		RGBA envColor = {(colours[i] >> 24) & 0xff, (colours[i] >> 16) & 0xff, (colours[i] >> 8) & 0xff, (int)((colours[i] & 0xff) * (alphafrac * f2))};
		gfx_Set_Env_Color(gdl++, envColor);

		sp17c[0] = f12;
		sp17c[1] = f14;

		sp174[1] = tmp * 0.5f;
		sp174[0] = tmp * 0.5f;

		sp174[0] *=  SCREEN_ASPECT / videoGetAspect();

		utilsRenderScreenTexture(&gdl, sp17c, sp174, g_TexLightGlareConfigs[1].width, g_TexLightGlareConfigs[1].height, 0, 0, 0, false);
	}

	// Check if the source is close to the center of the screen and create the bloom effect if so
	xdist = viGetViewWidth() / 2.0f - x;
	ydist = viGetViewHeight() / 2.0f - y;

	f12 = (40.0f - sqrtf(xdist * xdist + ydist * ydist)) * 0.0125f;

	if (f12 < 0.0f) {
		f12 = 0.0f;
	}

	f12 += 0.1f;

	if (flaretimer240 <= g_Vars.lvupdate240) {
		f12 = 0.0f;
	}

	if (f12 > 0.0f) {
		skySetOverexposure(alphafrac * f12 * 255.0f, alphafrac * f12 * 255.0f, alphafrac * f12 * 255.0f);
	}

	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
	gfx_Set_Texture_LOD(gdl++, G_TL_LOD);

	return gdl;
}

struct coord g_TeleportToPos = {0, 0, 0};
struct coord g_TeleportToUp = {0, 0, 1};
struct coord g_TeleportToLook = {0, 1, 0};

Gfx *skyRenderTeleportFlare(Gfx *gdl, float x, float y, float z, float size, float intensityfrac)
{
	struct coord sp64;

	sp64.x = x;
	sp64.y = y;
	sp64.z = z;

	mtx4TransformVecInPlace(camGetPlayerWorldToScreenMtx(), &sp64);
	mtx4TransformVecInPlace(camGetSkyMtx(), &sp64);

	if (sp64.z > 1.0f) {
		float xpos;
		float ypos;
		int16_t viewlefti = viGetViewLeft();
		int16_t viewtopi = viGetViewTop();
		int16_t viewwidthi = viGetViewWidth();
		int16_t viewheighti = viGetViewHeight();
		float viewleft = viewlefti;
		float viewwidth = viewwidthi;
		float viewtop = viewtopi;
		float viewheight = viewheighti;

		xpos = viewleft + (sp64.f[0] / sp64.f[2] + 1.0f) * 0.5f * viewwidth;
		ypos = viewtop + (-sp64.f[1] / sp64.f[2] + 1.0f) * 0.5f * viewheight;

		if (xpos >= viewleft && xpos < viewleft + viewwidth
				&& ypos >= viewtop && ypos < viewtop + viewheight) {
			gdl = skyRenderFlare(gdl, xpos, ypos, intensityfrac, size, TICKS(90), 1.0f);
		}
	}

	return gdl;
}

/**
 * Render lens flares during teleport.
 */
Gfx *skyRenderTeleportFlares(Gfx *gdl)
{
	float sp154 = g_20SecIntervalFrac * M_TAU;
	float sizefrac = 0.0f;
	float f20_2;
	float f22;
	float f22_3;
	struct pad pad;
	struct coord spe0;
	float spd0[4];
	Mtx mtx;
	float f24;
	float intensityfrac;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER) {
		sizefrac = g_Vars.currentplayer->teleporttime / 24.0f * 0.33f;
	} else if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		sizefrac = g_Vars.currentplayer->teleporttime / 48.0f * 0.66f + 0.33f;
	}

	intensityfrac = sizefrac * 6.0f;
	f22 = sizefrac * 1.3f;

	if (f22 > 1.0f) {
		f22 = 1.0f;
	}

	if (intensityfrac > 1.0f) {
		intensityfrac = 1.0f;
	}

	sizefrac *= 1.7f;

	if (sizefrac > 1.0f) {
		sizefrac = 1.0f;
	}

	padUnpack(g_Vars.currentplayer->teleportpad, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP, &pad);

	g_TeleportToPos.x = pad.pos.x;
	g_TeleportToPos.y = pad.pos.y;
	g_TeleportToPos.z = pad.pos.z;
	g_TeleportToLook.x = pad.look.x;
	g_TeleportToLook.y = pad.look.y;
	g_TeleportToLook.z = pad.look.z;
	g_TeleportToUp.x = pad.up.x;
	g_TeleportToUp.y = pad.up.y;
	g_TeleportToUp.z = pad.up.z;

	f22 = -cosf(f22 * M_PI) * 0.5f + .5f;
	f24 = 100 * f22;

	for (int i = 0; i < 5; i++) {
		spe0.x = g_TeleportToLook.f[0] * f24;
		spe0.y = g_TeleportToLook.f[1] * f24;
		spe0.z = g_TeleportToLook.f[2] * f24;

		f22_3 = sp154 + i * 1.2564370632172f;
		f20_2 = sinf(f22_3);

		spd0[0] = cosf(f22_3);
		spd0[1] = g_TeleportToUp.f[0] * f20_2;
		spd0[2] = g_TeleportToUp.f[1] * f20_2;
		spd0[3] = g_TeleportToUp.f[2] * f20_2;

		quaternionToMtx(spd0, &mtx);
		mtx4RotateVecInPlace(&mtx, &spe0);

		spe0.x += g_TeleportToPos.x;
		spe0.y += g_TeleportToPos.y;
		spe0.z += g_TeleportToPos.z;

		gdl = skyRenderTeleportFlare(gdl, spe0.x, spe0.y, spe0.z, sizefrac * 200, intensityfrac);
	}

	return gdl;
}

/**
 * Render teleport artifacts, and all suns and their artifacts.
 */
Gfx *skyRenderArtifacts(Gfx *gdl)
{
	struct environment *env = envGetCurrent();
	struct sun *sun;
	int i;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER
			|| g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		gdl = skyRenderTeleportFlares(gdl);
	}

	if (env->numsuns <= 0 || !g_ZbufPtr1 || g_Vars.mplayerisrunning) {
		return gdl;
	}

	sun = env->suns;

	for (i = 0; i < env->numsuns; i++) {
		if (sun->lens_flare && g_SunPositions[i].z > 1) {
			struct artifact *artifacts = schedGetFrontArtifacts() + i * 8;
			float intensityfrac = skyGetArtifactGroupIntensityFrac(artifacts);

			if (intensityfrac > 0.0f) {
				gdl = skyRenderFlare(gdl, g_SunScreenXPositions[i], g_SunScreenYPositions[i], intensityfrac, sun->orb_size, g_SunFlareTimers240[i], g_SunAlphaFracs[i]);
			}
		}

		sun++;
	}

	return gdl;
}

void skySetOverexposure(int r, int g, int b)
{
	g_Vars.currentplayer->overexposurered = sqrtf(g_Vars.currentplayer->overexposurered * g_Vars.currentplayer->overexposurered + r * r);
	g_Vars.currentplayer->overexposuregreen = sqrtf(g_Vars.currentplayer->overexposuregreen * g_Vars.currentplayer->overexposuregreen + g * g);
	g_Vars.currentplayer->overexposureblue = sqrtf(g_Vars.currentplayer->overexposureblue * g_Vars.currentplayer->overexposureblue + b * b);

	if (g_Vars.currentplayer->overexposurered > 0xcc) {
		g_Vars.currentplayer->overexposurered = 0xcc;
	}

	if (g_Vars.currentplayer->overexposuregreen > 0xcc) {
		g_Vars.currentplayer->overexposuregreen = 0xcc;
	}

	if (g_Vars.currentplayer->overexposureblue > 0xcc) {
		g_Vars.currentplayer->overexposureblue = 0xcc;
	}
}

int skyCalculateOverexposureComponent(int old, int new)
{
	if (new >= old) {
		if (new - old > 8) {
			return old + 8;
		} else {
			return new;
		}
	} else {
		if (old - new > 8) {
			return old - 8;
		} else {
			return new;
		}
	}
}

/**
 * Overexposure is used when the player looks at the sun, and when night vision
 * is overloaded. An almost-transparent rectangle is drawn across the viewport.
 */
Gfx *skyRenderOverexposure(Gfx *gdl)
{
	int value;

	g_Vars.currentplayer->overexposurered = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposurered, g_Vars.currentplayer->overexposurered);
	g_Vars.currentplayer->overexposuregreen = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposuregreen, g_Vars.currentplayer->overexposuregreen);
	g_Vars.currentplayer->overexposureblue = skyCalculateOverexposureComponent(g_Vars.currentplayer->prevoverexposureblue, g_Vars.currentplayer->overexposureblue);

	value = (g_Vars.currentplayer->overexposurered > g_Vars.currentplayer->overexposuregreen && g_Vars.currentplayer->overexposurered > g_Vars.currentplayer->overexposureblue)
		? g_Vars.currentplayer->overexposurered
		: g_Vars.currentplayer->overexposuregreen > g_Vars.currentplayer->overexposureblue
		? g_Vars.currentplayer->overexposuregreen
		: g_Vars.currentplayer->overexposureblue;

	if (!g_InCutscene && EYESPYINACTIVE() && value > 0) {
		float r = g_Vars.currentplayer->overexposurered * (255.0f / value);
		float g = g_Vars.currentplayer->overexposuregreen * (255.0f / value);
		float b = g_Vars.currentplayer->overexposureblue * (255.0f / value);

		float a = (g_Vars.currentplayer->overexposurered
			+ g_Vars.currentplayer->overexposuregreen
			+ g_Vars.currentplayer->overexposureblue) * (1.0f / 3.0f);

		gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
		gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
		gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

		if (USINGDEVICE(DEVICE_NIGHTVISION)) {
			r *= 0.5f;
			g *= 0.75f;
			b *= 0.5f;
		} else if (USINGDEVICE(DEVICE_IRSCANNER)) {
			r *= 0.75f;
			g *= 0.5f;
			b *= 0.5f;
		}

		RGBA color = {r, g, b, a};
		gfx_Set_Prim_Color(gdl++, color);

		gfx_Fill_Rectangle(gdl++,
				viGetViewLeft(),
				viGetViewTop(),
				viGetViewLeft() + viGetViewWidth(),
				viGetViewTop() + viGetViewHeight());
	}

	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

	g_Vars.currentplayer->prevoverexposurered = g_Vars.currentplayer->overexposurered;
	g_Vars.currentplayer->prevoverexposuregreen = g_Vars.currentplayer->overexposuregreen;
	g_Vars.currentplayer->prevoverexposureblue = g_Vars.currentplayer->overexposureblue;
	g_Vars.currentplayer->overexposurered = 0;
	g_Vars.currentplayer->overexposuregreen = 0;
	g_Vars.currentplayer->overexposureblue = 0;

	return gdl;
}
