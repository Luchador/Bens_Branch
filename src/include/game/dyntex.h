#pragma once

#include "data.h"
#include "types.h"

void dyntexTickRoom(int roomnum, Vtx *vertices);
void dyntexAddVertex(Vtx *vertex);
void dyntexSetCurrentType(int16_t type);
void dyntexSetCurrentRoom(RoomNum roomnum);
void dyntexReset(void);
bool dyntexHasRoom(void);

