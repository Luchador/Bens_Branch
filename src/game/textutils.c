#include <ultra64.h>
#include "constants.h"
#include "game/menuutils.h"
#include "game/debug.h"
#include "game/gfxmemory.h"
#include "game/savebuffer.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/lang.h"
#include "fs.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SPACE_WIDTH 5

#define BLENDTYPE_DIAGONAL   0x01
#define BLENDTYPE_VERTICAL   0x02
#define BLENDTYPE_WAVE       0x04
#define BLENDTYPE_MENU       0x08
#define BLENDTYPE_HORIZONTAL 0x10

#define ASCII_START 33
#define ASCII_END 126
#define TOTAL_CHARS (ASCII_END - ASCII_START)

struct blendsettings {
	/*0x00*/ uint8_t types;
	/*0x04*/ uint32_t colour04;
	/*0x08*/ uint32_t colour08;
	/*0x0c*/ int diagrefx;
	/*0x10*/ int diagrefy;
	/*0x14*/ float diagtimer;
	/*0x18*/ uint8_t diagmode;
	/*0x1c*/ int backupdiagrefx;
	/*0x20*/ int backupdiagrefy;
	/*0x24*/ float backupdiagtimer;
	/*0x28*/ uint8_t backupdiagmode;
	/*0x29*/ uint8_t backupdiagtypes;
	/*0x2a*/ uint8_t backuptypes;
	/*0x2c*/ int vertrefy1;
	/*0x30*/ int vertrefy2;
	/*0x34*/ int vert34;
	/*0x38*/ int horizrefx1;
	/*0x3c*/ int horizrefx2;
	/*0x40*/ int horiz40;
	/*0x44*/ uint32_t colour44;
	/*0x48*/ uint32_t colour48;
	/*0x4c*/ int wave4c;
	/*0x50*/ int wave50;
	/*0x54*/ int wave54;
	/*0x58*/ uint32_t wavecolour1;
	/*0x5c*/ uint32_t wavecolour2;
	/*0x60*/ uint32_t menuweight;
};

struct blendsettings g_Blend;
Gfx *var800a4634;
uint32_t g_TextOutlineColor;
uint32_t g_TextHasOutline = 0;

bool g_TextRotated90 = false;
int g_WrapIndentCount = 0;

struct font *g_FontNumeric = NULL;
struct fontchar *g_CharsNumeric = NULL;
struct font *g_FontHandelGothicXs = NULL;
struct fontchar *g_CharsHandelGothicXs = NULL;
struct font *g_FontHandelGothicSm = NULL;
struct fontchar *g_CharsHandelGothicSm = NULL;
struct font *g_FontHandelGothicMd = NULL;
struct fontchar *g_CharsHandelGothicMd = NULL;
struct font *g_FontHandelGothicLg = NULL;
struct fontchar *g_CharsHandelGothicLg = NULL;

struct fontchar *g_CharToRender;

struct fontchar g_HandelGothicData[93]; // HD Handel Gothic

uint16_t var8007fb3c[] = {
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
	0xff00, 0xff24, 0xff48, 0xff6c, 0xff90, 0xffb4, 0xffd8, 0xffff,
};

uint16_t var8007fb5c[] = {
	0xff00, 0xff58, 0xff74, 0xff90, 0xffac, 0xffc8, 0xffe4, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
	0xff00, 0xff18, 0xff30, 0xff5c, 0xff88, 0xffb4, 0xffd8, 0xffff,
};

bool g_DoRedrawEffect = false;
int var8007fba0 = 0;
int var8007fba4 = -1;
uint32_t var8007fbac = 0x00000001;
uint32_t var8007fbb0 = 0x00000064;
uint32_t var8007fbb4 = 0x0000002c;
uint32_t var8007fbb8 = 0x00000080;

int g_HudCenter = HUDCENTER_NONE;
uint32_t g_HudAlignModeL = G_ASPECT_LEFT_EXT;
uint32_t g_HudAlignModeR = G_ASPECT_RIGHT_EXT;

void textSetRotation90(bool rotated)
{
	g_TextRotated90 = rotated;
}

void textSetWrapIndent(int count)
{
	g_WrapIndentCount = count;
}

void textLoadFont(uint8_t *romstart, uint8_t *romend, struct font **fontptr, struct fontchar **charsptr, bool monospace)
{
	extern uint8_t EXT_SEG _fonthandelgothicsmSegmentRomStart;
	extern uint8_t EXT_SEG _fonthandelgothicxsSegmentRomStart;
	extern uint8_t EXT_SEG _fonthandelgothicmdSegmentRomStart;

	uint32_t len;
	int maxwidth;
	int i;
	struct font *font;
	struct fontchar *chars;

#define NUMCHARS() 94

	len = (romptr_t)romend - (romptr_t)romstart;
	font = mempAlloc(len, MEMPOOL_STAGE);
	chars = font->chars;

	dmaExec(font, (romptr_t) romstart, len);

	// Convert pointers
	for (i = 0; i < NUMCHARS(); i++) {
		chars[i].pixeldata += (uintptr_t)font;
	}

	if (monospace) {
		maxwidth = 0;

		for (i = 0; i < NUMCHARS(); i++) {
			if (chars[i].width > maxwidth) {
				maxwidth = chars[i].width;
			}
		}

		maxwidth--;

		for (i = 0; i < NUMCHARS(); i++) {
			chars[i].width = maxwidth;
		}
	}

	*fontptr = font;
	*charsptr = chars;
}

