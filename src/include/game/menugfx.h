#pragma once

#include "data.h"
#include "types.h"

void menugfxCreateBlur(void);
Gfx *menugfxRenderBgBlur(Gfx *gdl, uint32_t colour, int16_t arg2, int16_t arg3);
Gfx *menugfxRenderDialogBackground(Gfx *gdl, int x1, int y1, int x2, int y2, struct menudialog *dialog, uint32_t colour1, uint32_t colour2, float arg8);
Gfx *menugfxDrawDropdownBackground(Gfx *gdl, int x1, int y1, int x2, int y2);
Gfx *menugfxDrawListGroupHeader(Gfx *gdl, int x1, int y1, int x2, int y2, int x3, uint8_t alpha);
Gfx *menugfxRenderGradient(Gfx *gdl, int x, int y, int width, int height, uint32_t colour1, uint32_t colour2, uint32_t colour3);
Gfx *menugfxRenderSlider(Gfx *gdl, int x1, int y1, int x2, int y2, int markerx, uint32_t colour);
Gfx *menugfx0f0e2348(Gfx *gdl);
Gfx *menugfx0f0e2498(Gfx *gdl);
Gfx *menugfxDrawTri2(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2, bool arg7);
Gfx *menugfxDrawLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2);
Gfx *menugfxDrawProjectedLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2);
Gfx *menugfxDrawShimmer(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour, bool arg6, int arg7, bool lefttoright);
Gfx *menugfxDrawDialogBorderLine(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2);
Gfx *menugfxDrawFilledRect(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2);
Gfx *menugfxDrawCarouselChevron(Gfx *gdl, int x, int y, int size, int direction, uint32_t colour1, uint32_t colour2);
Gfx *menugfxDrawDialogChevron(Gfx *gdl, int x, int y, int arg2, int arg3, uint32_t colour1, uint32_t colour2, float arg7);
Gfx *menugfxDrawCheckbox(Gfx *gdl, int x, int y, int size, bool fill, uint32_t bordercolour, uint32_t fillcolour);
Gfx *menugfxRenderBgFailure(Gfx *gdl);
Gfx *menugfxRenderBgCone(Gfx *gdl);
void menugfxFreeParticles(void);
uint32_t menugfxGetParticleArraySize(void);
Gfx *menugfxRenderBgSuccess(Gfx *gdl);