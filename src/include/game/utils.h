#pragma once

#include "gfx.h"
#include "data.h"
#include "types.h"

int utilsClamp(int value, int min, int max);
float utilsClampF(float value, float min, float max);
uint64_t utilsGetCount(void);
uint32_t align4(uint32_t arg0);
uint32_t align16(uint32_t arg0);
uintptr_t align32(uintptr_t arg0);
void utilsRenderScreenTexture(Gfx **gdlptr, float *screenpos, float *screensize, int width, int height, bool flipU, bool flipV, bool arg8);
bool utilsNormalizeVec(struct coord *arg0, struct coord *arg1);
void utilsNormalizeF(float *x, float *y, float *z);
void utilsInterpTwoPoints(struct coord *arg0, struct coord *arg1, float standfrac, struct coord *vel);
void utilsCatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, float arg4, struct coord *arg5);
bool isPointInBBox(struct coord *a, struct coord *b);
float coordsGetDistance(struct coord *a, struct coord *b);
void utilsReset(void);
int utilCompressRoomData(uint8_t *input, int numEntries, uint8_t *output, int entrySize);
int utilsDecompressRoomData(uint8_t *arg0, int *arg1, int *roomnum);
bool utilsIntersectTest1(struct vec3s16 *arg0, struct vec3s16 *arg1, struct vec3s16 *arg2, struct coord *arg3, struct coord *arg4, struct coord *arg5, struct coord *arg6, struct coord *arg7, struct coord *arg8);
bool utilsIntersectTest2(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4, struct coord *arg5, struct coord *arg6, struct coord *arg7, struct coord *arg8);
bool utilsSphereIntersectsOrientedBbox(struct coord *sphereCenter, float radius, struct modelrodata_bbox *bbox, Mtx *mtx);
bool utilsIsPointInCone(struct coord *pos, struct coord *dir, struct coord *aimpos, float arg3);
struct RGBA utilsUnpackColorRGBA(uint32_t color);
