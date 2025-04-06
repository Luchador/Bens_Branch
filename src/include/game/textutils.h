#pragma once

#include "data.h"
#include "types.h"

#define DIAGMODE_FADEIN  0
#define DIAGMODE_REDRAW  1
#define DIAGMODE_FADEOUT 2

void textSetRotation90(bool rotated);
void textSetWrapIndent(int count);
void textLoadFont(uint8_t *romstart, uint8_t *romend, struct font **fontptr, struct fontchar **charsptr, bool monospace);
void textReset(void);
Gfx *textConfigureGfxPipeline(Gfx *gdl);
Gfx *text0f153780(Gfx *gdl);
Gfx *textSetPrimColour(Gfx *gdl, uint32_t colour);
Gfx *textSetCCCustom02(Gfx *gdl);
Gfx *text0f153858(Gfx *gdl, int *x1, int *y1, int *x2, int *y2);
Gfx *text0f1538e4(Gfx *gdl, int *x1, int *y1, int *x2, int *y2);
Gfx *text0f153990(Gfx *gdl, int left, int top, int width, int height);
Gfx *text0f153a34(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour);
Gfx *text0f153ab0(Gfx *gdl);
void textStopRedrawEffect(void);
void text0f153b6c(int arg0);
void textSetDiagonalBlend(int x, int y, float redrawtimer, uint8_t populated);
void textBackupDiagonalBlendSettings(void);
void textRestoreDiagonalBlendSettings(void);
void textSetHorizontalBlend(int x1, int x2, uint32_t arg2);
void textBackupAndResetBlends(void);
void textRestoreBlends(void);
void textSetWaveBlend(int arg0, int arg1, int cthresh);
void textSetMenuBlend(float arg0);
void textSetWaveColours(uint32_t colour1, uint32_t colour2);
void textResetBlends(void);
bool textHasDiagonalBlend(void);
uint32_t textApplyProjectionColour(int x, int y, uint32_t colour);
uint32_t textHighlightSweep(int x, int y, uint32_t colour);
Gfx *text0f154ecc(Gfx *gdl, uint32_t arg1, uint32_t arg2);
Gfx *textMakeCreditVerts(Gfx *gdl, int *arg1, struct fontchar *curchar, struct fontchar *prevchar, struct font *font, float widthscale, float heightscale, float x, float y);
Gfx *textRenderCredit(Gfx *gdl, float x, float y, float widthscale, float heightscale, char *text, struct fontchar *chars, struct font *font, uint32_t colour, int hdir, int vdir);
Gfx *textRenderUnhighlighted(Gfx *gdl, int *x, int *y, struct fontchar *curchar, struct fontchar *prevchar, struct font *font, int savedx, int savedy, int width, int height, int arg10);
void textSetHasOutline(int arg0);
void textSetOutlineColor(uint32_t colour);
Gfx *textRenderProjected(Gfx *gdl, int *x, int *y, char *text, struct fontchar *chars, struct font *font, int colour, int width, int height, int arg9, int lineheight);
Gfx *text0f1566cc(Gfx *gdl, uint32_t arg1, uint32_t arg2);
Gfx *textRenderOutline(Gfx *gdl, int x, int y, struct fontchar *char1, int arg4, int arg5, int arg6, int arg7);
Gfx *textRender(Gfx *gdl, int *x, int *y, char *text, struct fontchar *font1, struct font *font2, uint32_t arg6, uint32_t colour, int width, int height, uint32_t arg10, int arg11);
void textMeasure(int *textheight, int *textwidth, char *text, struct fontchar *font1, struct font *font2, int lineheight);
void textWrap(int width, char *in, char *out, struct fontchar *font1, struct font *font2);
struct fontchar *createChar(char* filename, uint16_t index);
unsigned char *textLoadBMP(const char *filename, uint16_t *width, uint16_t *height);
void textLoadCustomFont();
void textFreeFontCharacters();
//char *generateBitmapASCII(struct fontchar *charData);
