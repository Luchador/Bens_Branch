#include <ultra64.h>
#include "constants.h"
#include "game/menu.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/options.h"
#include "game/stagemusic.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/music.h"
#include "data.h"
#include "types.h"

#define FADETYPE_STOP  0
#define FADETYPE_PAUSE 1

int g_MusicStageNum;
struct musicevent g_MusicEventQueue[40];
struct seqchannel g_SeqChannels[3];
uint32_t g_AudioXReasonsActive[4];
int g_MusicXReasonMinDurations[4];
int g_MusicXReasonMaxDurations[4];

int g_MenuTrack = -1;
int g_MusicEventQueueLength = 0;
int g_TemporaryPrimaryTrack = -1;
int g_TemporaryAmbientTrack = -1;
int g_MusicSavedInterval240 = -1;

uint32_t g_MusicNextEventId = 0;
bool g_MusicNrgIsActive = false;
bool g_MusicMpDeathIsPlaying = false;
int g_MusicInterval240 = 15;
int g_MusicSleepRemaining240 = 0;
bool g_MusicSoloDeathIsPlaying = false;

uint16_t g_MusicVolume = 0x5000;
int g_MusicDeathTimer240 = 0;   // Counts down 5 seconds while death music plays
int g_MusicAge60 = 0;           // The current age of the MP track being played
int g_MusicLife60 = TICKS(120); // The max age of any MP track (this value is changed in MP code)
int g_MusicSilenceTimer60 = 0;  // Counts down the 2 second silence between MP track changes
bool g_MusicDisableMpDeath = false;


uint16_t musicGetVolume(void)
{
	uint32_t volume;

	if (g_Vars.stagenum == STAGE_CREDITS) {
		return 0x5000;
	}

	if (g_MusicVolume < 0x5000) {
		volume = g_MusicVolume;
	} else {
		volume = 0x5000;
	}

	return volume;
}

void musicSetVolume(uint16_t volume)
{
	int i;

	if (volume > 0x5000) {
		volume = 0x5000;
	}

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype != TRACKTYPE_NONE && g_SeqChannels[i].tracktype != TRACKTYPE_AMBIENT) {
			seqSetVolume(&g_SeqInstances[i], volume);
		}
	}

	g_MusicVolume = volume;
}

bool musicIsTrackState(int tracktype, int state)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			switch (state) {
			case AL_STOPPED:
				return !g_SeqChannels[i].inuse;
			case AL_PLAYING:
				return g_SeqChannels[i].inuse;
			case AL_STOPPING:
				return g_SeqChannels[i].keepafterfade;
			}
		}
	}

	return false;
}

int musicGetTrackState(int tracktype)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			if (g_SeqChannels[i].keepafterfade) {
				return AL_STOPPING;
			}

			if (g_SeqChannels[i].inuse) {
				return AL_PLAYING;
			}
		}
	}

	return AL_STOPPED;
}

int musicGetChannelByTrackType(int tracktype)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_SeqChannels); i++) {
		if (g_SeqChannels[i].tracktype == tracktype) {
			return i;
		}
	}

	return -1;
}

void musicQueueStartEvent(uint32_t tracktype, uint32_t tracknum, float arg2, uint16_t volume)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].tracknum = tracknum;
		g_MusicEventQueue[g_MusicEventQueueLength].unk0c = arg2;
		g_MusicEventQueue[g_MusicEventQueueLength].volume = volume;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_PLAY;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicQueueStopEvent(int tracktype)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_STOP;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicQueueFadeEvent(int tracktype, float arg1, bool keepafterfade)
{
	if (!g_SndDisabled) {
		g_MusicEventQueue[g_MusicEventQueueLength].tracktype = tracktype;
		g_MusicEventQueue[g_MusicEventQueueLength].unk0c = arg1;
		g_MusicEventQueue[g_MusicEventQueueLength].keepafterfade = keepafterfade;
		g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_FADE;
		g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
		g_MusicEventQueue[g_MusicEventQueueLength].numattempts = 0;
		g_MusicEventQueue[g_MusicEventQueueLength].failcount = 0;
		g_MusicEventQueueLength++;
	}
}