void textReset(void)
{
	extern uint8_t EXT_SEG _fontbankgothicSegmentRomStart,     EXT_SEG _fontbankgothicSegmentRomEnd;
	extern uint8_t EXT_SEG _fontzurichSegmentRomStart,         EXT_SEG _fontzurichSegmentRomEnd;
	extern uint8_t EXT_SEG _fontnumericSegmentRomStart,        EXT_SEG _fontnumericSegmentRomEnd;
	extern uint8_t EXT_SEG _fonthandelgothicsmSegmentRomStart, EXT_SEG _fonthandelgothicsmSegmentRomEnd;
	extern uint8_t EXT_SEG _fonthandelgothicxsSegmentRomStart, EXT_SEG _fonthandelgothicxsSegmentRomEnd;
	extern uint8_t EXT_SEG _fonthandelgothicmdSegmentRomStart, EXT_SEG _fonthandelgothicmdSegmentRomEnd;
	extern uint8_t EXT_SEG _fonthandelgothiclgSegmentRomStart, EXT_SEG _fonthandelgothiclgSegmentRomEnd;
	extern uint8_t EXT_SEG _fontocramdSegmentRomStart,         EXT_SEG _fontocramdSegmentRomEnd;
	extern uint8_t EXT_SEG _fontocralgSegmentRomStart,         EXT_SEG _fontocralgSegmentRomEnd;

	g_FontNumeric = NULL;
	g_FontHandelGothicXs = NULL;
	g_FontHandelGothicSm = NULL;
	g_FontHandelGothicMd = NULL;
	g_FontHandelGothicLg = NULL;

	g_CharsNumeric = NULL;
	g_CharsHandelGothicXs = NULL;
	g_CharsHandelGothicSm = NULL;
	g_CharsHandelGothicMd = NULL;
	g_CharsHandelGothicLg = NULL;

	g_TextRotated90 = false;
	g_WrapIndentCount = 0;

	textLoadFont(REF_SEG _fontnumericSegmentRomStart, REF_SEG _fontnumericSegmentRomEnd, &g_FontNumeric, &g_CharsNumeric, false);
	textLoadFont(REF_SEG _fonthandelgothicxsSegmentRomStart, REF_SEG _fonthandelgothicxsSegmentRomEnd, &g_FontHandelGothicXs, &g_CharsHandelGothicXs, false);
	textLoadFont(REF_SEG _fonthandelgothicsmSegmentRomStart, REF_SEG _fonthandelgothicsmSegmentRomEnd, &g_FontHandelGothicSm, &g_CharsHandelGothicSm, false);
	textLoadFont(REF_SEG _fonthandelgothicmdSegmentRomStart, REF_SEG _fonthandelgothicmdSegmentRomEnd, &g_FontHandelGothicMd, &g_CharsHandelGothicMd, false);
	textLoadFont(REF_SEG _fonthandelgothiclgSegmentRomStart, REF_SEG _fonthandelgothiclgSegmentRomEnd, &g_FontHandelGothicLg, &g_CharsHandelGothicLg, false);

	// Fonts are loaded again every time a stage is loaded so free the memory allocated for the previous fonts
	textFreeFontCharacters();

	textLoadCustomFont(); // Load custom HD fonts into memory
}

Gfx *textConfigureGfxPipeline(Gfx *gdl)
{
	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
	gDPSetTexturePersp(gdl++, G_TP_NONE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetTextureLOD(gdl++, G_TL_TILE);
	gDPSetTextureConvert(gdl++, G_TC_FILT);
	gDPSetTextureLUT(gdl++, G_TT_NONE);
	gDPSetTextureFilter(gdl++, TEX_FILTER_2D);

	return gdl;
}

Gfx *textSetPrimColour(Gfx *gdl, uint32_t colour)
{
	gDPPipeSync(gdl++);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);

	return gdl;
}

Gfx *textSetCCCustom02(Gfx *gdl)
{
	gDPSetCombineMode(gdl++, G_CC_CUSTOM_02, G_CC_CUSTOM_02);

	return gdl;
}

Gfx *text0f153858(Gfx *gdl, int *x1, int *y1, int *x2, int *y2)
{
	/*gdl = textSetPrimColour(gdl, 0x00000000);

	gDPFillRectangle(gdl++, *x1, *y1, *x2, *y2);

	gdl = textSetCCCustom02(gdl);*/

	return gdl;
}

Gfx *textDrawBlackRectScaled(Gfx *gdl, int *x1, int *y1, int *x2, int *y2)
{
	gdl = textSetPrimColour(gdl, 0x00000000);

	gDPFillRectangleScaled(gdl++, *x1, *y1, *x2, *y2);

	gdl = textSetCCCustom02(gdl);

	return gdl;
}

Gfx *textDrawBlackRectBordered(Gfx *gdl, int left, int top, int width, int height)
{
	gdl = textSetPrimColour(gdl, 0x00000000);

	gDPFillRectangle(gdl++, left - 1, top - 1, width + left + 1, top + height + 1);

	gdl = textSetCCCustom02(gdl);

	return gdl;
}

Gfx *textDrawColoredRect(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour)
{
	gdl = textSetPrimColour(gdl, colour);

	gDPFillRectangle(gdl++, x1, y1, x2, y2);

	gdl = textSetCCCustom02(gdl);

	return gdl;
}

Gfx *text0f153ab0(Gfx *gdl)
{
	Gfx *allocation;

	g_DoRedrawEffect = true;

	allocation = gfxAllocate(sizeof(Gfx) * 530);

	var800a4634 = allocation;


	gSPDisplayList(gdl++, var800a4634);

	gdl = func0f0d4c80(gdl);

	var800a4634 = func0f0d4a3c(var800a4634, 0);
	var8007fba4 = -1;

	return gdl;
}

void textStopRedrawEffect(void)
{
	g_DoRedrawEffect = false;

	gSPEndDisplayList(var800a4634++);
}

void text0f153b6c(int arg0)
{
	if (arg0 != var8007fba4) {
		float tmp = g_Blend.diagtimer * g_Blend.diagtimer - (float)((arg0 - g_Blend.diagrefy) * (arg0 - g_Blend.diagrefy));

		if (tmp > 0.0f) {
			var8007fba0 = sqrtf(tmp) + g_Blend.diagrefx;
		} else {
			var8007fba0 = 0;
		}

		var8007fba4 = arg0;
	}
}

