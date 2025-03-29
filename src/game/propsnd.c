#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/chraicommands.h"
#include "game/dlights.h"
#include "game/propsnd.h"
#include "game/hudmsg.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"

struct pschannel *g_PsChannels = NULL;

uint32_t g_AudioPrevUuid = 0;

uint32_t var8006ae2c = 0;
uint32_t var8006ae34 = 0;
bool g_PsPrintAll = false;

bool g_PsPrintFlagged = false;
int16_t var8006ae50 = -1;

#define CHANNELCOUNT()         (40)
#define CHANNEL_IS_AI(channel) (channel >= 0 && channel <= 7)
#define CHANNEL_HEAP_FIRST     CHANNEL_8

bool psPropHasSoundWithContext(struct prop *prop, int type)
{
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		if ((g_PsChannels[i].flags & PSFLAG_FREE) == 0
				&& g_PsChannels[i].prop == prop
				&& (g_PsChannels[i].type == type || type == PSTYPE_GENERAL)) {
			return true;
		}
	}

	return false;
}

void psStopSound(struct prop *prop, int type, uint16_t flags)
{
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		struct pschannel *channel = &g_PsChannels[i];

		if ((channel->flags & PSFLAG_FREE) == 0 && channel->prop == prop) {
			if (!channel->flags || !flags || (flags & channel->flags)) {
				if (channel->type == type || type == PSTYPE_GENERAL) {
					psStopChannel(i);
				}
			}
		}
	}
}

int psCalculateVolumeFromDistance(float playerdist, float dist1, float dist2, float dist3, int fullvolume)
{
	int result = 0;

	if (playerdist < dist3) {
		if (dist1 > 5501) {
			dist1 = 5501;
		}

		if (dist2 > 5801) {
			dist2 = 5801;
		}

		if (dist3 > 6000) {
			dist3 = 6000;
		}

		if (playerdist < dist1) {
			// Within dist1 -> vol full
			result = fullvolume;
		} else if (playerdist < dist2) {
			// Range dist1 to dist2 -> scale down using curve
			result = fullvolume - (int) (sqrtf((playerdist - dist1) / (dist2 - dist1)) * (fullvolume - 1000.0f));
		} else {
			// Range dist2 to dist3 -> scale to zero linearly
			result = (dist3 - playerdist) * 1000.0f / (dist3 - dist2);
		}
	}

	if (result > AL_VOL_FULL) {
		result = AL_VOL_FULL;
	}

	if (result < 40) {
		result = 0;
	}

	return result;
}

int psGetVolume(int channelnum)
{
	return CHANNEL_IS_AI(channelnum) ? g_PsChannels[channelnum].currentvol : 0;
}

/**
 * Play the given soundnum for the prop, provided the prop doesn't already have
 * a sound playing with this type.
 */
void psCreateIfNotDupe(struct prop *prop, int16_t soundnum, int type)
{
	int i;

	if (psCalculateVol(&prop->pos, 400, 2500, 3000, prop->rooms, soundnum, AL_VOL_FULL, 0) != 0) {
		for (i = CHANNEL_HEAP_FIRST; i < CHANNELCOUNT(); i++) {
			if ((g_PsChannels[i].flags & PSFLAG_FREE) == 0
					&& g_PsChannels[i].prop == prop
					&& g_PsChannels[i].type == type) {
				return;
			}
		}

		psCreate(NULL, prop, soundnum, -1, -1, PSFLAG_REPEATING, 0, type, 0, -1.0f, 0, -1, -1.0f, -1.0f, -1.0f);
	}
}

/**
 * Stop the sound that's playing in the given channel.
 * The channel remains allocated and configured.
 */
void psStopChannel(int channelnum)
{
	struct pschannel *channel = &g_PsChannels[channelnum];

	channel->flags2 |= PSFLAG2_STOPPED;

	if (channel->flags & PSFLAG_FORHUDMSG) {
		hudmsgsHideByChannel(channelnum);
	}

	if (channel->flags & PSFLAG_FORPROP) {
		propDecrementSoundCount(channel->prop);
	}

	if (channel->flags & PSFLAG_ISMP3) {
		sndStopMp3(channel->soundnum26);
	} else if (channel->audiohandle && sndGetState(channel->audiohandle) != AL_STOPPED) {
		audioStop(channel->audiohandle);
	}
}

int psGetSubtitleOpacity(int channelnum)
{
	if (channelnum == -1) {
		return 1;
	}

	if ((g_PsChannels[channelnum].flags & PSFLAG_FREE) == 0) {
		int value = g_PsChannels[channelnum].currentvol;

		if (value == -1 || value > 200) {
			int tmp = g_PsChannels[channelnum].currentvol - 200;
			int opacity = tmp * 255 / 15800;

			if (opacity > 255) {
				opacity = 255;
			}

			return opacity;
		}
	}

	return 0;
}

