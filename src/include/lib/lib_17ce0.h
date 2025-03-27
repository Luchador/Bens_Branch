#ifndef _IN_LIB_LIB_17CE0_H
#define _IN_LIB_LIB_17CE0_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

#define PORTALINTERSECTION_NONE          0
#define PORTALINTERSECTION_BEHINDTOFRONT 1
#define PORTALINTERSECTION_FRONTTOBEHIND 2

void portalGetAvgVertexPos(int portalnum, struct coord *avg);
void portalAppendRoom(RoomNum *rooms, RoomNum roomnum);
int portalCalculateIntersection(int portalnum, struct coord *pos1, struct coord *pos2);
void portalComputeReachableRooms(struct coord *pos, struct coord *pos2, RoomNum *rooms, RoomNum *arg3, RoomNum *arg4, int maxvisited);

#endif
