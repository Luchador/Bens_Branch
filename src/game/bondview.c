#include <ultra64.h>
#include <math.h>
#include <stdio.h>
#include "constants.h"
#include "game/dlights.h"
#include "game/debug.h"
#include "game/menuutils.h"
#include "game/savebuffer.h"
#include "game/sky.h"
#include "game/bondview.h"
#include "game/textutils.h"
#include "game/gfxmemory.h"
#include "game/lang.h"
#include "game/utils.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/rng.h"
#include "string.h"
#include "data.h"
#include "types.h"
#include "gfx.h"
#include "game/player.h"
#include "video.h"

uint8_t g_IrScanlines[2][480];
int g_NumActiveEffects = 0;
uint8_t g_BlurChange = 0;
bool g_DoNotRedrawBlur = false;
int g_IrBinocularRadius = 90;
int var8007f850 = 3;

Gfx *bviewDrawIrRect(Gfx *gdl, int x1, int y1, int x2, int y2)
{
	gfx_Fill_Rectangle(gdl++, x1, y1, x2, y2);

	return gdl;
}

Gfx *bviewCopyPixels(Gfx *gdl, uint16_t *fb, int top, uint32_t tile, int arg4, float arg5, int left, int width)
{
	// TODO: add an extended GBI opcode for this
	return gdl;
}

Gfx *bviewDrawFisheyeRect(Gfx *gdl, int arg1, float arg2, int arg3, int arg4)
{
	if (arg2 < 1) {
		float tmp = arg4 * 0.5f;
		float fVar4 = arg3 + tmp;
		float fVar7 = (int)(arg2 * tmp);

		gfx_Fill_Rectangle(gdl++, arg3, arg1, fVar4 - fVar7, arg1 + 1);
		gfx_Fill_Rectangle(gdl++, fVar4 + fVar7, arg1, arg3 + arg4, arg1 + 1);
	}

	return gdl;
}

Gfx *bviewPrepareStaticRgba16(Gfx *gdl, uint32_t colour, uint32_t alpha)
{
	static uint32_t envcol = 0xffffffff;
	static uint32_t primcol = 0x7f7f7fff;

	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0000, 5, 0,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Texture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 160, 0, G_TX_RENDERTILE, 0,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Tile_Size(gdl++, G_TX_RENDERTILE, 0, 0, 2048, 32);
	gfx_Set_Texture_Filter(gdl++, G_TF_POINT);
	gfx_Set_Env_Color(gdl++, utilsUnpackColorRGBA((uintptr_t)((colour & 0xffffff00) | (alpha & 0xff))));
	gfx_Set_Combine_LERP(gdl++,
			G_CCMUX_TEXEL0, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT,
			G_CCMUX_TEXEL0, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);

	return gdl;
}

Gfx *bviewPrepareStaticI8(Gfx *gdl, uint32_t colour, uint32_t alpha)
{
	static uint32_t envcol = 0xffffffff;
	static uint32_t primcol = 0x7f7f7fff;

	gfx_Set_Tile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_8b, 0, 0x0000, 5, 0,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Texture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Tile(gdl++, G_IM_FMT_I, G_IM_SIZ_8b, 160, 0, G_TX_RENDERTILE, 0,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
			G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gfx_Set_Tile_Size(gdl++, G_TX_RENDERTILE, 0, 0, 2048, 32);
	gfx_Set_Texture_Filter(gdl++, G_TF_POINT);
	gfx_Set_Env_Color(gdl++, utilsUnpackColorRGBA((uintptr_t)((colour & 0xffffff00) | (alpha & 0xff))));
	gfx_Set_Combine_LERP(gdl++,
			G_CCMUX_TEXEL0, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT,
			G_CCMUX_TEXEL0, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT);
	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);

	return gdl;
}

Gfx *bviewDrawMotionBlur(Gfx *gdl, uint32_t colour, uint32_t alpha)
{
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	float somefloat;
	int newalpha;
	int i;

	if (g_DoNotRedrawBlur) {
		return gdl;
	}

	g_DoNotRedrawBlur = true;

	newalpha = alpha;
	newalpha += g_BlurChange;

	if (newalpha > 100) {
		newalpha = 100;
	}

	g_BlurChange = 0;

	if (!videoFramebuffersSupported()) {
		return gdl;
	}

	// capture fb at the end of this frame
	g_BlurFbCapTimer = 0;

	// don't render first blur frame as we haven't captured the fb yet
	if (g_BlurFbDirty) {
		return gdl;
	}

	gdl = bviewPrepareStaticRgba16(gdl, colour, newalpha);

	gfx_Set_Framebuffer_Texture_EXT(gdl++, 0, 0, 0, (uintptr_t)g_BlurFb);
	gdl += gfx_Image_Rectangle_EXT(gdl,
		viewleft, viewtop, viewleft, viewtop,
		(viewleft + viewwidth), (viewtop + viewheight), viewleft + viewwidth, viewtop + viewheight,
		0, videoGetNativeWidth(), videoGetNativeHeight());

	return gdl;
}

/**
 * Draw static for the Infiltration intro cutscene and Slayer rockets.
 */
Gfx *bviewDrawStatic(Gfx *gdl, uint32_t arg1, int arg2)
{
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	uint16_t *fb2 = (uint16_t *) (k_ptr_t)(rngRandom() & 0xfff00);

	gdl = bviewPrepareStaticI8(gdl, arg1, arg2);

	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_NOISE, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT,
		G_CCMUX_NOISE, 0, G_CCMUX_ENVIRONMENT, 0, 0, 0, 0, G_ACMUX_ENVIRONMENT);
	gdl += gfx_HUD_Rectangle_EXT(gdl, viewleft, viewtop, viewleft + viewwidth + 1, viewtop + viewheight + 1);

	return gdl;
}

/**
 * Draw the yellow interlace effect for Slayer rockets.
 */
