#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/menuutils.h"
#include "game/bondgun.h"
#include "game/tex.h"
#include "game/savebuffer.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/mtxutils.h"
#include "game/textutils.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/rng.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "video.h"
#include "game/bondview.h"

#define NUM_SUCCESS_PARTICLES 280

#define BLURIMG_WIDTH  40
#define BLURIMG_HEIGHT 30
#define SAMPLE_WIDTH  8
#define SAMPLE_HEIGHT 8
#define PXTOBYTES(val) ((val) * 2)

static int g_MenuBlurFb = -1;
static int g_MenuScreenFb = -1;
static bool g_MenuBlurDone = false;

/**
 * Blur the gameplay background for the pause menu.
 *
 * The blurred image is 30x40 pixels at 16 bits per pixel. At standard
 * resolution this is 1/8th the size of the framebuffer.
 *
 * This function reads the framebuffer in blocks of 8x8 pixels. Each block's
 * R/G/B components are averaged and used to set a pixel in the blurred buffer.
 *
 * If hi-res is being used, every second horizontal pixel on the framebuffer is
 * read instead. The blurred image is the same size regardless of hi-res.
 *
 * The transition effect when pausing and unpausing is implemented elsewhere.
 * It's a simple fade between the source framebuffer and the blurred image.
 * Only one blurred image is made.
 */
void menugfxCreateBlur(void)
{
	if (g_MenuBlurFb < 0) {
		g_MenuBlurFb = videoCreateFramebuffer(BLURIMG_WIDTH, BLURIMG_HEIGHT, true, false);
		g_MenuScreenFb = videoCreateFramebuffer(0, 0, false, true);
	}
	// copy full viewport and downscale to 40x30
	videoCopyFramebuffer(g_MenuBlurFb, 0, -1, -1);
	// we'll generate a blurred version later
	g_MenuBlurDone = false;
}

Gfx *menugfxRenderBgBlur(Gfx *gdl, uint32_t colour, int16_t arg2, int16_t arg3)
{
	Col *colours;
	Vtx *vertices;
	int width;
	int height;

	width = viGetWidth();
	height = viGetHeight();
	if (g_MenuBlurFb >= 0 && !g_MenuBlurDone) {
		// blit the small blur texture onto a screen-sized framebuffer while blurring it
		g_MenuBlurDone = true;
		gdl = bviewPrepareStaticRgba16(gdl, 0xffffffff, 0xff);
		gfx_Set_Texture_Filter(gdl++, G_TF_BLUR_EXT);
		gfx_Set_Framebuffer_Texture_EXT(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, BLURIMG_WIDTH, (uintptr_t)g_MenuBlurFb);
		gfx_Set_Framebuffer_Target_EXT(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (uintptr_t)g_MenuScreenFb);
		gdl += gfx_Image_Rectangle_EXT(gdl, 0, 0, 0, 0, width, height, BLURIMG_WIDTH, BLURIMG_HEIGHT, 0, BLURIMG_WIDTH, BLURIMG_HEIGHT);
		gfx_Set_Framebuffer_Target_EXT(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (uintptr_t)0);
		gfx_Set_Framebuffer_Texture_EXT(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, BLURIMG_WIDTH, (uintptr_t)0);
	}

	colours = gfxAllocateColours(1);
	vertices = gfxAllocateVertices(4);

	gfx_Texture(gdl++, 0xffff, 0xffff, 0, G_TX_RENDERTILE, G_ON);

	gDPLoadTextureBlock(gdl++, g_BlurBuffer, G_IM_FMT_RGBA, G_IM_SIZ_16b, BLURIMG_WIDTH, BLURIMG_HEIGHT, 0,
			G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
			G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

	// LoadTextureBlock will set up the sizes, but we'll use the framebuffer instead of g_BlurBuffer
	gfx_Set_Framebuffer_Texture_EXT(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, width, (uintptr_t)g_MenuScreenFb);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	width = SCREEN_320 * 10;
	height = viGetHeight() * 10;

	*(uint16_t *)&vertices[0].x = arg2;
	*(uint16_t *)&vertices[0].y = arg3;
	vertices[0].z = -10;
	*(uint16_t *)&vertices[1].x = (int)width + arg2 + 40;
	*(uint16_t *)&vertices[1].y = arg3;
	vertices[1].z = -10;
	*(uint16_t *)&vertices[2].x = (int)width + arg2 + 40;
	*(uint16_t *)&vertices[2].y = (int)height + arg3 + 50;
	vertices[2].z = -10;
	*(uint16_t *)&vertices[3].x = arg2;
	*(uint16_t *)&vertices[3].y = (int)height + arg3 + 50;
	vertices[3].z = -10;

	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = SCREEN_320 * 4;
	vertices[1].t = 0;
	vertices[2].s = SCREEN_320 * 4;
	vertices[2].t = SCREEN_320 * 3;
	vertices[3].s = 0;
	vertices[3].t = SCREEN_320 * 3;

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 0;
	vertices[3].colour = 0;

	colours[0].word = PD_BE32(colour);

	gfx_Color(gdl++, colours, 1);
	gfx_Vertex(gdl++, vertices, 4, 0);

	gfx_Tri2(gdl++, 0, 1, 2, 2, 3, 0);

	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);

	return gdl;
}

