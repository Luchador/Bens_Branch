#pragma once

#include "data.h"
#include "types.h"

#define PORTALINTERSECTION_NONE          0
#define PORTALINTERSECTION_BEHINDTOFRONT 1
#define PORTALINTERSECTION_FRONTTOBEHIND 2

void portalGetAvgVertexPos(int portalnum, struct coord *avg);
void portalTryAppendRoom(RoomNum *rooms, RoomNum roomnum);
int portalCalculateIntersection(int portalnum, struct coord *pos1, struct coord *pos2);
void portalTraceLineThroughRooms(struct coord *startPos, struct coord *endPos, RoomNum *startRooms, RoomNum *outputRooms, RoomNum *allVisitedRooms, int maxVisitedRooms);
