#ifndef _IN_LIB_MP3_INTERNAL_H
#define _IN_LIB_MP3_INTERNAL_H
#include <stdint.h>
#include "mp3.h"

bool mp3decInit(void);
bool mp3decSetSideInfo(struct asistream *stream);
bool mp3decDecodeFrame(struct asistream *stream);

int mp3FillBitstreamBuffer(struct asistream *stream);
int mp3utilGetBits(uint8_t *buffer, int *count, int numbits);
int mp3util000462f8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int arg5, int16_t **arg6, uint8_t **arg7);
int mp3util000464a8(uint8_t *arg0, int *arg1, int arg2, int arg3, int arg4, int16_t **arg5, uint8_t **arg6);

#endif