Gfx *menugfxRenderDialogBackground(Gfx *gdl, int x1, int y1, int x2, int y2, struct menudialog *dialog, uint32_t colour1, uint32_t colour2, float arg8)
{
	uint32_t leftcolour;
	uint32_t rightcolour;

	// Render the dialog's background fill
	gdl = textSetPrimColour(gdl, colour1);

	gfx_Fill_Rectangle(gdl++, x1, y1, x2, y2);

	gdl = textSetCCPrimColorTexAlpha(gdl);

	if (dialog->transitionfrac < 0.0f) {
		leftcolour = g_MenuColours[dialog->type].dialog_border1;
	} else {
		leftcolour = colourBlend(g_MenuColours[dialog->type2].dialog_border1, g_MenuColours[dialog->type].dialog_border1, dialog->colourweight);
	}

	if (dialog->transitionfrac < 0.0f) {
		rightcolour = g_MenuColours[dialog->type].dialog_border2;
	} else {
		rightcolour = colourBlend(g_MenuColours[dialog->type2].dialog_border2, g_MenuColours[dialog->type].dialog_border2, dialog->colourweight);
	}

	// Right, left, bottom border
	gdl = menugfxDrawDialogBorderLine(gdl, x2 - 1, y1, x2, y2, rightcolour, rightcolour);
	gdl = menugfxDrawDialogBorderLine(gdl, x1, y1, x1 + 1, y2, leftcolour, leftcolour);
	gdl = menugfxDrawDialogBorderLine(gdl, x1, y2 - 1, x2, y2, leftcolour, rightcolour);

	return gdl;
}

Gfx *menugfxDrawDropdownBackground(Gfx *gdl, int x1, int y1, int x2, int y2)
{
	Col *colours = gfxAllocateColours(3);
	Vtx *vertices = gfxAllocateVertices(6);
	uint32_t colour1;
	uint32_t colour2;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);

	texSelect(&gdl, NULL, 2, 0, 2, true, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	vertices[0].x = x1 * 10;
	vertices[0].y = y1 * 10;
	vertices[0].z = -10;

	vertices[1].x = x2 * 10;
	vertices[1].y = y1 * 10;
	vertices[1].z = -10;

	vertices[2].x = x1 * 10;
	vertices[2].y = (y2 + y1) / 2 * 10;
	vertices[2].z = -10;

	vertices[3].x = x2 * 10;
	vertices[3].y = (y2 + y1) / 2 * 10;
	vertices[3].z = -10;

	vertices[4].x = x1 * 10;
	vertices[4].y = y2 * 10;
	vertices[4].z = -10;

	vertices[5].x = x2 * 10;
	vertices[5].y = y2 * 10;
	vertices[5].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 4;
	vertices[3].colour = 4;
	vertices[4].colour = 8;
	vertices[5].colour = 8;

	colour1 = textHighlightSweep((x1 + x2) / 2, (y2 + y1) / 2, 0xffffffff) & 0xff;
	colour2 = (textHighlightSweep((x1 + x2) / 2, (y2 + y1) / 2, 0xffffff7f) & 0xff) | 0x00006f00;

	colours[0].word = PD_BE32(colour1 | 0x00006f00);
	colours[1].word = PD_BE32(colour2);
	colours[2].word = PD_BE32(colour1 | 0x00003f00);

	gfx_Color(gdl++, colours, 3);
	gfx_Vertex(gdl++, vertices, 6, 0);

	gfx_Tri4(gdl++, 0, 1, 3, 3, 2, 0, 2, 3, 4, 4, 3, 5);

	return gdl;
}

Gfx *menugfxDrawListGroupHeader(Gfx *gdl, int x1, int y1, int x2, int y2, int x3, uint8_t alpha)
{
	Col *colours = gfxAllocateColours(7);
	Vtx *vertices = gfxAllocateVertices(9);
	uint32_t alpha1;
	uint32_t alpha2;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	vertices[0].x = x1 * 10;
	vertices[0].y = y1 * 10;
	vertices[0].z = -10;
	vertices[1].x = x2 * 10;
	vertices[1].y = y1 * 10;
	vertices[1].z = -10;
	vertices[2].x = x1 * 10;
	vertices[2].y = (y2 + y1) / 2 * 10;
	vertices[2].z = -10;
	vertices[3].x = x2 * 10;
	vertices[3].y = (y2 + y1) / 2 * 10;
	vertices[3].z = -10;
	vertices[4].x = x1 * 10;
	vertices[4].y = y2 * 10;
	vertices[4].z = -10;
	vertices[5].x = x2 * 10;
	vertices[5].y = y2 * 10;
	vertices[5].z = -10;
	vertices[6].x = x3 * 10;
	vertices[6].y = y1 * 10;
	vertices[6].z = -10;
	vertices[7].x = x3 * 10;
	vertices[7].y = (y2 + y1) / 2 * 10;
	vertices[7].z = -10;
	vertices[8].x = x3 * 10;
	vertices[8].y = y2 * 10;
	vertices[8].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 24;
	vertices[2].colour = 4;
	vertices[3].colour = 4;
	vertices[4].colour = 8;
	vertices[5].colour = 8;
	vertices[6].colour = 12;
	vertices[7].colour = 16;
	vertices[8].colour = 20;

	alpha1 = alpha;
	alpha2 = alpha;

	colours[0].word = PD_BE32(0x00006f00 | alpha1);
	colours[1].word = PD_BE32(0x00006f00 | alpha2);
	colours[2].word = PD_BE32(0x00003f00 | alpha2);
	colours[3].word = PD_BE32(0xffffff00);
	colours[4].word = PD_BE32((0x00006f00 | alpha2) & 0xffffff00);
	colours[5].word = PD_BE32((0x00003f00 | alpha1) & 0xffffff00);
	colours[6].word = PD_BE32(0x6f6f6f00 | alpha1);

	gfx_Color(gdl++, colours, 7);
	gfx_Vertex(gdl++, vertices, 9, 0);

	gfx_Tri4(gdl++, 0, 1, 3, 3, 2, 0, 2, 3, 4, 4, 3, 5);
	gfx_Tri4(gdl++, 1, 6, 7, 7, 3, 1, 3, 7, 8, 8, 5, 3);

	gdl = menugfxDrawShimmer(gdl, x1, y1, x2, y1 + 1, (alpha1 & 0xff) >> 2, 1, 0x28, 0);
	gdl = menugfxDrawShimmer(gdl, x1, y2, x2, y2 + 1, (alpha1 & 0xff) >> 2, 0, 0x28, 1);

	return gdl;
}

	// Mismatch: Goal has the if statement with empty contents
