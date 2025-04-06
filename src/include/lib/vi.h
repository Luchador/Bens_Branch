#pragma once

#include "data.h"
#include "types.h"

void viConfigureForLogos(void);
void viConfigureForLegal(void);
void viReset(int stagenum);
void viHandleShake(void);
void viUpdateMode(void);
void viShake(float intensity);
void viSetMode();
uint16_t *viGetBackBuffer(void);
Vp *viGetCurrentPlayerViewport(void);
Gfx *viSetCamNoTranslation(Gfx *gdl);
Gfx *vi0000aca4(Gfx *gdl, float znear, float zfar);
Gfx *vi0000ad5c(Gfx *gdl, Vp *vp);
Gfx *vi0000af00(Gfx *gdl, Vp *vp);
Gfx *vi0000b0e8(Gfx *gdl, float fovy, float aspect);
Gfx *vi0000b1a8(Gfx *gdl);
Gfx *vi0000b1d0(Gfx *gdl);
Gfx *viPrepareZbuf(Gfx *gdl);
Gfx *viFillBuffer(Gfx *gdl);
Gfx *viRenderViewportEdges(Gfx *gdl);
void viSetBufSize(int16_t width, int16_t height);
int16_t viGetBufWidth(void);
int16_t viGetBufHeight(void);
void viSetSize(int16_t width, int16_t height);
int16_t viGetWidth(void);
int16_t viGetHeight(void);
void viSetViewSize(int16_t width, int16_t height);
int16_t viGetViewWidth(void);
int16_t viGetViewHeight(void);
void viSetViewPosition(int16_t left, int16_t top);
int16_t viGetViewLeft(void);
int16_t viGetViewTop(void);
void viSetUseZBuf(bool use);
void viSetFovY(float fovy);
void viSetAspect(float aspect);
float viGetAspect(void);
void viSetFovAspectAndSize(float fovy, float aspect, int16_t width, int16_t height);
float viGetFovY(void);
void viSetZRange(float arg0, float arg1);
void viGetZRange(struct zrange *zrange);
Gfx *viSetFillColour(Gfx *gdl, int r, int g, int b);