Gfx *bviewDrawSlayerRocketInterlace(Gfx *gdl, uint32_t colour, uint32_t alpha)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int y;
	int viewleft = viGetViewLeft();
	float angle = 0.52359879016876f;
	int offset = (int)(g_20SecIntervalFrac * 600.0f) % 12;
	float increment;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	increment = (2.6179938316345f - angle) / viewheight;

	gdl = bviewPrepareStaticRgba16(gdl, colour, alpha);

	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	gSPSetExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	for (y = viewtop; y < viewtop + viewheight; y++) {
		int offsety = y - offset;

		if (offsety % 8 == 0 || y == viewtop) {
			if (offsety % 16 < 8) {
				struct RGBA color = {255, 255, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				struct RGBA color = {255, 255, 191, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}
		}

		gfx_Fill_Rectangle(gdl++, viewleft, y, viewleft + viewwidth, y + 1);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	return gdl;
}

/**
 * Draw the blue film interlace effect for the Infiltration intro cutscene.
 */
Gfx *bviewDrawFilmInterlace(Gfx *gdl, uint32_t colour, uint32_t alpha)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int y;
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	int offset = (int)(g_20SecIntervalFrac * 600.0f) % 12;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	gdl = bviewPrepareStaticRgba16(gdl, colour, alpha);

	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	gSPSetExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	for (y = viewtop; y < viewtop + viewheight; y++) {
		int offsety = y - offset;
		int tmpy = y;

		if (offsety % 6 == 0 || y == viewtop) {
			if (offsety % 12 < 6) {
				struct RGBA color = {127, 255, 255, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				struct RGBA color = {0, 175, 255, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}
		}

		if (rngRandom() % 20 == 1) {
			tmpy += rngRandom() % 200;
		}

		gfx_Fill_Rectangle(gdl++, viewleft, tmpy, viewleft + viewwidth, tmpy + 1);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	return gdl;
}

/**
 * Draw a zoom in/out motion blur effect.
 *
 * Used when entering/exiting combat boosts and when entering/exiting xray mode.
 */
Gfx *bviewDrawZoomBlur(Gfx *gdl, uint32_t colour, int alpha, float arg3, float arg4)
{
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	float somefloat;
	int i;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	if (!videoFramebuffersSupported()) {
		return gdl;
	}

	// capture fb at the end of this frame
	g_BlurFbCapTimer = 0;

	// don't render first blur frame as we haven't captured the fb yet
	if (g_BlurFbDirty) {
		return gdl;
	}

	somefloat = (viewheight - viewheight / arg4) * 0.5f;

	gdl = bviewPrepareStaticRgba16(gdl, colour, alpha);

	const float xcenter = viewleft + viewwidth * 0.5f;
	const float ycenter = viewtop + viewheight * 0.5f;
	const float halfw = viewwidth * 0.5f * arg3;
	const float halfh = viewheight * 0.5f * arg4;
	const int left = xcenter - halfw;
	const int top = ycenter - halfh;
	const int right = xcenter + halfw;
	const int bottom = ycenter + halfh;
	gfx_Set_Framebuffer_Texture_EXT(gdl++, 0, 0, 0, (uintptr_t)g_BlurFb);
	gdl += gfx_Image_Rectangle_EXT(gdl,
		left, top, viewleft, viewtop,
		right, bottom, viewleft + viewwidth, viewtop + viewheight,
		0, videoGetNativeWidth(), videoGetNativeHeight());

	return gdl;
}

float bview0f142d74(int arg0, float arg1, float arg2, float arg3)
{
	float result;
	float value = arg2;

	if (arg0 < 0 || arg0 >= 0x80) {
		return 0.01f;
	}

	value += arg0 * arg1;

	if (arg3 > value * value) {
		result = sqrtf(arg3 - value * value) * 0.00625f;
	} else {
		result = 0.01f;
	}

	return result;
}

static inline Gfx *bviewDrawFisheyeLine(Gfx *gdl, int viewleft, int viewwidth, int y, float scale)
{
	if (!videoFramebuffersSupported()) {
		return gdl;
	}

	const float orighalfw = viewwidth * 0.5f;
	const float xcenter = viewleft + orighalfw;
	const float halfw = orighalfw * scale;
	const int left = xcenter - halfw;
	const int right = xcenter + halfw;

	gdl += gfx_Image_Rectangle_EXT(gdl,
		left, y, viewleft, y,
		right, (y + 1), viewleft + viewwidth, y + 1,
		0, videoGetNativeWidth(), videoGetNativeHeight());

	return gdl;
}

/**
 * Draw the fisheye curved effect when using an eyespy.
 *
 * PAL Beta adds padding above and below to compensate for the higher vertical
 * screen resolution, by adjusting the viewtop and viewheight variables and
 * drawing black filler at the end of the function. However the size of these
 * bars is static regardless of the screen layout and size being used.
 *
 * PAL Final improves on beta's mistake by checking the screen size, but there's
 * no check for a vertical split being used, and as a result the fisheye radius
 * is smaller than it should be when using a horizontal split. @bug
 */
Gfx *bviewDrawFisheye(Gfx *gdl, uint32_t colour, uint32_t alpha, int shuttertime60, int8_t startuptimer60, uint8_t hit)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop;
	int viewheight;
	float f26;
	float halfheight;
	float sqhalfheight;
	int viewwidth;
	int viewleft;
	int s2;
	int i;
	int s3;
	uint8_t starting;
	int curradius;
	float startupfrac;
	float fullradius;
	int one = 1;
	int spec;
	uint8_t alpha2;
	float tmp;

	viewtop = viGetViewTop();
	viewheight = viGetViewHeight();
	halfheight = viewheight * 0.5f;
	sqhalfheight = halfheight * halfheight;
	f26 = -(halfheight + halfheight) / viewheight;
	viewwidth = viGetViewWidth();
	viewleft = viGetViewLeft();
	startupfrac = 1.0f;
	s2 = 0;

	starting = (startuptimer60 < TICKS(50));

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	s3 = 1;

	if (starting) {
		fullradius = viewheight * 0.5f;
		startupfrac = startuptimer60 / 50.0f;
		curradius = fullradius * startupfrac;
		spec = startupfrac * 255.0f;

		if (spec > 255) {
			spec = 255;
		}
	}

	gdl = bviewPrepareStaticRgba16(gdl, colour, alpha);

	// make a copy of the current back buffer contents that we will be using as a texture
	gfx_No_Param(gdl++, G_RDPFLUSH_EXT);
	gfx_Copy_Framebuffer_EXT(gdl++, g_PrevFrameFb, 0, 0, 0, G_ON);
	gfx_Set_Framebuffer_Texture_EXT(gdl++, 0, 0, 0, (uintptr_t)g_PrevFrameFb);

	if (starting) {
		for (i = viewtop; i < viewtop + viewheight; i++) {
			if (i % 2) {
				if (i > viewtop + fullradius - curradius && i < viewtop + fullradius + curradius) {
					gfx_Set_Env_Color(gdl++, utilsUnpackColorRGBA((colour & 0xffffff00) | (spec & 0xff)));

					tmp = bview0f142d74(s2, f26, halfheight, sqhalfheight) * startupfrac;
					gdl = bviewDrawFisheyeLine(gdl, viewleft, viewwidth, i, tmp);
				}
			}

			s2 += s3;

			if (s2 >= viewheight * 0.5f) {
				s2 = viewheight * 0.5f;
				s3 = -s3;
			}
		}
	} else {
		float f22 = 1.0f;

		for (i = viewtop; i < viewtop + viewheight; i++) {
			if (hit == EYESPYHIT_DAMAGE) {
				alpha2 = (rngRandom() % 120) + 120;
				colour = 0xff333300 | (alpha2 & 0xff);
				f22 = ((rngRandom() % 32) + (float) FBALLOC_HEIGHT) * (1.0f / 256.0f);

				gfx_Set_Env_Color(gdl++, utilsUnpackColorRGBA(colour));
			} else {
				RGBA color = {255, 255, 255, 255};
				gfx_Set_Env_Color(gdl++, color);
			}

			tmp = bview0f142d74(s2, f26, halfheight, sqhalfheight) * f22;
			gdl = bviewDrawFisheyeLine(gdl, viewleft, viewwidth, i, tmp);

			if (hit == EYESPYHIT_DAMAGE) {
				RGBA color = {221, 170, 170, 153};
				gfx_Set_Env_Color(gdl++, color);

				tmp = bview0f142d74(s2, f26, halfheight, sqhalfheight) * 1.03f;
				gdl = bviewDrawFisheyeLine(gdl, viewleft, viewwidth, i, tmp);
			}

			s2 += s3;

			if ((i % 2) == 0) {
				RGBA color = {0, 0, 0, 85};
				gfx_Set_Env_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, viewleft, i, viewleft + viewwidth, i + 1);
			}

			if (s2 >= viewheight * 0.5f) {
				s2 = viewheight * 0.5f;
				s3 = -s3;
			}
		}
	}

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	struct RGBA color = {0, 0, 0, 255};
	gfx_Set_Prim_Color(gdl++, color);

	s3 = 1;

	if (shuttertime60 != 0 || starting) {
		int s7;
		int spa8 = viewheight * 0.5f;
		float f20;

		if (!starting) {
			shuttertime60 -= TICKS(12);

			if (shuttertime60 < 0) {
				shuttertime60 = -shuttertime60;
			}

			s7 = spa8 * (shuttertime60 / TICKS(12.0f));
		} else {
			s7 = curradius;
		}

		for (i = viewtop; i < viewtop + spa8 - s7; i++) {
			gdl = bviewDrawFisheyeRect(gdl, i, 0.0f, viewleft, viewwidth);
			gdl = bviewDrawFisheyeRect(gdl, viewtop + viewtop + viewheight - i, 0.0f, viewleft, viewwidth);
		}

		gfx_Set_Prim_Color(gdl++, color);

		tmp = (float) one * halfheight;
		f20 = halfheight;

		for (i = viewtop + spa8 - s7; i <= viewtop + spa8; i++) {
			float f2;

			if (sqhalfheight > f20 * f20) {
				f2 = sqrtf(sqhalfheight - f20 * f20) * (1.0f / 160.0f);
			} else {
				f2 = 0.01f;
			}

			f20 += -tmp / s7;

			gdl = bviewDrawFisheyeRect(gdl, i, f2 * startupfrac, viewleft, viewwidth);

			if (i != viewtop + viewtop + viewheight - i) {
				gdl = bviewDrawFisheyeRect(gdl, viewtop + viewtop + viewheight - i, f2 * startupfrac, viewleft, viewwidth);
			}
		}
	} else {
		s2 = 0;

		for (i = viewtop; i < viewtop + viewheight; i++) {
			tmp = bview0f142d74(s2, f26, halfheight, sqhalfheight);
			gdl = bviewDrawFisheyeRect(gdl, i, tmp, viewleft, viewwidth);

			s2 += s3;

			if (s2 >= viewheight * 0.5f) {
				s2 = viewheight * 0.5f;
				s3 = -s3;
			}
		}
	}

	return gdl;
}

/**
 * Draw a black rectangle to the side of the circular fisheye lens.
 *
 * These are each 1px high, and go from the edge of the circle to the edge of
 * the screen. There is one drawn on every row on both sides.
 */
Gfx *bviewDrawEyespySideRect(Gfx *gdl, int *points, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
	Vtx *vertices = gfxAllocateVertices(4);
	Col *colours = gfxAllocateColours(2);

	vertices[0].x = points[0] * 10.0f;
	vertices[0].y = points[1] * 10.0f;
	vertices[0].z = -10;

	vertices[1].x = points[2] * 10.0f;
	vertices[1].y = points[3] * 10.0f;
	vertices[1].z = -10;

	vertices[2].x = points[4] * 10.0f;
	vertices[2].y = points[5] * 10.0f;
	vertices[2].z = -10;

	vertices[3].x = points[6] * 10.0f;
	vertices[3].y = points[7] * 10.0f;
	vertices[3].z = -10;

	colours[0].word = PD_BE32(r << 0x18 | g << 0x10 | b << 8 | 0xff);
	colours[1].word = PD_BE32(r << 0x18 | g << 0x10 | b << 8 | alpha);

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 4;
	vertices[3].colour = 4;

	gfx_Color(gdl++, colours, 2);
	gfx_Vertex(gdl++, vertices, 4, 0);

	gfx_Tri2(gdl++, 0, 1, 2, 0, 2, 3);

	return gdl;
}

/**
 * Renders the eyespy user interface, excluding the fisheye lens. The lens is
 * drawn first by another function, then this one is called to draw the outer
 * information. Care must be taken not to draw over the top of the fisheye lens.
 *
 * Note that the dimensions of the view can differ based on hi-res on/off, as
 * well as using coop mode in both the vertical and horizontal screen splits.
 * Some elements are omitted if a vertical split is being used, and to handle
 * hi-res a scale variable is used to multiply X values and widths where needed.
 *
 * @bug: Many of the X values and widths are not multiplied by the scale which
 * causes them to display incorrectly when using hi-res:
 * - Some of the horizontal lines don't touch the lens circle.
 * - The vertical lines are thinner and closer to the screen edges than intended.
 * - The speed and height bars are stretched.
 * - The device name and model are closer to the screen edge than intended.
 */
Gfx *bviewDrawEyespyMetrics(Gfx *gdl)
{
	char text[256];
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();
	int viewright = viewleft + viewwidth - 1;
	int viewbottom = viewtop + viewheight - 1;
	int x;
	int y;
	int textwidth;
	int textheight;
	int x2;
	int y2;
	struct chrdata *chr;
	int savedy;
	int movex;
	int movey;
	int movez;
	float movedist;
	float sqmovedist = 0.0f;
	uint32_t colourtextbright;
	uint32_t colourtextdull;
	uint32_t colourglow;
	int scale = viewwidth > SCREEN_WIDTH_LO ? 2 : 1;
	bool vsplit = false;
	uint32_t umask, dmask, lmask, rmask;

	if (g_Vars.currentplayer->eyespy == NULL
			|| g_Vars.currentplayer->eyespy->prop == NULL
			|| g_Vars.currentplayer->eyespy->prop->chr == NULL) {
		return gdl;
	}

	chr = g_Vars.currentplayer->eyespy->prop->chr;

	if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL && PLAYERCOUNT() >= 2) {
		vsplit = true;
	}

	if (optionsGetControlMode(g_Vars.currentplayerstats->mpindex) == CONTROLMODE_PC) {
		umask = U_CBUTTONS;
		dmask = D_CBUTTONS;
		lmask = L_CBUTTONS;
		rmask = R_CBUTTONS;
	} else {
		umask = U_JPAD | U_CBUTTONS;
		dmask = D_JPAD | D_CBUTTONS;
		lmask = L_JPAD | L_CBUTTONS;
		rmask = R_JPAD | R_CBUTTONS;
	}

	movex = chr->prop->pos.x - chr->prevpos.x;
	movey = chr->prop->pos.y - chr->prevpos.y;
	movez = chr->prop->pos.z - chr->prevpos.z;

	if (movex != 0.0f || movey != 0.0f || movez != 0.0f) {
		sqmovedist = movex * movex + movey * movey + movez * movez;
	}

	if (sqmovedist > 0.001f) {
		movedist = sqrtf(sqmovedist);
	} else {
		movedist = 0.0f;
	}

	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
		gdl = textSetPrimColour(gdl, 0x00ff0028);
	} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		gdl = textSetPrimColour(gdl, 0x2244ffa0);
	} else {
		gdl = textSetPrimColour(gdl, 0xff3300a0);
	}

	if (!vsplit)
	{
		// Render borders/lines in background
		gfx_Fill_Rectangle(gdl++, viewleft + 25, viewtop + 55, viewleft + 26, viewbottom - 24);
		gfx_Fill_Rectangle(gdl++, viewleft + 31, viewtop + 55, viewleft + 32, viewbottom - 42);
		gfx_Fill_Rectangle(gdl++, viewleft + 25, viewbottom - 25, viewleft + 25 + viewwidth / 5.0f + 1, viewbottom - 24);
		gfx_Fill_Rectangle(gdl++, viewleft + 31, viewbottom - 43, viewleft + 25 + viewwidth / 7.0f + 1, viewbottom - 42);
		gfx_Fill_Rectangle(gdl++, viewright - 25, viewtop + 25, viewright - 24, viewbottom - 54);
		gfx_Fill_Rectangle(gdl++, viewright - 31, viewtop + 43, viewright - 30, viewbottom - 54);
		gfx_Fill_Rectangle(gdl++, viewright - 25 - viewwidth / 5.0f, viewtop + 25, viewright - 24, viewtop + 26);
		gfx_Fill_Rectangle(gdl++, viewright - 25 - viewwidth / 7.0f, viewtop + 43, viewright - 30, viewtop + 44);
		gfx_Fill_Rectangle(gdl++, viewleft, viewtop + 55, viewleft + viewwidth / 5.0f + 1, viewtop + 56);
		gfx_Fill_Rectangle(gdl++, viewright - viewwidth / 5.0f, viewbottom - 55, viewright + 1, viewbottom - 54);
	}

	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		// Render crosshair
		int x = viewleft + (viewwidth >> 1);
		int y = viewtop + (viewheight >> 1);

		gfx_Set_Subpixel_Offset_EXT(gdl++, -2, -2);

		gfx_Fill_Rectangle(gdl++, x + 2, y + 0, x + 7, y + 1);
		gfx_Fill_Rectangle(gdl++, x + 2, y + 0, x + 5, y + 1);
		gfx_Fill_Rectangle(gdl++, x - 6, y + 0, x - 1, y + 1);
		gfx_Fill_Rectangle(gdl++, x - 4, y + 0, x - 1, y + 1);
		gfx_Fill_Rectangle(gdl++, x + 0, y + 2, x + 1, y + 7);
		gfx_Fill_Rectangle(gdl++, x + 0, y + 2, x + 1, y + 5);
		gfx_Fill_Rectangle(gdl++, x + 0, y - 6, x + 1, y - 1);
		gfx_Fill_Rectangle(gdl++, x + 0, y - 4, x + 1, y - 1);

		gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);
	}

	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
		colourtextbright = 0x00ff00a0;
		colourtextdull = 0x005000ff;
		colourglow = 0x000f00ff;
	} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		colourtextbright = 0x2244ffff;
		colourtextdull = 0x2244ffff;
		colourglow = 0x00000fff;
	} else {
		colourtextbright = 0xff3300ff;
		colourtextdull = 0xff3300ff;
		colourglow = 0x0f0000ff;
	}

	// "S/MPS"
	sprintf(text, "%s %s%5.2f", langRemoveNewline(langGet(L_MISC_073)), "", movedist * 0.6f);
	savedy = viewtop + 14;
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = viewleft + 25 * scale;
	y = savedy;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextbright, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	// "H/M"
	sprintf(text, "%s %s%4.2f", langRemoveNewline(langGet(L_MISC_074)), "", g_Vars.currentplayer->eyespy->height * 0.01f);
	savedy += 9;
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = viewleft + 25 * scale;
	y = savedy;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextbright, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	// "Y/D"
	sprintf(text, "%s %d", langRemoveNewline(langGet(L_MISC_075)), (int)g_Vars.currentplayer->eyespy->theta);
	savedy += 9;
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = viewleft + 25 * scale;
	y = savedy;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextbright, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	// "P/D"
	sprintf(text, "%s %d", langRemoveNewline(langGet(L_MISC_076)), (int)g_Vars.currentplayer->eyespy->verta);
	savedy += 9;
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = viewleft + 25 * scale;
	y = savedy;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextbright, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	// "CI 2023"
	sprintf(text, "%s", langRemoveNewline(langGet(L_MISC_077)));
	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

	x = (vsplit ? -3 : 0) + viewleft + 25 * scale + 5;
	y = (vsplit ? 18 : 0) + viewbottom - 41;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
		sprintf(text, "%s", langRemoveNewline(langGet(L_MISC_078))); // "YKK: 95935"
	} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		sprintf(text, "%s", langRemoveNewline(langGet(L_MISC_208))); // "JM: 201172"
	} else {
		sprintf(text, "%s", langRemoveNewline(langGet(L_MISC_217))); // "BNC: 15877"
	}

	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

	x = viewleft + 25 * scale + (vsplit ? -3 : 0) + 5;
	y = viewbottom + (vsplit ? 20 : 0) - 34;

	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
		// "CAMSPY"
		sprintf(text, "   %s", langGet(L_GUN_060));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
		x = viewright - scale * 53 - 25;
		y = (vsplit ? -13 : 0) + viewtop + 27;
		x2 = x + textwidth; \
		y2 = y + textheight; \
		gdl = text0f153858(gdl, &x, &y, &x2, &y2);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
				colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);
	} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		// "DRUGSPY"
		sprintf(text, "   %s", langGet(L_GUN_061));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		x = viewright - scale * 53 - 25;
		y = viewtop + 27;
		x2 = x + textwidth; \
		y2 = y + textheight; \
		gdl = text0f153858(gdl, &x, &y, &x2, &y2);
		gdl = textRenderProjected(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
				colourtextdull, viGetWidth(), viGetHeight(), 0, 0);
	} else {
		// "BOMBSPY"
		sprintf(text, "   %s", langGet(L_GUN_062));
		textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);

		x = viewright - scale * 59 - 25;
		y = viewtop + 27;
		x2 = x + textwidth; \
		y2 = y + textheight; \
		gdl = text0f153858(gdl, &x, &y, &x2, &y2);
		gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
				colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);
	}

	// Model number
	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
		sprintf(text, "%s", langGet(L_MISC_080)); // "MODEL 1.2"
	} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
		sprintf(text, "%s", langGet(L_MISC_207)); // "MODEL 1.4"
	} else {
		sprintf(text, "%s", langGet(L_MISC_216)); // "MODEL 1.3"
	}

	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = (vsplit ? 3 : 0) + viewright - scale * 46 - 25;
	y = (vsplit ? -11 : 0) + viewtop + 34;
	x = viewright - scale * 46 - 25;
	y = viewtop + 34;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	// Gyrostat/dartammo text
	if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY
			|| g_Vars.currentplayer->eyespy->mode == EYESPYMODE_BOMBSPY) {
		sprintf(text, "%s", langGet(L_MISC_081)); // "GYROSTAT"
	} else {
		sprintf(text, "%s", langGet(L_MISC_206)); // "DARTAMMO"
	}

	textMeasure(&textheight, &textwidth, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0);
	x = (vsplit ? -35 : 0) + (viewright - viewwidth / 5.0f - 3 - (scale * 5 - 5));
	y = (vsplit ? -2 : 0) + viewbottom - 12;
	x2 = x + textwidth; \
	y2 = y + textheight; \
	gdl = text0f153858(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs,
			colourtextdull, colourglow, viGetWidth(), viGetHeight(), 0, 0);

	gdl = textSetCCPrimColorTexAlpha(gdl);

	{
		int8_t contpadnum = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);
		uint32_t buttonsdown = joyGetButtons(contpadnum, 0xffffffff); \
		uint32_t buttonsthisframe = joyGetButtonsPressedThisFrame(contpadnum, 0xffffffff);
		int8_t cstickx = joyGetStickX(contpadnum); \
		int8_t csticky = joyGetStickY(contpadnum);
		int xpos;
		int tmpval;
		uint8_t brightness;
		uint8_t brightness2;
		int points[8];
		int r;
		int g;
		int b;

		gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
		gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
		gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
		gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
		gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
		gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
		gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
		gfx_Set_Render_Mode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
		gfx_Set_Combine_LERP(gdl++,
			G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
			G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
			G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
			G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);

		if (!vsplit)
		{
			xpos = (scale == 2) ? -76 : -85;

			// Up
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;
				if (buttonsdown & umask) {
					brightness += 20;
				}
				if (buttonsthisframe & umask) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & umask) {
					brightness += 63;
				}

				if (buttonsthisframe & umask) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & umask) {
					brightness += 20;
				}

				if (buttonsthisframe & umask) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
			xpos += 10;

			// Down
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;

				if (buttonsdown & dmask) {
					brightness += 20;
				}
				if (buttonsthisframe & dmask) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & dmask) {
					brightness += 63;
				}

				if (buttonsthisframe & dmask) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & dmask) {
					brightness += 20;
				}

				if (buttonsthisframe & dmask) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
			xpos += 10;

			// Left
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;

				if (buttonsdown & lmask) { \
					brightness += 20; \
				} \
				if (buttonsthisframe & lmask) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & lmask) {
					brightness += 63;
				}

				if (buttonsthisframe & lmask) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & lmask) {
					brightness += 20;
				}

				if (buttonsthisframe & lmask) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
			xpos += 10;

			// Right
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;

				if (buttonsdown & rmask) { \
					brightness += 20; \
				} \
				if (buttonsthisframe & rmask) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & rmask) {
					brightness += 63;
				}

				if (buttonsthisframe & rmask) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & rmask) {
					brightness += 20;
				}

				if (buttonsthisframe & rmask) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
			xpos += 10;

			// Shoulder buttons
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;

				if (buttonsdown & (R_TRIG)) { \
					brightness += 20; \
				} \
				if (buttonsthisframe & (R_TRIG)) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & (R_TRIG)) {
					brightness += 63;
				}

				if (buttonsthisframe & (R_TRIG)) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & (R_TRIG)) {
					brightness += 20;
				}

				if (buttonsthisframe & (R_TRIG)) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
			xpos += 10;

			// Z button
			if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
				brightness = 20;

				if (buttonsdown & Z_TRIG) { \
					brightness += 20; \
				} \
				if (buttonsthisframe & Z_TRIG) {
					brightness += 20;
				}
				struct RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
				brightness = 127;

				if (buttonsdown & Z_TRIG) {
					brightness += 63;
				}

				if (buttonsthisframe & Z_TRIG) {
					brightness += 63;
				}
				struct RGBA color = {16, 32, brightness, 255};
				gfx_Set_Prim_Color(gdl++, color);
			} else {
				brightness = 20;

				if (buttonsdown & Z_TRIG) {
					brightness += 20;
				}

				if (buttonsthisframe & Z_TRIG) {
					brightness += 20;
				}
				struct RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
			}

			gfx_Fill_Rectangle(gdl++, xpos * scale + viewright, viewtop + 10, (xpos + 8) * scale + viewright, viewtop + 18);
		}

		xpos = (scale == 2) ? -48 : -55;

		// Stick X
		tmpval = cstickx * 96.0f / 80.0f;
		brightness = tmpval < 0 ? -tmpval : tmpval;

		if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
			struct RGBA color = {0, brightness, 0, 255};
			gfx_Set_Prim_Color(gdl++, color);
		} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
			r = brightness / 96.0f * 16.0f;
			g = brightness / 96.0f * 32.0f;
			b = brightness * 2.5f;
			struct RGBA color = {r, g, b, 255};
			gfx_Set_Prim_Color(gdl++, color);
		} else {
			struct RGBA color = {brightness, brightness / 4, 0, 255};
			gfx_Set_Prim_Color(gdl++, color);
		}

		if (!vsplit)
		{
			tmpval = cstickx * 28.0f / 80.0f;

			if (cstickx > 0) {
				gfx_Fill_Rectangle(gdl++,
						xpos * scale + viewright,
						viewtop + 19,
						(tmpval + xpos) * scale + viewright,
						viewtop + 21);
			} else {
				gfx_Fill_Rectangle(gdl++,
						(tmpval + xpos) * scale + viewright,
						viewtop + 19,
						xpos * scale + viewright,
						viewtop + 21);
			}
		}

		// Stick Y
		tmpval = csticky * 96.0f / 80.0f;
		brightness = tmpval < 0 ? -tmpval : tmpval;

		if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
			RGBA color = {0, brightness, 0, 255};
			gfx_Set_Prim_Color(gdl++, color);
		} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
			r = brightness / 96.0f * 16.0f;
			g = brightness / 96.0f * 32.0f;
			b = brightness * 2.5f;
			RGBA color = {r, g, b, 255};
			gfx_Set_Prim_Color(gdl++, color);
		} else {
			RGBA color = {brightness, brightness / 4, 0, 255};
			gfx_Set_Prim_Color(gdl++, color);
		}

		if (!vsplit)
		{
			tmpval = csticky * 28.0f / 80.0f;

			if (csticky > 0) {
				gfx_Fill_Rectangle(gdl++,
						xpos * scale + viewright,
						viewtop + 22,
						(tmpval + xpos) * scale + viewright,
						viewtop + 24);
			} else {
				gfx_Fill_Rectangle(gdl++,
						(tmpval + xpos) * scale + viewright,
						viewtop + 22,
						xpos * scale + viewright,
						viewtop + 24);
			}
		}

		if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
			if (!vsplit)
			{
				// Camspy gyrobar 1
				x = viewright - viewwidth / 5.0f;
				y = viewbottom - 13;

				tmpval = g_Vars.currentplayer->eyespy->theta * 96.0f / 360.0f;
				textheight = g_Vars.currentplayer->eyespy->theta * 35.0f / 360.0f;
				brightness = tmpval < 0 ? -tmpval : tmpval;
				RGBA color = {0, brightness, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Camspy gyrobar 2
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->costheta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->costheta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Camspy gyrobar 3
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->sintheta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->sintheta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Camspy gyrobar 4
				x += scale * 2 + scale * 5;

				tmpval = g_Vars.currentplayer->eyespy->verta * 96.0f / 360.0f;
				textheight = g_Vars.currentplayer->eyespy->verta * 35.0f / 360.0f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Camspy gyrobar 5
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->cosverta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->cosverta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Camspy gyrobar 6
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->sinverta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->sinverta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				x += scale * 2 + scale * 5;
			}
		} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_BOMBSPY) {
			if (!vsplit)
			{
				// Bombspy gyrobar 1
				x = viewright - viewwidth / 5.0f;
				y = viewbottom - 13;

				tmpval = g_Vars.currentplayer->eyespy->theta * 96.0f / 360.0f;
				textheight = g_Vars.currentplayer->eyespy->theta * 35.0f / 360.0f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				RGBA color = {brightness, brightness / 4, 0, 255};
				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Bombspy gyrobar 2
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->costheta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->costheta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Bombspy gyrobar 3
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->sintheta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->sintheta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Bombspy gyrobar 4
				x += scale * 2 + scale * 5;

				tmpval = g_Vars.currentplayer->eyespy->verta * 96.0f / 360.0f;
				textheight = g_Vars.currentplayer->eyespy->verta * 35.0f / 360.0f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Bombspy gyrobar 5
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->cosverta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->cosverta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				// Bombspy gyrobar 6
				x += scale * 2 + scale * 5;

				tmpval = (g_Vars.currentplayer->eyespy->sinverta + 1.0f) * 96.0f * 0.5f;
				textheight = (g_Vars.currentplayer->eyespy->sinverta + 1.0f) * 35.0f * 0.5f;
				brightness = tmpval < 0 ? -tmpval : tmpval;

				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - textheight, x + scale * 5, y);

				x += scale * 2 + scale * 5;
			}
		} else {
			// Drugspy ammo
			int width;

			brightness2 = 255;
			width = scale * 30;

			y = viewbottom - 13;
			x = viewright - viewwidth / 5.0f + 5;

			if (vsplit) {
				x -= 12;
				y -= 3;
				width = 15;
			}

			for (int i = 0; i < MAX_EYESPYDARTS; i++) {
				if (i >= g_Vars.currentplayer->eyespydarts) {
					brightness2 = 0x88;
				}
				
				RGBA color = {16, 32, brightness2, 255};
				gfx_Set_Prim_Color(gdl++, color);
				gfx_Fill_Rectangle(gdl++, x, y - 4, x + width, y);

				y -= 5;
			}
		}

		gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
		gfx_Set_Texture_LOD(gdl++, G_TL_LOD);

		if (g_Vars.coopplayernum < 0 && g_Vars.antiplayernum < 0) {
			int barheight = (viewbottom - viewtop - 103) / 17.0f - 1;
			int centrey = viewheight / 2.0f;
			int sqcentrey = centrey * centrey;
			int ypos;
			uint8_t alpha;
			int8_t yoffset;
			int value;
			int i;

			value = 17.0f * movedist / 25.0f;

			if (value > 17) {
				value = 17;
			}

			value = 17 - value;

			if (viewheight == FBALLOC_HEIGHT) {
				yoffset = 10;
			} else if (viewheight == 180) { // screen size: wide
				barheight--;
				yoffset = -8;
			} else {
				yoffset = 0;
			}

			gdl = savebufferSetCustomProjection(gdl);

			gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
			gfx_Set_Geometry_Mode(gdl++, G_SHADE | G_SHADING_SMOOTH);
			gfx_Set_Combine_LERP(gdl++,
				G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 0
				G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE,              // Alpha 0
				G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 1
				G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE);             // Alpha 1
			gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
			gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
			gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

			// Speed bars (left side)
			y = viewtop + 58;

			if (viewheight == 180) { // screen size: wide
				y += 5;
			}

			ypos = centrey - y + 10;

			for (i = 0; i < 17; i++) {
				brightness = i < value ? 40 : 80;
				alpha = i < value ? 0x22 : 0x56;

				points[0] = x = viewleft + 34;
				points[1] = y;
				points[2] = x = viewleft + 34;
				points[3] = y + barheight;
				points[6] = x = viewwidth / 2.0f - sqrtf(sqcentrey - (ypos - yoffset) * (ypos - yoffset)) * scale - 5.0f;
				points[5] = y + barheight;

				ypos -= barheight;

				points[4] = x = viewwidth / 2.0f - sqrtf(sqcentrey - (ypos - yoffset) * (ypos - yoffset)) * scale - 5.0f;
				points[7] = y;

				ypos -= 2;

				if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
					gdl = bviewDrawEyespySideRect(gdl, points, 0, brightness, 0, alpha);
				} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
					gdl = bviewDrawEyespySideRect(gdl, points, 0x10, brightness, brightness * 3, alpha);
				} else {
					gdl = bviewDrawEyespySideRect(gdl, points, brightness, brightness >> 2, 0, alpha);
				}

				y += barheight;
				y += 2;
			}

			// Height bars (right side)
			value = g_Vars.currentplayer->eyespy->height * 17.0f * (1.0f / 160.0f);

			if (value > 17) {
				value = 17;
			}

			value = 17 - value;

			y = viewtop + 46;

			if (viewheight == 180) { // screen size: wide
				y += 5;
			}

			ypos = centrey - y + 10;

			for (i = 0; i < 17; i++) {
				brightness = i < value ? 40 : 80;
				alpha = i < value ? 0x22 : 0x56;

				points[0] = x = viewright - 34;
				points[1] = y;
				points[3] = y + barheight;
				points[2] = x = viewright - 34;
				points[6] = x = viewwidth / 2.0f + sqrtf(sqcentrey - (ypos - yoffset) * (ypos - yoffset)) * scale + 5.0f;
				points[5] = y + barheight;

				ypos -= barheight;

				points[4] = x = viewwidth / 2.0f + sqrtf(sqcentrey - (ypos - yoffset) * (ypos - yoffset)) * scale + 5.0f;
				points[7] = y;

				ypos -= 2;

				if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
					gdl = bviewDrawEyespySideRect(gdl, points, 0, brightness, 0, alpha);
				} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
					gdl = bviewDrawEyespySideRect(gdl, points, 0x10, brightness, brightness * 3U, alpha);
				} else {
					gdl = bviewDrawEyespySideRect(gdl, points, brightness, brightness >> 2, 0, alpha);
				}

				y += barheight;
				y += 2;
			}
		}

		gdl = savebufferSetup2DRender(gdl);
	}

	return gdl;
}