Gfx *menugfxRenderGradient(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colourstart, uint32_t colourmid, uint32_t colourend)
{
	Col *colours = gfxAllocateColours(3);
	Vtx *vertices = gfxAllocateVertices(6);
	int ymid;

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	ymid = (y1 + y2) / 2;

	if (x2 - x1 < ymid * 2) {
		ymid <<= 0;
	}

	// 0 = top left
	// 1 = top right
	// 2 = bottom right
	// 3 = bottom left
	// 4 = mid left
	// 5 = mid right

	vertices[0].z = -10;
	vertices[1].z = -10;
	vertices[2].z = -10;
	vertices[3].z = -10;
	vertices[4].z = -10;
	vertices[5].z = -10;

	vertices[0].x = x1 * 10;
	vertices[0].y = y1 * 10;
	vertices[0].colour = 0;

	vertices[1].x = x2 * 10;
	vertices[1].y = y1 * 10;
	vertices[1].colour = 0;

	vertices[2].x = x2 * 10;
	vertices[2].y = y2 * 10;
	vertices[2].colour = 4;

	vertices[3].x = x1 * 10;
	vertices[3].y = y2 * 10;
	vertices[3].colour = 4;

	vertices[4].x = x1 * 10;
	vertices[4].y = ymid * 10;
	vertices[4].colour = 8;

	vertices[5].x = x2 * 10;
	vertices[5].y = ymid * 10;
	vertices[5].colour = 8;

	colours[0].word = PD_BE32(colourstart);
	colours[2].word = PD_BE32(colourmid);
	colours[1].word = PD_BE32(colourend);

	gfx_Color(gdl++, colours, 3);
	gfx_Vertex(gdl++, vertices, 6, 0);
	gfx_Tri4(gdl++, 0, 1, 5, 5, 4, 0, 2, 3, 4, 4, 5, 2);

	return gdl;
}

Gfx *menugfxRenderSlider(Gfx *gdl, int x1, int y1, int x2, int y2, int markerx, uint32_t colour)
{
	Col *colours = gfxAllocateColours(3);
	Vtx *vertices = gfxAllocateVertices(6);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	// Marker triangle
	vertices[0].x = markerx * 10 - 40;
	vertices[0].y = y2 * 10 - 80;
	vertices[0].z = -10;
	vertices[1].x = markerx * 10 + 40;
	vertices[1].y = y2 * 10 - 80;
	vertices[1].z = -10;
	vertices[2].x = markerx * 10;
	vertices[2].y = y2 * 10 + 10;
	vertices[2].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 4;

	// Background triangle
	vertices[3].x = x1 * 10;
	vertices[3].y = y2 * 10;
	vertices[3].z = -10;
	vertices[4].x = (int16_t)(x2 * 10) - 40;
	vertices[4].y = y1 * 10;
	vertices[4].z = -10;
	vertices[5].x = x2 * 10;
	vertices[5].y = y2 * 10;
	vertices[5].z = -10;

	vertices[3].colour = 8;
	vertices[4].colour = 8;
	vertices[5].colour = 8;

	colours[0].word = PD_BE32((colour & 0xffffff00) | 0x4f);
	colours[1].word = PD_BE32(0xffffffff);
	colours[2].word = PD_BE32(0x0000ff4f);

	gfx_Color(gdl++, colours, 3);
	gfx_Vertex(gdl++, vertices, 6, 0);

	gfx_Tri1(gdl++, 3, 4, 5);

	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	gfx_Tri1(gdl++, 0, 1, 2);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	// Line to the left of the marker: blue -> white gradient
	gdl = menugfxDrawLine(gdl, x1, y2, markerx, y2 + 1, 0x0000ffff, 0xffffffff);

	// Line to the right of the marker: solid blue
	gdl = menugfxDrawLine(gdl, markerx, y2, x2, y2 + 1, 0x0000ffff, 0x0000ffff);

	return gdl;
}

Gfx *menugfx0f0e2348(Gfx *gdl)
{
	gfx_Set_Geometry_Mode(gdl++, G_CULL_BACK);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 0
		G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE,              // Alpha 0
		G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 1
		G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE);             // Alpha 1
	gfx_Set_Combine_Key(gdl++, G_CK_NONE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Render_Mode(gdl++, G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
	gfx_Set_Geometry_Mode(gdl++, G_ZBUFFER);

	return gdl;
}

Gfx *menugfx0f0e2498(Gfx *gdl)
{
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, 0, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	return gdl;
}

Gfx *menugfxDrawTri2(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2, bool arg7)
{
	Vtx *vertices;
	Col *colours;

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(4);

	vertices[0].x = x1 * 10;
	vertices[0].y = y1 * 10;
	vertices[0].z = -10;

	vertices[1].x = x2 * 10;
	vertices[1].y = y1 * 10;
	vertices[1].z = -10;

	vertices[2].x = x2 * 10;
	vertices[2].y = y2 * 10;
	vertices[2].z = -10;

	vertices[3].x = x1 * 10;
	vertices[3].y = y2 * 10;
	vertices[3].z = -10;

	if (!arg7) {
		vertices[0].colour = 0;
		vertices[1].colour = 4;
		vertices[2].colour = 4;
		vertices[3].colour = 0;
	} else {
		vertices[0].colour = 0;
		vertices[1].colour = 0;
		vertices[2].colour = 4;
		vertices[3].colour = 4;
	}

	colours[0].word = PD_BE32(colour1);
	colours[1].word = PD_BE32(colour2);

	gfx_Color(gdl++, colours, 2);
	gfx_Vertex(gdl++, vertices, 4, 0);
	gfx_Tri2(gdl++, 0, 1, 2, 2, 3, 0);

	return gdl;
}

Gfx *menugfxDrawLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2)
{
	gdl = menugfx0f0e2498(gdl);
	gdl = menugfxDrawTri2(gdl, x1, y1, x2, y2, colour1, colour2, false);

	return gdl;
}