void psTickChannel(int channelnum)
{
	struct pschannel *channel = &g_PsChannels[channelnum];

	if ((channel->flags2 & PSFLAG2_STOPPED) == 0
			&& channel->type != PSTYPE_MARKER
			&& ((channel->audiohandle != NULL && sndGetState(channel->audiohandle) != AL_STOPPED)
				|| (channel->flags & PSFLAG_REPEATING)
				|| (channel->flags & PSFLAG_FIRSTTICK)
				|| ((channel->flags & PSFLAG_ISMP3) && sndIsPlayingMp3())))
	{
		struct coord *pos = NULL;
		RoomNum *rooms = NULL;
		int newvol;
		int newpan;
		int newfx;
		float newpitch;

		if (channel->prop) {
			pos = &channel->prop->pos;
			rooms = channel->prop->rooms;
		} else if (channel->rooms[0] != -1) {
			rooms = channel->rooms;
		}

		if (channel->posptr != NULL) {
			pos = channel->posptr;
		}

		if (1);

		if (g_Vars.langfilteron && (channel->flags2 & PSFLAG2_OFFENSIVE)) {
			channel->targetvol = 0;
		} else if (channel->flags2 & PSFLAG2_0010) {
			if (channel->flags & PSFLAG_FIRSTTICK) {
				channel->targetvol = channel->vol10;
			} else {
				return;
			}
		} else {
			if (pos && rooms) {
				RoomNum *tmprooms;

				if (channel->flags & PSFLAG_IGNOREROOMS) {
					tmprooms = NULL;
				} else {
					tmprooms = rooms;
				}

				var8006ae50 = channel->soundnum2c;

				channel->targetvol = psCalculateVol(pos, channel->dist1, channel->dist2, channel->dist3,
						tmprooms, channel->soundnum26, channel->vol10, &channel->distance);
			}

			if ((channel->flags & PSFLAG_HASCONFIGPAN) == 0) {
				channel->targetpan = psCalculatePan(pos, channel->dist1, channel->dist2, channel->dist3,
						channel->distance, channel->flags & PSFLAG_0800, channel);
			}
		}

		if (rooms != NULL && rooms[0] != -1) {
			channel->targetfx = 0;
			channel->fxbus = 1;
		} else {
			channel->targetfx = 0;
			channel->fxbus = 1;
		}

		if (channel->audiohandle != NULL && channel->targetpitch > 0.0f) {
			if (channel->currentpitch < 0.0f) {
				newpitch = channel->targetpitch;
			} else if (channel->pitchchangespeed > 0.0f) {
				newpitch = channel->currentpitch + (channel->targetpitch - channel->currentpitch) * g_Vars.lvupdate240 / channel->pitchchangespeed;
			} else {
				newpitch = channel->targetpitch;
			}
		} else {
			newpitch = -1.0f;
		}

		newpan = channel->targetpan;
		newvol = channel->currentvol;
		newfx = channel->targetfx;

		if (channel->currentvol == -1) {
			newvol = channel->targetvol;
		} else if (channel->volchangetimer60 >= 0) {
			if (channel->volchangetimer60 > g_Vars.lvupdate60) {
				newvol = channel->currentvol + (channel->targetvol - channel->currentvol) * g_Vars.lvupdate60 / channel->volchangetimer60;
			}

			channel->volchangetimer60 -= g_Vars.lvupdate60;
		} else if (channel->volchangespeed && channel->currentvol != channel->targetvol) {
			float f12 = channel->targetvol - channel->currentvol;
			float f14 = (1.0f / 6000.0f) * g_Vars.lvupdate60 * channel->volchangespeed;

			if (fabsf(f12) > 1.0f) {
				if (f14 > 1.0f) {
					f14 = 1.0f;
				}

				if (fabsf(f14 * f12) > 1.0f) {
					newvol = channel->currentvol + (int) (f14 * f12);
				}
			}
		} else {
			newvol = channel->targetvol;
		}

		if (lvIsPaused()
				|| (mpIsPaused() && (channel->flags2 & PSFLAG2_MPPAUSABLE))
				|| (mpIsPaused() && PLAYERCOUNT() == 1)) {
			channel->currentvol = -1;
			newvol = 0;
		}

		if (newvol != channel->currentvol) {
			channel->currentvol = newvol;
		} else {
			newvol = -1;
		}

		if (channel->targetpan != channel->currentpan) {
			if (channel->flags & PSFLAG_FIRSTTICK) {
				channel->currentpan = channel->targetpan;
				newpan = channel->currentpan;
			} else {
				int diff = channel->targetpan - channel->currentpan;
				int lvupdate = g_Vars.lvupdate240 * 512 / 240;
				int dir = diff < 0 ? -1 : 1;
				int absdiff = abs(diff);
				int amount = absdiff < lvupdate ? absdiff : lvupdate;

				channel->currentpan += amount * dir;
				newpan = channel->currentpan;
			}

			channel->flags |= PSFLAG_CHANGINGPAN;
		} else {
			newpan = -1;
		}

		if (newfx != channel->currentfx) {
			channel->currentfx = newfx;
		} else {
			newfx = -1;
		}

		if (newpitch > 0.0f && fabsf(newpitch - channel->currentpitch) > 0.01f) {
			channel->currentpitch = newpitch;
		} else {
			newpitch = -1.0f;
		}

		/**
		 * Channels which repeat (such as terminal hums) are stopped
		 * when out of range and resumed when range is re-entered.
		 */
		if (channel->flags & PSFLAG_REPEATING) {
			if (channel->currentvol > 0) {
				if (channel->flags & PSFLAG_OUTOFRANGE) {
					channel->flags &= ~PSFLAG_OUTOFRANGE;
					channel->flags |= PSFLAG_FIRSTTICK;
				}
			} else {
				if ((channel->flags & PSFLAG_OUTOFRANGE) == 0) {
					if (channel->audiohandle != NULL && sndGetState(channel->audiohandle) != AL_STOPPED) {
						audioStop(channel->audiohandle);
					}

					channel->flags |= PSFLAG_OUTOFRANGE;
				}
				channel->flags &= ~PSFLAG_FIRSTTICK;
			}
		}

		/**
		 * Handle starting, restarting or adjusting the audio.
		 */
		if ((channel->flags & PSFLAG_OUTOFRANGE) == 0) {
			if (channel->flags & PSFLAG_FIRSTTICK) {
				if (channel->flags & PSFLAG_ISMP3) {
					sndStartMp3(channel->soundnum26, newvol, newpan, (channel->flags2 & PSFLAG2_RESPONDHELLO) ? 1 : 0);
				} else {
					if (channel->flags & PSFLAG_0400) {
						if (newvol) {
							snd00010718(&channel->audiohandle, channel->flags & PSFLAG_ISMP3, newvol, newpan,
									channel->soundnum26, newpitch, channel->fxbus, newfx, 1);
						}
					} else {
						if (newvol) {
							snd00010718(&channel->audiohandle, channel->flags & PSFLAG_ISMP3, newvol, newpan,
									channel->soundnum26, newpitch, channel->fxbus, newfx, 1);
						}
					}
				}

				channel->flags &= ~PSFLAG_FIRSTTICK;
			} else {
				sndAdjust(&channel->audiohandle, channel->flags & PSFLAG_ISMP3, newvol, newpan,
						channel->soundnum26, newpitch, channel->fxbus, newfx, channel->flags & PSFLAG_CHANGINGPAN);
			}
		}
	} else {
		/**
		 * The channel doesn't need to tick any more.
		 * If it's not marked, free it.
		 */
		if (channel->type != PSTYPE_MARKER) {
			if (channel->flags & PSFLAG_ISMP3) {
				if (!sndIsPlayingMp3()) {
					if (channel->flags & PSFLAG_FORPROP) {
						propDecrementSoundCount(channel->prop);
					}

					if (channel->flags & PSFLAG_FORHUDMSG) {
						hudmsgsHideByChannel(channelnum);
					}
				}

				channel->flags = PSFLAG_FREE;
			} else if (channel->audiohandle == NULL) {
				if (channel->flags & PSFLAG_FORPROP) {
					propDecrementSoundCount(channel->prop);
				}

				channel->flags = PSFLAG_FREE;
			}
		}
	}

	channel->flags &= ~PSFLAG_FIRSTTICK;
	channel->flags &= ~PSFLAG_CHANGINGPAN;
}

