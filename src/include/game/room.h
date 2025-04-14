#pragma once

#include "data.h"
#include "types.h"

void roomsAllocate(void);

void roomsTick(void);

void roomSetLastForOffset(int room);
void roomLinkMtx(int index, int roomnum);
void roomUnlinkMtx(int index, int roomnum);
void roomFreeMtx(int index);
int roomAllocateMtx(void);
void roomPopulateMtx(Mtx *matrix, int roomnum);
int roomTouchMtx(int roomnum);
Gfx *roomApplyMtx(Gfx *gdl, int roomnum);
struct coord *roomGetPosPtr(int room);
void roomGetPos(int room, struct coord *pos);
bool roomArrayIntersects(RoomNum *a, RoomNum *b);
void roomsFree();
