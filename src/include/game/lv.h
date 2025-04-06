#pragma once

#include "data.h"
#include "types.h"

// game runs at 60+, so slomo = 1/2 of whatever framerate we're running at
#define LV_SLOMO_TICK_CAP 1 // capping at >= 240fps would make it explode
#define LV_SLOMO_TICK_RATE (g_Vars.lvupdate240 / 2)

void lvInit(void);
void lvResetMiscSfx(void);
int lvGetMiscSfxIndex(uint32_t arg0);
void lvSetMiscSfxState(uint32_t type, bool play);
void lvUpdateMiscSfx(void);
void lvReset(int stagenum);
Gfx *lvRenderFade(Gfx *gdl);
void lvFadeReset(void);
bool lvUpdateTrackedProp(struct trackedprop *trackedprop, int index);
void lvFindThreatsForProp(struct prop *prop, bool inchild, struct coord *playerpos, bool *activeslots, float *param_5);
void lvPositionThreatBox(struct prop *prop, bool inchild, struct coord *playerpos, bool *activeslots, float *distances);
void lvFindThreats(void);
Gfx *lvRender(Gfx *gdl);
void lvUpdateSoloHandicaps(void);
int sub54321(int value);
void lvUpdateCutsceneTime(void);
int lvGetSlowMotionType(void);
void lvTick(void);
void lvTickPlayer(void);
void lvCheckPauseStateChanged(void);
void lvSetPaused(bool paused);
void lvConfigureFade(uint32_t color, int16_t num_frames);
bool lvIsFadeActive(void);
void lvStop(void);
bool lvIsPaused(void);
int lvGetDifficulty(void);
void lvSetDifficulty(int difficulty);
void lvSetMpTimeLimit60(uint32_t limit);
void lvSetMpScoreLimit(uint32_t limit);
void lvSetMpTeamScoreLimit(uint32_t limit);
float lvGetStageTimeInSeconds(void);
int lvGetStageTime60(void);