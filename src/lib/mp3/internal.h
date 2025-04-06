#pragma once

#include <stdint.h>
#include "mp3.h"

bool mp3decInit(void);
bool mp3decSetSideInfo(struct asistream *stream);
bool mp3decDecodeFrame(struct asistream *stream);

int mp3main00043dd0(struct asistream *stream);

float func00045ed0(float arg0, float arg1);

int mp3utilGetBits(uint8_t *buffer, int *count, int numbits);
int mp3util000462f8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int arg5, int16_t **arg6, uint8_t **arg7);
int mp3util000464a8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int16_t **arg5, uint8_t **arg6);

void func00046650(struct asistream_4f64 *arg0, int arg1, struct asistream_4f64 *arg2, struct asistream_4f64 *arg3, void *arg4);

void func00047550(struct asistream_4f64 *arg0, int arg1, struct asistream_4f64 *arg2, struct asistream_4f64 *arg3);

float func00047d20(float arg0);

float func00047ef0(float arg0);
