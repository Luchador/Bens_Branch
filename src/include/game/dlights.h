#ifndef IN_GAME_DLIGHTS_H
#define IN_GAME_DLIGHTS_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

uint32_t roomGetUpperAndLowerPortals(int portalnum1, int portalnum2);
struct light *roomGetLight(int roomnum, int lightnum);
uint8_t roomGetFinalBrightness(int roomnum);
uint8_t roomGetFinalBrightnessForPlayer(int roomnum);
uint8_t roomGetSettledRegionalBrightnessForPlayer(int roomnum);
uint8_t roomGetSettledLocalBrightness(int room);
int roomGetFlashBrightness(int roomnum);
float roomGetLightOpCurFrac(int roomnum);
float roomGetSettledLocalBrightnessFrac(int roomnum);
bool lightGetBboxCentre(int roomnum, uint32_t lightnum, struct coord *pos);
bool lightIsHealthy(int roomnum, int lightnum);
bool lightIsVulnerable(int roomnum, int lightnum);
bool lightIsOn(int roomnum, int lightnum);
void roomSetFlashBrightness(int roomnum, int value);
void roomSetDefaults(struct room *room);
Gfx *lightsSetForRoom(Gfx *gdl, RoomNum roomnum);
Gfx *lightsSetDefault(Gfx *gdl);
void roomInitLights(int roomnum);
bool lightsHandleHit(struct coord *gunpos, struct coord *hitpos, int roomnum);
void roomSetLightsFaulty(int roomnum, int chance);
void roomSetLightBroken(int roomnum, int lightnum);
void lightsReset(void);
void func0f001c0c(void);
void func0f00215c(uint8_t *arg0);
void lightsCalculateRoomDimensions(void);
void func0f00259c(int roomnum);
void func0f002844(int roomnum, float arg1, int arg2, int portalnum);
void func0f002a98(void);
void roomSetLightsOn(int roomnum, int enable);
void roomSetLightOp(int roomnum, int operation, uint8_t br_to, uint8_t br_from, uint8_t duration60);
bool lightTickBroken(int roomnum, int lightnum);
void lightingTick(void);
void lightsConfigureForPerfectDarknessCutscene(void);
void lightsConfigureForPerfectDarknessGameplay(void);
void lightsTickPerfectDarkness(void);
void roomsTickLighting(void);
void lightsTick(void);
void roomFlashLighting(int roomnum, int start, int limit);
void roomFlashLocalLighting(int roomnum, int increment, int limit);
void roomHighlight(int roomnum);
void func0f004c6c(void);
void func0f00505c(void);
float func0f0053d0(int room1, struct coord *arg1, int portal1, int room2, struct coord *arg4, int portal2, float *arg6);
void func0f0056f4(int room1, struct coord *coord1, int room2, struct coord *coord2, int arg4, float *arg5, int arg6);
void func0f005bb0(void);

#endif