void psTick(void)
{
	static int peakcount = 0;
	int count = 0;
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		struct pschannel *channel = &g_PsChannels[i];

		if ((channel->flags & PSFLAG_FREE) == 0) {
			psTickChannel(i);
			count++;
		}
	}

	if (g_PsPrintAll) {
		g_PsPrintAll = false;
	}

	if (count > peakcount) {
		peakcount = count;
	}
}

void psSetPitch(struct prop *prop, float targetpitch, int changespeed)
{
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		if ((g_PsChannels[i].flags & PSFLAG_FREE) == 0 && g_PsChannels[i].prop == prop) {
			g_PsChannels[i].targetpitch = targetpitch;

			if (changespeed > 0) {
				g_PsChannels[i].pitchchangespeed = changespeed * 4;
			} else {
				g_PsChannels[i].pitchchangespeed = -1;
			}

			psTickChannel(i);
		}
	}
}

void psSetVolume(struct prop *prop, int volpercentage)
{
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		if ((g_PsChannels[i].flags & PSFLAG_FREE) == 0 && prop == g_PsChannels[i].prop) {
			if (volpercentage > 100) {
				volpercentage = 100;
			}

			g_PsChannels[i].vol10 = volpercentage * AL_VOL_FULL / 100;
			psTickChannel(i);
		}
	}
}

