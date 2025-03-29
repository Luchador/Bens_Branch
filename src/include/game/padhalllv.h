#ifndef IN_GAME_PADHALLLV_H
#define IN_GAME_PADHALLLV_H
#include "data.h"
#include "types.h"

void navSetSeed(uint32_t upper, uint32_t lower);
struct waypoint *waypointFindClosestToPos(struct coord *pos, RoomNum *rooms);
struct waygroup *waygroupChooseNeighbour(int *groupnums, int step, uint32_t ignoremask);
void waygroupSetStepIfUndiscovered(int *groupnums, int step, uint32_t ignoremask);
bool waygroupDiscoverOneStep(struct waygroup *group, int step, uint32_t ignoremask);
bool waygroupDiscoverSteps(struct waygroup *from, struct waygroup *to, struct waygroup *groups, bool discoverall, uint32_t ignoremask);
bool waygroupFindRoute(struct waygroup *from, struct waygroup *to, struct waygroup *groups);
struct waypoint *waypointChooseNeighbour(int *pointnums, int step, int groupnum, uint32_t ignoremask);
void waypointSetStepIfUndiscovered(int *pointnums, int value, int groupnum, uint32_t ignoremask);
bool waypointDiscoverOneStep(int *pointnums, int step, int groupnum, uint32_t ignoremask);
void waypointDiscoverSteps(struct waypoint *from, struct waypoint *to, bool discoverall, uint32_t ignoremask);
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