uint8_t g_NightVisionFrameCounter = 0;

Gfx *bviewDrawNvLens(Gfx *gdl)
{
	uint16_t *fb = viGetBackBuffer();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewtop = viGetViewTop();
	int viewleft = viGetViewLeft();
	int viewbottom = viewtop + viewheight;
	int brightness;
	int y;
	uint32_t mpindex = g_Vars.currentplayerstats->mpindex % MAX_PLAYERS;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	g_NVBGBrightness = 0xbc;
	g_NVChrHighlight = 0xbe; // Character brightness when using NV
	g_NVChrBrightness = 0xde;
	g_NVPropBrightness = 0x50;
	g_NVPropHighlight = 0xc0;

	brightness = roomGetFinalBrightness(g_Vars.currentplayer->prop->rooms[0]);

	if (brightness > 128) {
		skySetOverexposure(brightness, brightness, brightness);
	}

	if (g_Menus[mpindex].curdialog == NULL) {
		gdl = bviewDrawMotionBlur(gdl, 0x00ff0000, 0x60);
	}

	gdl = bviewPrepareStaticRgba16(gdl, 0xffffffff, 0xff);

	g_NightVisionFrameCounter++;

	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	gSPSetExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	for (y = viewtop; y < viewbottom; y++) {
		uint8_t green;

		if (((g_NightVisionFrameCounter & 1) != (y & 1)) != 0) {
			uint8_t tmp = rngRandom() % 12;
			green = 0xff - tmp;
		} else {
			green = 0x94;
		}

		struct RGBA color = {0, green, 0, 255};
		gfx_Set_Prim_Color(gdl++, color);

		gfx_Fill_Rectangle(gdl++, viewleft, y, viewleft + viewwidth, y + 1);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	return gdl;
}

/**
 * Night vision doesn't have binoculars.
 */
Gfx *bviewDrawNvBinoculars(Gfx *gdl)
{
	return gdl;
}

Gfx *bviewDrawIrLens(Gfx *gdl)
{
	int i;
	int fadeincrement;
	uint16_t *fb = viGetBackBuffer();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewtop = viGetViewTop();
	int viewleft = viGetViewLeft();
	int viewright;
	int viewbottom;
	int viewcentrex;
	int sqinnerradius;
	int scanincrement;
	int scantop;
	int scanbottom;
	int scanrate = 4;
	int faderate = 2;
	uint32_t red;
	int outerradius;
	int innerradius;
	int viewcentrey;
	float viewheightf;
	int a0;
	uint32_t mpindex = g_Vars.currentplayerstats->mpindex % MAX_PLAYERS;

	viewright = viewleft + viewwidth;
	viewcentrex = (viewleft + viewright) / 2;

	outerradius = g_IrBinocularRadius;
	innerradius = g_IrBinocularRadius / var8007f850;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	viewbottom = viewtop + viewheight;
	viewcentrey = (viewtop + viewbottom) / 2;
	scantop = viewcentrey - outerradius;
	scanbottom = viewcentrey + outerradius;
	i = viewheight;

	if (scantop > viewbottom) {
		scantop = viewbottom;
	}

	if (scanbottom > viewbottom) {
		scanbottom = viewbottom;
	}

	if (scantop < viewtop) {
		scantop = viewtop;
	}

	if (scanbottom < viewtop) {
		scanbottom = viewtop;
	}

	scanincrement = (float) scanrate * i / 240.0f;
	fadeincrement = (float) faderate * i / 240.0f;

	// This code runs on the first frame of IR use (90 != 0),
	// and in debug versions developers could change the radius at runtime.
	if (g_Vars.currentplayer->fslastradius != outerradius) {
		for (i = 0; i < 480; i++) {
			g_IrScanlines[g_Vars.currentplayernum][i] = 0xff;
		}

		g_Vars.currentplayer->fsscanline = 0;
		g_Vars.currentplayer->fslastradius = outerradius;
	}

	// Increment the scanline
	for (i = 0; i < scanincrement; i++) {
		if (g_Vars.currentplayer->fsscanline >= scanbottom) {
			g_Vars.currentplayer->fsscanline = scantop;
		}

		g_IrScanlines[g_Vars.currentplayernum][g_Vars.currentplayer->fsscanline] = 0xff - i;

		g_Vars.currentplayer->fsscanline++;
	}

	g_NVBGBrightness = 0xff;
	g_NVChrHighlight = 0xde;
	g_NVChrBrightness = 0xde;

	gdl = bviewPrepareStaticRgba16(gdl, 0xffffffff, 255);

	sqinnerradius = innerradius * innerradius;

	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	gSPSetExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	for (i = scantop; i < scanbottom; i++) {
		if (i & 1) {
			red = g_IrScanlines[g_Vars.currentplayernum][i];
		} else {
			red = g_IrScanlines[g_Vars.currentplayernum][i] * 2 / 3;
		}

		red += rngRandom() % 8;

		if (red > 255) {
			red = 255;
		}

		struct RGBA ircolor = {red, 0, 0, 255};
		gfx_Set_Prim_Color(gdl++, ircolor);

		a0 = viewcentrey - i;

		if (a0 * a0 < sqinnerradius) {
			// Rendering a line that overlaps the semicircle
			// in the middle of the screen
			float f0 = a0;
			int semicirclewidth = sqrtf(sqinnerradius - (int) (f0 * f0)) * (viewwidth / (float) SCREEN_WIDTH_LO);
			int semicircleright = viewcentrex + semicirclewidth;
			int rightsidewidth = viewwidth - semicircleright;

			// Left and right of semicircle
			gfx_Fill_Rectangle(gdl++, viewleft, i, viewcentrex, i + 1);
			gfx_Fill_Rectangle(gdl++, semicircleright, i, semicircleright + rightsidewidth, i + 1);

			// The semicircle itself has a static colour
			struct RGBA semicolor = {238, 0, 0, 255};
			gfx_Set_Prim_Color(gdl++, semicolor);
			gfx_Fill_Rectangle(gdl++, viewcentrex, i, viewcentrex + semicirclewidth, i + 1);
		} else {
			gfx_Fill_Rectangle(gdl++, viewleft, i, viewleft + viewwidth, i + 1);
		}

		if (g_IrScanlines[g_Vars.currentplayernum][i] > fadeincrement) {
			g_IrScanlines[g_Vars.currentplayernum][i] -= fadeincrement;
		}
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_MODULATE_EXT);

	if (g_Menus[g_Vars.currentplayerstats->mpindex].curdialog == NULL) {
		gdl = bviewDrawMotionBlur(gdl, 0xff000000, 0x40);
	}

	return gdl;
}

/**
 * Draw a horizontal blur/sretch effect. Unused.
 *
 * The term "Intro" used in the string suggests that was made for an older
 * version of the title screen, similar to bviewDrawIntroText.
 */
Gfx *bviewDrawIntroFaderBlur(Gfx *gdl, int arg1)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	float halfheight;
	float extra;
	int y;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	gdl = bviewPrepareStaticRgba16(gdl, 0xffffffff, 255);

	halfheight = viewheight * 0.5f;

	extra = 0.5f;
	extra += 0.5f;

	for (y = viewtop; y < viewtop + viewheight; y++) {
		float frac = (y - viewtop - halfheight) / halfheight;

		if (frac < 0.0f) {
			frac = -frac;
		}

		frac += extra;

		if (frac > 1.0f) {
			frac = 1.0f;
		}

		gdl = bviewCopyPixels(gdl, fb, y, 5, y, RANDOMFRAC() * frac + 1.0f, viewleft, viewwidth);
	}

	return gdl;
}