void psStopOneShootChannel(struct prop *prop)
{
	int lowestuuid = -1;
	int count = 0;
	int bestindex = -1;
	int i;

	for (i = 0; i < CHANNELCOUNT(); i++) {
		struct pschannel *channel = &g_PsChannels[i];

		if ((channel->flags & PSFLAG_FREE) == 0
				&& (channel->flags2 & PSFLAG2_STOPPED) == 0
				&& prop == channel->prop
				&& channel->type == PSTYPE_CHRSHOOT) {
			count++;

			if (channel->uuid < lowestuuid) {
				lowestuuid = channel->uuid;
				bestindex = i;
			}
		}
	}

	if (1);

	if (count >= 2 && bestindex != -1) {
		psStopChannel(bestindex);
	}
}

int16_t psCreate(struct pschannel *channel, struct prop *prop, int16_t soundnum, int16_t padnum,
		int vol, uint16_t flags, uint16_t flags2, int type,
		struct coord *pos, float pitch, RoomNum *rooms, int room,
		float dist1, float dist2, float dist3)
{
	union soundnumhack spac;
	int pan;

	struct pad pad;
	int i;
	int j;

	if (type == PSTYPE_CHRSHOOT) {
		psStopOneShootChannel(prop);
	}

	spac.packed = soundnum;

	if (channel == NULL) {
		if (type != PSTYPE_FOOTSTEP && g_SndNumPlaying > 12) {
			return -1;
		}

		for (i = CHANNEL_HEAP_FIRST; i < CHANNELCOUNT(); i++) {
			if (g_PsChannels[i].flags & PSFLAG_FREE) {
				channel = &g_PsChannels[i];
				channel->channelnum = i;
				break;
			}
		}
	}

	if (padnum >= 0) {
		padUnpack(padnum, PADFIELD_POS | PADFIELD_ROOM, &pad);
		pos = &pad.pos;
		room = pad.room;
		prop = NULL;
	}

	if (channel == NULL) {
		return -1;
	}

	if (g_AudioPrevUuid < 0xffffffff) {
		g_AudioPrevUuid++;
	} else {
		g_AudioPrevUuid = 0;
	}

	channel->flags = flags;
	channel->flags2 = flags2;
	channel->audiohandle = NULL;
	channel->currentvol = -1;
	channel->currentpan = -1;
	channel->currentfx = -1;
	channel->targetvol = (vol != -1) ? vol : -1;
	channel->targetpan = AL_PAN_CENTER;
	channel->targetfx = 0;
	channel->vol10 = AL_VOL_FULL;
	channel->fxbus = 1;
	channel->targetpitch = (pitch > 0) ? pitch : -1;
	channel->currentpitch = channel->targetpitch;
	channel->pitchchangespeed = -1;
	channel->volchangetimer60 = -1;
	channel->padnum = padnum;
	channel->prop = prop;
	channel->type = type;
	channel->vol12 = vol;
	channel->unk40 = -1;
	channel->dist1 = (dist1 > 0) ? dist1 : 400;
	channel->dist2 = (dist2 > 0) ? dist2 : 2500;
	channel->dist3 = (dist3 > 0) ? dist3 : 3000;
	channel->volchangespeed = 0;
	channel->uuid = g_AudioPrevUuid;

	if (spac.hasconfig) {
		int id = spac.confignum;
		int confignum = g_AudioRussMappings[id].audioconfig_index;
		int newid = g_AudioRussMappings[id].soundnum;

		channel->dist1 = g_AudioConfigs[confignum].dist1;
		channel->dist2 = g_AudioConfigs[confignum].dist2;
		channel->dist3 = g_AudioConfigs[confignum].dist3;
		channel->volchangespeed = g_AudioConfigs[confignum].volchangespeed;

		if (g_AudioConfigs[confignum].volpercentage != 100) {
			channel->vol10 = g_AudioConfigs[confignum].volpercentage * (AL_VOL_FULL / 100);
		}

		if (g_AudioConfigs[confignum].pitch > 0) {
			channel->targetpitch = g_AudioConfigs[confignum].pitch;
		}

		pan = g_AudioConfigs[confignum].pan;

		if (pan != -1) {
			channel->targetpan = pan;
			channel->flags |= PSFLAG_HASCONFIGPAN;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_01) {
			channel->flags |= PSFLAG_0800;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_RESPONDHELLO) {
			channel->flags2 |= PSFLAG2_RESPONDHELLO;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_08) {
			if (channel->targetvol == -1) {
				channel->targetvol = channel->vol10;
			}

			channel->flags |= PSFLAG_IGNOREROOMS;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_OFFENSIVE) {
			channel->flags2 |= PSFLAG2_OFFENSIVE;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_20) {
			channel->flags2 |= PSFLAG2_0010;
		}

		if (g_AudioConfigs[confignum].flags & AUDIOCONFIGFLAG_40) {
			channel->flags2 |= PSFLAG2_0040;
		}

		channel->flags |= PSFLAG_HASCONFIG;

		spac.packed = newid;
		spac.hasconfig = false;
		soundnum = spac.packed;
	}

	if (channel->volchangespeed) {
		channel->flags |= PSFLAG_REPEATING;
	}

	channel->soundnum26 = spac.packed;
	channel->soundnum2c = spac.id;

	if (sndIsFiltered(channel->soundnum2c)) {
		channel->flags2 |= PSFLAG2_OFFENSIVE;
	}

	if (spac.unk02) {
		channel->flags2 |= PSFLAG2_0010;
	}

	if (pos) {
		channel->pos.x = pos->x;
		channel->pos.y = pos->y;
		channel->pos.z = pos->z;
		channel->posptr = &channel->pos;
	} else {
		channel->posptr = NULL;
	}

	if (rooms) {
		// @dangerous: Array overflow will occur if rooms has more than 8 elements
		for (j = 0; rooms[j] != -1; j++) {
			channel->rooms[j] = rooms[j];
		}

		channel->rooms[j] = -1;
	} else if (room != -1) {
		channel->rooms[0] = room;
		channel->rooms[1] = -1;
	} else {
		channel->rooms[0] = -1;
	}

	if (!pos && !channel->prop) {
		channel->flags2 |= PSFLAG2_0010;
	}

	if ((channel->flags2 & PSFLAG2_0010) && channel->targetvol == -1) {
		channel->targetvol = channel->vol10;
	}

	channel->flags |= PSFLAG_FIRSTTICK;

	if (sndIsMp3(soundnum)) {
		channel->flags |= PSFLAG_ISMP3;
		psTickChannel(channel->channelnum);
	} else {
		psTickChannel(channel->channelnum);
	}

	if (channel->flags & PSFLAG_0400) {
		channel->flags &= ~PSFLAG_0400;
		channel->flags2 |= PSFLAG2_0010;
	}

	channel->flags &= ~PSFLAG_FIRSTTICK;

	return channel->channelnum;
}

