#ifndef IN_GAME_MUSIC_H
#define IN_GAME_MUSIC_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

uint16_t musicGetVolume(void);
void musicSetVolume(uint16_t volume);
bool musicIsTrackState(int tracktype, int state);
int musicGetTrackState(int tracktype);
void musicQueueStartEvent(uint32_t tracktype, uint32_t tracknum, float arg2, uint16_t volume);
void musicQueueStopEvent(int tracktype);
void musicQueueFadeEvent(int tracktype, float arg1, bool keepafterfade);
void musicReset(void);
void musicQueueStopAllEvent(void);
void musicSaveInterval(void);
void musicRestoreInterval(void);
void musicStartPrimary(float arg0);
void musicStartAmbient(float arg0);
bool musicIsAnyPlayerInAmbientRoom(void);
void musicStartNrg(float arg0);
void musicStartTrackAsMenu(int tracknum);
void musicSetStageAndStartMusic(int stagenum);
void musicSetStage(int stagenum);
void musicStop(void);
void musicActivateNrg(void);
void musicDeactivateNrg(void);
void musicStartMenu(void);
void musicEndMenu(void);
void musicStartSoloDeath(void);
void _musicStartMpDeath(float arg0);
void musicStartMpDeath(void);
void musicEndDeath(void);
void musicPlayTrackIsolated(int tracknum);
void musicPlayDefaultTracks(void);
void musicStartTemporaryPrimary(int tracknum);
void musicStartCutscene(int tracknum);
void musicEndCutscene(void);
void musicStartTemporaryAmbient(int tracknum);
void musicEndTemporaryAmbient(void);
void musicSetXReason(int reason, uint32_t minsecs, uint32_t maxsecs);
void musicUnsetXReason(int reason);
void musicTickAmbient(void);

#endif
