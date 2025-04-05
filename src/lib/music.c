#include <ultra64.h>
#include <n_libaudio.h>
#include "constants.h"
#include "game/music.h"
#include "game/zbuf.h"
#include "game/stagemusic.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/music.h"
#include "lib/lib_39c80.h"
#include "data.h"
#include "types.h"

#define RESULT_FAIL     0
#define RESULT_OK_NEXT  1
#define RESULT_OK_BREAK 2

const uint8_t var70053ca0[] = {0, 0, 0, 0, 0, 5};

int g_MusicNextAmbientTick240 = -1;

int musicHandlePlayEvent(struct musicevent *event, int result)
{
	int i;
	uint8_t value;
	int j;
	int index;

	// Check if this tracktype is currently in use. If it is then that's
	// an error - the caller should have stopped the existing track first.
	for (i = 0; i < 3; i++) {
		if (event->tracktype == g_SeqChannels[i].tracktype && n_alCSPGetState(g_SeqInstances[i].seqp) == AL_PLAYING) {
			value = event->tracktype == TRACKTYPE_AMBIENT ? 24 : 32;

			for (j = 0; j < 16; j++) {
				func00039e5c(g_SeqInstances[i].seqp, j, 0xff, value);
			}

			g_SeqChannels[i].keepafterfade = false;
			g_SeqChannels[i].unk0c = 0;

			event->eventtype = 0;
			result = RESULT_OK_BREAK;
			break;
		}
	}

	if (result == RESULT_FAIL) {
		// Find an unused channel
		for (i = 0; i < 3; i++) {
			/**
			 * @bug: When adding a new track, the seqp's state remains at AL_STOPPED
			 * and is only changed to AL_PLAYING once the audio thread has run.
			 * Scheduling two sequences in quick succession will cause it to choose
			 * the same sequence player if the audio thread hasn't run between
			 * the two calls and updated the state.
			 *
			 * With IDO, the compiled code is so inefficient that the audio thread
			 * is likely to run between two consecutive calls. However it does
			 * still happen occasionally. Eg. sometimes when unpausing the stage's
			 * main theme does not resume.
			 *
			 * For GCC, it's more likely to occur, so we introduce a new state:
			 * AL_STARTING. This is assigned to the sequence player in seqPlay.
			 */
			if (n_alCSPGetState(g_SeqInstances[i].seqp) == AL_STOPPED) {
				if (seqPlay(&g_SeqInstances[i], event->tracknum)) {
					seqSetVolume(&g_SeqInstances[i], event->volume);

					g_SeqChannels[i].tracktype = event->tracktype;
					g_SeqChannels[i].inuse = true;
					g_SeqChannels[i].keepafterfade = false;
					g_SeqChannels[i].unk0c = 0;

					result = RESULT_OK_BREAK;
				}
				break;
			}
		}

		if (result == RESULT_FAIL) {
			index = -1;

			for (i = 0; i < 3; i++) {
				if ((g_SeqChannels[i].tracktype == TRACKTYPE_NONE || event->tracktype == g_SeqChannels[i].tracktype)
						&& n_alCSPGetState(g_SeqInstances[i].seqp) != AL_STOPPED) {
					index = i;
					break;
				}
			}

			if (index == -1) {
				if (event->failcount >= 3) {
					for (i = 0; i < 3; i++) {
						if (g_SeqChannels[i].tracktype == TRACKTYPE_AMBIENT
								&& n_alCSPGetState(g_SeqInstances[i].seqp) != AL_STOPPED) {
							index = i;
							break;
						}
					}
				}
			}

			if (index != -1) {
				n_alSeqpStop((N_ALSeqPlayer *)g_SeqInstances[index].seqp);

				g_SeqChannels[index].tracktype = TRACKTYPE_NONE;
				g_SeqChannels[index].inuse = false;
				g_SeqChannels[index].keepafterfade = false;
				g_SeqChannels[index].unk0c = 0;
			} else {
				event->failcount++;

				if (event->failcount >= 6) {
					result = RESULT_OK_BREAK;
				}
			}
		}
	}

	return result;
}