int psPlayFromProp(int channelnum, int16_t soundnum, int vol, struct prop *prop, int16_t type, uint16_t flags)
{
	int retchannelnum = -1;

	if (type == PSTYPE_MARKER) {
		if (CHANNEL_IS_AI(channelnum)) {
			if (g_PsChannels[channelnum].flags & PSFLAG_FREE) {
				g_PsChannels[channelnum].soundnum26 = soundnum;
				g_PsChannels[channelnum].type = PSTYPE_MARKER;
				g_PsChannels[channelnum].flags &= ~PSFLAG_FREE;
				retchannelnum = channelnum;
			} else {
				g_PsChannels[channelnum].soundnum26 = soundnum;
				g_PsChannels[channelnum].type = PSTYPE_MARKER;
				g_PsChannels[channelnum].flags &= ~PSFLAG_FREE;
				retchannelnum = channelnum;
			}
		} else {
			// empty
		}
	} else if (channelnum == CHANNEL_CUTSCENE) {
		/**
		 * An AI script has requested the AI multi channel.
		 * The multi channel is an abstraction for more channels.
		 * Let psCreate choose the real channel number from the heap area.
		 * The real channel number is returned but AI will ignore it.
		 *
		 * Flag PSFLAG_CUTSCENE is set so propsnd knows the channel
		 * was allocated as part of AIMULTI instead of via the game engine.
		 */

		retchannelnum = psCreate(NULL, prop, soundnum, -1,
				(vol ? 0 : -1), flags | PSFLAG_CUTSCENE, 0, type, 0, -1, 0, -1, -1, -1, -1);
	}
	else if (channelnum < 0 || channelnum >= 8 || channelnum == 9) {
		/**
		 * This is a game engine sound.
		 * Allocate a channel automatically from the heap and return it.
		 */
		retchannelnum = psCreate(NULL, prop, soundnum, -1,
			(vol ? 0 : -1), flags, 0, type, 0, -1, 0, -1, -1, -1, -1);
	}
	else {
		/**
		 * An AI script has asked for a specific channel in range 0-9.
		 * Replace the channel if necessary.
		 */
		if ((g_PsChannels[channelnum].flags & PSFLAG_FREE) == 0) {
			psStopChannel(channelnum);
		}

		g_PsChannels[channelnum].channelnum = channelnum;

		psCreate(&g_PsChannels[channelnum], prop, soundnum, -1,
			(vol ? 0 : -1), flags, 0, type, 0, -1, 0, -1, -1, -1, -1);

		retchannelnum = channelnum;
	}

	return retchannelnum;
}

