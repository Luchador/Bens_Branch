#pragma once

#include "data.h"
#include "types.h"

void camSetScreenSize(float width, float height);
void camSetScreenPosition(float left, float top);
void camSetPerspective(float near, float fovy, float aspect);
float camGetEraserFOV(float arg0);
void camSetScale(void);
void camProjectScreenToWorldDir(float pos2d[2], struct coord *dir2d, float arg2);
void camProjectViewToScreen(struct coord *in, float *out);
void camProjectViewToScreenSafe(struct coord *in, float out[2]);
void camProjectViewToScreenAbsZ(struct coord *in, float out[2]);
void camScaleViewToScreen(float in[2], float divisor, float out[2]);
void camProjectWithZoomAndAspect(struct coord *arg0, float arg1[2], float zoom, float aspect);
//void camSetMtxL1738(Mtx *mtx);
//void camSetMtxL173c(Mtx *mtx);
//Mtx *camGetMtxL173c(void);
void camSetArtifactMtx(Mtx *mtx);
Mtx *camGetArtifactMtx(void);
void camSetPerspectiveMtxL(Mtx *value);
Mtx *camGetPerspectiveMtxL(void);
void camSetOrthogonalMtxL(Mtx *mtx);
Mtx *camGetOrthogonalMtxL(void);
void camSetWorldToScreenMtx(Mtx *mtx);
Mtxf *camGetWorldToScreenMtx(uint8_t *arg0);
Mtxf *camGetProjectionMtxForPlayers(uint8_t *arg0);
Mtxf *camGetWorldToScreenMtxf(void);
void camSetSkyMtx(Mtxf *mtx);
Mtxf *camGetSkyMtx(void);
void camSetProjectionMtxF(Mtxf *mtx);
Mtxf *camGetProjectionMtx(void);
void camSetLookAt(LookAt *lookat);
LookAt *camGetLookAt(void);
float camGetLodScaleZ(void);
float camGetScreenWidth(void);
float camGetScreenHeight(void);
float camGetScreenLeft(void);
float camGetScreenTop(void);
float camGetPerspAspect(void);
void camComputeFrustumEdgePlanes(void);
bool camIsPointInFrustum(struct coord *arg0, float arg1);
bool camIsPosInScreenBox(struct coord *pos, float arg1, struct drawslot *drawslot);
bool camIsPosInFovAndVisibleRoom(RoomNum *rooms, struct coord *pos, float arg2);