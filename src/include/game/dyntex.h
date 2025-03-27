#ifndef _IN_GAME_DYNTEX_H
#define _IN_GAME_DYNTEX_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void dyntexTickRoom(int roomnum, Vtx *vertices);
void dyntexAddVertex(Vtx *vertex);
void dyntexSetCurrentType(int16_t type);
void dyntexSetCurrentRoom(RoomNum roomnum);
void dyntexReset(void);
bool dyntexHasRoom(void);

#endif
