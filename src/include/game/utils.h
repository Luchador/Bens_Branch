#ifndef IN_GAME_UTILS_H
#define IN_GAME_UTILS_H
#include "data.h"
#include "types.h"

void func0f176d70(int arg0);
uint64_t utilsGetCount(void);
uint32_t align4(uint32_t arg0);
uint32_t align16(uint32_t arg0);
uintptr_t align32(uintptr_t arg0);
void utilsInit(void);
void textureCalcScreenCoords(Gfx **gdl, float *arg1, float *arg2, int width, int height, bool arg5, bool arg6, bool arg7, bool arg8);
bool normalizeVector(struct coord *arg0, struct coord *arg1, uint32_t line, char *file);
void utilsNormalizeF(float *x, float *y, float *z);
void InterpTwoPoints(struct coord *arg0, struct coord *arg1, float standfrac, struct coord *vel);
void CatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, float arg4, struct coord *arg5);
bool isPointInBBox(struct coord *a, struct coord *b);
float coordsGetDistance(struct coord *a, struct coord *b);
void utilsReset(void);
int utilCompressZeroRuns(uint8_t *input, int numEntries, uint8_t *output, int entrySize);
int untilCompressRoomData(uint8_t *arg0, int *arg1, int *roomnum);

#endif
