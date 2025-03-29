#ifndef IN_GAME_CRC_H
#define IN_GAME_CRC_H
#include "data.h"
#include "types.h"

void crcCalculateU16Pair(uint8_t *start, uint8_t *end, uint16_t *checksum);

#endif