void musicReset(void)
{
	int i;

	if (!g_SndDisabled) {
		for (i = 0; i < ARRAYCOUNT(g_AudioXReasonsActive); i++) {
			g_AudioXReasonsActive[i] = 0;
			g_MusicXReasonMinDurations[i] = 0;
			g_MusicXReasonMaxDurations[i] = 0;
		}

		musicSaveInterval();
		musicQueueStopAllEvent();
		musicRestoreInterval();

		g_MusicSoloDeathIsPlaying = false;
		g_MusicDeathTimer240 = 0;
		g_MenuTrack = -1;
		g_TemporaryPrimaryTrack = -1;
		g_TemporaryAmbientTrack = -1;
		g_MusicNrgIsActive = false;
	}
}

void musicQueueStopAllEvent(void)
{
	g_MusicEventQueue[0].tracktype = TRACKTYPE_6;

	g_MusicEventQueue[0].eventtype = MUSICEVENTTYPE_STOPALL;
	g_MusicEventQueue[0].id = g_MusicNextEventId++;
	g_MusicEventQueue[0].numattempts = 0;
	g_MusicEventQueue[0].failcount = 0;

	g_MusicEventQueueLength = 1;

	musicTickEvents();
}

void musicSaveInterval(void)
{
	g_MusicSavedInterval240 = g_MusicInterval240;
	g_MusicInterval240 = 0;
}

void musicRestoreInterval(void)
{
	g_MusicEventQueue[g_MusicEventQueueLength].tracktype = TRACKTYPE_6;
	g_MusicEventQueue[g_MusicEventQueueLength].eventtype = MUSICEVENTTYPE_SETINTERVAL;
	g_MusicEventQueue[g_MusicEventQueueLength].id = g_MusicNextEventId++;
	g_MusicEventQueue[g_MusicEventQueueLength].timer240 = g_MusicSavedInterval240;
	g_MusicEventQueueLength++;

	// @bug: This should be modifying the interval queue item, not the first queue item
	g_MusicEventQueue[0].numattempts = 0;
	g_MusicEventQueue[0].failcount = 0;
}

#define PRIMARYTRACK() (g_TemporaryPrimaryTrack != -1 ? g_TemporaryPrimaryTrack : stageGetPrimaryTrack(g_MusicStageNum))
#define AMBIENTTRACK() (g_TemporaryAmbientTrack != -1 ? g_TemporaryAmbientTrack : stageGetAmbientTrack(g_MusicStageNum))

void musicStartPrimary(float arg0)
{
	if (PRIMARYTRACK() >= 0) {
		musicQueueStartEvent(TRACKTYPE_PRIMARY, PRIMARYTRACK(), arg0, musicGetVolume());
	}
}

void musicStartAmbient(float arg0)
{
	int pass = false;

	if (AMBIENTTRACK() >= 0) {
		if (g_TemporaryAmbientTrack != -1) {
			pass = true;
		} else if (musicIsAnyPlayerInAmbientRoom()) {
			if (g_Vars.tickmode != TICKMODE_CUTSCENE && AMBIENTTRACK() != stageGetAmbientTrack(g_MusicStageNum)) {
				musicQueueStopEvent(TRACKTYPE_AMBIENT);
				musicStartTemporaryAmbient(stageGetAmbientTrack(g_MusicStageNum));
				return;
			}

			pass = true;
		}
	}

	if (pass) {
		switch (musicGetTrackState(TRACKTYPE_AMBIENT)) {
		case AL_STOPPED:
		case AL_STOPPING:
			musicQueueStartEvent(TRACKTYPE_AMBIENT, AMBIENTTRACK(), arg0, VOLUME(g_SfxVolume));
			break;
		}
	}
}

bool musicIsAnyPlayerInAmbientRoom(void)
{
	int i;

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		return false;
	}

	if (lvIsPaused()) {
		return false;
	}

	if (g_MusicSoloDeathIsPlaying) {
		return false;
	}

	if (g_MusicNrgIsActive && g_MusicMpDeathIsPlaying) {
		return false;
	}

	for (i = 0; i < PLAYERCOUNT(); i++) {
		if (g_Vars.players[i]->prop
				&& g_Vars.players[i]->prop->rooms
				&& g_Vars.players[i]->prop->rooms[0] != -1) {
			bool hasflag;

			if (g_Rooms[g_Vars.players[i]->prop->rooms[0]].flags & ROOMFLAG_PLAYAMBIENTTRACK) {
				hasflag = true;
			} else {
				hasflag = false;
			}

			if (hasflag) {
				return true;
			}
		}
	}

	return false;
}

void musicStartNrg(float arg0)
{
	musicQueueStartEvent(TRACKTYPE_NRG, stageGetNrgTrack(g_MusicStageNum), arg0, musicGetVolume());
}

/**
 * Play a specific track as a menu track.
 *
 * Used in credits and the soundtrack dialog in MP setup.
 */