void psMuteChannel(int channelnum)
{
	if (channelnum == CHANNEL_CUTSCENE) {
		int i;

		for (i = CHANNEL_HEAP_FIRST; i < CHANNELCOUNT(); i++) {
			if ((g_PsChannels[i].flags & PSFLAG_FREE) == 0
					&& (g_PsChannels[i].flags & PSFLAG_CUTSCENE)) {
				psStopChannel(i);
			}
		}
	} else if (CHANNEL_IS_AI(channelnum)) {
		psStopChannel(channelnum);
	}
}

bool psIsChannelFree(int channelnum)
{
	if (CHANNEL_IS_AI(channelnum)) {
		return (g_PsChannels[channelnum].flags & PSFLAG_FREE) ? true : false;
	}

	if (channelnum == CHANNEL_CUTSCENE) {
		int i;

		for (i = CHANNEL_HEAP_FIRST; i < CHANNELCOUNT(); i++) {
			if (g_PsChannels[i].flags & PSFLAG_CUTSCENE) {
				return false;
			}
		}
	}

	return true;
}

void psModify(int channelnum, int volume, int16_t padnum, struct prop *prop, int volchangetimer60, int dist2, int dist3, uint16_t flags)
{
	struct pschannel *channel = &g_PsChannels[channelnum];
	bool hastimer = (volchangetimer60 >= 6) ? true : false;
	bool repeating = (flags & PSFLAG_REPEATING) ? true : false;

	if (CHANNEL_IS_AI(channelnum)) {
		if (channel->type == PSTYPE_MARKER) {
			g_PsChannels[channelnum].channelnum = (uint16_t)channelnum;

			psCreate(&g_PsChannels[channelnum], prop, channel->soundnum26, -1,
					-1, flags, 0, PSTYPE_NONE, 0, -1, 0, -1, 400, dist2, dist3);
		} else {
			if ((channel->flags & PSFLAG_OUTOFRANGE) == 0 && volume >= 0) {
				channel->targetvol = volume;
			}

			if (hastimer) {
				channel->volchangetimer60 = TICKS(volchangetimer60);
			}

			if (padnum != -1) {
				channel->padnum = padnum;
			}

			if (prop) {
				channel->prop = prop;
			}

			if (repeating) {
				channel->flags |= PSFLAG_REPEATING;
			}

			if ((channel->flags & PSFLAG_HASCONFIG) == 0) {
				channel->dist1 = 400;
				channel->dist2 = dist2;
				channel->dist3 = dist3;
				channel->volchangespeed = 0;
			}

			if (!hastimer || channel->volchangetimer60 == 0) {
				psTickChannel(channelnum);
			}
		}
	}
}

int psCalculateVol(struct coord *pos, float dist1, float dist2, float dist3, RoomNum *rooms, int16_t soundnum, int arg6, float *playerdistptr)
{
	union soundnumhack sp6c;
	union soundnumhack sp68;
	float playerdist;
	RoomNum roomnum;
	int s0;
	int i;

	playerdist = dist3 + 10.0f;

	if (rooms != NULL) {
		roomnum = *rooms;
	} else {
		roomnum = -1;
	}

	sp68.packed = soundnum;
	sp6c.packed = soundnum;

	if (sp68.hasconfig) {
		int confignum = sp68.confignum;
		int index = g_AudioRussMappings[confignum].audioconfig_index;

		sp6c.packed = g_AudioRussMappings[confignum].soundnum;

		dist1 = g_AudioConfigs[index].dist1;
		dist2 = g_AudioConfigs[index].dist2;
		dist3 = g_AudioConfigs[index].dist3;

		if (dist3 < playerdist) {
			playerdist = dist3;
		}
	}

	if (var8006ae34 && var8006ae2c == sp6c.id) {
		s0 = 1;
	} else {
		s0 = 0;
	}

	// Figure out which player is closest and store their distance in playerdist
	for (i = 0; i < PLAYERCOUNT(); i++) {
		struct player *player = g_Vars.players[i];
		int camroom;

		if (sp6c.unk02 == 0) {
			camroom = player->cam_room;
		} else {
			camroom = -1;
		}

		func0f0056f4(camroom, &player->cam_pos, roomnum, pos, 0, &playerdist, s0);
	}

	if (playerdistptr != NULL) {
		*playerdistptr = playerdist;
	}

	return psCalculateVolumeFromDistance(playerdist, dist1, dist2, dist3, arg6);
}