void textSetDiagonalBlend(int x, int y, float timer, uint8_t mode)
{
	g_Blend.types |= BLENDTYPE_DIAGONAL;
	g_Blend.diagrefx = x;
	g_Blend.diagrefy = y;
	g_Blend.diagtimer = timer;
	g_Blend.diagmode = mode;
}

void textBackupDiagonalBlendSettings(void)
{
	g_Blend.backupdiagrefx = g_Blend.diagrefx;
	g_Blend.backupdiagrefy = g_Blend.diagrefy;
	g_Blend.backupdiagtimer = g_Blend.diagtimer;
	g_Blend.backupdiagmode = g_Blend.diagmode;
	g_Blend.backupdiagtypes = g_Blend.types & BLENDTYPE_DIAGONAL;
}

void textRestoreDiagonalBlendSettings(void)
{
	g_Blend.diagrefx = g_Blend.backupdiagrefx;
	g_Blend.diagrefy = g_Blend.backupdiagrefy;
	g_Blend.diagtimer = g_Blend.backupdiagtimer;
	g_Blend.diagmode = g_Blend.backupdiagmode;
	g_Blend.types |= g_Blend.backupdiagtypes;
}

void textSetVerticalBlend(int y1, int y2, uint32_t arg2)
{
	g_Blend.types |= BLENDTYPE_VERTICAL;
	g_Blend.vertrefy1 = y1;
	g_Blend.vertrefy2 = y2;
	g_Blend.vert34 = arg2;
}

void textSetHorizontalBlend(int x1, int x2, uint32_t arg2)
{
	g_Blend.types |= BLENDTYPE_HORIZONTAL;
	g_Blend.horizrefx1 = x1;
	g_Blend.horizrefx2 = x2;
	g_Blend.horiz40 = arg2;
}

void textResetBlends2(void)
{
	g_Blend.types = 0;
}

void textResetBlends3(void)
{
	g_Blend.types = 0;
}

void textBackupAndResetBlends(void)
{
	g_Blend.backuptypes = g_Blend.types;
	g_Blend.types = 0;
}

void textRestoreBlends(void)
{
	g_Blend.types = g_Blend.backuptypes;
}

void textSetWaveBlend(int arg0, int arg1, int cthresh)
{
	g_Blend.types |= BLENDTYPE_WAVE;
	g_Blend.wave4c = arg0;
	g_Blend.wave50 = arg1;
	g_Blend.wave54 = cthresh;
	g_Blend.wavecolour1 = 0x44444400;
	g_Blend.wavecolour2 = 0xffffff00;
}

void textSetMenuBlend(float arg0)
{
	g_Blend.types |= BLENDTYPE_MENU;
	g_Blend.menuweight = arg0 * arg0 * 110.0f;
}

void textSetWaveColours(uint32_t colour1, uint32_t colour2)
{
	g_Blend.wavecolour1 = colour1;
	g_Blend.wavecolour2 = colour2;
}

void textResetBlends(void)
{
	g_Blend.types = 0;
}

bool textHasDiagonalBlend(void)
{
	return (g_Blend.types & BLENDTYPE_DIAGONAL)
		&& (g_Blend.diagmode == DIAGMODE_FADEIN || g_Blend.diagmode == DIAGMODE_FADEOUT);
}