Gfx *menugfxDrawProjectedLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2)
{
	int numfullblocks;
	int i;
	uint32_t partcolourtop;
	uint32_t partcolourbottom;
	int partbottom;
	int parttop;

	if (textHasDiagonalBlend()) {
		if (x2 - x1 < y2 - y1) {
			// Portrait
			numfullblocks = (y2 - y1) / 15;
			parttop = y1;
			partcolourtop = textApplyProjectionColour(x1, y1, colour1);

			for (i = 0; i < numfullblocks; i++) {
				partbottom = y1 + i * 15;

				if (y2 - partbottom < 3) {
					partbottom = y2;
					partcolourbottom = textApplyProjectionColour(x2, partbottom, colour2);
				} else {
					partcolourbottom = colourBlend(colour2, colour1, (partbottom - y1) * 255 / (y2 - y1));
					// @bug: y1 should be x1
					partcolourbottom = textApplyProjectionColour(y1, partbottom, partcolourbottom);
				}

				gdl = menugfxDrawTri2(gdl, x1, parttop, x2, partbottom, partcolourtop, partcolourbottom, false);

				parttop = partbottom;
				partcolourtop = partcolourbottom;
			}

			partcolourbottom = textApplyProjectionColour(x2, y2, colour2);
			gdl = menugfxDrawTri2(gdl, x1, parttop, x2, y2, partcolourtop, partcolourbottom, false);
		} else {
			// Landscape
			int numfullblocks;
			int i;
			uint32_t partcolourleft;
			uint32_t partcolourright;
			int partright;
			int partleft;

			numfullblocks = (x2 - x1) / 15;
			partleft = x1;
			partcolourleft = textApplyProjectionColour(x1, y1, colour1);

			for (i = 0; i < numfullblocks; i++) {
				partright = x1 + i * 15;

				if (x2 - partright < 3) {
					partright = x2;
					partcolourright = textApplyProjectionColour(x2, y2, colour2);
				} else {
					partcolourright = colourBlend(colour2, colour1, (partright - x1) * 255 / (x2 - x1));
					partcolourright = textApplyProjectionColour(partright, y1, partcolourright);
				}

				gdl = menugfxDrawTri2(gdl, partleft, y1, partright, y2, partcolourleft, partcolourright, false);

				partleft = partright;
				partcolourleft = partcolourright;
			}

			if (partcolourtop);

			partcolourright = textApplyProjectionColour(x2, y2, colour2);
			gdl = menugfxDrawTri2(gdl, partleft, y1, x2, y2, partcolourleft, partcolourright, false);
		}
	} else {
		gdl = menugfxDrawTri2(gdl, x1, y1, x2, y2, colour1, colour2, false);
	}

	return gdl;
}

/**
 * Consider rendering a shimmer effect along a line, based on timing.
 *
 * Also known as the white comets that travel along the dialog borders and
 * within some elements in the dialog.
 *
 * The given coordinates are the coordinates of the full line. Colour is the
 * natural colour of the line.
 *
 * The shimmer will go right to left or bottom to top unless the reverse
 * argument is set to true.
 */
