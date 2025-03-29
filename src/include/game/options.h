#ifndef IN_GAME_PDOPTIONS_H
#define IN_GAME_PDOPTIONS_H
#include <ultra64.h>
#include <math.h>
#include "data.h"
#include "types.h"

int optionsGetControlMode(int mpchrnum);
void optionsSetControlMode(int mpchrnum, int mode);
int optionsGetContpadNum1(int mpchrnum);
int optionsGetContpadNum2(int mpchrnum);
int optionsGetForwardPitch(int mpchrnum);
int optionsGetAutoAim(int mpchrnum);
int optionsGetLookAhead(int mpchrnum);
int optionsGetAimControl(int mpchrnum);
int optionsGetSightOnScreen(int mpchrnum);
int optionsGetAmmoOnScreen(int mpchrnum);
int optionsGetShowGunFunction(int mpchrnum);
int optionsGetAlwaysShowTarget(int mpchrnum);
int optionsGetShowZoomRange(int mpchrnum);
int optionsGetPaintball(int mpchrnum);
int optionsGetShowMissionTime(int mpchrnum);
uint8_t optionsGetInGameSubtitles(void);
uint8_t optionsGetCutsceneSubtitles(void);
int optionsGetHeadRoll(int mpchrnum);

void optionsSetForwardPitch(int mpchrnum, bool enable);
void optionsSetAutoAim(int mpchrnum, bool enable);
void optionsSetLookAhead(int mpchrnum, bool enable);
void optionsSetAimControl(int mpchrnum, int index);
void optionsSetSightOnScreen(int mpchrnum, bool enable);
void optionsSetAmmoOnScreen(int mpchrnum, bool enable);
void optionsSetShowGunFunction(int mpchrnum, bool enable);
void optionsSetAlwaysShowTarget(int mpchrnum, bool enable);
void optionsSetShowZoomRange(int mpchrnum, bool enable);
void optionsSetPaintball(int mpchrnum, bool enable);
void optionsSetShowMissionTime(int mpchrnum, bool enable);
void optionsSetInGameSubtitles(int enable);
void optionsSetCutsceneSubtitles(int enable);
void optionsSetHeadRoll(int mpchrnum, bool enable);
int optionsGetEffectiveScreenSize(void);
int optionsGetScreenSize(void);
void optionsSetScreenSize(int size);
int optionsGetScreenRatio(void);
void optionsSetScreenRatio(int ratio);
uint8_t optionsGetScreenSplit(void);
void optionsSetScreenSplit(uint8_t split);
uint16_t optionsGetMusicVolume(void);
void optionsSetMusicVolume(uint16_t volume);

#endif