/**
 * Called from the title screen's "Rare Presents" mode, which is unused.
 */
Gfx *bviewDrawIntroText(Gfx *gdl)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	int y;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	gdl = bviewPrepareStaticRgba16(gdl, 0x8f8f8f8f, 255);

	for (y = viewtop; y < viewtop + viewheight; y += 2) {
		gdl = bviewCopyPixels(gdl, fb, y, 5, y, 1.0f, viewleft, viewwidth);
	}

	return gdl;
}

Gfx *bviewDrawHorizonScanner(Gfx *gdl)
{
	uint16_t *fb = viGetBackBuffer();
	int viewtop = viGetViewTop();
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewleft = viGetViewLeft();
	char directiontext[32];
	char hertztext[24];
	char zoomtext[24];
	char nametext[52];
	float lookx = g_Vars.currentplayer->cam_look.x;
	float lookz = g_Vars.currentplayer->cam_look.z;
	int x;
	int y;

	char directions[][3] = {
		{'n', '\0', '\0'},
		{'n', 'e',  '\0'},
		{'e', '\0', '\0'},
		{'s', 'e',  '\0'},
		{'s', '\0', '\0'},
		{'s', 'w',  '\0'},
		{'w', '\0', '\0'},
		{'n', 'w',  '\0'},
		{'n', '\0', '\0'},
	};

	int turnangle = atan2f(-lookx, lookz) * 180.0f / M_PI;
	float fovy;
	char arrows[12];
	int tmplensheight = 130;
	int lenstop;
	int lensheight;
	int liney;
	int scale = 1;
	int vsplit = false;
	uint32_t colour;
	float range;

	g_NumActiveEffects++;

	if (g_NumActiveEffects >= 2) {
		return gdl;
	}

	if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL && PLAYERCOUNT() >= 2) {
		vsplit = true;
	}

	if (tmplensheight > viewheight - 30) {
		tmplensheight = viewheight - 30;
	}

	if (((int)(g_20SecIntervalFrac * 30.0f) & 1) == 1) {
		sprintf(arrows, ">> ");
	} else {
		sprintf(arrows, " >>");
	}

	lensheight = tmplensheight;
	lenstop = viewtop + (viewheight / 2) - (lensheight / 2);

	// Black out areas above and below lens
	gdl = textSetPrimColour(gdl, 0x000000ff);

	gfx_Fill_Rectangle(gdl++, viewleft, viewtop, viewleft + viewwidth, lenstop);
	gfx_Fill_Rectangle(gdl++, viewleft, lenstop + lensheight, viewleft + viewwidth, viewtop + viewheight);

	gdl = textSetCCPrimColorTexAlpha(gdl);

	int index = ((int)(atan2f(-lookx, lookz) * (180.0f / M_PI) + 360 + 22) % 360) / 45;
	index = utilsClamp(index, 0, 8);

	// Prepare text buffers
	sprintf(directiontext, "%s %s:%03d", arrows, &directions[index], turnangle);
	sprintf(hertztext, "%s %s%s%4.2fh", arrows, "", "", menuGetCosOscFrac(4) * 4.6f + 917.4f);

	fovy = viGetFovY();

	if (fovy == 0 || fovy == 60.0f) {
		fovy = 1;
	} else {
		fovy = ADJUST_ZOOM_FOV(60.0f) / fovy;
	}

	sprintf(zoomtext, "%s %s%s%4.2fX", arrows, "", "", fovy);

	gdl = textConfigureGfxPipeline(gdl);

	// Arrows left of product name
	if (vsplit) {
		x = viewleft + 15 * scale;
	} else {
		x = viewleft + 25 * scale;
	}

	y = lenstop - 7;
	gdl = textRenderProjected(gdl, &x, &y, arrows,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffff7f, viGetWidth(), viGetHeight(), 0, 0);

	// Product name
	strcpy(nametext, " JMBC");

	if (!vsplit) {
		strcat(nametext, " WIDE BAND");
	}

	strcat(nametext, " SCANNER\n");

	gdl = textRenderProjected(gdl, &x, &y, nametext,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffff7f, viGetWidth(), viGetHeight(), 0, 0);

	// Hertz
	x = viewleft + 75 * scale;
	y = lenstop + lensheight + 1;
	gdl = textRenderProjected(gdl, &x, &y, hertztext,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffff7f, viGetWidth(), viGetHeight(), 0, 0);

	// Zoom level
	if (vsplit) {
		x = viewleft + 75 * scale;
		y = lenstop + lensheight + 8;
	} else {
		x = viewleft + 150 * scale;
		y = lenstop + lensheight + 1;
	}

	gdl = textRenderProjected(gdl, &x, &y, zoomtext,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffff7f, viGetWidth(), viGetHeight(), 0, 0);

	// Direction
	if (vsplit) {
		x = viewleft + 75 * scale;
		y = lenstop + lensheight + 15;
	} else {
		x = viewleft + 225 * scale;
		y = lenstop + lensheight + 1;
	}

	gdl = textRenderProjected(gdl, &x, &y, directiontext,
			g_CharsHandelGothicXs, g_FontHandelGothicXs, 0xffffff7f, viGetWidth(), viGetHeight(), 0, 0);
	gdl = textSetPerspAndLOD(gdl);

	gdl = bviewPrepareStaticRgba16(gdl, 0xffffffff, 255);

	if (vsplit) {
		vsplit = 14;
	}

	if (!videoFramebuffersSupported()) {
		return gdl;
	}
	// make a copy of what we have drawn so far and use it as a texture
	gfx_No_Param(gdl++, G_RDPFLUSH_EXT);
	gfx_Copy_Framebuffer_EXT(gdl++, g_PrevFrameFb, 0, 0, 0, G_ON);
	gfx_Set_Framebuffer_Texture_EXT(gdl++, 0, 0, 0, (uintptr_t)g_PrevFrameFb);

	// Iterate horizontal lines down the lens with a bit extra on top and bottom
	for (liney = lenstop - 9; liney < lenstop + lensheight + vsplit + 9; liney++) {
		if (liney < lenstop + lensheight && liney >= lenstop) {
			// Inside the lens
			if ((liney % 2) == 0) {
				colour = 0x00ffffff;
			} else {
				colour = 0x7fffffff;
			}

			range = (liney - lenstop - lensheight * 0.5f) / (lensheight * 0.5f);

			if (range < 0) {
				range = -range;
			}

			if (range > 1) {
				range = 0;
			}

			range = (range - 0.75f) * 4.0f;

			if (range < 0) {
				range = 0;
			}

			if (range > 0) {
				colour = colourBlend(0x000000ff, colour, range * 255.0f);
			}
		} else {
			// Outside of the lens
			if ((liney % 2) == 0) {
				colour = 0x007f7fff;
			} else {
				colour = 0x7fffffff;
			}

			range = 0;
		}

		// Different coloured lines at 1/4 and 3/4 marks in the lens
		if (liney == lenstop + lensheight / 4 || liney == lenstop + lensheight - lensheight / 4) {
			colour = 0xffffffff;
		}

		gfx_Set_Env_Color(gdl++, utilsUnpackColorRGBA(colour));

		const float xscale = RANDOMFRAC() * range + 1;
		const float halfwidth = viewwidth / 2.f;
		const int left = viewleft + halfwidth * (1.f - xscale);
		const int right = viewleft + halfwidth * (1.f + xscale);
		gdl += gfx_Image_Rectangle_EXT(gdl,
			0, liney, viewleft, liney,
			right, liney + 1, viewleft + viewwidth, liney + 1,
			0, videoGetNativeWidth(), videoGetNativeHeight());
	}

	return gdl;
}