Gfx *menugfxDrawShimmer(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour, bool arg6, int arg7, bool reverse)
{
	int shimmerleft;
	int shimmertop;
	int shimmerright;
	int shimmerbottom;
	int v0;
	int alpha;
	int minalpha;
	uint32_t tailcolour;

	alpha = 0;
	minalpha = 0;

	if (reverse) {
		v0 = 6.0f * g_20SecIntervalFrac * 600.0f;
	} else {
		v0 = (1.0f - g_20SecIntervalFrac) * 6.0f * 600.0f;
	}

	if (y2 - y1 < x2 - x1) {
		// Horizontal
		v0 += (uint32_t)(y1 + x1);
		v0 %= 600;
		shimmerleft = x1 + v0 - arg7;
		shimmerright = shimmerleft + arg7;

		if (shimmerleft < x1) {
			alpha = x1 - shimmerleft;
			shimmerleft = x1;
		}

		if (shimmerright > x2) {
			minalpha = shimmerright - x2;
			shimmerright = x2;
		}

		if (alpha < minalpha) {
			alpha = minalpha;
		}

		alpha = alpha * 255 / arg7;

		if (alpha > 255) {
			alpha = 255;
		}

		if (x1 <= shimmerright && x2 >= shimmerleft) {
			if (arg6) {
				gdl = menugfx0f0e2498(gdl);
			}

			tailcolour = ((((colour & 0xff) * (0xff - alpha)) / 255) & 0xff) | 0xffffff00;

			if (reverse) {
				// Left to right
				gdl = menugfxDrawTri2(gdl, shimmerleft, y1, shimmerright, y2, 0xffffff00, tailcolour, 0);
			} else {
				// Right to left
				gdl = menugfxDrawTri2(gdl, shimmerleft, y1, shimmerright, y2, tailcolour, 0xffffff00, 0);
			}
		}
	} else {
		// Vertical
		v0 += (uint32_t)(y1 + x1);
		v0 %= 600;
		shimmertop = y1 + v0 - arg7;
		shimmerbottom = shimmertop + arg7;

		if (shimmertop < y1) {
			alpha = y1 - shimmertop;
			shimmertop = y1;
		}

		if (shimmerbottom > y2) {
			minalpha = shimmerbottom - y2;
			shimmerbottom = y2;
		}

		if (alpha < minalpha) {
			alpha = minalpha;
		}

		alpha = alpha * 255 / arg7;

		if (alpha > 255) {
			alpha = 255;
		}

		if (y1 <= shimmerbottom && y2 >= shimmertop) {
			if (arg6) {
				gdl = menugfx0f0e2498(gdl);
			}

			tailcolour = ((((colour & 0xff) * (0xff - alpha)) / 255) & 0xff) | 0xffffff00;

			if (reverse) {
				// Top to bottom
				gdl = menugfxDrawTri2(gdl, x1, shimmertop, x2, shimmerbottom, 0xffffff00, tailcolour, 1);
			} else {
				// Bottom to top
				gdl = menugfxDrawTri2(gdl, x1, shimmertop, x2, shimmerbottom, tailcolour, 0xffffff00, 1);
			}
		}
	}

	return gdl;
}

Gfx *menugfxDrawDialogBorderLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2)
{
	gdl = menugfxDrawLine(gdl, x1, y1, x2, y2, colour1, colour2);
	gdl = menugfxDrawShimmer(gdl, x1, y1, x2, y2, colour1, 0, 10, false);

	return gdl;
}

Gfx *menugfxDrawFilledRect(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2)
{
	gdl = menugfx0f0e2498(gdl);
	gdl = menugfxDrawProjectedLine(gdl, x1, y1, x2, y2, colour1, colour2);
	gdl = menugfxDrawShimmer(gdl, x1, y1, x2, y2, colour1, 0, 10, false);

	return gdl;
}

/**
 * Draw a chevron/arrow using a simple triangle.
 *
 * Used for the left/right arrows on the Combat Simulator head/body selectors.
 *
 * The x and y arguments refer to the apex of the chevron. Size is the distance
 * from the apex to the opposite side.
 */
Gfx *menugfxDrawCarouselChevron(Gfx *gdl, int x, int y, int size, int direction, uint32_t colour1, uint32_t colour2)
{
	Vtx *vertices;
	Col *colours;
	int16_t halfwidth;
	int16_t halfheight;
	int16_t relx;
	int16_t rely;

	relx = 0;
	rely = 0;
	halfwidth = 0;
	halfheight = 0;

	size *= 10;

	switch (direction) {
	case 0: // Down
		rely = -size;
		halfwidth = -size / 2;
		break;
	case 1: // Left
		relx = size;
		halfheight = -size / 2;
		break;
	case 2: // Up
		rely = size;
		halfwidth = size / 2;
		break;
	case 3: // Right
		relx = size * -1;
		halfheight = size / 2;
		break;
	}

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(3);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	vertices[0].x = x * 10;
	vertices[0].y = y * 10;
	vertices[0].z = -10;

	vertices[1].x = x * 10 + relx + halfwidth;
	vertices[1].y = y * 10 + rely + halfheight;
	vertices[1].z = -10;

	vertices[2].x = x * 10 + relx - halfwidth;
	vertices[2].y = y * 10 + rely - halfheight;
	vertices[2].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 4;
	vertices[2].colour = 4;

	colours[0].word = PD_BE32(colour1);
	colours[1].word = PD_BE32(colour2);

	gfx_Color(gdl++, colours, 2);
	gfx_Vertex(gdl++, vertices, 3, 0);
	gfx_Tri1(gdl++, 0, 1, 2);

	return gdl;
}

/**
 * Draw a chevron/arrow for dialogs.
 *
 * These chevons are drawn with two triangles, and the middle rear vertex's
 * position can be adjusted on each frame to create an animated chevron.
 *
 * The x and y arguments refer to the apex of the chevron. Size is the distance
 * from the apex to the opposite side.
 */
