#ifndef _IN_GAME_CAMERA_H
#define _IN_GAME_CAMERA_H
#include "data.h"
#include "types.h"

void camSetScreenSize(float width, float height);
void camSetScreenPosition(float left, float top);
void camSetPerspective(float near, float fovy, float aspect);
float cam0f0b49b8(float arg0);
void camSetScale(void);
void cam0f0b4c3c(float pos2d[2], struct coord *dir2d, float arg2);
void cam0f0b4d04(struct coord *in, float *out);
void cam0f0b4d68(struct coord *in, float out[2]);
void cam0f0b4dec(struct coord *in, float out[2]);
void cam0f0b4e68(float in[2], float divisor, float out[2]);
void cam0f0b4eb8(struct coord *arg0, float arg1[2], float zoom, float aspect);
void camSetMtxL1738(Mtx *mtx);
void camSetMtxL173c(Mtx *mtx);
Mtx *camGetMtxL173c(void);
void camSetMtxF006c(Mtxf *mtx);
Mtxf *camGetMtxF006c(void);
void camSetPerspectiveMtxL(Mtx *value);
Mtx *camGetPerspectiveMtxL(void);
void camSetOrthogonalMtxL(Mtx *mtx);
Mtx *camGetOrthogonalMtxL(void);
void camSetWorldToScreenMtxf(Mtxf *mtx);
Mtxf *camGetWorldToScreenMtx(u8 *arg0);
Mtxf *camGetProjectionMtx(u8 *arg0);
Mtxf *camGetWorldToScreenMtxf(void);
void camSetMtxF1754(Mtxf *mtx);
Mtxf *camGetMtxF1754(void);
void camSetProjectionMtxF(Mtxf *mtx);
Mtxf *camGetProjectionMtxF(void);
void camSetLookAt(LookAt *lookat);
LookAt *camGetLookAt(void);
float camGetLodScaleZ(void);
float camGetScreenWidth(void);
float camGetScreenHeight(void);
float camGetScreenLeft(void);
float camGetScreenTop(void);
float camGetPerspAspect(void);
void cam0f0b5838(void);
bool cam0f0b5b9c(struct coord *arg0, float arg1);
bool camIsPosInScreenBox(struct coord *pos, float arg1, struct drawslot *drawslot);
bool camIsPosInFovAndVisibleRoom(RoomNum *rooms, struct coord *pos, float arg2);

#endif