int psCalculatePan3(int degrees, float arg1, struct pschannel *channel)
{
	int result;

	while (degrees >= 180) {
		degrees -= 360;
	}

	while (degrees < -180) {
		degrees += 360;
	}

	switch (g_SoundMode) {
	default:
		{
			int absdegrees = degrees > 0 ? degrees : -degrees;
			int dir;
			int v1;

			if (absdegrees > 90) {
				absdegrees = 180 - absdegrees;
			}

			v1 = absdegrees && absdegrees;
			dir = (degrees > 0) ? (1) : (-1);

			if (v1);

			degrees = dir * absdegrees;
		}
		break;
	case SOUNDMODE_SURROUND:
		if (degrees > -45 && degrees < 45) {
			degrees *= 2;
		} else {
			int dir = degrees > 0 ? 1 : -1;
			int v1 = dir && dir;
			int absdegrees = degrees > 0 ? degrees : -degrees;
			int t4 = (180 - absdegrees) * 0.6666667f;

			if (v1);

			degrees = (180 - t4) * dir;
		}
		break;
	}

	/**
	 * At this point, in sound modes other than surround, degrees is:
	 *
	 *   0 if sound is in front
	 *  90 if sound is on the right
	 *   0 if sound is behind
	 * -90 if sound is to the left
	 *
	 * ...and scaled between those for angles between, so effectively a pan value.
	 *
	 * For surround mode, degrees is:
	 *
	 *        0 if sound is in front
	 *       90 if sound is 45 degrees to the right
	 * 180/-180 if sound is behind
	 *      -90 if sound is 45 degrees to the left
	 *
	 * So an actual degrees value but with weighting towards the front.
	 */

	if (degrees >= -90 && degrees <= 90) {
		result = AL_PAN_CENTER + degrees * 0.7f;
	} else {
		int v0;
		int dir = degrees > 0 ? 1 : -1;
		int absdegrees = abs(degrees);

		result = 128 + (int) (AL_PAN_CENTER + (180 - absdegrees) * dir * 0.7f);
	}

	if (channel != NULL) {
		channel->degrees = degrees;
		channel->unk16 = 100.0f * arg1;
	}

	return result;
}

int psCalculatePan2(struct coord *pos, int arg1, float arg2, struct pschannel *channel)
{
	int result = AL_PAN_CENTER;
	uint32_t stack[4];
	int degrees;
	float f2;

	if (PLAYERCOUNT() < 2) {
		struct coord *campos = &g_Vars.currentplayer->cam_pos;
		float sp3c;
		float sp38;

		f2 = -(atan2f(pos->x - campos->x, pos->z - campos->z) * 180.0f / M_PI + g_Vars.currentplayer->vv_theta);

		if (arg2 >= 0.0f) {
			sp3c = sinf(0.017453292f * f2);
			sp38 = cosf(0.017453292f * f2);

			sp3c *= arg2;

			f2 = atan2f(fabsf(sp3c), fabsf(sp38));

			if (sp3c >= 0.0f && sp38 >= 0.0f) {
				// empty
			} else if (sp3c >= 0.0f) {
				f2 = M_PI - f2;
			} else if (sp38 >= 0.0f) {
				f2 = -f2;
			} else {
				f2 = -(M_PI - f2);
			}

			degrees = f2 * 57.295776f;
		} else {
			degrees = f2;
		}

		if (channel != NULL) {
			channel->degrees = degrees;
			channel->unk16 = 100.0f * arg2;
		}

		result = psCalculatePan3(degrees, arg2, channel);
	}

	return result;
}

