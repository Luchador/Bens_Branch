#ifndef IN_GAME_SETUP_H
#define IN_GAME_SETUP_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void setupPreparePads(void);
void setupLoadWaypoints(void);
void setupPrepareCover(void);

void propsReset(void);
void setupCreateLiftDoor(struct linkliftdoorobj *link);
void setupCreatePadlockedDoor(struct padlockeddoorobj *link);
void setupCreateSafeItem(struct safeitemobj *link);
void setupCreateConditionalScenery(struct linksceneryobj *link);
void setupCreateBlockedPath(struct blockedpathobj *link);
void setupResetTVScreens(void);
void setupResetProxyMines(void);
int setupCountCommandType(u32 type);
void setupCreateObject(struct defaultobj *obj, int cmdindex);
void setupPlaceWeapon(struct weaponobj *weapon, int cmdindex);
//void setupCreateHat(struct hatobj *hat, int cmdindex);
void setupCreateKey(struct keyobj *key, int cmdindex);
void setupCreateMine(struct mineobj *mine, int cmdindex);
void setupCreateCctv(struct cctvobj *camera, int cmdindex);
void setupCreateAutogun(struct autogunobj *autogun, int cmdindex);
void setupCreateHangingMonitors(struct hangingmonitorsobj *monitors, int cmdindex);
void setupCreateSingleMonitor(struct singlemonitorobj *monitor, int cmdindex);
void setupCreateMultiMonitor(struct multimonitorobj *monitor, int cmdindex);
int setupGetPortalByPad(int padnum);
int setupGetPortalByDoorPad(int padnum);
void setupCreateDoor(struct doorobj *door, int cmdindex);
void setupCreateHov(struct defaultobj *obj, struct hov *hov);
void setupLoadBriefing(int stagenum, u8 *buffer, int bufferlen, struct briefing *briefing);
void setupLoadFiles(int stagenum);
void setupCreateProps(int stagenum);

#endif
