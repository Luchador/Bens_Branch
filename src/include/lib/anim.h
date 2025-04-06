#pragma once

#include "data.h"
#include "types.h"

void animsInit(void);
void animsInitTables(void);
void animsReset(void);
int animGetNumFrames(int16_t anim_id);
bool animHasFrames(int16_t animnum);
int animGetNumAnimations(void);
uint8_t *animDma(uint8_t *dst, unsigned int segoffset, unsigned int len);
int animGetRemappedFrame(int16_t animnum, int frame);
bool animRemapFrameForLoad(int16_t animnum, int frame, int *frameptr);
bool animIsFrameCutSkipped(int16_t animnum, int frame);
uint8_t animLoadFrame(int16_t animnum, int framenum);
void animForgetFrameBirths(void);
void animLoadHeader(int16_t animnum);
int animReadBits(uint8_t *ptr, uint8_t readbitlen, unsigned int bitoffset);
int animReadSignedShort(uint8_t *arg0, uint8_t arg1, int arg2);
void animGetRotTranslateScale(int part, bool flip, struct skeleton *skel, int16_t animnum, uint8_t frameslot, struct coord *rot, struct coord *translate, struct coord *scale);
uint16_t animGetPosAngleAsInt(int part, bool flip, struct skeleton *skel, int16_t animnum, int framenum, int16_t inttranslate[3], bool arg6);
float animGetTranslateAngle(int part, bool flip, struct skeleton *skel, int16_t animnum, int framenum, struct coord *pos, bool arg6);
float animGetCameraValue(int part, int16_t animnum, uint8_t frameslot);