int musicHandleStopEvent(struct musicevent *event, int result)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (event->tracktype == g_SeqChannels[i].tracktype) {
			n_alSeqpStop((N_ALSeqPlayer *)g_SeqInstances[i].seqp);

			g_SeqChannels[i].tracktype = TRACKTYPE_NONE;
			g_SeqChannels[i].inuse = false;
			g_SeqChannels[i].keepafterfade = false;
			g_SeqChannels[i].unk0c = 0;

			break;
		}
	}

	return RESULT_OK_NEXT;
}

int musicHandleFadeEvent(struct musicevent *event, int result)
{
	int i;
	int j;

	for (i = 0; i < 3; i++) {
		if (event->tracktype == g_SeqChannels[i].tracktype && g_SeqChannels[i].inuse) {
			for (j = 0; j < 16; j++) {
				func00039e5c(g_SeqInstances[i].seqp, j, var70053ca0[event->tracktype], 32);
			}

			g_SeqChannels[i].inuse = event->keepafterfade;
			g_SeqChannels[i].keepafterfade = event->keepafterfade;
			g_SeqChannels[i].unk0c = g_SeqInstances[i].seqp->chanState[0].unk0d;
		}
	}

	return RESULT_OK_NEXT;
}

int musicHandleStopAllEvent(int result)
{
	int i;

	for (i = 0; i < 3; i++) {
		n_alSeqpStop((N_ALSeqPlayer *)g_SeqInstances[i].seqp);

		g_SeqChannels[i].tracktype = 0;
		g_SeqChannels[i].inuse = false;
		g_SeqChannels[i].keepafterfade = false;
		g_SeqChannels[i].unk0c = 0;
	}

	return RESULT_OK_NEXT;
}

int musicHandleSetIntervalEvent(struct musicevent *event, int result)
{
	g_MusicInterval240 = event->timer240;
	return RESULT_OK_NEXT;
}

