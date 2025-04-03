#ifndef IN_GAME_PADHALLLV_H
#define IN_GAME_PADHALLLV_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void navSetSeed(u32 upper, u32 lower);
struct waypoint *waypointFindClosestToPos(struct coord *pos, RoomNum *rooms);
struct waygroup *waygroupChooseNeighbour(int *groupnums, int step, u32 ignoremask);
void waygroupSetStepIfUndiscovered(int *groupnums, int step, u32 ignoremask);
bool waygroupDiscoverOneStep(struct waygroup *group, int step, u32 ignoremask);
bool waygroupDiscoverSteps(struct waygroup *from, struct waygroup *to, struct waygroup *groups, bool discoverall, u32 ignoremask);
bool waygroupFindRoute(struct waygroup *from, struct waygroup *to, struct waygroup *groups);
struct waypoint *waypointChooseNeighbour(int *pointnums, int step, int groupnum, u32 ignoremask);
void waypointSetStepIfUndiscovered(int *pointnums, int value, int groupnum, u32 ignoremask);
bool waypointDiscoverOneStep(int *pointnums, int step, int groupnum, u32 ignoremask);
void waypointDiscoverSteps(struct waypoint *from, struct waypoint *to, bool discoverall, u32 ignoremask);
void waypointFindRoute(struct waypoint *from, struct waypoint *to);
int waypointCollectLocal(struct waypoint *from, struct waypoint *to, struct waypoint **arr, int arrlen);
void waypointFindSegmentIntoGroup(struct waygroup *fromgroup, struct waygroup *togroup, struct waypoint **frompoint, struct waypoint **topoint);
int navFindRoute(struct waypoint *from, struct waypoint *to, struct waypoint **arr, int arrlen);
void waypointResetAllSteps(void);
struct waypoint *waypointFindRandomAtStep(int *pointnums, int step);
struct waygroup *waygroupFindRandomAtStep(int *groupnums, int step);
struct waypoint *navChooseRetreatPoint(struct waypoint *chrpoint, struct waypoint *targetpoint);
void navDisableSegmentInDirection(struct waypoint *a, struct waypoint *b);
void navEnableSegmentInDirection(struct waypoint *a, struct waypoint *b);
void navDisableSegment(struct waypoint *a, struct waypoint *b);
void navEnableSegment(struct waypoint *a, struct waypoint *b);

#endif