Gfx *menugfxDrawDialogChevron(Gfx *gdl, int x, int y, int size, int direction, uint32_t colour1, uint32_t colour2, float arg7)
{
	Col *colours;
	Vtx *vertices;
	int16_t halfwidth;
	int16_t halfheight;
	int16_t relx;
	int16_t rely;

	relx = 0;
	rely = 0;
	halfwidth = 0;
	halfheight = 0;

	size *= 10;

	switch (direction) {
	case 0: // Down
		rely = -size;
		halfwidth = -((int)(size * arg7 * 0.5f) + size) / 2;
		break;
	case 1: // Left
		relx = size;
		halfheight = -((int)(size * arg7 * 0.5f) + size) / 2;
		break;
	case 2: // Up
		rely = size;
		halfwidth = ((int)(size * arg7 * 0.5f) + size) / 2;
		break;
	case 3: // Right
		relx = -size;
		halfheight = ((int)(size * arg7 * 0.5f) + size) / 2;
		break;
	}

	colours = gfxAllocateColours(2);
	vertices = gfxAllocateVertices(4);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 0
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE,         // alpha cycle 0
		G_CCMUX_TEXEL0, G_CCMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // color cycle 1 (same as cycle 0)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE          // alpha cycle 1 (same as cycle 0)
	);
	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	// Apex
	vertices[0].x = x * 10;
	vertices[0].y = y * 10;
	vertices[0].z = -10;

	vertices[1].x = x * 10 + relx + halfwidth;
	vertices[1].y = y * 10 + rely + halfheight;
	vertices[1].z = -10;

	vertices[2].x = x * 10 + relx - halfwidth;
	vertices[2].y = y * 10 + rely - halfheight;
	vertices[2].z = -10;

	relx = (relx / 3.0f + (relx * 1.5f * (1.0f - arg7)) / 3.0f);
	rely = rely / 3.0f + (rely * 1.5f * (1.0f - arg7)) / 3.0f;

	// Middle vertex
	vertices[3].x = x * 10 + relx;
	vertices[3].y = y * 10 + rely;
	vertices[3].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 4;
	vertices[2].colour = 4;
	vertices[3].colour = 4;

	colours[0].word = PD_BE32(colour1);
	colours[1].word = PD_BE32(colour2);

	gfx_Color(gdl++, colours, 2);
	gfx_Vertex(gdl++, vertices, 4, 0);
	gfx_Tri2(gdl++, 0, 1, 3, 3, 2, 0);

	return gdl;
}

Gfx *menugfxDrawCheckbox(Gfx *gdl, int x, int y, int size, bool fill, uint32_t bordercolour, uint32_t fillcolour)
{
	if (fill) {
		gdl = textSetPrimColour(gdl, fillcolour);
		gfx_Fill_Rectangle(gdl++, x, y, x + size, y + size);
		gdl = textSetCCPrimColorTexAlpha(gdl);
	}

	gdl = textSetPrimColour(gdl, bordercolour);

	gfx_Fill_Rectangle(gdl++, x, y, x + size + 1, y + 1);
	gfx_Fill_Rectangle(gdl++, x, y + size, x + size + 1, y + size + 1);
	gfx_Fill_Rectangle(gdl++, x, y + 1, x + 1, y + size);
	gfx_Fill_Rectangle(gdl++, x + size, y + 1, x + size + 1, y + size);

	gdl = textSetCCPrimColorTexAlpha(gdl);

	return gdl;
}

Gfx *menugfxRenderBgFailure(Gfx *gdl)
{
	float spb4;
	int i;
	float angle;
	int s0;
	int s1;
	int s2;
	int s3;
	int s6;
	int s7;
	uint32_t alpha1;
	uint32_t alpha2;

	spb4 = M_TAU * g_20SecIntervalFrac;

	g_MenuProjectFromX = g_MenuProjectFromY = 0;

	gdl = func0f0d4a3c(gdl);

	var8009de90 = -100000;
	var8009de94 = 100000;

	gdl = menugfxDrawPlane(gdl, -10000, 0, 10000, 0, 0x00007f7f, 0x00007f7f, MENUPLANE_04);
	gdl = menugfxDrawPlane(gdl, -10000, 300, 10000, 300, 0x00007f7f, 0x00007f7f, MENUPLANE_04);

	for (i = 0; i < 3; i++) {
		angle = (2.0f * i * M_PI) / 3.0f + spb4;
		s6 = sinf(angle) * 600.0f;
		s3 = cosf(angle) * 600.0f;

		angle += 0.52359879016876f;
		s2 = sinf(angle) * 600.0f;
		s0 = cosf(angle) * 600.0f;

		angle += 0.52359879016876f;
		s1 = sinf(angle) * 600.0f;
		s7 = cosf(angle) * 600.0f;

		s6 += 160;
		s2 += 160;
		s1 += 160;
		s3 += 120;
		s0 += 120;
		s7 += 120;

		gdl = menugfxDrawPlane(gdl, s6, s3, s2, s0, 0xff000040, 0xff00007f, MENUPLANE_02);
		gdl = menugfxDrawPlane(gdl, s2, s0, s1, s7, 0xff00007f, 0xff000040, MENUPLANE_03);

		angle = -2.0f * spb4 + (2.0f * i * M_PI) / 3.0f;
		s6 = sinf(angle) * 600.0f;
		s3 = cosf(angle) * 600.0f;

		angle += 0.17453293502331f;
		s2 = sinf(angle) * 600.0f;
		s0 = cosf(angle) * 600.0f;

		angle += 0.099733099341393f;
		s1 = sinf(angle) * 600.0f;
		s7 = cosf(angle) * 600.0f;

		s6 += 160;
		s2 += 160;
		s1 += 160;
		s3 += 120;
		s0 += 120;
		s7 += 120;

		alpha1 = menuGetCosOscFrac(4) * 127.0f;
		alpha2 = menuGetCosOscFrac(4) * 55.0f;

		gdl = menugfxDrawPlane(gdl, s6, s3, s2, s0, 0xffff0000 | alpha2, 0xffff0000 | alpha1, MENUPLANE_02);
		gdl = menugfxDrawPlane(gdl, s2, s0, s1, s7, 0xffff0000 | alpha1, 0xffff0000 | alpha2, MENUPLANE_03);

		angle = -2.0f * spb4 + (2.0f * i * M_PI) / 3.0f + M_PI;
		s6 = sinf(angle) * 600.0f;
		s3 = cosf(angle) * 600.0f;

		angle += 0.17453293502331f;
		s2 = sinf(angle) * 600.0f;
		s0 = cosf(angle) * 600.0f;

		angle += 0.099733099341393f;
		s1 = sinf(angle) * 600.0f;
		s7 = cosf(angle) * 600.0f;

		s6 += 160;
		s2 += 160;
		s1 += 160;
		s3 += 120;
		s0 += 120;
		s7 += 120;

		alpha1 = (1.0f - menuGetCosOscFrac(4)) * 99.0f;
		alpha2 = (1.0f - menuGetCosOscFrac(4)) * 33.0f;

		gdl = menugfxDrawPlane(gdl, s6, s3, s2, s0, 0xffffff00 | alpha2, 0xffffff00 | alpha1, MENUPLANE_02);
		gdl = menugfxDrawPlane(gdl, s2, s0, s1, s7, 0xffffff00 | alpha1, 0xffffff00 | alpha2, MENUPLANE_03);
	}

	gdl = func0f0d4c80(gdl);

	return gdl;
}