int psCalculatePan(struct coord *pos, float dist1, float dist2, float dist3, float playerdist, bool arg5, struct pschannel *channel)
{
	int result = AL_PAN_CENTER;

	if (pos != NULL && playerdist > 0) {
		if (dist1 < 0) {
			dist1 = 400;
		}

		if (dist2 < 0) {
			dist2 = 2500;
		}

		if (dist3 < 0) {
			dist3 = 3000;
		}

		if (playerdist < dist3) {
			if (dist1 > 5501) {
				dist1 = 5501;
			}

			if (dist2 > 5801) {
				dist2 = 5801;
			}

			if (arg5) {
				if (playerdist < dist1) {
					// empty
				} else if (playerdist < dist2) {
					float frac = (playerdist - dist1) / (dist2 - dist1);

					if (frac < 0) {
						frac = 0;
					}

					if (frac > 1) {
						frac = 1;
					}

					result = psCalculatePan2(pos, 0, frac, channel);
				} else {
					result = psCalculatePan2(pos, 0, -1, NULL);
				}
			} else {
				result = psCalculatePan2(pos, 0, -1, channel);
			}
		}
	}

	return result;
}

/**
 * If the given soundnum were to play at the given world position, calculate the
 * final volume and pan and write them to the vol and pan pointers.
 */
void psGetTheoreticalVolPan(struct coord *pos, RoomNum *rooms, int16_t soundnum, int *vol, int *pan)
{
	float dist1;
	float dist2;
	float dist3;
	struct audiorussmapping *russ;
	struct audioconfig *config;
	union soundnumhack sp48;
	union soundnumhack sp44;
	float distance;
	bool sp3c;
	int index;
	int confignum;
	float *distanceptr = &distance;

	dist1 = 400;
	dist2 = 2500;
	dist3 = 3000;

	sp44.packed = soundnum;
	distance = 9999999;
	sp3c = false;

	if (sp44.hasconfig) {
		confignum = sp44.confignum;
		russ = &g_AudioRussMappings[confignum];
		index = russ->audioconfig_index;
		config = &g_AudioConfigs[index];

		dist1 = config->dist1;
		dist2 = config->dist2;
		dist3 = config->dist3;

		sp48.packed = russ->soundnum;

		if (config->flags & AUDIOCONFIGFLAG_01) {
			sp3c = true;
		}

		if (dist3 < *distanceptr) {
			*distanceptr = dist3;
		}

		sp48.hasconfig = false;
		soundnum = sp48.packed;
	}

	*vol = psCalculateVol(pos, dist1, dist2, dist3, rooms, soundnum, AL_VOL_FULL, distanceptr);
	*pan = psCalculatePan(pos, dist1, dist2, dist3, *distanceptr, sp3c, 0);
}

void psApplyVolPan(struct sndstate *handle, struct coord *pos, float dist1, float dist2, float dist3, RoomNum *rooms, int16_t soundnum, int arg7, float *distanceptr)
{
	union soundnumhack sp5c;
	union soundnumhack sp58;
	float distance;
	bool sp50;
	int vol;
	int pan;

	sp50 = false;
	distance = 9999999;
	sp58.packed = soundnum;

	if (distanceptr == NULL) {
		distanceptr = &distance;
	}

	if (sp58.hasconfig) {
		int confignum = sp58.confignum;
		int index = g_AudioRussMappings[confignum].audioconfig_index;

		dist1 = g_AudioConfigs[index].dist1;
		dist2 = g_AudioConfigs[index].dist2;
		dist3 = g_AudioConfigs[index].dist3;

		sp5c.packed = g_AudioRussMappings[confignum].soundnum;

		if (g_AudioConfigs[index].flags & AUDIOCONFIGFLAG_01) {
			sp50 = true;
		}

		if (dist3 < *distanceptr) {
			*distanceptr = dist3;
		}

		sp5c.hasconfig = false;
		soundnum = sp5c.packed;
	}

	vol = psCalculateVol(pos, dist1, dist2, dist3, rooms, soundnum, arg7, distanceptr);
	pan = psCalculatePan(pos, dist1, dist2, dist3, *distanceptr, sp50, 0);

	sndAdjust(&handle, sndIsMp3(soundnum), vol, pan, soundnum, 1.0f, 1, -1, 1);
}

int psGetRandomSparkSound(void)
{
	int index = rngRandom() % 6;

	int16_t sounds[] = {
		SFX_80B0,
		SFX_80B1,
		SFX_80B2,
		SFX_80B3,
		SFX_80B4,
		SFX_80B5,
	};

	return sounds[index];
}

/**
 * Get the duration for an MP3 file by channel number.
 *
 * All MP3 files are 24 kilobits per second
 * so this is just math based on the filesize.
 */
int psGetDuration60(int channelnum)
{
	struct pschannel *channel = &g_PsChannels[channelnum];

	if (channelnum >= 0 && channelnum < CHANNELCOUNT()
			&& (channel->flags & PSFLAG_FREE) == 0
			&& (channel->flags & PSFLAG_ISMP3)) {
		union soundnumhack soundnum;
		soundnum.packed = channel->soundnum26;

		return fileGetRomSize(soundnum.id) * 60 / (1024 * 24 / 8);
	}

	return -1;
}
