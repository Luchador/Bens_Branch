#ifndef _IN_LIB_CRASH_H
#define _IN_LIB_CRASH_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

extern bool g_CrashEnabled;

u32 crashGetParentStackFrame(u32 *ptr, u32 *start, u32 sp, u32 *regs);
bool crashIsReturnAddress(u32 *instruction);
u32 crashGetStackEnd(u32 sp, s32 tid);
u32 crashGetStackStart(u32 arg0, s32 tid);
bool crashIsDouble(f32 value);
void crashPrintFloat(s32 index, f32 arg1);
void crashPrint2Floats(s32 index, f32 value1, f32 value2);
void crashPrint3Floats(s32 index, f32 value1, f32 value2, f32 value3);
void crashPutChar(s32 x, s32 y, char c);
void crashAppendChar(char c);
void crashScroll(s32 numlines);
void crashRenderChar(s32 x, s32 y, char c);
void crashRenderFrame(u16 *fb);

#endif
