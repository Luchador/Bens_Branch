#ifndef IN_GAME_SAVEBUFFER_H
#define IN_GAME_SAVEBUFFER_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void func0f0d4690(Mtxf *mtx);
void func0f0d475c(Mtxf *mtx);
Gfx *func0f0d479c(Gfx *gdl);
Gfx *func0f0d49c8(Gfx *gdl);
Gfx *func0f0d4a3c(Gfx *gdl, int arg1);
Gfx *func0f0d4c80(Gfx *gdl);
Gfx *menugfxDrawPlane(Gfx *gdl, int x1, int y1, int x2, int y2, u32 colour1, u32 colour2, int type);
void savebufferOr(struct savebuffer *buffer, u32 arg1, int arg2);
void savebufferWriteBits(struct savebuffer *buffer, u32 value, int numbits, u8 *dst);
u32 savebufferReadBits(struct savebuffer *buffer, int offset);
void savebufferClear(struct savebuffer *buffer);
void savebufferWriteData(struct savebuffer *buffer, u8 *data, u8 len);
void func0f0d54c4(struct savebuffer *buffer);
void savebufferReadString(struct savebuffer *buffer, char *dst, bool addlinebreak);
void func0f0d55a4(struct savebuffer *buffer, char *src);
void func0f0d564c(u8 *data, char *dst, bool addlinebreak);
void func0f0d5690(u8 *dst, char *buffer);
void savebufferWriteGuid(struct savebuffer *buffer, struct fileguid *guid);
void savebufferReadGuid(struct savebuffer *buffer, struct fileguid *guid);
void formatTime(char *dst, int time60, int precision);
void savebufferResetVp(void);

#endif