// Mismatch: In the "Remove the marked events" loop, goal reloads
// g_MusicEventQueueLength if the if statement passed. Suspect there's some code
// being optimised out that overwrites a1 or wrote to g_MusicEventQueueLength.
// The code below uses += 0 to get the mismatch down to one instruction,
void musicTickEvents(void)
{
	int i;
	int j;
	int result;
	struct musicevent *event;

	if (!g_SndDisabled) {
		if (g_MusicEventQueueLength);

		// Release channels if their track has finished fading out
		for (i = 0; i < 3; i++) {
			if (!g_SeqChannels[i].inuse && n_alCSPGetState(g_SeqInstances[i].seqp) == AL_PLAYING) {
				if (g_SeqInstances[i].seqp->chanState[0].unk0d <= var70053ca0[g_SeqChannels[i].tracktype]) {
					n_alSeqpStop((N_ALSeqPlayer *)g_SeqInstances[i].seqp);

					g_SeqChannels[i].tracktype = TRACKTYPE_NONE;
					g_SeqChannels[i].inuse = false;
					g_SeqChannels[i].keepafterfade = false;
					g_SeqChannels[i].unk0c = 0;
				} else if (g_SeqInstances[i].seqp->chanState[0].unk0d == g_SeqChannels[i].unk0c) {
					n_alSeqpStop((N_ALSeqPlayer *)g_SeqInstances[i].seqp);

					g_SeqChannels[i].tracktype = TRACKTYPE_NONE;
					g_SeqChannels[i].inuse = false;
					g_SeqChannels[i].keepafterfade = false;
					g_SeqChannels[i].unk0c = 0;
				}
			}
		}

		// Figure out which events can be removed from the queue due to later
		// events superseding them. This loop just marks those events as
		// removable by setting their tracktype to none.
		for (i = g_MusicEventQueueLength - 1; i >= 0; i--) {
			event = &g_MusicEventQueue[i];

			if (event->eventtype == MUSICEVENTTYPE_SETINTERVAL) {
				continue;
			}

			if (event->tracktype == TRACKTYPE_NONE) {
				continue;
			}

			for (j = i - 1; j >= 0; j--) {
				struct musicevent *earlier = &g_MusicEventQueue[j];

				if (event->eventtype == MUSICEVENTTYPE_STOPALL) {
					earlier->tracktype = TRACKTYPE_NONE;
					continue;
				}

				if (earlier->eventtype == MUSICEVENTTYPE_SETINTERVAL) {
					continue;
				}

				if (earlier->tracktype == TRACKTYPE_NONE) {
					continue;
				}

				if (earlier->tracktype == event->tracktype) {
					switch (event->eventtype) {
					case MUSICEVENTTYPE_STOP:
						earlier->tracktype = TRACKTYPE_NONE;
						break;
					case MUSICEVENTTYPE_PLAY:
						switch (earlier->eventtype) {
						case MUSICEVENTTYPE_PLAY:
						case MUSICEVENTTYPE_FADE:
							earlier->tracktype = TRACKTYPE_NONE;
							break;
						}
						break;
					case MUSICEVENTTYPE_FADE:
						if (earlier->eventtype == MUSICEVENTTYPE_FADE) {
							earlier->tracktype = TRACKTYPE_NONE;
						}
						break;
					}
				}
			}
		}

		// Remove the marked events from the queue, shift the remaining
		// events forward and recount the queue length.
		for (i = 0, j = 0; i < g_MusicEventQueueLength; i++) {
			if (g_MusicEventQueue[i].tracktype) {
				g_MusicEventQueue[j] = g_MusicEventQueue[i];
				j++;

				g_MusicEventQueueLength += 0;
			}
		}

		g_MusicEventQueueLength = j;

		// Process the queue, but only on certain timer intervals,
		// or every frame if the interval timer is disabled
		event = &g_MusicEventQueue[0];

		if (g_MusicInterval240 == 0 || g_MusicSleepRemaining240 < g_Vars.diffframe240) {
			g_MusicSleepRemaining240 = g_MusicInterval240;

			while (g_MusicEventQueueLength) {
				event->numattempts++;

				result = RESULT_FAIL;

				switch (event->eventtype) {
				case MUSICEVENTTYPE_PLAY:
					result = musicHandlePlayEvent(event, result);
					break;
				case MUSICEVENTTYPE_STOP:
					result = musicHandleStopEvent(event, result);
					break;
				case MUSICEVENTTYPE_FADE:
					result = musicHandleFadeEvent(event, result);
					break;
				case MUSICEVENTTYPE_STOPALL:
					result = musicHandleStopAllEvent(result);
					break;
				case MUSICEVENTTYPE_SETINTERVAL:
					result = musicHandleSetIntervalEvent(event, result);
					break;
				}

				if (result);

				if (result != RESULT_FAIL) {
					// Remove the item from the queue
					g_MusicEventQueueLength--;

					for (i = 0; i < g_MusicEventQueueLength; i++) {
						g_MusicEventQueue[i] = g_MusicEventQueue[i + 1];
					}

					// Break from processing further events on this frame
					// if requested
					if (result == RESULT_OK_BREAK) {
						break;
					}
				} else {
					break;
				}
			}
		}

		if (g_MusicInterval240) {
			g_MusicSleepRemaining240 -= g_Vars.diffframe240;
		} else {
			g_MusicSleepRemaining240 = 0;
		}
	}
}

