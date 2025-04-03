#ifndef _IN_GAME_ROOM_H
#define _IN_GAME_ROOM_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void roomsReset(void);

void roomsTick(void);

void roomSetLastForOffset(int room);
void roomLinkMtx(int index, int roomnum);
void roomUnlinkMtx(int index, int roomnum);
void roomFreeMtx(int index);
int roomAllocateMtx(void);
void roomPopulateMtx(Mtxf *matrix, int roomnum);
int roomTouchMtx(int roomnum);
Gfx *roomApplyMtx(Gfx *gdl, int roomnum);
struct coord *roomGetPosPtr(int room);
void roomGetPos(int room, struct coord *pos);

#endif
