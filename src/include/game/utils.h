#ifndef IN_GAME_UTILS_H
#define IN_GAME_UTILS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void func0f176d70(s32 arg0);
u32 FloatToUInt32(f32 arg0);
u32 align4(u32 arg0);
u32 align16(u32 arg0);
uintptr_t align32(uintptr_t arg0);
void utilsInit(void);
void textureCalcScreenCoords(Gfx **gdl, f32 *arg1, f32 *arg2, s32 width, s32 height, bool arg5, bool arg6, bool arg7, bool arg8);
//void utilsCalcLeftHandedCross(struct coord *a, struct coord *b, struct coord *out);
bool normalizeVector(struct coord *arg0, struct coord *arg1, u32 line, char *file);
void InterpTwoPoints(struct coord *arg0, struct coord *arg1, f32 standfrac, struct coord *vel);
void CatmullRomSplineInterp(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3, f32 arg4, struct coord *arg5);
bool isPointInBBox(struct coord *a, struct coord *b);
f32 coordsGetDistance(struct coord *a, struct coord *b);
//f32 func0f1776cc(struct coord *a, struct coord *b, struct coord *c);
//bool func0f17776c(struct coord *a, struct coord *b, f32 mult, struct coord *out);
void utilsReset(void);
s32 func0f177a54(u8 *arg0, s32 arg1, u8 *arg2, s32 arg3);
//s32 func0f177bb4(u8 *arg0, s32 *arg1, s32 *arg2);
s32 func0f177c8c(u8 *arg0, s32 *arg1, s32 *roomnum);

#endif