void musicStartTrackAsMenu(int tracknum)
{
	if (tracknum != g_MenuTrack) {
		musicQueueStopEvent(TRACKTYPE_MENU);
		musicQueueStopEvent(TRACKTYPE_DEATH);
		musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.5f, FADETYPE_PAUSE);
		musicQueueFadeEvent(TRACKTYPE_NRG, 0.5f, FADETYPE_PAUSE);
		musicQueueFadeEvent(TRACKTYPE_AMBIENT, 0.5f, FADETYPE_PAUSE);
		musicQueueStartEvent(TRACKTYPE_MENU, tracknum, 0, musicGetVolume());
	}

	g_MenuTrack = tracknum;
}

/**
 * Used when starting combat simulator matches.
 */
void musicSetStageAndStartMusic(int stagenum)
{
	g_MusicStageNum = stagenum;

	musicStartPrimary(0);

	if (stageGetAmbientTrack(g_MusicStageNum) >= 0) {
		musicStartAmbient(0);
	}
}

/**
 * Used for solo missions.
 */
void musicSetStage(int stagenum)
{
	g_MusicStageNum = stagenum;
}

void musicStop(void)
{
	musicSaveInterval();
	musicQueueStopAllEvent();
	musicRestoreInterval();
}

void musicActivateNrg(void)
{
	if (!g_MusicNrgIsActive)
	{
		if (stageGetNrgTrack(g_MusicStageNum) >= 0) {
			musicQueueStopEvent(TRACKTYPE_NRG);
			musicQueueStopEvent(TRACKTYPE_MENU);
			musicQueueStopEvent(TRACKTYPE_DEATH);
			musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.5, FADETYPE_PAUSE);
			musicStartNrg(0);

			g_MusicNrgIsActive = true;
		}
	}
}

void musicDeactivateNrg(void)
{
	if (g_MusicNrgIsActive)
	{
		musicQueueStopEvent(TRACKTYPE_MENU);
		musicQueueStopEvent(TRACKTYPE_DEATH);
		musicQueueFadeEvent(TRACKTYPE_NRG, 1, FADETYPE_STOP);

		if (g_Vars.dontplaynrg == false) {
			musicStartPrimary(0.5);
		}

		g_MusicNrgIsActive = false;
	}
}

/**
 * Called in many places when opening a pause menu.
 */
void musicStartMenu(void)
{
	musicStartTrackAsMenu(menuChooseMusic());
}

void musicEndMenu(void)
{
	musicQueueFadeEvent(TRACKTYPE_MENU, 1, FADETYPE_STOP);

	if (musicIsTrackState(TRACKTYPE_NRG, AL_PLAYING)) {
		musicStartNrg(1);
	} else {
		musicStartPrimary(1);
	}

	g_MenuTrack = -1;
}

void musicStartSoloDeath(void)
{
	g_MusicSoloDeathIsPlaying = true;

	musicSaveInterval();
	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicQueueStartEvent(TRACKTYPE_PRIMARY, MUSIC_DEATH_SOLO, 0, VOLUME(g_SfxVolume) > musicGetVolume() ? VOLUME(g_SfxVolume) : musicGetVolume());
	musicRestoreInterval();
}

void _musicStartMpDeath(float arg0)
{
	musicSaveInterval();
	musicQueueStartEvent(TRACKTYPE_DEATH, MUSIC_DEATH_MP, arg0, VOLUME(g_SfxVolume) > musicGetVolume() ? VOLUME(g_SfxVolume) : musicGetVolume());
	musicRestoreInterval();
}

void musicStartMpDeath(void)
{
	if (g_MusicDisableMpDeath) {
		return;
	}

	musicSaveInterval();
	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);

	if (g_MusicNrgIsActive) {
		musicQueueFadeEvent(TRACKTYPE_NRG, 0.1f, FADETYPE_PAUSE);
	} else {
		musicQueueFadeEvent(TRACKTYPE_PRIMARY, 0.1f, FADETYPE_PAUSE);
	}

	_musicStartMpDeath(0);

	g_MusicDeathTimer240 = TICKS(1200);
	g_MusicMpDeathIsPlaying = true;

	musicRestoreInterval();
}

void musicEndDeath(void)
{
	musicQueueFadeEvent(TRACKTYPE_DEATH, 2, FADETYPE_STOP);

	if (g_MusicNrgIsActive) {
		musicStartNrg(2);
	} else {
		musicStartPrimary(2);
	}

	g_MusicMpDeathIsPlaying = false;
}