uint32_t textApplyProjectionColour(int x, int y, uint32_t colour)
{
	uint32_t result = colour;

	if (g_Blend.types & BLENDTYPE_DIAGONAL) {
		float weightf;
		float f12;
		float f14;
		float f16;
		float f18;

		if (x - g_Blend.diagrefx > -3000 && x - g_Blend.diagrefx < 3000
				&& y - g_Blend.diagrefy > -3000 && y - g_Blend.diagrefy < 3000) {
			f12 = sqrtf((x - g_Blend.diagrefx) * (x - g_Blend.diagrefx) + (y - g_Blend.diagrefy) * (y - g_Blend.diagrefy));
		} else {
			f12 = 3000.0f;
		}

		f14 = var8007fbac;
		f18 = var8007fbb0;
		f16 = var8007fbb4;

		if (g_Blend.diagmode == 0) {
			if (g_Blend.diagtimer < f12) {
				result = 0;
			} else if (g_Blend.diagtimer - f14 < f12) {
				uint32_t intensity;
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				intensity = 255 - (uint32_t) weightf;
				result = intensity << 8 | intensity | intensity << 16 | intensity << 24;
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				result = (((colour & 0xff) + 0xff) >> 1) | (colour & 0xffffff00);
			} else if ((g_Blend.diagtimer - (f14 + f18 + f16)) < f12) {
				uint32_t colour2 = (((colour & 0xff) + 0xff) / 2) | (colour & 0xffffff00);
				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				result = colourBlend(colour, colour2, 0xff - (uint32_t) weightf);
			}
		} else if (g_Blend.diagmode == 2) {
			f16 = 0.0f;

			if (g_Blend.diagtimer < f12) {
				result = 0x00000000;
			} else if (g_Blend.diagtimer - f14 < f12) {
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				result = colourBlend(0x00000000, colour & 0xff, weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				result = colour & 0xff;
			} else if ((g_Blend.diagtimer - (f14 + f18 + f16)) < f12) {
				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				result = colourBlend(colour & 0xff, colour, weightf);
			}
		}
	}

	return result;
}

uint32_t textHighlightSweep(int x, int y, uint32_t colourarg)
{
	float f14;
	float f18;
	float f16;
	uint32_t colour = colourarg;

	if (g_Blend.types & BLENDTYPE_MENU) {
		colour = (colourBlend(0x00000000, colour, g_Blend.menuweight) & 0xffffff00) | (colour & 0xff);
	}

	if (g_Blend.types & BLENDTYPE_VERTICAL) {
		int v0 = y - g_Blend.vertrefy1;
		int v1 = y - g_Blend.vertrefy2;

		if (v0 < 0) {
			v0 = -v0;
		}

		if (v1 < 0) {
			v1 = -v1;
		}

		if (v1 < v0) {
			v0 = v1;
		}

		if (g_Blend.vert34 >= v0) {
			colour = colourBlend(colour, 0x00000000, v0 * 255 / g_Blend.vert34);
		}
	}

	if (g_Blend.types & BLENDTYPE_HORIZONTAL) {
		int v0 = x - g_Blend.horizrefx1;
		int v1 = x - g_Blend.horizrefx2;

		if (v0 < 0) {
			v0 = 0;
		}

		if (v1 < 0) {
			v1 = -v1;
		}

		if (v1 < v0) {
			v0 = v1;
		}

		if (g_Blend.horiz40 >= v0) {
			colour = colourBlend(colour, 0x00000000, v0 * 255 / g_Blend.horiz40);
		}
	}

	if (g_Blend.types & BLENDTYPE_DIAGONAL) {
		float f12;
		uint32_t stack[3];
		float weightf;

		if (x - g_Blend.diagrefx > -3000 && x - g_Blend.diagrefx < 3000
				&& y - g_Blend.diagrefy > -3000 && y - g_Blend.diagrefy < 3000) {
			f12 = sqrtf((x - g_Blend.diagrefx) * (x - g_Blend.diagrefx) + (y - g_Blend.diagrefy) * (y - g_Blend.diagrefy));
		} else {
			f12 = 3000.0f;
		}

		f14 = var8007fbac;
		f18 = var8007fbb0;
		f16 = var8007fbb4;

		if (g_Blend.diagmode == 0) {
			if (g_Blend.diagtimer < f12) {
				colour = 0;
			} else if (g_Blend.diagtimer - f14 < f12) {
				uint32_t intensity;
				weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				intensity = 255 - (uint32_t) weightf;
				colour = intensity << 8 | intensity | intensity << 16 | intensity << 24;
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				colour = 0xffffffff;
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				uint32_t add;
				uint32_t mult;

				weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				add = (uint32_t) weightf * 255;
				mult = 255 - (uint32_t) weightf;

				colour = ((((colour >> 24) & 0xff) * mult + add) >> 8) << 24
					| ((((colour >> 16) & 0xff) * mult + add) >> 8) << 16
					| ((((colour >> 8) & 0xff) * mult + add) >> 8) << 8
					| ((colour & 0xff) * mult + add) >> 8;
			}
		} else if (g_Blend.diagmode == 2) {
			f14 = 0.00f;
			f18 = 66.0f;
			f16 = 0.0f;

			if (g_Blend.diagtimer < f12) {
				colour = 0x00000000;
			} else if (g_Blend.diagtimer - f14 < f12) {
				float weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				colour = colourBlend(0x00000000, colour & 0xff, weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				colour &= 0xff;
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				float weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				colour = colourBlend(0x00000000, colour, weightf);
			}
		} else {
			uint32_t alpha[4];

			static int burncol = 0xffffff00;

			alpha[0] = colour & 0xff;
			f18 = 50.0f;
			f16 = 22.0f;

			if (g_Blend.diagtimer < f12) {
				colour = colourBlend(alpha[0], colour, 110);
			} else if (g_Blend.diagtimer - f14 < f12) {
				float weightf = (f12 - (g_Blend.diagtimer - f14)) / f14 * 255.0f;
				colour = colourBlend(
						colourBlend(burncol | (colour & 0xff), colour, 0xc0),
						colourBlend(alpha[0], colour, 110),
						255 - (uint32_t) weightf);
			} else if (g_Blend.diagtimer - (f14 + f16) < f12) {
				uint32_t stack;
				colour = colourBlend(burncol | (colour & 0xff), colour, 0xc0);
			} else if (g_Blend.diagtimer - (f14 + f18 + f16) < f12) {
				float weightf = (f12 - (g_Blend.diagtimer - (f14 + f18 + f16))) / f18 * 255.0f;
				colour = colourBlend(
						colour,
						colourBlend(burncol | (colour & 0xff), colour, 0xc0),
						255 - (uint32_t) weightf);
			}
		}
	}

	if (g_Blend.types & BLENDTYPE_WAVE) {
		uint32_t stack[2];
		float f0 = (int)(g_Blend.wave4c - x + g_Blend.wave50 - y + 800);
		f0 = 4.0f * f0 / g_Blend.wave54;
		f0 -= (int) (f0 * 0.25f) * 4.0f;
		f0 -= 1.0f;

		if (f0 > 1.0f) { \
			f0 = 2.0f - f0;
		}

		if (f0 < 0.0f) {
			int weight = 60 * (0 - f0);
			colour = colourBlend(g_Blend.wavecolour1 | (colour & 0xff), colour, weight);
		} else {
			int weight = var8007fbb8 * f0;
			colour = colourBlend(g_Blend.wavecolour2 | (colour & 0xff), colour, weight);
		}
	}

	return colour;
}

Gfx *text0f154ecc(Gfx *gdl, uint32_t arg1, uint32_t arg2)
{
	uint32_t colour = textHighlightSweep(arg1, arg2, g_Blend.colour04);

	if (colour != g_Blend.colour44) {
		gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	}

	g_Blend.colour44 = colour;

	return gdl;
}

Gfx *textMakeCreditVerts(Gfx *gdl, int *arg1, struct fontchar *curchar, struct fontchar *prevchar,
		struct font *font, float widthscale, float heightscale, float x, float y)
{
	int tmp1;
	int tmp2;
	int16_t sp3e;
	int16_t sp3c;
	int16_t sp3a;
	int16_t sp38;
	int16_t sp36;
	int16_t sp34;
	int16_t sp32;
	int16_t sp30;
	Vtx *vertices;
	Col *colours;

	tmp2 = font->kerning[prevchar->kerningindex * 13 + curchar->kerningindex];
	*arg1 = *arg1 - tmp2 + 1;

	gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, curchar->pixeldata);
	gDPLoadSync(gdl++);
	gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((curchar->height * 8 + 17) >> 1) - 1, 2048);
	gDPPipeSync(gdl++);

	tmp1 = 0;
	tmp2 = 0;

	sp3e = (*arg1 * 4) * widthscale * 10.0f + 40.0f * x;
	sp3c = (curchar->baseline * 4) * heightscale * 10.0f + 40.0f * y;
	sp3a = ((*arg1 + (curchar->width + 1)) * 4) * widthscale * 10.0f + 40.0f * x;
	sp38 = ((curchar->baseline + (curchar->height + 1)) * 4) * heightscale * 10.0f + 40.0f * y;

	sp36 = 1;
	sp32 = 1;
	sp34 = curchar->width;
	sp30 = curchar->height;
	sp34 = sp36 + ((sp34 + 1) << 6);
	sp30 = sp32 + ((sp30 + 1) << 6);

	vertices = gfxAllocateVertices(4);

	colours = gfxAllocateColours(1);
	colours[0].word = 0xff0000ff;

	vertices[0].z = -10;
	vertices[1].z = -10;
	vertices[2].z = -10;
	vertices[3].z = -10;

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 0;
	vertices[3].colour = 0;

	vertices[0].x = sp3e / 4;
	vertices[0].y = sp3c / 4;
	vertices[0].s = sp36;
	vertices[0].t = sp32;
	vertices[1].x = sp3a / 4;
	vertices[1].y = sp3c / 4;
	vertices[1].s = sp34;
	vertices[1].t = sp32;
	vertices[2].x = sp3a / 4;
	vertices[2].y = sp38 / 4;
	vertices[2].s = sp34;
	vertices[2].t = sp30;
	vertices[3].x = sp3e / 4;
	vertices[3].y = sp38 / 4;
	vertices[3].s = sp36;
	vertices[3].t = sp30;

	gSPColor(gdl++, colours, 1);
	gSPVertex(gdl++, vertices, 4, 0);

	gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);

	*arg1 += curchar->width;

	return gdl;
}

