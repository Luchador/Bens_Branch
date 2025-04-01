#ifndef _IN_LIB_COLLISION_H
#define _IN_LIB_COLLISION_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

float func0f1577f0(float arg0[2], float arg1[2], float arg2[2], float arg3[2]);
float rayIntersectCircleXZ(struct widthxz *arg0, struct xz *arg1, struct xz *arg2);
float getSlideTimeToEdgeXZ(struct widthxz *circle, struct xz *edgeStart, struct xz *edgeEnd, struct xz *movement);

float cd00024e40(void);
void cdGetEdge(struct coord *pos1, struct coord *pos2);
float cd00024e98(void);
int cd00024ea4(void);
struct prop *cdGetObstacleProp(void);
void cdGetPos(struct coord *pos);
void cdGetObstacleNormal(struct coord *normal);
int32_t cdGetGeoFlags(void);
void cdSetSavedPos(struct coord *pos1, struct coord *pos2);
bool cdGetSavedPos(struct coord *arg0, struct coord *arg1);
void cdGetGeoNormal(struct geo *geo, struct coord *normal);
void cdGetFloorCol(struct geo *tile, int16_t *floorcol);
void cdGetFloorType(struct geo *tile, uint8_t *floortype);
bool cd000266a4(float x, float z, struct geo *tile);
void cdGetPropsOnPlatform(struct prop *platform, int16_t *propnums, int len);
int cd000274e0Block(struct geoblock *tile, float x, float z, float width, struct prop *prop, struct collision *collision);
bool cd000276c8Cyl(struct geocyl *tile, float x, float z, float width, struct prop *prop, struct collision *collision);
bool cdFindLadder(struct coord *pos, float width, float ymax, float ymin, RoomNum *rooms, int16_t geoflags, struct coord *laddernormal);
bool cd0002a13c(struct coord *pos, float radius, float arg2, float arg3, RoomNum *rooms, int16_t geoflags);
float cdFindGroundInfoAtCyl(struct coord *pos, float radius, RoomNum *rooms, int16_t *floorcol, uint8_t *floortype, int16_t *floorflags, RoomNum *floorroom, bool *inlift, struct prop **lift);
float cdFindGroundAtCyl(struct coord *pos, float radius, RoomNum *rooms, int16_t *floorcol, uint8_t *floortype);
float cdFindFloorYColourTypeAtPos(struct coord *pos, RoomNum *rooms, int16_t *floorcol, uint8_t *floortype);
int cdFindFloorRoomAtPos(struct coord *pos, RoomNum *nearrooms);
RoomNum cdFindFloorRoomYColourFlagsAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcolptr, int16_t *flagsptr);
RoomNum cdFindCeilingRoomYColourFlagsAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcolptr, int16_t *flagsptr);
RoomNum cdFindFloorRoomYColourNormalPropAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcol, struct coord *normal, struct prop **propptr);
RoomNum cdFindCeilingRoomYColourFlagsNormalAtPos(struct coord *pos, RoomNum *rooms, float *arg2, int16_t *floorcol, int16_t *geoflags, struct coord *normal);
int cdTestVolume(struct coord *pos, float radius, RoomNum *rooms, int types, bool checkvertical, float ymax, float ymin);
int cdExamCylMove01(struct coord *pos, struct coord *pos2, float radius, RoomNum *rooms, int types, bool checkvertical, float ymax, float ymin);
int cdExamCylMove02(struct coord *origpos, struct coord *dstpos, float width, RoomNum *dstrooms, int types, bool checkvertical, float ymax, float ymin);
bool cdTestCylMove01(struct coord *pos, RoomNum *rooms, struct coord *targetpos, int32_t types, int32_t arg4, float ymax, float ymin);
int cdTestCylMove02(struct coord *pos, RoomNum *rooms, struct coord *coord2, RoomNum *rooms2, int32_t types, bool arg5, float ymax, float ymin);
int cdExamCylMove03(struct coord *pos, RoomNum *rooms, struct coord *arg2, int32_t types, int32_t arg4, float ymax, float ymin);
int cdTestCylMove04(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int arg5, float ymax, float ymin);
int cdExamCylMove05(struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2, int types, bool arg5, float ymax, float ymin);
int cdExamCylMove06(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, float arg4, int types, int arg6, float ymax, float ymin);
int cdExamCylMove07(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int arg5, float ymax, float ymin);
int cdExamCylMove08(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, float width, int32_t types, int arg6, float ymax, float ymin);
bool cdTestLos03(struct coord *viewpos, RoomNum *rooms, struct coord *targetpos, int32_t types, int16_t geoflags);
bool cdTestLos04(struct coord *coord, RoomNum *rooms, struct coord *coord2, int arg3);
bool cdTestLos05(struct coord *coord, RoomNum *rooms, struct coord *coord2, RoomNum *rooms2, int cdtypes, int16_t geoflags);
bool cdTestLos06(struct coord *arg0, RoomNum *rooms1, struct coord *arg2, RoomNum *rooms2, int32_t types);
bool cdTestLos07(struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2, RoomNum *rooms3, int32_t types, int16_t geoflags);
int cdExamLos08(struct coord *pos, RoomNum *rooms, struct coord *pos2, int32_t types, int16_t geoflags);
int cdExamLos09(struct coord *pos, RoomNum *rooms, struct coord *pos2, int32_t types);
int cdTestLos10(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types, int16_t geoflags);
int cdTestLos11(struct coord *arg0, RoomNum *arg1, struct coord *arg2, RoomNum *arg3, int32_t types);
bool cdIsPathClearToProp(struct coord *arg0, struct coord *arg1, struct prop *prop);
int cdTestBlockOverlapsAnyProp(struct geoblock *geo, RoomNum *rooms, int32_t types);
int cd0002f02c(struct geoblock *block, RoomNum *rooms, int types);
bool cdIsNearlyInSight(struct coord *viewpos, RoomNum *rooms, struct coord *targetpos, float distance, int arg4);
bool cdTestAToB(struct coord *pos, struct coord *coord2, RoomNum *rooms, int32_t types, int16_t geoflags, bool checkvertical, int arg6, float ymax, float ymin);

#endif