/**
 * Stop all other music and play the given track.
 *
 * It's used by the AI scripting language, specifically for CI training and
 * the Skedar King battle.
 *
 * The track type used is primary.
 */
void musicPlayTrackIsolated(int tracknum)
{
	musicSaveInterval();
	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicQueueStartEvent(TRACKTYPE_PRIMARY, tracknum, 0, musicGetVolume());
	musicRestoreInterval();
}

/**
 * Restart the level's default tracks after using the isolated track above.
 *
 * It's used by the AI scripting language, specifically when ending CI training.
 */
void musicPlayDefaultTracks(void)
{
	musicQueueStopEvent(TRACKTYPE_PRIMARY);
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
	musicStartPrimary(0.5f);
}

/**
 * Used by the title screen, as well as AF1's NRG theme which never ends.
 */
void musicStartTemporaryPrimary(int tracknum)
{
	musicQueueStopEvent(TRACKTYPE_PRIMARY);

	g_TemporaryPrimaryTrack = tracknum;

	musicStartPrimary(0.5f);
}

/**
 * Used by AI scripting on each stage.
 *
 * The cutscene track is played with a primary tracktype.
 *
 * The NRG theme will not play while a cutscene theme is active.
 */
void musicStartCutscene(int tracknum)
{
	uint32_t volume;

	musicQueueStopEvent(TRACKTYPE_MENU);
	musicQueueStopEvent(TRACKTYPE_DEATH);
	musicUnsetXReason(-1);
	musicQueueStopEvent(TRACKTYPE_NRG);
	musicQueueStopEvent(TRACKTYPE_PRIMARY);

	if (g_SfxVolume < musicGetVolume()) {
		volume = musicGetVolume();
	} else {
		volume = g_SfxVolume;
	}

	musicQueueStartEvent(TRACKTYPE_PRIMARY, tracknum, 0, volume);

	g_Vars.dontplaynrg = true;
}

/**
 * Used by AI scripting on each stage.
 */
void musicEndCutscene(void)
{
	g_Vars.dontplaynrg = false;

	if (!g_IsTitleDemo) {
		musicQueueStopEvent(TRACKTYPE_PRIMARY);
		musicQueueStopEvent(TRACKTYPE_AMBIENT);
		musicStartPrimary(0.5f);
	}
}

/**
 * Used by AI scripting, and only to set the ambient track during the Defection
 * intro and Extraction outro to traffic noises.
 */
void musicStartTemporaryAmbient(int tracknum)
{
	g_TemporaryAmbientTrack = tracknum;
	musicQueueStopEvent(TRACKTYPE_AMBIENT);

	musicQueueStartEvent(TRACKTYPE_AMBIENT, tracknum, 0, VOLUME(g_SfxVolume));
}

void musicEndTemporaryAmbient(void)
{
	g_TemporaryAmbientTrack = -1;
	musicQueueStopEvent(TRACKTYPE_AMBIENT);
}

void musicSetXReason(int reason, uint32_t minsecs, uint32_t maxsecs)
{
	if (g_AudioXReasonsActive[reason] == false) {
		g_AudioXReasonsActive[reason] = true;
		g_MusicXReasonMinDurations[reason] = minsecs * TICKS(240);
		g_MusicXReasonMaxDurations[reason] = maxsecs * TICKS(240);
	}
}

void musicUnsetXReason(int reason)
{
	int i;

	if (reason >= 0) {
		g_AudioXReasonsActive[reason] = false;
	} else {
		for (i = 0; i < ARRAYCOUNT(g_AudioXReasonsActive); i++) {
			g_AudioXReasonsActive[i] = false;
			g_MusicXReasonMinDurations[i] = 0;
			g_MusicXReasonMaxDurations[i] = 0;
		}

		if (g_MusicNrgIsActive) {
			musicDeactivateNrg();
		}
	}
}

/**
 * Called by musicTick every 0.25 seconds.
 */
void musicTickAmbient(void)
{
	if (g_TemporaryAmbientTrack == -1) {
		if (musicIsAnyPlayerInAmbientRoom()) {
			musicStartAmbient(1);
		} else if (musicGetTrackState(TRACKTYPE_AMBIENT) == AL_PLAYING) {
			musicQueueFadeEvent(TRACKTYPE_AMBIENT, 1, FADETYPE_PAUSE);
		}
	} else if (stageGetAmbientTrack(g_MusicStageNum) >= 0) {
		musicStartAmbient(1);
	}
}