Gfx *textRenderCredit(Gfx *gdl, float x, float y, float widthscale, float heightscale,
		char *text, struct fontchar *chars, struct font *font, uint32_t colour, int hdir, int vdir)
{
	int totalheight;
	uint8_t prevchar;
	int textwidth;
	int textheight;
	int lineheight;
	int relx;
	float *ptr;
	float fx;
	float fy;

	totalheight = 0;
	prevchar = 'H';
	relx = 0;
	lineheight = chars['['].height + chars['['].baseline;

	textMeasure(&textheight, &textwidth, text, chars, font, 0);

	ptr = &x;
	fx = *ptr - (widthscale - 1.0f) * textwidth * 0.5f * hdir;
	fy = y - (heightscale - 1.0f) * lineheight * 0.5f * vdir;

	if (fx);
	if (fy);

	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (uintptr_t)(var8007fb3c));

	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 15);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPPipeSync(gdl++);

	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				prevchar = 'H';
				text += 1;
				relx = relx + 5;
			} else if (*text == '\n') {
				prevchar = 'H';
				text += 1;
				totalheight += lineheight;
				relx = 0;
			} else if (*text < 0x80) {
				gdl = textMakeCreditVerts(gdl, &relx, &chars[*text - 0x21], &chars[prevchar - 0x21], font,
						widthscale, heightscale, fx, fy);
				prevchar = *text;
				text += 1;
			} else {

			}
		}
	}


	return gdl;
}

// Render the text in menus. Doesn't do the highlight effect for the focused menu option.
Gfx *textRenderUnhighlighted(Gfx *gdl, int *x, int *y, struct fontchar *curchar, struct fontchar *prevchar,
		struct font *font, int savedx, int savedy, int width, int height, int arg10)
{
	int tmp;
	int sp90;

	sp90 = *y + arg10;
	tmp = font->kerning[prevchar->kerningindex * 13 + curchar->kerningindex];
	*x -= (tmp - 1);

	if (g_TextRotated90 || (*x > 0 && *x <= viGetWidth() && sp90 + curchar->baseline <= viGetHeight())) {
		if (savedx + width >= *x
				&& savedy + height >= curchar->baseline + sp90
				&& *x >= savedx
				&& curchar->baseline + sp90 + curchar->height >= savedy) {
			gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, curchar->pixeldata);
			g_CharToRender = curchar;
			//gDPSetTextureImage(gdl++, G_IM_FMT_CUSTOMFONT, G_IM_SIZ_16b, 1, &g_HandelGothicData[g_CharToRender->index].pixeldata);
			gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
			gDPLoadSync(gdl++);
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((curchar->height * 8 + 17) >> 1) - 1, 2048);
			gDPPipeSync(gdl++);

			if (g_Blend.types) {
				gdl = text0f154ecc(gdl, *x, *y + arg10);
			}

			if (*x + 1 * curchar->width <= savedx + width) {
				if (savedy <= curchar->baseline + sp90) {
					if (curchar->baseline + sp90 + curchar->height <= savedy + height) {
						if (g_TextRotated90) {
							gSPTextureRectangleFlip(gdl++,
									(sp90 - curchar->baseline - curchar->height) * 4,
									*x * 4,
									(sp90 - curchar->baseline) * 4,
									(*x + curchar->width) * 4,
									G_TX_RENDERTILE,
									32,
									((curchar->height) << 5) + 32,
									1024,
									65536 - 1024);
						} else {
							gSPTextureRectangle(gdl++,
									*x * 4,
									(sp90 + curchar->baseline) * 4,
									(*x + curchar->width) * 4,
									(sp90 + curchar->baseline + curchar->height) * 4,
									G_TX_RENDERTILE,
									32,
									32,
									1024,
									1024);

							// Does the redraw effect for menus
							if (g_DoRedrawEffect) {
								text0f153b6c(*y + arg10);

								if (var8007fba0 >= *x && *x + curchar->width >= var8007fba0) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline + sp90,
											var8007fba0,
											curchar->baseline + sp90 + curchar->height,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}

								if (var8007fba0 - 3 >= *x && *x + curchar->width >= var8007fba0 - 3) {
									var800a4634 = menugfxDrawPlane(var800a4634,
											var8007fba0,
											curchar->baseline + sp90,
											var8007fba0,
											curchar->baseline + sp90 + curchar->height,
											g_Blend.colour04,
											g_Blend.colour04,
											MENUPLANE_00);
								}
							}
						}
					} else if (savedy + height >= curchar->baseline + sp90) {
						gSPTextureRectangle(gdl++,
								*x * 4,
								(sp90 + curchar->baseline) * 4,
								(*x + curchar->width) * 4,
								(savedy + height) * 4,
								G_TX_RENDERTILE,
								32,
								32,
								1024,
								1024);
					}
				} else {
					if (curchar->baseline + sp90 + curchar->height >= savedy) {
						gSPTextureRectangle(gdl++,
								*x * 4,
								savedy * 4,
								(*x + curchar->width) * 4,
								(curchar->baseline + sp90 + curchar->height) * 4,
								G_TX_RENDERTILE,
								32,
								((savedy - sp90 - curchar->baseline) << 5) + 32,
								1024,
								1024);
					}
				}
			}
		}
	}

	*x += curchar->width;

	return gdl;
}