/**
 * Draws the black part of the IR scanner, which obscures the edges of the
 * screen and creates the binocular effect.
 *
 * The two circles are "placed" at the 1/3 and 2/3 marks horizontally. The
 * screen is then iterated top to bottom, one line at a time, and draws
 * black rectangles on each line to fill in the area outside the circles.
 */
Gfx *bviewDrawIrBinoculars(Gfx *gdl)
{
	int viewheight = viGetViewHeight();
	int viewwidth = viGetViewWidth();
	int viewtop = viGetViewTop();
	int viewleft = viGetViewLeft();
	int viewright = viewleft + viewwidth;
	int viewbottom = viewtop + viewheight;
	int leftx = viewleft + viewwidth / 3;
	int rightx = viewleft + (viewwidth * 2) / 3;
	int centrey = (viewtop + viewbottom) / 2;
	int radius = g_IrBinocularRadius;
	int sqradius = radius * radius;
	int y;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	RGBA primColor = {0, 0, 0, 255};
	gfx_Set_Prim_Color(gdl++, primColor);

	for (y = viewtop; y < viewbottom; y++) {
		int ytocentre = centrey - y;
		int sqytocentre = ytocentre * ytocentre;

		if (sqytocentre < sqradius) {
			int xoffset = (viewwidth / (float) SCREEN_WIDTH_LO) * sqrtf(sqradius - sqytocentre);

			// Left side
			if (leftx - xoffset > viewleft) {
				gdl = bviewDrawIrRect(gdl, viewleft, y, leftx - xoffset, y + 1);
			}

			// Middle (top and bottom)
			if (leftx + xoffset < rightx - xoffset) {
				gdl = bviewDrawIrRect(gdl, leftx + xoffset, y, rightx - xoffset, y + 1);
			}

			// Right side
			if (rightx + xoffset < viewright) {
				gdl = bviewDrawIrRect(gdl, rightx + xoffset, y, viewright, y + 1);
			}
		} else {
			// Very top or bottom - whole line is black
			gdl = bviewDrawIrRect(gdl, viewleft, y, viewright, y + 1);
		}
	}

	return gdl;
}

void bviewSetMotionBlur(uint32_t bluramount)
{
	g_NumActiveEffects = 0;
	g_DoNotRedrawBlur = false;
	g_BlurChange = (bluramount << 1) / 3; // same as multiplying by 2/3
}

void bviewClearMotionBlur(void)
{
	g_BlurChange = 0;
}
