#pragma once

#include "data.h"
#include "types.h"

bool mp3decInit(void);
bool mp3dec00040164(struct asistream *stream, uint32_t gr, uint32_t ch);
bool mp3decUnpackScaleFac(struct asistream *stream, uint32_t gr, uint32_t ch);
bool mp3dec00041600(struct asistream *stream, uint32_t gr, uint32_t ch);
bool mp3dec00042238(struct asistream *stream, uint32_t gr, uint32_t ch);
bool mp3dec000427d8(struct asistream *stream, uint32_t gr);
bool mp3decReduceAliases(struct asistream *stream, uint32_t gr, uint32_t ch);
bool mp3decSetSideInfo(struct asistream *stream);
bool mp3decDecodeFrame(struct asistream *stream);