/**
 * Draw the background for the Combat Simulator menus.
 *
 * The background has two "cones" being projected into the camera's eye and
 * rotating in opposite directions.
 */
Gfx *menugfxRenderBgCone(Gfx *gdl)
{
	float baseangle;
	float angle;
	int x1;
	int y1;
	int x2;
	int y2;
	int i;
	uint32_t colourupper;
	uint32_t colour;

	// Cone 1
	baseangle = M_TAU * g_20SecIntervalFrac * 2.0f;
	colourupper = (uint32_t) (menuGetSinOscFrac(1.0f) * 255.0f) << 16;

	gdl = func0f0d4a3c(gdl);

	var8009de90 = -100000;
	var8009de94 = 100000;

	for (i = 0; i < 8; i++) {
		angle = baseangle + i * 2.0f * M_PI * 0.125f;

		x1 = 600.0f * sinf(angle);
		y1 = 600.0f * cosf(angle);
		x2 = 600.0f * sinf(angle + 0.78539818525314f);
		y2 = 600.0f * cosf(angle + 0.78539818525314f);

		x1 += 160;
		x2 += 160;
		y1 += 120;
		y2 += 120;

		colour = colourupper | 0xff00007f;

		gdl = menugfxDrawPlane(gdl, x1, y1, x2, y2, colour, colour, MENUPLANE_08);
	}

	// Cone 2
	colourupper = (uint32_t) (255.0f - menuGetCosOscFrac(1.0f) * 255.0f) << 16;

	baseangle = M_TAU * g_20SecIntervalFrac;

	for (i = 0; i < 8; i++) {
		if (gdl && gdl);

		angle = -baseangle + 2.0f * i * M_PI * 0.125f;

		x1 = 600.0f * sinf(angle);
		y1 = 600.0f * cosf(angle);
		x2 = 600.0f * sinf(angle + 0.78539818525314f);
		y2 = 600.0f * cosf(angle + 0.78539818525314f);

		x1 += 160;
		x2 += 160;
		y1 += 120;
		y2 += 120;

		colour = colourupper | 0xff00007f;

		gdl = menugfxDrawPlane(gdl, x1, y1, x2, y2, colour, colour, MENUPLANE_09);
	}

	gdl = func0f0d4c80(gdl);

	return gdl;
}

struct coord *g_MenuParticles = NULL;

/**
 * Menu particles are stored in gunmem. Gunmem doesn't need to be freed because
 * the memory gets reused for other things, so all that needs to happen here is
 * clearing the pointer that the menugfx system maintains.
 */
void menugfxFreeParticles(void)
{
	g_MenuParticles = NULL;
}

uint32_t menugfxGetParticleArraySize(void)
{
	return align16(NUM_SUCCESS_PARTICLES * sizeof(struct coord));
}

/**
 * Render the "success" background, which is used on the solo mission completed
 * endscreen.
 *
 * The endscreen features two haze layers, and a bunch of particles (stars?)
 * flying towards the camera.
 *
 * In the Defense stage, everything is gray and the particles move slower.
 */
