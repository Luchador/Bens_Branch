#pragma once

#include "data.h"
#include "types.h"

int utilsClamp(int value, int min, int max);
float utilsClampF(float value, float min, float max);
uint64_t utilsGetCount(void);
uint32_t align4(uint32_t arg0);
uint32_t align16(uint32_t arg0);
uintptr_t align32(uintptr_t arg0);
void utilsRenderScreenTexture(Gfx **gdlptr, float *screenpos, float *brightness, int width, int height, int arg5, int arg6, int arg7, bool arg8);
bool utilsNormalizeVec(struct coord *arg0, struct coord *arg1);
void utilsNormalizeF(float *x, float *y, float *z);
void InterpTwoPoints(struct coord *arg0, struct coord *arg1, float standfrac, struct coord *vel);
void CatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, float arg4, struct coord *arg5);
bool isPointInBBox(struct coord *a, struct coord *b);
float coordsGetDistance(struct coord *a, struct coord *b);
void utilsReset(void);
int utilCompressZeroRuns(uint8_t *input, int numEntries, uint8_t *output, int entrySize);
int untilCompressRoomData(uint8_t *arg0, int *arg1, int *roomnum);
bool func0002f490(struct vec3s16 *arg0, struct vec3s16 *arg1, struct vec3s16 *arg2, struct coord *arg3, struct coord *arg4, struct coord *arg5, struct coord *arg6, struct coord *arg7, struct coord *arg8);
bool func0002f560(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, struct coord *arg4, struct coord *arg5, struct coord *arg6, struct coord *arg7, struct coord *arg8);