void musicTick(void)
{
	int i;
	bool playnrg = false;

	if (!g_SndDisabled) {
		if (g_MusicDeathTimer240 > 0
				&& (g_Vars.normmplayerisrunning
					|| (g_Vars.antiplayernum >= 0 && !g_Vars.bond->isdead)
					|| (g_Vars.coopplayernum >= 0 && (!g_Vars.bond->isdead || !g_Vars.coop->isdead)))) {
			// Someone is dying in MP, or anti is dying, or *one* person is dying in coop
			g_MusicSilenceTimer60 = 0;
			g_MusicDeathTimer240 -= g_Vars.lvupdate240;

			if (g_MusicDeathTimer240 <= 0) {
				musicEndDeath();

				// The death is complete. Are we due to start a new track?
				if (g_MpEnableMusicSwitching && g_Vars.normmplayerisrunning && g_MusicLife60 < g_MusicAge60) {
					g_MusicAge60 = 0;
					musicQueueStopEvent(TRACKTYPE_MENU);
					musicQueueStopEvent(TRACKTYPE_DEATH);
					musicQueueStopEvent(TRACKTYPE_PRIMARY);
					musicQueueStartEvent(TRACKTYPE_PRIMARY, stageGetPrimaryTrack(g_MusicStageNum), 0, musicGetVolume());
				}
			}
		} else if (g_MpEnableMusicSwitching && g_Vars.normmplayerisrunning && g_MusicLife60 < g_MusicAge60) {
			// Due to start a new track. Fade out the old one,
			// then start a 2 second timer before starting the new one.
			g_MusicAge60 = 0;
			musicQueueFadeEvent(TRACKTYPE_PRIMARY, 2, true);
			g_MusicSilenceTimer60 = TICKS(120);
		}

		if (g_MpEnableMusicSwitching && g_Vars.normmplayerisrunning) {
			g_MusicAge60 += g_Vars.diffframe60;

			// If the silence timer is set, it means we're transitioning between
			// songs in multiplayer. Tick the timer down, and when it reaches
			// zero start a new track.
			if (g_MusicSilenceTimer60 > 0) {
				g_MusicSilenceTimer60 -= g_Vars.diffframe60;

				if (g_MusicSilenceTimer60 <= 0) {
					musicQueueStopEvent(TRACKTYPE_MENU);
					musicQueueStopEvent(TRACKTYPE_DEATH);
					musicQueueStopEvent(TRACKTYPE_PRIMARY);
					musicQueueStartEvent(TRACKTYPE_PRIMARY, stageGetPrimaryTrack(g_MusicStageNum), 0, musicGetVolume());
				}
			}
		}

		// Handle stopping of NRG tune
		for (i = 0; i < 4; i++) {
			if (g_AudioXReasonsActive[i] || g_MusicXReasonMinDurations[i] > 0) {
				if (g_MusicXReasonMinDurations[i] >= g_Vars.lvupdate240) {
					g_MusicXReasonMinDurations[i] -= g_Vars.lvupdate240;
				} else {
					g_MusicXReasonMinDurations[i] = 0;
				}

				if (g_MusicXReasonMaxDurations[i] != 0) {
					if (g_MusicXReasonMaxDurations[i] >= g_Vars.lvupdate240) {
						g_MusicXReasonMaxDurations[i] -= g_Vars.lvupdate240;
					} else {
						g_MusicXReasonMaxDurations[i] = 0;
					}

					if (g_MusicXReasonMaxDurations[i] != 0) {
						if (g_AudioXReasonsActive[i] || g_MusicXReasonMinDurations[i]) {
							playnrg = true;
						}
					} else {
						g_AudioXReasonsActive[i] = 0;
					}
				}
			}
		}

		if (g_Vars.lvupdate240 != 0) {
			if (g_MusicNrgIsActive) {
				if (!playnrg) {
					musicDeactivateNrg();
				}
			} else {
				if (playnrg && !g_Vars.dontplaynrg) {
					musicActivateNrg();
				}
			}
		}

		// Check if the player is in an ambient room every 0.25 seconds
		if (g_Vars.lvupdate240 > g_MusicNextAmbientTick240) {
			musicTickAmbient();
			g_MusicNextAmbientTick240 = TICKS(60);
		} else {
			g_MusicNextAmbientTick240 -= g_Vars.lvupdate240;
		}

		musicTickEvents();
	}
}

bool musicIsTrackTypePlaying(int tracktype)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (tracktype == g_SeqChannels[i].tracktype && n_alCSPGetState(g_SeqInstances[i].seqp) == AL_PLAYING) {
			return true;
		}
	}

	return false;
}