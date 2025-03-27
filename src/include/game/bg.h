#ifndef IN_GAME_BG_H
#define IN_GAME_BG_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void bgUnpausePropsInRoom(unsigned int roomnum, bool tintedglassonly);
void bgSetRoomOnscreen(int room, int draworder, struct screenbox *arg2);
void bgGetRoomBrightnessRange(int roomnum, int8_t *min, int8_t *max);
struct drawslot *bgGetRoomDrawSlot(int roomnum);
Gfx *bgRenderXrayData(Gfx *gdl, struct xraydata *xraydata);
Gfx *bgAddXrayTri(Gfx *gdl, struct xraydata *xraydata, int16_t vertices1[3], int16_t vertices2[3], int16_t vertices3[3], uint32_t colour1, uint32_t colour2, uint32_t colour3);
void bgChooseXrayVtxColour(bool *inrange, int16_t vertex[3], uint32_t *colour, struct xraydata *xraydata);
Gfx *bgProcessXrayTri(Gfx *gdl, struct xraydata *xraydata, int16_t arg2[3], int16_t arg3[3], int16_t arg4[3], int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);
Gfx *bgRenderGdlInXray(Gfx *gdl, s8 *readgdl, Vtx *vertices, int16_t arg3[3]);
Gfx *bgRenderRoomXrayPass(Gfx *gdl, int roomnum, struct roomblock *blocks, bool recurse, int16_t arg4[3]);
Gfx *bgRenderRoomInXray(Gfx *gdl, int roomnum);
Gfx *bgRenderSceneInXray(Gfx *gdl);
Gfx *bgRenderScene(Gfx *gdl);
Gfx *bgRenderArtifacts(Gfx *gdl);
void bgLoadFile(void *memaddr, uint32_t offset, uint32_t len);
int bgGetStageIndex(int stagenum);
float bgCalculatePortalSurfaceArea(int portal);
void bgReset(int stagenum);
void bgBuildTables(int stagenum);
void bgStop(void);
float bgGetStageTranslationThing(void);
float bgGetScaleBg2Gfx(void);
void bgSetScaleBg2Gfx(float arg0);
void bgTickCounter(void);
void bgTick(void);
Gfx *bgRender(Gfx *gdl);
Gfx *bgScissorToViewport(Gfx *gdl);
Gfx *bgScissorWithinViewportF(Gfx *gdl, float viewleft, float viewtop, float viewright, float viewbottom);
Gfx *bgScissorWithinViewport(Gfx *gdl, int viewleft, int viewtop, int viewright, int viewbottom);
void bgClearPortalCameraCache(void);
bool bgRoomIntersectsScreenBox(int room, struct screenbox *arg1);
bool bg3dPosTo2dPos(struct coord *cornerpos, struct coord *screenpos);
bool bgGetPortalScreenBbox(int portal, struct screenbox *arg1);
Gfx *bgDrawBoxEdge(Gfx *gdl, int x1, int y1, int x2, int y2);
bool bgGetBoxIntersection(struct screenbox *a, struct screenbox *b);
void bgExpandBox(struct screenbox *a, struct screenbox *b);
void bgCopyBox(struct screenbox *dst, struct screenbox *src);
bool bgRoomIsOnscreen(int room);
bool bgRoomIsStandby(int room);
bool bgRoomIsOnPlayerScreen(int room, uint32_t playernum);
bool bgRoomIsOnPlayerStandby(int room, uint32_t aibotindex);
int bgFindPortalByVertices(struct portalvertices *pvertices);
uint32_t bgInflate(uint8_t *src, u8 *dst, uint32_t len);
Gfx *bgGetNextGdlInBlock(struct roomblock *block, Gfx *start, Gfx *end);
Gfx *bgGetNextGdlInLayer(int roomnum, Gfx *start, uint32_t types);
Vtx *bgFindVerticesForGdl(int roomnum, Gfx *gdl);
void bgLoadRoom(int roomnum);
void bgUnloadRoom(int room);
void bgUnloadAllRooms(void);
void bgTickRooms(void);
Gfx *bgRenderRoomPass(Gfx *gdl, int roomnum, struct roomblock *blocks, bool includetransp);
Gfx *bgRenderRoomOpaque(Gfx *gdl, int roomnum);
Gfx *bgRenderRoomXlu(Gfx *gdl, int roomnum);
int bgPopulateVtxBatchType(int roomnum, struct vtxbatch *batches, Gfx *gdl, int batchindex, Vtx *vertices, int arg5);
void bgFindRoomVtxBatches(int roomnum);
bool bgTestLineIntersectsIntBbox(struct coord *arg0, struct coord *arg1, int *arg2, int *arg3);
bool bgTestLineIntersectsBbox(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3);
bool bgTestHitOnObj(struct coord *arg0, struct coord *arg1, struct coord *arg2, Gfx *gdl, Gfx *gdl2, Vtx *vertices, struct hitthing *hitthing);
bool bgTestHitOnChr(struct model *model, struct coord *arg1, struct coord *arg2, struct coord *arg3, Gfx *arg4, Gfx *arg5, Vtx *vertices, float *arg7, struct hitthing *hitthing);
bool bgTestHitInVtxBatch(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct vtxbatch *batches, int roomnum, struct hitthing *hitthing);
int bg0f1612e4(struct coord *bbmin, struct coord *bbmax, struct coord *frompos, struct coord *dist, struct coord *arg4, struct coord *arg5);
bool bgTestHitInRoom(struct coord *frompos, struct coord *topos, int roomnum, struct hitthing *hitthing);
bool bgRoomIsLoaded(int room);
bool bgRoomContainsCoord(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoomCheap(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoomExpensive(struct coord *pos, RoomNum roomnum);
bool bgTestPosInRoom(struct coord *pos, RoomNum roomnum);
void bgFindRoomsByPos(struct coord *pos, RoomNum *inrooms, RoomNum *aboverooms, int max, RoomNum *bestroom);
bool bgCmdPushValue(bool value);
bool bgCmdPopValue(void);
bool bgCmdGetNthValueFromEnd(int offset);
struct bgcmd *bgCmdExecuteBranch(struct bgcmd *cmd, bool s2);
struct bgcmd *bgCmdExecute(struct bgcmd *cmd);
void bgTickPortalsXray(void);
void bgAddToSnake(RoomNum fromroomnum, RoomNum roomnum, int16_t draworder, struct screenbox *box);
void bgConsumeSnakeItem(struct bgsnakeitem *arg0);
bool bgTryConsumeSnake(void);
void bgChooseRoomsToLoad(void);
void bgTickPortals(void);
Gfx *bgRenderSceneAndLoadCandidate(Gfx *gdl);
int bgGetForceOnscreenRooms(RoomNum *rooms, int len);
int bgRoomGetNeighbours(int room, RoomNum *rooms, int len);
bool bgRoomsAreNeighbours(int roomnum1, int roomnum2);
void bgCalculateScreenProperties(void);
void bgExpandRoomToPortals(int roomnum);
void bgPortalSwapRooms(int portal);
void bgInitPortal(int portalnum);
void bgInitRoom(int roomnum);
void bgSetPortalOpenState(int portal, bool open);
int bgFindPortalBetweenPositions(struct coord *pos1, struct coord *pos2);
bool bgIsBboxOverlapping(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3);
void bgCalculatePortalBbox(int portalnum, struct coord *bbmin, struct coord *bbmax);
void bgFindEnteredRooms(struct coord *bbmin, struct coord *upper, RoomNum *rooms, int maxlen, bool arg4);
#ifndef PLATFORM_N64
void bgCalculateGlaresForVisibleRooms(void);
#endif

#endif
