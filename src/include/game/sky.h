#pragma once

#include "data.h"
#include "types.h"

void skyReset(uint32_t stagenum);

void skyTick(void);

void skyGetWorldPosFromScreenPos(float left, float top, struct coord *dst);
bool skyIsScreenCornerInSky(struct coord *corner3dpos, struct coord *dstpos, float *dstfrac);
bool skyIsCornerInWater(struct coord *corner3dpos, struct coord *dstpos, float *dstfrac);
void skyCalculateEdgeVertex(struct coord *arg0, struct coord *arg1, struct coord *out);
float skyClamp(float value, float min, float max);
float skyRound(float value);
void skyChooseCloudVtxColour(struct skyvtx3d *arg0, float arg1);
void skyChooseWaterVtxColour(struct skyvtx3d *arg0, float arg1);
Gfx *skyRender(Gfx *gdl);
void skyConvertVertex(struct skyvtx3d *arg0, Mtxf *arg1, uint16_t arg2, float arg3, float arg4, struct skyvtx2d *arg5);
bool skyVerticesAreSame(struct skyvtx2d *arg0, struct skyvtx2d *arg1);
void skyCreateSunArtifact(struct artifact *artifact, int x, int y);
float skyGetArtifactGroupIntensityFrac(struct artifact *artifacts);
Gfx *skyRenderSuns(Gfx *gdl, bool xray);
Gfx *skyRenderFlare(Gfx *gdl, float x, float y, float intensityfrac, float size, int flaretimer240, float alphafrac);
Gfx *skyRenderTeleportFlare(Gfx *gdl, float x, float y, float z, float size, float intensityfrac);
Gfx *skyRenderTeleportFlares(Gfx *gdl);
Gfx *skyRenderArtifacts(Gfx *gdl);
void skySetOverexposure(int arg0, int arg1, int arg2);
int skyCalculateOverexposureComponent(int arg0, int arg1);
Gfx *skyRenderOverexposure(Gfx *gdl);