void textSetHasOutline(int arg0)
{
	g_TextHasOutline = arg0;
}

void textSetOutlineColor(uint32_t colour)
{
	g_TextOutlineColor = colour;
}

Gfx *textRenderProjected(Gfx *gdl, int *x, int *y, char *text, struct fontchar *chars, struct font *font,
		int colour, int width, int height, int arg9, int lineheight)
{
	int savedx;
	int savedy;
	uint8_t prevchar;
	int spb0;
	uint32_t colour2;
	uint32_t tmpcolour;
	int newx;
	int newy;
	float alpha;

	static uint32_t sbrd = 0x00000000;

	spb0 = 1;

	if (g_TextRotated90) {
		spb0 = 1;
	}

	if (g_TextHasOutline) {
		alpha = (1.0f - menuGetSinOscFrac(40.0f)) * 100.0f + 150.0f;
		newx = *x;
		newy = *y;
		tmpcolour = g_TextOutlineColor;
		colour2 = (colour & 0xffffff00) | (uint32_t) alpha;

		if (sbrd) {
			tmpcolour = sbrd;
		}

		gdl = textRender(gdl, &newx, &newy, text, chars, font, colour2, tmpcolour, width, height, arg9, lineheight);
	}

	savedx = *x;
	savedy = *y;
	prevchar = 'H';

	if (lineheight == 0) {
		lineheight = chars['['].height + chars['['].baseline;
	}
 
	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (uintptr_t)(var8007fb3c));
	//gDPSetTextureImage(gdl++, G_IM_FMT_CUSTOMFONT, G_IM_SIZ_16b, 1, &g_HandelGothicData[50].pixeldata);
	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 15);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	//gDPSetTile(gdl++, G_IM_FMT_CUSTOMFONT, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 128, 128);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPPipeSync(gdl++);

	g_Blend.colour04 = colour;
	g_Blend.colour44 = colour;

	if (text != NULL) {
		while (*text != '\0') {
			if (*text == ' ') {
				prevchar = 'H';
				*x += spb0 * 5;
				text++;
			} else if (*text == '\n') {
				prevchar = 'H';
				text++;
				*y += lineheight;
				*x = savedx;
			} else if (*text < 0x80) {
				gdl = textRenderUnhighlighted(gdl, x, y, &chars[*text - 0x21], &chars[prevchar - 0x21], font, savedx, savedy, width, height, arg9);
				prevchar = *text;
				text++;
			} else {

			}
		}
	}

	return gdl;
}

Gfx *text0f1566cc(Gfx *gdl, uint32_t arg1, uint32_t arg2)
{
	uint32_t colour = textHighlightSweep(arg1, arg2, g_Blend.colour04);

	if (colour != g_Blend.colour44) {
		gDPSetColor(gdl++, G_SETENVCOLOR, colour);
	}

	g_Blend.colour44 = colour;

	colour = (g_Blend.colour08 & 0xffffff00) | (textHighlightSweep(arg1, arg2, g_Blend.colour08) & 0xff);

	if (colour != g_Blend.colour48) {
		gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	}

	g_Blend.colour48 = colour;

	return gdl;
}

Gfx *textRenderChar(Gfx *gdl, int *x, int *y, struct fontchar *char1, struct fontchar *char2,
		struct font *font, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	int tmp;
	int sp38;

	sp38 = *y + arg10;

	tmp = font->kerning[char2->kerningindex * 13 + char1->kerningindex];
	*x -= (tmp - 1);

	if (*x > 0
			&& *x <= viGetWidth()
			&& sp38 + char1->baseline <= viGetHeight()
			&& *x <= arg6 + arg8
			&& char1->baseline + sp38 <= arg7 + arg9
			&& *x >= arg6
			&& sp38 + char1->baseline + char1->height >= arg7) {
		if (g_Blend.types) {
			gdl = text0f1566cc(gdl, *x, *y + arg10);
		}

		gDPSetTextureImage(gdl++, G_IM_FMT_CI, G_IM_SIZ_16b, 1, char1->pixeldata);
		gDPLoadSync(gdl++);
		gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, ((char1->height * 8 + 17) >> 1) - 1, 2048);
		gDPPipeSync(gdl++);

		gdl = textRenderOutline(gdl, *x - 1, sp38 - 1, char1, arg6, arg7 - 1, arg8, arg9);
	}

	*x += char1->width;

	return gdl;
}