Gfx *menugfxRenderBgSuccess(Gfx *gdl)
{
	Mtx sp110;
	Mtx *modelmtx;
	Col *colours;
	Col *ptr;
	int i;
	int j;
	float f0;
	float speed = 5.0f;
	bool gray = false;

	if (g_StageIndex == STAGEINDEX_DEFENSE) {
		speed = 2.0f;
		gray = true;
	}

	// Initialise particles if they haven't been already
	if (g_MenuParticles == NULL) {
		g_MenuParticles = (struct coord *) bgunGetGunMem();

		if (g_MenuParticles == NULL) {
			return gdl;
		}

		for (i = 0; i < NUM_SUCCESS_PARTICLES; i++) {
			do {
				g_MenuParticles[i].x = RANDOMFRAC() * 10000.0f - 5000.0f;
				g_MenuParticles[i].y = RANDOMFRAC() * 10000.0f - 5000.0f;
			} while (g_MenuParticles[i].x < 640.0f && g_MenuParticles[i].x > -640.0f
					&& g_MenuParticles[i].y < 480.0f && g_MenuParticles[i].y > -480.0f);

			g_MenuParticles[i].z = -RANDOMFRAC() * 8000.0f;
		}
	}

	// Move the particles closer, and reset the ones which have gone behind the camera
	for (i = 0; i < NUM_SUCCESS_PARTICLES; i++) {
		int mult = (i % 5) + 1;

		g_MenuParticles[i].z += mult * g_Vars.diffframe240f * speed;

		if (g_MenuParticles[i].z > 0.0f) {
			do {
				g_MenuParticles[i].x = RANDOMFRAC() * 10000.0f - 5000.0f;
				g_MenuParticles[i].y = RANDOMFRAC() * 10000.0f - 5000.0f;
			} while (g_MenuParticles[i].x < 640.0f && g_MenuParticles[i].x > -640.0f
					&& g_MenuParticles[i].y < 480.0f && g_MenuParticles[i].y > -480.0f);

			g_MenuParticles[i].z = -8000.0f - RANDOMFRAC() * 500.0f;
		}
	}

	f0 = menuGetLinearIntervalFrac(10.0f);

	// Draw the two haze layers
	g_MenuProjectFromX = 0;
	g_MenuProjectFromY = 0;

	gdl = func0f0d4a3c(gdl);

	var8009de90 = -100000;
	var8009de94 = 100000;

	if (gray) {
		gdl = menugfxDrawPlane(gdl, -1000, -10, 2000, -10, 0x6060607f, 0x6060607f, MENUPLANE_05);
		gdl = menugfxDrawPlane(gdl, -1000, viGetHeight() + 10, 2000, viGetHeight() + 10, 0x9090907f, 0x9090907f, MENUPLANE_05);
	} else {
		gdl = menugfxDrawPlane(gdl, -1000, -10, 2000, -10, 0x0000947f, 0x0000947f, MENUPLANE_10);
		gdl = menugfxDrawPlane(gdl, -1000, viGetHeight() + 10, 2000, viGetHeight() + 10, 0x6200947f, 0x6200947f, MENUPLANE_06);
	}

	// Prepare stuff for drawing the particles
	gdl = func0f0d4c80(gdl);
	gdl = savebufferSetup2DRender(gdl);

	texSelect(&gdl, &g_TexGeneralConfigs[1], 2, 1, 2, true, NULL);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE,         // Color 0
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0,    // Alpha 0
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE,         // Color 1 (mirror)
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0);   // Alpha 1 (mirror)
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);

	mtxIdent(&sp110);

	modelmtx = gfxAllocateMatrix();

	mtx4Copy(&sp110, modelmtx);

	gfx_Matrix(gdl++, modelmtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	colours = gfxAllocateColours(20);

	if (gray) {
		for (j = 0, ptr = colours; j < 5;) {
			ptr[0].word = PD_BE32(0xffffff00 | ((5 - j) * 127u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			ptr[1].word = PD_BE32(0xaaaaaa00 | ((5 - j) * 16u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			ptr[2].word = PD_BE32(0xffffff00 | ((5 - j) * 127u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			ptr[3].word = PD_BE32(0xaaaaaa00 | ((5 - j) * 16u / 5));
			ptr += 4;
			j++;
		}
	} else {
		for (j = 0, ptr = colours; j < 5;) {
			ptr[0].word = PD_BE32(0xffffff00 | ((5 - j) * 127u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			ptr[1].word = PD_BE32(0xaaaaff00 | ((5 - j) * 16u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			ptr[2].word = PD_BE32(0xffffff00 | ((5 - j) * 127u / 5));
			ptr += 4;
			j++;
		}

		for (j = 0, ptr = colours; j < 5;) {
			if (colours);
			ptr[3].word = PD_BE32(0xffaaff00 | ((5 - j) * 16u / 5));
			ptr += 4;
			j++;
		}
	}

	{
		struct coord pos;

		gfx_Color(gdl++, colours, 20);

		// Draw the particles
		for (i = NUM_SUCCESS_PARTICLES - 1; i >= 0; i--) {
			int s3 = 0;
			float sine = sinf(f0 * M_TAU + M_TAU * (i / 15.0f));
			float cosine = cosf(f0 * M_TAU + M_TAU * (i / 15.0f));

			pos.x = g_MenuParticles[i].x;
			pos.y = g_MenuParticles[i].y;
			pos.z = g_MenuParticles[i].z;

			if (pos.z < -6600.0f) {
				s3 = -(pos.f[2] + 6600.0f) / 1400.0f * 5.0f;
			}

			if (pos.z);

			if (pos.z < 0.0f && s3 < 6) {
				float invsine2 = -sine;
				float invsine = -sine;
				float invcosine = -cosine;
				Vtx *vertices = gfxAllocateVertices(5);

				vertices[0].x = pos.f[0];
				vertices[0].y = pos.f[1];
				vertices[0].z = pos.f[2];

				vertices[1].x = pos.f[0] + 25.0f * (sine + cosine);
				vertices[1].y = pos.f[1] + 25.0f * (cosine + invsine);
				vertices[1].z = pos.f[2];

				vertices[2].x = pos.f[0] + 25.0f * (sine - cosine);
				vertices[2].y = pos.f[1] + 25.0f * (cosine - invsine);
				vertices[2].z = pos.f[2];

				vertices[3].x = pos.f[0] + 25.0f * (invsine - cosine);
				vertices[3].y = pos.f[1] + 25.0f * (invcosine - invsine);
				vertices[3].z = pos.f[2];

				vertices[4].x = pos.f[0] + 25.0f * (invsine2 + cosine);
				vertices[4].y = pos.f[1] + 25.0f * (invcosine + invsine);
				vertices[4].z = pos.f[2];

				vertices[0].colour = (s3 * 4 + (i % 2) * 2) * 4;
				vertices[1].colour = (s3 * 4 + (i % 2) * 2 + 1) * 4;
				vertices[2].colour = (s3 * 4 + (i % 2) * 2 + 1) * 4;
				vertices[3].colour = (s3 * 4 + (i % 2) * 2 + 1) * 4;
				vertices[4].colour = (s3 * 4 + (i % 2) * 2 + 1) * 4;

				gfx_Vertex(gdl++, vertices, 5, 0);

				gfx_Tri4(gdl++, 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1);
			}
		}
	}

	gdl = savebufferSetCustomProjection(gdl);

	return gdl;
}
