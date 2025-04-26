#pragma once

#include "data.h"
#include "types.h"

void func0f0d4690(Mtxf *mtx);
void func0f0d475c(Mtxf *mtx);
Gfx *gfxSetCustomProjection(Gfx *gdl);
Gfx *savebufferSetup2DRender(Gfx *gdl);
Gfx *func0f0d4a3c(Gfx *gdl);
Gfx *func0f0d4c80(Gfx *gdl);
Gfx *menugfxDrawPlane(Gfx *gdl, int x1, int y1, int x2, int y2, uint32_t colour1, uint32_t colour2, int type);
void savebufferOr(struct savebuffer *buffer, uint32_t arg1, int arg2);
void savebufferWriteBits(struct savebuffer *buffer, uint32_t value, int numbits, uint8_t *dst);
uint32_t savebufferReadBits(struct savebuffer *buffer, int offset);
void savebufferClear(struct savebuffer *buffer);
void savebufferWriteData(struct savebuffer *buffer, uint8_t *data, uint8_t len);
void savebufferReadString(struct savebuffer *buffer, char *dst, bool addlinebreak);
void func0f0d55a4(struct savebuffer *buffer, char *src);
void func0f0d564c(uint8_t *data, char *dst, bool addlinebreak);
void func0f0d5690(uint8_t *dst, char *buffer);
void savebufferWriteGuid(struct savebuffer *buffer, struct fileguid *guid);
void savebufferReadGuid(struct savebuffer *buffer, struct fileguid *guid);
void formatTime(char *dst, int time60, int precision);
void savebufferResetVp(void);