Gfx *textRenderOutline(Gfx *gdl, int x, int y, struct fontchar *char1, int arg4, int arg5, int arg6, int arg7)
{
	if (arg4 + arg6 >= char1->width + x + 2) {
		if (y + char1->baseline >= arg5) {
			if (arg5 + arg7 >= y + char1->baseline + char1->height + 2) {
				if (g_TextRotated90) {
					gSPTextureRectangleFlip(gdl++,
							((y - char1->baseline) - ((char1->height + 2))) * 4,
							x * 4,
							(y - char1->baseline) * 4,
							(x + char1->width + 2) * 4,
							G_TX_RENDERTILE,
							0,
							(char1->height + 1) << 5,
							1024,
							-1024);
				} else {
					gSPTextureRectangle(gdl++,
							x * 4,
							(y + char1->baseline) * 4,
							(x + char1->width + 2) * 4,
							(y + char1->baseline + char1->height + 2) * 4,
							G_TX_RENDERTILE,
							0,
							0,
							1024,
							1024);
				}
			} else {
				if (arg5 + arg7 >= y + char1->baseline) {
					gSPTextureRectangle(gdl++,
							x * 4,
							(y + char1->baseline) * 4,
							(x + char1->width + 2) * 4,
							(arg5 + arg7) * 4,
							G_TX_RENDERTILE,
							0,
							0,
							1024,
							1024);
				}
			}
		} else {
			if (y + char1->baseline + char1->height + 2 >= arg5) {
				gSPTextureRectangle(gdl++,
						x * 4,
						arg5 * 4,
						(x + char1->width + 2) * 4,
						(y + char1->baseline + char1->height + 2) * 4,
						G_TX_RENDERTILE,
						0,
						(arg5 - char1->baseline - y) << 5,
						1024,
						1024);
			}
		}
	}

	return gdl;
}

Gfx *textRender(Gfx *gdl, int *x, int *y, char *text,
		struct fontchar *chars, struct font *font, uint32_t arg6, uint32_t colour,
		int width, int height, uint32_t arg10, int lineheight)
{
	int savedx;
	int savedy;
	int prevchar;

	savedx = *x;
	savedy = *y;
	prevchar = 'H';

	if (lineheight == 0) {
		lineheight = chars['['].height + chars['['].baseline;
	}

	gDPPipeSync(gdl++);
	gDPSetTextureLUT(gdl++, G_TT_IA16);
	gDPSetTextureImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, (uintptr_t)(&var8007fb5c));
	gDPLoadSync(gdl++);
	gDPLoadTLUTCmd(gdl++, 6, 31);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, G_TX_RENDERTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
	gDPSetTileSize(gdl++, G_TX_RENDERTILE, 0, 0, 0x007c, 0x007c);
	gDPSetTile(gdl++, G_IM_FMT_CI, G_IM_SIZ_4b, 1, 0x0000, 1, 1, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);

	gDPSetTileSize(gdl++, 1, 0, 0, 0x007c, 0x007c);
	gDPSetCycleType(gdl++, G_CYC_2CYCLE);
	gDPSetCombineLERP(gdl++,
			ENVIRONMENT, PRIMITIVE, TEXEL1_ALPHA, PRIMITIVE, 0, 0, 0, TEXEL0,
			0, 0, 0, COMBINED, COMBINED, 0, ENVIRONMENT, 0);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);
	gDPSetEnvColorViaWord(gdl++, arg6);
	gDPPipeSync(gdl++);

	g_Blend.colour08 = colour;
	g_Blend.colour48 = colour;
	g_Blend.colour04 = arg6;
	g_Blend.colour44 = arg6;

	while (*text != '\0') {
		if (*text == ' ') {
			*x += 5;
			prevchar = 'H';
			text++;
		} else if (*text == '\n') {
			*x = savedx;
			*y += lineheight;
			prevchar = 'H';
			text++;
		} else if (*text < 0x80) {
			gdl = textRenderChar(gdl, x, y, &chars[*text - 0x21], &chars[prevchar - 0x21],
					font, savedx, savedy, width, height, arg10);
			prevchar = *text;
			text++;
		} else {

		}
	}

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetCombineLERP(gdl++,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
			0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

	return gdl;
}

// Mismatch: Regalloc
void textMeasure(int *textheight, int *textwidth, char *text, struct fontchar *font1, struct font *font2, int lineheight)
{
	char prevchar;
	char thischar;
	int longest;
	int tmp;

	prevchar = 'H';
	thischar = '\0';
	longest = 0;
	*textheight = 0;
	*textwidth = 0;
	if (lineheight == 0) {
		lineheight = font1['['].baseline + font1['['].height;
	}

	if (text) {
		while (*text != '\0') {
			if (*text == ' ') {
				// Space
				if (text[1] != '\n') {
					*textwidth += 5;
				}

				prevchar = 'H';
				text++;
			} else if (*text == '\n') {
				// Line break

				if (*textwidth > longest) {
					longest = *textwidth;
				}

				*textwidth = 0;
				*textheight += lineheight;
				text++;
			} else {
				if (*text < 0x80) {
					// Normal single-byte character
					thischar = *text;
					tmp = font2->kerning[font1[prevchar - 0x21].kerningindex * 13 + font1[thischar - 0x21].kerningindex] - 1;
					*textwidth = font1[thischar - 0x21].width + *textwidth - tmp;

					prevchar = *text;
					text++;
				} else if (*text < 0xc0) {
					// Multi-byte character
					tmp = font2->kerning[0] - 1;
					*textwidth = *textwidth - tmp + 11;
					text += 2;
				} else {
					// Multi-byte character
					tmp = font2->kerning[0] - 1;
					*textwidth = *textwidth - tmp + 15;
					text += 2;
				}
			}
		}
	}

	if (longest > *textwidth) {
		*textwidth = longest;
	}
}

