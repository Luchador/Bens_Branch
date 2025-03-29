#ifndef _IN_GAME_BONDVIEW_H
#define _IN_GAME_BONDVIEW_H
#include "data.h"
#include "types.h"

Gfx *bviewDrawIrRect(Gfx *gdl, int x1, int y1, int x2, int y2);
Gfx *bviewCopyPixels(Gfx *gdl, uint16_t *fb, int top, uint32_t tile, int arg4, float arg5, int left, int width);
Gfx *bviewDrawFisheyeRect(Gfx *gdl, int arg1, float arg2, int arg3, int arg4);
Gfx *bviewPrepareStaticRgba16(Gfx *gdl, uint32_t colour, uint32_t alpha);
Gfx *bviewPrepareStaticI8(Gfx *gdl, uint32_t colour, uint32_t alpha);
Gfx *bviewDrawMotionBlur(Gfx *gdl, uint32_t colour, uint32_t alpha);
Gfx *bviewDrawStatic(Gfx *gdl, uint32_t arg1, int arg2);
Gfx *bviewDrawSlayerRocketInterlace(Gfx *gdl, uint32_t arg1, uint32_t arg2);
Gfx *bviewDrawFilmInterlace(Gfx *gdl, uint32_t colour, uint32_t alpha);
Gfx *bviewDrawZoomBlur(Gfx *gdl, uint32_t colour, int alpha, float arg3, float arg4);
float bview0f142d74(int arg0, float arg1, float arg2, float arg3);
Gfx *bviewDrawFisheye(Gfx *gdl, uint32_t colour, uint32_t alpha, int shuttertime60, int8_t startuptimer60, uint8_t hit);
Gfx *bviewDrawEyespySideRect(Gfx *gdl, int *points, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
Gfx *bviewDrawEyespyMetrics(Gfx *gdl);
Gfx *bviewDrawNvLens(Gfx *gdl);
Gfx *bviewDrawIrLens(Gfx *gdl);
Gfx *bviewDrawIntroText(Gfx *gdl);
Gfx *bviewDrawHorizonScanner(Gfx *gdl);
Gfx *bviewDrawIrBinoculars(Gfx *gdl);
void bviewSetMotionBlur(uint32_t bluramount);
void bviewClearMotionBlur(void);
Gfx *bviewDrawNvBinoculars(Gfx *gdl);

#endif