void textWrap(int wrapwidth, char *src, char *dst, struct fontchar *chars, struct font *font)
{
	int curlinewidth = 0;
	bool itfits;
	int wordlen;
	int wordwidth;
	int wordheight = 0;
	bool more = true;
	int v1;
	int i;
	uint32_t stack;
	char curword[32];

	while (more == true) {
		// Load the next word
		wordwidth = 0;
		wordlen = 0;
		v1 = 0;

		while (*src > ' ') {
			curword[wordlen] = *src;
			v1 += chars[*src - 0x21].width;
			src++;
			wordlen++;

			{
				if (curword[wordlen - 1] >= 0x80) {
					curword[wordlen] = *src;
					v1 += chars[*src - 0x21].width;
					src++;
					wordlen++;
				}
			}
		}

		curword[wordlen] = '\0';

		textMeasure(&wordheight, &wordwidth, curword, chars, font, 0);

		curlinewidth += wordwidth;

		if (curlinewidth <= wrapwidth) {
			itfits = true;
		} else {
			itfits = false;
		}

		if (*src == '\n') {
			// Write a new line and indent
			if (!itfits) {
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			curlinewidth = 0;

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the original new line that was in src
			*dst = '\n';
			dst++;
		} else if (*src == ' ') {
			if (!itfits) {
				// Write a new line and indent
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}

				curlinewidth = g_WrapIndentCount * SPACE_WIDTH + wordwidth;
			}

			curlinewidth += SPACE_WIDTH;

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the trailing space
			*dst = ' ';
			dst++;
		} else if (*src == '\0') {
			more = false;

			if (!itfits) {
				// Write a new line and indent
				*dst = '\n';
				dst++;

				for (i = 0; i < g_WrapIndentCount; i++) {
					*dst = ' ';
					dst++;
				}
			}

			// Write the current word
			for (i = 0; i < wordlen; i++) {
				*dst = curword[i];
				dst++;
			}

			// Write the null terminator
			*dst = '\0';
		}

		src++;
	}
}

uint8_t *textLoadBMP(const char *filename, uint16_t *width, uint16_t *height) {
    FILE *file = fopen(filename, "rb");  // Open in binary mode
    if (!file) {
        printf("Error: Could not open BMP file.\n");
        return NULL;
    }

    // Read BMP headers
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Check BMP signature ("BM")
    if (fileHeader.type != 0x4D42) {
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return NULL;
    }

    // Store width & height
    *width = infoHeader.width;
    *height = infoHeader.height;

    // Ensure it's a 24-bit BMP (uncompressed)
    if (infoHeader.bitCount != 24 || infoHeader.compression != 0) {
        printf("Error: Only uncompressed 24-bit BMP files are supported.\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for pixel data (3 bytes per pixel: R, G, B)
    int row_padded = (*width * 3 + 3) & (~3);  // Align rows to 4 bytes
    uint8_t *data = (uint8_t *)malloc(row_padded * (*height));
    if (!data) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    // Temporary buffer for flipped data
    uint8_t *flipped_data = (uint8_t *)malloc(row_padded * (*height));
    if (!flipped_data) {
        printf("Error: Memory allocation for flipping failed.\n");
        free(data);
        fclose(file);
        return NULL;
    }

    // Move file pointer to the pixel data location
    fseek(file, fileHeader.offset, SEEK_SET);

    // Read pixel data (BMP is stored bottom-to-top)
    for (int i = 0; i < *height; i++) {
        fread(data + (i * row_padded), 1, row_padded, file);
    }

    fclose(file);

    // Flip the image vertically
    for (int i = 0; i < *height; i++) {
        memcpy(flipped_data + (i * row_padded), data + ((*height - 1 - i) * row_padded), row_padded);
    }

    free(data);  // Free old data
    return flipped_data;  // Return the flipped image
}

struct fontchar *createChar(char *filename, uint16_t index)
{
	struct fontchar *newchar = malloc(sizeof(struct fontchar));
	newchar->index = index;

	uint16_t width;
	uint16_t height;

	if (!newchar) return NULL;  // Handle memory allocation failure

	char *fullpath = "./" DEFAULT_BASEDIR_NAME "/fonts/handelgothic/"; // ./data/fonts/handelgothic

	static int dirExists = -1;
	if (dirExists < 0) {
		dirExists = (fsFileSize(fullpath) >= 0);
	}

	newchar->pixeldata = textLoadBMP(buildDynamicPath(fullpath, filename), &width, &height);
	newchar->width = width;
	newchar->height = height;
	newchar->baseline = 0;
	newchar->kerningindex = -1;

	return newchar;
}

// Load the characters in the HD Handel Gothic font. The bmp's are named hg_0.bmp, hg_1.bmp, etc...with the images in ASCII order
void textLoadCustomFont()
{
	uint16_t i = 0;
	for (i = 0; i < TOTAL_CHARS; i++) {
		char filename[20];
		snprintf(filename, sizeof(filename), "hg_%d.bmp", ASCII_START + i);
        g_HandelGothicData[i] = *createChar(filename, i); // Load in our .bmp
    }
}

// Memory cleanup for custom font .bmp's on level reset
void textFreeFontCharacters() {
    for (int i = 0; i < TOTAL_CHARS; i++) {
        free(g_HandelGothicData[i].pixeldata);  // Free pixel data if allocated
    }
}

// Test function to see if .bmp's are loading correctly
/*char *generateBitmapASCII(struct fontchar *charData) {
    if (!charData || !charData->pixeldata) {
        return strdup("Error: No pixel data available!\n");
    }

    int width = charData->width;
    int height = charData->height;
    uint8_t *pixelData = charData->pixeldata;

    // Calculate required string size (each pixel = 1 char, each row = width + newline)
    int totalSize = (width + 1) * height + 1;  // +1 for null terminator
    char *asciiArt = (char *)malloc(totalSize);
    if (!asciiArt) {
        return strdup("Error: Memory allocation failed!\n");
    }

    char *ptr = asciiArt;

    // BMP data is stored bottom-to-top, so we print it in reverse row order
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = (y * width + x) * 3;  // 24-bit BMP (3 bytes per pixel: R, G, B)
            
            // Since it's 24-bit BMP, we assume grayscale and check the RED channel
            *ptr++ = (pixelData[pixelIndex] == 0) ? '0' : '1';
        }
        *ptr++ = '\n';  // Newline at the end of each row
    }

    *ptr = '\0';  // Null-terminate the string
    return asciiArt;
}*/