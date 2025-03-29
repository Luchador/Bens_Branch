#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "bss.h"
#include "data.h"
#include "game/activemenu.h"
#include "game/bg.h"
#include "game/body.h"
#include "game/bondgun.h"
#include "game/bondhead.h"
#include "game/bondmove.h"
#include "game/bondview.h"
#include "game/casing.h"
#include "game/cheats.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/credits.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/explosions.h"
#include "game/filemgr.h"
#include "game/menuutils.h"
#include "game/chrutils.h"
#include "game/gunfx.h"
#include "game/weaponutils.h"
#include "game/modelmgr.h"
#include "game/portal.h"
#include "game/sky.h"
#include "game/artifacts.h"
#include "game/textutils.h"
#include "game/zbuf.h"
#include "game/challenge.h"
#include "game/chrmgr.h"
#include "game/env.h"
#include "game/gfxmemory.h"
#include "game/gunfx.h"
#include "game/hudmsg.h"
#include "game/inv.h"
#include "game/lang.h"
#include "game/lv.h"
#include "game/menu.h"
#include "game/mplayer/mplayer.h"
#include "game/mplayer/scenarios.h"
#include "game/mplayer/setup.h"
#include "game/music.h"
#include "game/nbomb.h"
#include "game/objectives.h"
#include "game/pak.h"
#include "game/pdmode.h"
#include "game/player.h"
#include "game/playermgr.h"
#include "game/playerreset.h"
#include "game/prop.h"
#include "game/propobj.h"
#include "game/propobjstop.h"
#include "game/propsnd.h"
#include "game/room.h"
#include "game/savebuffer.h"
#include "game/setup.h"
#include "game/shards.h"
#include "game/sky.h"
#include "game/smoke.h"
#include "game/sparks.h"
#include "game/splat.h"
#include "game/stars.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "game/tiles.h"
#include "game/title.h"
#include "game/training.h"
#include "game/utils.h"
#include "game/vtxstore.h"
#include "game/wallhit.h"
#include "game/weather.h"
#include "lib/anim.h"
#include "lib/args.h"
#include "lib/collision.h"
#include "lib/joy.h"
#include "lib/lib_06440.h"
#include "lib/lib_317f0.h"
#include "lib/main.h"
#include "lib/mtx.h"
#include "lib/music.h"
#include "lib/rng.h"
#include "lib/sched.h"
#include "lib/snd.h"
#include "lib/vars.h"
#include "lib/vi.h"
#include "types.h"
#include "video.h"

struct sndstate *g_MiscSfxAudioHandles[3];
int g_MiscSfxActiveTypes[3];

uint32_t g_LvlIsPausedMP = 0;
bool g_IsLvlPaused = false;

int g_Difficulty = DIFF_A;

int g_StageTimeElapsed60 = 0;
int g_MpTimeLimit60 = SECSTOTIME60(60 * 10); // 10 minutes
int g_MpScoreLimit = 10;
int g_MpTeamScoreLimit = 20;
struct sndstate *g_MiscAudioHandle = NULL;
int g_NumReasonsToEndMpMatch = 0;
float g_StageTimeElapsed1f = 0;

uint32_t g_MiscSfxSounds[] = {
	SFX_HEARTBEAT,
	SFX_SLAYER_WHIR,
	SFX_SLAYER_BEEP,
};

int g_LockScreenTimer = 0;
int16_t g_FadeNumFrames = 0;
float g_FadeFrac = -1;
uint32_t g_FadePrevColour = 0;
uint32_t g_FadeColour = 0;
int16_t g_FadeDelay = 0;

void lvInit(void)
{
	g_Vars.lockscreen = 0;
	g_Vars.joydisableframestogo = -1;
}

void lvResetMiscSfx(void)
{
	int i;

	for (i = 0; i != ARRAYCOUNT(g_MiscSfxAudioHandles); i++) {
		g_MiscSfxAudioHandles[i] = NULL;
		g_MiscSfxActiveTypes[i] = -1;
	}
}

int lvGetMiscSfxIndex(uint32_t type)
{
	int i;

	for (i = 0; i != ARRAYCOUNT(g_MiscSfxActiveTypes); i++) {
		if (g_MiscSfxActiveTypes[i] == type) {
			return i;
		}
	}

	return -1;
}

void lvSetMiscSfxState(uint32_t type, bool play)
{
	if (play) {
		if (lvGetMiscSfxIndex(type) == -1) {
			int index = lvGetMiscSfxIndex(-1);

			if (index != -1 && g_MiscSfxAudioHandles[index] == NULL)
			{
				sndStart(var80095200, g_MiscSfxSounds[type], &g_MiscSfxAudioHandles[index], -1, -1, -1, -1, -1);
				g_MiscSfxActiveTypes[index] = type;
			}
		}
	} else {
		int index = lvGetMiscSfxIndex(type);

		if (index != -1) {
			audioStop(g_MiscSfxAudioHandles[index]);
			g_MiscSfxActiveTypes[index] = -1;
		}
	}
}

void lvUpdateMiscSfx(void)
{
	int i;

	if (g_Vars.lvupdate240 == 0) {
		for (i = 0; i != ARRAYCOUNT(g_MiscSfxActiveTypes); i++) {
			lvSetMiscSfxState(i, false);
		}
	} else {
		bool usingboost = g_Vars.speedpillon
			&& lvGetSlowMotionType() == SLOWMOTION_OFF
			&& g_Vars.in_cutscene == false;
		bool usingrocket;

		lvSetMiscSfxState(MISCSFX_BOOSTHEARTBEAT, usingboost);

		usingrocket = false;

		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.players[i]->visionmode == VISIONMODE_SLAYERROCKET) {
				usingrocket = true;
			}
		}

		lvSetMiscSfxState(MISCSFX_SLAYERROCKETHUM, usingrocket);
		lvSetMiscSfxState(MISCSFX_SLAYERROCKETBEEP, usingrocket);
	}

	if (g_Vars.lvupdate240 == 0 && g_MiscAudioHandle && sndGetState(g_MiscAudioHandle) != AL_STOPPED) {
		audioStop(g_MiscAudioHandle);
	}
}

void lvReset(int stagenum)
{
	lvFadeReset();
	
	g_IsLvlPaused = false;
	g_LvlIsPausedMP = 0;

	joyLockCyclicPolling();

	g_Vars.joydisableframestogo = 10;

	g_Vars.paksneededforgame = 0;
	g_Vars.paksneededformenu = 0;
	g_Vars.stagenum = stagenum;

	cheatsReset();

	g_Vars.lvframenum = 0;
	g_LockScreenTimer = 0;

	g_Vars.lvframe60 = 0;
	g_Vars.lvupdate240 = 4;
	g_Vars.lvupdate60f = 1.0f;
	g_Vars.lvupdate60frealprev = 1.0f;

	g_Vars.lvupdate60freal = g_Vars.lvupdate60frealprev;

	g_StageTimeElapsed60 = 0;
	g_StageTimeElapsed1f = 0;

	g_Vars.speedpilltime = 0;
	g_Vars.speedpillchange = 0;
	g_Vars.speedpillwant = 0;
	g_Vars.speedpillon = false;

	g_Vars.restartlevel = false;
	g_Vars.aibuddiesspawned = false;
	g_Vars.totalkills = 0;
	g_Vars.antiheadnum = -1;
	g_Vars.antibodynum = -1;
	g_Vars.dontplaynrg = false;
	g_Vars.in_cutscene = false;
	g_Vars.autocutplaying = false;
	g_Vars.autocutfinished = false;
	g_Vars.autocutgroupskip = false;

	g_MiscAudioHandle = NULL;

	musicReset();
	modelmgrSetLvResetting(true);
	surfaceReset();
	texReset();
	textReset();
	hudmsgsReset();

	if (stagenum == STAGE_TITLE) {
		titleReset();
	} else {
		int i;
		int j;

		tilesReset();
		bgReset(g_Vars.stagenum);
		bgBuildTables(g_Vars.stagenum);
		skyReset(g_Vars.stagenum);

		if (g_Vars.normmplayerisrunning) {
			musicSetStageAndStartMusic(stagenum);
		} else {
			musicSetStage(stagenum);
		}

		if (g_Vars.normmplayerisrunning) {
			mpApplyLimits();
		}

		if (g_Vars.mplayerisrunning == false) {
			g_Vars.playerstats[0].mpindex = 4;
			g_PlayerConfigsArray[4].contpad1 = 0;
			g_PlayerConfigsArray[4].contpad2 = 1;
		}

		for (i = 0; i != ARRAYCOUNT(g_Vars.playerstats); i++) {
			g_Vars.playerstats[i].damagescale = 1;
			g_Vars.playerstats[i].drawplayercount = 0;
			g_Vars.playerstats[i].distance = 0;
			g_Vars.playerstats[i].backshotcount = 0;
			g_Vars.playerstats[i].armourcount = 0;
			g_Vars.playerstats[i].fastest2kills = S32_MAX;
			g_Vars.playerstats[i].slowest2kills = 0;
			g_Vars.playerstats[i].maxkills = 0;
			g_Vars.playerstats[i].maxsimulkills = 0;
			g_Vars.playerstats[i].longestlife = 0;
			g_Vars.playerstats[i].shortestlife = S32_MAX;
			g_Vars.playerstats[i].tokenheldtime = 0;
			g_Vars.playerstats[i].damreceived = 0;
			g_Vars.playerstats[i].damtransmitted = 0;

			for (j = 0; j != ARRAYCOUNT(g_Vars.playerstats[i].kills); j++) {
				g_Vars.playerstats[i].kills[j] = 0;
			}
		}
	}

	mpSetDefaultNamesIfEmpty();
	animsReset();
	objectivesReset();
	vtxstoreReset();
	modelmgrReset();
	psReset();
	setupLoadFiles(stagenum);
	scenarioReset();
	varsReset();
	propsReset();
	chrmgrReset();
	bodiesReset(stagenum);
	setupCreateProps(stagenum);
	tagsReset();
	explosionsReset();
	smokeReset();
	sparksReset();
	weatherReset();
	lvResetMiscSfx();

	switch (g_Vars.stagenum) {
	case STAGE_ESCAPE:
	case STAGE_EXTRACTION:
	case STAGE_INFILTRATION:
	case STAGE_DEFECTION:
	case STAGE_ATTACKSHIP:
	case STAGE_MBR: // Enable stars on MBR
		starsReset();
		break;
	}

	nbombClearAllNBombs();
	boltbeamsReset();
	lasersightsReset();
	shardsReset();
	frReset();

	if (g_Vars.stagenum == STAGE_TITLE) { // Ben's comment: this if statement is necessary or the game crashes when returning from a mission.
		// empty
	} else if (stagenum == STAGE_BOOTPAKMENU) {
		setCurrentPlayerNum(0);
		menuReset();
	} else if (stagenum == STAGE_CREDITS) {
		creditsReset();
	} else {
		int i;

		casingsReset();

		for (i = 0; i < PLAYERCOUNT(); i++) {
			setCurrentPlayerNum(i);
			g_Vars.currentplayer->usedowntime = 0;
			g_Vars.currentplayer->invdowntime = g_Vars.currentplayer->usedowntime;

			menuReset();
			amReset();
			invReset();
			bgunReset();
			playerLoadDefaults();
			playerReset();
			playerSpawn();
			bheadReset();

			if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
				playermgrCalculateAiBuddyNums();
			}
		}

		acousticReset();
		portalsReset();
		lightsReset();
		setCurrentPlayerNum(0);
	}

	if (g_Vars.lvmpbotlevel) {
		mpCalculateTeamIsOnlyAi();
	}

	sndResetCurMp3();

	if (stagenum == STAGE_BOOTPAKMENU) {
		bootmenuReset();
	}

	modelmgrSetLvResetting(false);
	schedResetArtifacts();
	lvSetPaused(0);
}

void lvConfigureFade(uint32_t color, int16_t num_frames)
{
	g_FadeNumFrames = num_frames;
	g_FadePrevColour = g_FadeColour;

	if (g_FadeNumFrames == 0) {
		g_FadeColour = color;
		g_FadeFrac = -1;
		return;
	}

	g_FadeFrac = 0;
	g_FadeColour = color;
	g_FadeDelay = 2;
}

Gfx *lvRenderFade(Gfx *gdl)
{
	uint32_t colour = g_FadeColour;
	uint32_t inset = 0;

	if (g_FadeFrac >= 0) {
		if (g_FadeDelay > 0) {
			g_FadeDelay--;
		} else {
			g_FadeFrac += g_Vars.diffframe60f / g_FadeNumFrames;

			if (g_FadeFrac >= 1) {
				g_FadeFrac = -1;
			}
		}
	}

	if (g_FadeFrac < 0) {
		if ((g_FadeColour & 0xff) == 0) {
			return gdl;
		}
	} else {
		colour = colourBlend(g_FadeColour, g_FadePrevColour, g_FadeFrac * 255);
	}

	if ((colour & 0xff) == 0) {
		return gdl;
	}

	gDPPipeSync(gdl++);
	gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
	gDPSetPrimColorViaWord(gdl++, 0, 0, colour);

	gDPFillRectangle(gdl++,
			viGetViewLeft(),
			viGetViewTop() + inset,
			viGetViewLeft() + viGetViewWidth() + 1,
			viGetViewTop() + viGetViewHeight() - inset + 2);

	return textSetCCCustom02(gdl);
}

bool lvIsFadeActive(void)
{
	return g_FadeFrac >= 0;
}

void lvFadeReset(void)
{
	g_FadeNumFrames = 0;
	g_FadeFrac = -1;
	g_FadePrevColour = 0;
	g_FadeColour = 0;
	g_FadeDelay = 0;
}

bool lvUpdateTrackedProp(struct trackedprop *trackedprop, int index)
{
	float y1;
	float x1;
	float y2;
	float x2;
	struct prop *prop = trackedprop->prop;
	struct chrdata *chr;

	if (trackedprop->prop && prop->chr) {
		switch (trackedprop->prop->type) {
		case PROPTYPE_PLAYER:
			if (playermgrGetPlayerNumByProp(prop) == g_Vars.currentplayernum) {
				return false;
			}
			// fall through
		case PROPTYPE_CHR:
			chr = trackedprop->prop->chr;

			if (chrIsDead(trackedprop->prop->chr)) {
				if (index >= 0) {
					// Existing trackedprop
					if (g_Vars.currentplayer->targetset[index] < TICKS(129)) {
						g_Vars.currentplayer->targetset[index] = TICKS(129);
					}

					if (g_Vars.currentplayer->targetset[index] >= 175) {
						trackedprop->prop = NULL;
						return false;
					}
				} else {
					// lookingatprop
					trackedprop->prop = NULL;
					return false;
				}
			}

			if ((trackedprop->prop->flags & PROPFLAG_ONTHISSCREENTHISTICK)
					&& (chr->chrflags & CHRCFLAG_NOAUTOAIM) == 0) {
				struct model *model = chr->model;
				x1 = -1;
				y1 = -1;
				x2 = -2;
				y2 = -2;

				if (modelGetScreenCoords(model, &x2, &x1, &y2, &y1)) {
					break;
				}
				return false;
			}
			return false;
		case PROPTYPE_OBJ:
		case PROPTYPE_WEAPON:
			if (trackedprop->prop->flags & PROPFLAG_ONTHISSCREENTHISTICK) {
				struct defaultobj *obj = trackedprop->prop->obj;
				struct model *model = obj->model;
				x1 = -1;
				y1 = -1;
				x2 = -2;
				y2 = -2;

				if (modelGetScreenCoords(model, &x2, &x1, &y2, &y1)) {
					break;
				}
				return false;
			}
			return false;
		case PROPTYPE_DOOR:
		case PROPTYPE_EYESPY:
		case PROPTYPE_EXPLOSION:
		case PROPTYPE_SMOKE:
		default:
			return false;
		}

		trackedprop->x1 = x1 - 2;
		trackedprop->x2 = x2 + 2;
		trackedprop->y1 = y1 - 2;
		trackedprop->y2 = y2 + 2;
	}

	return true;
}

void lvFindThreatsForProp(struct prop *prop, bool inchild, struct coord *playerpos, bool *activeslots, float *distances)
{
	bool condition = true;
	struct defaultobj *obj;
	bool pass;
	float sp88;
	float sp84;
	float sp80;
	float sp76;
	int i;
	struct model *model;
	struct weaponobj *weapon;

	if (!inchild && prop->z < 0) {
		condition = false;
	}

	if (prop->obj
			&& (prop->flags & PROPFLAG_ONTHISSCREENTHISTICK)
			&& (prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_WEAPON)
			&& condition) {
		pass = false;
		obj = prop->obj;
		model = prop->obj->model;

		if (obj
				&& obj->type == OBJTYPE_AUTOGUN
				&& (obj->flags2 & (OBJFLAG2_AUTOGUN_MALFUNCTIONING1 | OBJFLAG2_AICANNOTUSE)) == 0) {
			pass = true;
		}

		if (obj && obj->modelnum == MODEL_SK_SHUTTLE) {
			pass = true;
		}

		weapon = (struct weaponobj *)prop->obj;

		if (weapon && prop->obj->type == OBJTYPE_WEAPON) {
			switch (weapon->weaponnum) {
			case WEAPON_GRENADE:
			case WEAPON_NBOMB:
			case WEAPON_TIMEDMINE:
			case WEAPON_PROXIMITYMINE:
			case WEAPON_REMOTEMINE:
				pass = true;
				break;
			case WEAPON_DRAGON:
				if (weapon->gunfunc == (uint32_t)FUNC_SECONDARY) {
					pass = true;
				}
				break;
			}
		}

		if (obj->modelnum == MODEL_TARGET && frIsTargetOneHitExplodable(prop)) {
			pass = true;
		}

		if (pass) {
			for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
				if (g_Vars.currentplayer->trackedprops[i].prop == prop) {
					pass = false;
				}
			}
		}

		if (pass) {
			sp84 = -1;
			sp88 = -1;
			sp76 = -2;
			sp80 = -2;

			if (!modelGetScreenCoords(model, &sp76, &sp84, &sp80, &sp88)) {
				pass = false;
			}
		}

		if (pass) {
			float furtherestdist = 0;
			int index = -1;

			float sqdist =
				(prop->pos.f[0] - playerpos->f[0]) * (prop->pos.f[0] - playerpos->f[0]) +
				(prop->pos.f[1] - playerpos->f[1]) * (prop->pos.f[1] - playerpos->f[1]) +
				(prop->pos.f[2] - playerpos->f[2]) * (prop->pos.f[2] - playerpos->f[2]);

			for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
				if (!activeslots[i]) {
					index = i;
				}
			}

			if (index == -1) {
				// No slots available - consider replacing the furtherest
				for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
					if (distances[i] > furtherestdist) {
						furtherestdist = distances[i];
						index = i;
					}
				}

				if (sqdist >= furtherestdist) {
					index = -1;
				}
			}

			if (index >= 0) {
				g_Vars.currentplayer->trackedprops[index].prop = prop;
				g_Vars.currentplayer->trackedprops[index].x1 = sp84 - 2;
				g_Vars.currentplayer->trackedprops[index].x2 = sp76 + 2;
				g_Vars.currentplayer->trackedprops[index].y1 = sp88 - 2;
				g_Vars.currentplayer->trackedprops[index].y2 = sp80 + 2;
				g_Vars.currentplayer->targetset[index] = 0;
				activeslots[index] = true;
				distances[index] = sqdist;
			}
		}
	}

	if (prop->child) {
		lvFindThreatsForProp(prop->child, true, playerpos, activeslots, distances);
	}

	if (inchild && prop->next) {
		lvFindThreatsForProp(prop->next, inchild, playerpos, activeslots, distances);
	}
}

// This function positions the four corners of the threat box on the screen
void lvPositionThreatBox(struct prop *prop, bool inchild, struct coord *playerpos, int *activeslots, float *distances)
{
	int i;
	float sp128;
	float sp124;
	float sp120;
	float sp116;
	struct model *model;

	for (i = 0; i != 4; i++) {
		if (g_Vars.currentplayer->trackedprops[i].prop == prop
				&& (prop->flags & PROPFLAG_ONTHISSCREENTHISTICK)) {
			model = NULL;

			if (prop->type == PROPTYPE_OBJ
					|| prop->type == PROPTYPE_WEAPON
					|| prop->type == PROPTYPE_DOOR) {
				model = g_Vars.currentplayer->trackedprops[i].prop->obj->model;
			} else {
				if (prop->type == PROPTYPE_CHR
						|| (prop->type == PROPTYPE_PLAYER
							&& playermgrGetPlayerNumByProp(prop) != g_Vars.currentplayernum)) {
					model = g_Vars.currentplayer->trackedprops[i].prop->chr->model;
				}
			}

			if (model) {
				sp124 = -1;
				sp128 = -1;
				sp116 = -2;
				sp120 = -2;

				if (modelGetScreenCoords(model, &sp116, &sp124, &sp120, &sp128)) {
					activeslots[i] = true;
					g_Vars.currentplayer->trackedprops[i].x1 = sp124 - 2;
					g_Vars.currentplayer->trackedprops[i].x2 = sp116 + 2;
					g_Vars.currentplayer->trackedprops[i].y1 = sp128 - 2;
					g_Vars.currentplayer->trackedprops[i].y2 = sp120 + 2;

					distances[i] =
						(prop->pos.f[0] - playerpos->f[0]) * (prop->pos.f[0] - playerpos->f[0]) +
						(prop->pos.f[1] - playerpos->f[1]) * (prop->pos.f[1] - playerpos->f[1]) +
						(prop->pos.f[2] - playerpos->f[2]) * (prop->pos.f[2] - playerpos->f[2]);
				}
			}
		}
	}

	if (prop->child) {
		lvPositionThreatBox(prop->child, true, playerpos, activeslots, distances);
	}

	if (inchild && prop->next) {
		lvPositionThreatBox(prop->next, inchild, playerpos, activeslots, distances);
	}
}

void lvFindThreats(void)
{
	int i;
	struct prop *prop;
	float distances[ARRAYCOUNT(g_Vars.currentplayer->trackedprops)] = {0};
	int activeslots[ARRAYCOUNT(g_Vars.currentplayer->trackedprops)] = {false};
	struct prop **propptr = g_Vars.endonscreenprops - 1;
	struct coord campos;

	campos.x = g_Vars.currentplayer->cam_pos.x;
	campos.y = g_Vars.currentplayer->cam_pos.y;
	campos.z = g_Vars.currentplayer->cam_pos.z;

	while (propptr >= g_Vars.onscreenprops) {
		prop = *propptr;

		if (prop) {
			lvPositionThreatBox(prop, false, &campos, activeslots, distances);
		}

		propptr--;
	}

	for (i = 0; i != ARRAYCOUNT(activeslots); i++) {
		if (!activeslots[i]) {
			g_Vars.currentplayer->trackedprops[i].prop = NULL;
			g_Vars.currentplayer->trackedprops[i].x1 = -1;
			g_Vars.currentplayer->trackedprops[i].x2 = -2;
		}
	}

	propptr = g_Vars.endonscreenprops - 1;

	while (propptr >= g_Vars.onscreenprops) {
		prop = *propptr;

		if (prop) {
			lvFindThreatsForProp(prop, false, &campos, activeslots, distances);
		}

		propptr--;
	}
}

Gfx *lvRenderFPS(Gfx *gdl)
{
	const float fps = videoGetAverageFPS();
	const uint8_t a = 160;
	int x = 27, y = 13;
	uint32_t color;
	char buffer[16];

	if (fps <= 30.f) {
		// red -> yellow
		color = 0xff000000 | a | ((uint32_t)((fps / 30.f) * 255.f) << 16);
	} else if (fps <= 60.f) {
		// yellow -> green
		color = 0x00ff0000 | a | ((uint32_t)((1.f - (fps - 30.f) / 30.f) * 255.f) << 24);
	} else if (fps <= 90.f) {
		// green -> cyan
		color = 0x00ff0000 | a | ((uint32_t)(((fps - 60.f) / 30.f) * 255.f) << 8);
	} else {
		// cyan
		color = 0x00ffff00 | a;
	}

	if (g_CharsNumeric && g_FontNumeric) {
		snprintf(buffer, sizeof buffer, "%.2f", fps);

		gSPSetExtraGeometryModeEXT(gdl++, g_HudAlignModeL);

		gdl = textConfigureGfxPipeline(gdl);
		gdl = textRender(gdl, &x, &y, buffer, g_CharsNumeric, g_FontNumeric, color, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);
		gdl = text0f153780(gdl);

		gSPClearExtraGeometryModeEXT(gdl++, g_HudAlignModeL);
	}

	return gdl;
}

/**
 * Renders a complete frame for all players, and also does some other game logic
 * that really doesn't belong here.
 *
 * This function is pretty big, so here's an overview of its structure:
 *
 * if (stage == STAGE_TITLE) {
 *     // title screen rendering
 * } else if (stage == STAGE_BOOTPAKMENU) {
 *     // boot pak menu rendering
 * } else if (stage == STAGE_CREDITS) {
 *     // credits rendering
 * } else {
 *     for (i = 0; i < numplayers; i++) {
 *         // rendering and logic per player
 *     }
 * }
 * // logic for auto-playing cutscene advancement
 *
 * The player loop takes up the majority of the function. In addition to
 * rendering the scene and HUD, it also handles the following logic:
 * - decreasing dizziness
 * - detecting if the prop being looked at is still valid
 * - pressing Z when using eyespy
 * - opening doors and reloading
 * - random static in the Infiltration intro cutscene
 * - combat boost activation and reverting
 */
Gfx *lvRender(Gfx *gdl)
{
	gSPSegment(gdl++, SPSEGMENT_PHYSICAL, 0x00000000);

	savebufferResetVp();

	if (g_Vars.stagenum == STAGE_TITLE) {
		gSPDisplayList(gdl++, &var800613a0);
		gSPDisplayList(gdl++, &var80061380);

		gdl = viPrepareZbuf(gdl);
		gdl = vi0000b1d0(gdl);

		gDPSetScissorFrac(gdl++, 0,
				viGetViewLeft() * 4.0f, viGetViewTop() * 4.0f,
				(viGetViewLeft() + viGetViewWidth()) * 4.0f,
				(viGetViewTop() + viGetViewHeight()) * 4.0f);

		gdl = titleRender(gdl);
		gdl = lvRenderFade(gdl);
	} else if (g_Vars.stagenum == STAGE_BOOTPAKMENU) {
		gSPClipRatio(gdl++, FRUSTRATIO_2);
		gSPDisplayList(gdl++, &var800613a0);
		gSPDisplayList(gdl++, &var80061380);

		setCurrentPlayerNum(0);
		viSetViewPosition(g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop);
		viSetFovAspectAndSize(g_Vars.currentplayer->fovy, g_Vars.currentplayer->aspect,
				g_Vars.currentplayer->viewwidth, g_Vars.currentplayer->viewheight);
		mtx00016748(1);

		gdl = vi0000b1d0(gdl);
		gdl = viRenderViewportEdges(gdl);
		gdl = bgScissorToViewport(gdl);
		gdl = menuRender(gdl);
	} else if (g_Vars.stagenum == STAGE_CREDITS) {
		gSPClipRatio(gdl++, FRUSTRATIO_2);
		gSPDisplayList(gdl++, &var800613a0);
		gSPDisplayList(gdl++, &var80061380);

		setCurrentPlayerNum(0);
		viSetViewPosition(g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop);
		viSetFovAspectAndSize(g_Vars.currentplayer->fovy, g_Vars.currentplayer->aspect,
				g_Vars.currentplayer->viewwidth, g_Vars.currentplayer->viewheight);
		mtx00016748(1);

		gdl = vi0000b1a8(gdl);
		gdl = vi0000b1d0(gdl);
		gdl = viRenderViewportEdges(gdl);
		gdl = creditsDraw(gdl);
	} else {
		// Normal stages
		int i;
		int playercount;
		Gfx *savedgdl;
		bool forcesingleplayer = (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
			&& playerHasSharedViewport();
		struct player *player;
		struct chrdata *chr;

		playercount = forcesingleplayer ? 1 : PLAYERCOUNT();

		gSPClipRatio(gdl++, FRUSTRATIO_2);

		for (i = 0; i < playercount; i++) {
			bool islastplayer;
			uint32_t bluramount = 0;

			savedgdl = gdl;

			if (forcesingleplayer) {
				setCurrentPlayerNum(0);
				g_Vars.currentplayerindex = 0;
				islastplayer = true;
			} else {
				int nextplayernum = i + 1;
				setCurrentPlayerNum(playermgrGetPlayerAtOrder(i));
				islastplayer = playercount == nextplayernum;
			}

			// Calculate bluramount - this will be used later
			if (g_Vars.tickmode != TICKMODE_CUTSCENE) {
				player = g_Vars.currentplayer;
				chr = player->prop->chr;

				if (chr->blurdrugamount > 0
						&& !g_Vars.currentplayer->invincible
						&& !g_Vars.currentplayer->training) {
					bluramount = (chr->blurdrugamount * 130) / TICKS(5000) + 100;

					if (bluramount > 230) {
						bluramount = 230;
					}

					if (chr->blurdrugamount > TICKS(5000)) {
						chr->blurdrugamount = TICKS(5000);
					}

					chr->blurdrugamount -= g_Vars.lvupdate60 * (chr->blurnumtimesdied + 1);

					if (chr->blurdrugamount < 1) {
						chr->blurdrugamount = 0;
						chr->blurnumtimesdied = 0;
					}

					// reset the drug blur to 0 if it's disabled in MP settings
					if (g_Vars.mplayerisrunning && (g_MpSetup.options & MPOPTION_NODRUGBLUR)) {
						bluramount = 0;
					}
				}
			}

			bviewSetMotionBlur(bluramount);

			gSPDisplayList(gdl++, &var800613a0);
			gSPDisplayList(gdl++, &var80061380);

			viSetViewPosition(g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop);
			viSetFovAspectAndSize(g_Vars.currentplayer->fovy, g_Vars.currentplayer->aspect,
					g_Vars.currentplayer->viewwidth, g_Vars.currentplayer->viewheight);
			mtx00016748(g_Vars.currentplayerstats->scale_bg2gfx);
			envTick();
			gdl = viPrepareZbuf(gdl);
			gdl = vi0000b1d0(gdl);
			gdl = bgScissorToViewport(gdl);
			artifactsClear();

			if ((g_Vars.stagenum != STAGE_CITRAINING || (!g_MpMatchHasEnded && g_MenuData.root != MENUROOT_MPSETUP))
					&& g_Vars.lvframenum <= 5
					&& !g_Vars.normmplayerisrunning
					&& g_Vars.tickmode != TICKMODE_CUTSCENE) {
				if (g_LockScreenTimer < 6) {
					g_Vars.lockscreen = 1;
				}

				g_LockScreenTimer++;
			} else if (g_Vars.currentplayer->gunctrl.loadall
					&& g_Vars.currentplayer->cameramode != CAMERAMODE_THIRDPERSON
					&& g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY
					&& !g_GamePaused) {
				g_Vars.currentplayer->gunctrl.loadall = bgunLoadAll();
			}

			if (g_Vars.lockscreen) {
				gdl = bviewDrawMotionBlur(gdl, 0xffffffff, 255);
				g_Vars.lockscreen--;
			} else if (g_GamePaused) {
				gdl = viRenderViewportEdges(gdl);
				gdl = bgScissorToViewport(gdl);
				mtx00016748(1);

				if (g_Vars.currentplayer->menuisactive) {
					gdl = menuRender(gdl);
				}
			} else {
				gdl = playerUpdateShootRot(gdl);
				gdl = viRenderViewportEdges(gdl);
				gdl = skyRender(gdl);
				bgTick();
				lightsTick();
				propsTickPlayer(islastplayer);
				scenarioTickChr(NULL);
				propsSort();
				autoaimTick();
				handsTickAttack();

				// glares calculated earlier on PC, before prop matrices turn into garbage
				bgCalculateGlaresForVisibleRooms();

				// Calculate lookingatprop
				if (PLAYERCOUNT() == 1
						|| g_Vars.coopplayernum >= 0
						|| g_Vars.antiplayernum >= 0
						|| (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_AIMTRACK) && bmoveIsInSightAimMode())) {
					g_Vars.currentplayer->lookingatprop.prop = propFindAimingAt(HAND_RIGHT, false, FINDPROPCONTEXT_QUERY);

					if (g_Vars.currentplayer->lookingatprop.prop) {
						if (g_Vars.currentplayer->lookingatprop.prop->type == PROPTYPE_CHR
								|| g_Vars.currentplayer->lookingatprop.prop->type == PROPTYPE_PLAYER) {
							chr = g_Vars.currentplayer->lookingatprop.prop->chr;

							if ((chr->hidden & CHRHFLAG_CLOAKED) && !USINGDEVICE(DEVICE_IRSCANNER)) {
								g_Vars.currentplayer->lookingatprop.prop = NULL;
							}
						} else if (g_Vars.currentplayer->lookingatprop.prop->type == PROPTYPE_OBJ
								|| g_Vars.currentplayer->lookingatprop.prop->type == PROPTYPE_WEAPON
								|| g_Vars.currentplayer->lookingatprop.prop->type == PROPTYPE_DOOR) {
							struct defaultobj *obj = g_Vars.currentplayer->lookingatprop.prop->obj;

							if ((obj->flags3 & OBJFLAG3_REACTTOSIGHT) == 0) {
								if (g_Vars.stagenum != STAGE_CITRAINING
										|| (obj->modelnum != MODEL_TARGET
											&& obj->modelnum != MODEL_CIHUB
											&& obj->modelnum != MODEL_COMHUB)) {
									g_Vars.currentplayer->lookingatprop.prop = NULL;
								}
							}
						} else {
							g_Vars.currentplayer->lookingatprop.prop = NULL;
						}
					}
				} else {
					g_Vars.currentplayer->lookingatprop.prop = NULL;
				}

				if (gsetHasFunctionFlags(&g_Vars.currentplayer->hands[0].gset, FUNCFLAG_THREATDETECTOR)) {
					lvFindThreats();
				} else if (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_AIMTRACK)) {
					int j;

					if (frIsInTraining()
							&& g_Vars.currentplayer->lookingatprop.prop
							&& bmoveIsInSightAimMode()) {
						func0f1a0924(g_Vars.currentplayer->lookingatprop.prop); // Ben's comment: I'm not sure if this function is ever called.
					} else if (lvUpdateTrackedProp(&g_Vars.currentplayer->lookingatprop, -1) == 0) {
						g_Vars.currentplayer->lookingatprop.prop = NULL;
					}

					for (j = 0; j < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); j++) {
						if (!lvUpdateTrackedProp(&g_Vars.currentplayer->trackedprops[j], j)) {
							g_Vars.currentplayer->trackedprops[j].x1 = -1;
							g_Vars.currentplayer->trackedprops[j].x2 = -2;
						}
					}
				}

				// Handle eyespy Z presses
				if (g_Vars.currentplayer->eyespy
						&& (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_EYESPY)
						&& g_Vars.currentplayer->eyespy->camerabuttonheld) {
					if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_CAMSPY) {
						objectiveCheckHolograph(400);
						sndStart(var80095200, SFX_CAMSPY_SHUTTER, 0, -1, -1, -1, -1, -1);
					} else if (g_Vars.currentplayer->eyespy->mode == EYESPYMODE_DRUGSPY) {
						if (g_Vars.currentplayer->eyespydarts) {
							// Fire dart
							struct coord direction;
							sndStart(var80095200, SFX_DRUGSPY_FIREDART, 0, -1, -1, -1, -1, -1);
							g_Vars.currentplayer->eyespydarts--;

							direction.x = g_Vars.currentplayer->eyespy->look.x;
							direction.y = g_Vars.currentplayer->eyespy->look.y;
							direction.z = g_Vars.currentplayer->eyespy->look.z;

							projectileCreate(g_Vars.currentplayer->eyespy->prop, 0,
									&g_Vars.currentplayer->eyespy->prop->pos, &direction, WEAPON_TRANQUILIZER, NULL);
						} else {
							// No dart ammo
							sndStart(var80095200, SFX_FIREEMPTY, 0, -1, -1, -1, -1, -1);
						}
					} else { // EYESPYMODE_BOMBSPY
						struct coord vel = {0, 0, 0};
						struct gset gset = {WEAPON_GRENADE, 0, 0, FUNC_PRIMARY};
						explosionCreateSimple(g_Vars.currentplayer->eyespy->prop,
								&g_Vars.currentplayer->eyespy->prop->pos,
								g_Vars.currentplayer->eyespy->prop->rooms,
								EXPLOSIONTYPE_DRAGONBOMBSPY, 0);
						chrBeginDeath(g_Vars.currentplayer->eyespy->prop->chr, &vel, 0, 0, &gset, false, 0);
					}
				}

				// Handle opening doors and reloading
				if (g_Vars.currentplayer->bondactivateorreload & JO_ACTION_ACTIVATE) {
					if (!currentPlayerInteract(false)) {

						// n64 behavior: interact sucessful, cancel reload
						if (!PLAYER_EXTCFG().extcontrols || PLAYER_EXTCFG().usereloads) {
							g_Vars.currentplayer->bondactivateorreload = (g_Vars.currentplayer->bondactivateorreload & ~JO_ACTION_RELOAD);
						}
					}
				} else if (g_Vars.currentplayer->eyespy
						&& g_Vars.currentplayer->eyespy->active
						&& g_Vars.currentplayer->eyespy->opendoor) {
					currentPlayerInteract(true);
					g_Vars.currentplayer->bondactivateorreload = (g_Vars.currentplayer->bondactivateorreload & ~JO_ACTION_RELOAD);
				}

				if (g_Vars.currentplayer->bondactivateorreload & JO_ACTION_RELOAD) {
					if (g_Vars.currentplayer->hands[HAND_RIGHT].state != HANDSTATE_RELOAD) {
						bgunReloadIfPossible(HAND_RIGHT);
					}
					if (g_Vars.currentplayer->hands[HAND_LEFT].state != HANDSTATE_RELOAD) {
						bgunReloadIfPossible(HAND_LEFT);
					}
					g_Vars.currentplayer->bondactivateorreload = (g_Vars.currentplayer->bondactivateorreload & ~JO_ACTION_RELOAD);
				}

				propsTestForPickup();

				gdl = bgRender(gdl);
				gdl = propsRenderBeams(gdl);
				gdl = shardsRender(gdl);
				gdl = sparksRender(gdl);
				gdl = weatherRender(gdl);

				if (g_NbombsActive) {
					gdl = nbombsRender(gdl);
				}

				gdl = playerRenderHud(gdl);

				static struct sndstate *g_CutsceneStaticAudioHandle = NULL;
				static int g_CutsceneStaticTimer = 100;
				static uint8_t g_CutsceneStaticActive = false;
				bool cutscenehasstatic = false;
				uint32_t alpha;

				if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
					// Handle visual effects in cutscenes
					switch (g_CutsceneAnimNum) {
					case ANIM_CUT_CAVE_INTRO_CAM:
						// Horizon scanner in Air Base intro
						if (g_CutsceneCurAnimFrame60 > 839 && g_CutsceneCurAnimFrame60 < 1411) {
							gdl = bviewDrawHorizonScanner(gdl);
						}
						break;
					case ANIM_CUT_LUE_INTRO_CAM_01:
					case ANIM_CUT_LUE_INTRO_CAM_02:
					case ANIM_CUT_LUE_INTRO_CAM_03:
						{
							// Show static randomly in Infiltration intro
							int cutscenestatic = 0;
							cutscenehasstatic = true;

							if (g_CutsceneStaticAudioHandle == NULL) {
								sndStart(var80095200, SFX_INFIL_STATIC_LONG, &g_CutsceneStaticAudioHandle, -1, -1, -1, -1, -1);
							}

							g_CutsceneStaticTimer -= g_Vars.diffframe60;

							if (g_CutsceneStaticTimer < 0) {
								g_CutsceneStaticTimer = rngRandom() % TICKS(200) + TICKS(40);
								g_CutsceneStaticActive = false;
							}

							gdl = bviewDrawFilmInterlace(gdl, 0xffffffff, 0xffffffff);

							if (g_CutsceneStaticTimer < TICKS(15)) {
								if (g_CutsceneStaticActive == false) {
									g_CutsceneStaticActive = true;
									sndStart(var80095200, SFX_INFIL_STATIC_MEDIUM, NULL, -1, -1, -1, -1, -1);
								}

								cutscenestatic = 225 - g_CutsceneStaticTimer * PALUP(10);
							}

							// Consider a single frame of static, separate
							// to the main static above
							if (rngRandom() % 60 == 1) {
								cutscenestatic = 255;
								sndStart(var80095200, SFX_INFIL_STATIC_SHORT, NULL, -1, -1, -1, -1, -1);
							}

							if (cutscenestatic) {
								gdl = bviewDrawStatic(gdl, 0xffffffff, cutscenestatic);
							}
						}
						break;
					}
					

					if (g_CutsceneStaticAudioHandle && !cutscenehasstatic) {
						audioStop(g_CutsceneStaticAudioHandle);
					}

					// Slayer rocket shows static when flying out of bounds
					if (g_Vars.currentplayer->visionmode == VISIONMODE_SLAYERROCKET
							&& g_Vars.tickmode != TICKMODE_CUTSCENE) {
						gdl = bviewDrawSlayerRocketInterlace(gdl, 0xffffffff, 0xffffffff);

						if (g_Vars.currentplayer->badrockettime > 0) {
							uint32_t slayerstatic = g_Vars.currentplayer->badrockettime * 255 / TICKS(90);

							if (slayerstatic > 255) {
								slayerstatic = 255;
							}

							gdl = bviewDrawStatic(gdl, 0x4fffffff, slayerstatic);
						}
					}

					if (g_Vars.currentplayer->visionmode == VISIONMODE_SLAYERROCKETSTATIC) {
						gdl = bviewDrawStatic(gdl, 0x4fffffff, 255);
						g_Vars.currentplayer->visionmode = VISIONMODE_NORMAL;
					}

					if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY
							&& g_Vars.tickmode != TICKMODE_CUTSCENE) {
						int xraything = 99;

						if (g_Vars.currentplayer->erasertime < TICKS(200)) {
							xraything = 249 - (g_Vars.currentplayer->erasertime * 3 >> 2);
						}

						gdl = bviewDrawZoomBlur(gdl, 0xffffffff, xraything, 1.05f, 1.05f);
					}

					// Handle combat boosts
					if ((g_Vars.speedpillchange > 0 && g_Vars.speedpillchange < 30)
							|| (g_Vars.speedpillwant && !g_Vars.speedpillon)
							|| (!g_Vars.speedpillwant && g_Vars.speedpillon)) {
						if (g_Vars.speedpillchange == 30 && !g_Vars.speedpillwant) {
							sndStart(var80095200, lvGetSlowMotionType() ? SFX_JO_BOOST_ACTIVATE : SFX_ARGH_JO_02AD, 0, -1, -1, -1, -1, -1);
						}

						if (g_Vars.speedpillchange < 15) {
							gdl = bviewDrawZoomBlur(gdl, 0xffffffff,
									g_Vars.speedpillchange * 180 / 15,
									(float)g_Vars.speedpillchange * (PAL ? 0.023076923564076f : 0.02000000141561f) + 1.1f,
									(float)g_Vars.speedpillchange * (PAL ? 0.023076923564076f : 0.02000000141561f) + 1.1f);
							gdl = playerDrawFade(gdl, 0xff, 0xff, 0xff,
									g_Vars.speedpillchange * 0.0066666668280959f);
						} else {
							gdl = bviewDrawZoomBlur(gdl, 0xffffffff,
									(30 - g_Vars.speedpillchange) * 180 / 15,
									(float)(30 - g_Vars.speedpillchange) * 0.02000000141561f + 1.1f,
									(float)(30 - g_Vars.speedpillchange) * 0.02000000141561f + 1.1f);
							gdl = playerDrawFade(gdl, 0xff, 0xff, 0xff,
									(30.0f - g_Vars.speedpillchange) * 0.0066666668280959f);
						}

						if (g_Vars.currentplayernum == 0) {
							if (g_Vars.speedpillwant) {
								g_Vars.speedpillchange++;
							} else {
								g_Vars.speedpillchange--;
							}
						}

						if (g_Vars.speedpillchange > 30) {
							g_Vars.speedpillchange = 30;
						} else if (g_Vars.speedpillchange < 0) {
							g_Vars.speedpillchange = 0;
						}
					}

					if (g_Vars.speedpillchange > 15) {
						g_Vars.speedpillon = true;
					} else {
						g_Vars.speedpillon = false;
					}

					if (bluramount) {
						bviewClearMotionBlur();
						gdl = bviewDrawMotionBlur(gdl, 0xffffffff, bluramount);
					}

					// Handle blur effect in cutscenes (Extraction intro?)
					if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
						float cutsceneblurfrac = playerGetCutsceneBlurFrac();

						if (cutsceneblurfrac > 0) {
							gdl = bviewDrawMotionBlur(gdl, 0xffffff00, cutsceneblurfrac * 255);
						}
					}

					// Render white when teleporting
					if (g_Vars.currentplayer->teleportstate > TELEPORTSTATE_INACTIVE) {
						alpha = 0;

						if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_WHITE) {
							alpha = 255;
						}

						if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_EXITING
								&& g_Vars.currentplayer->teleporttime < 16) {
							alpha = -g_Vars.currentplayer->teleporttime * 16 + 240;
						}

						if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
							if (g_Vars.currentplayer->teleporttime > 32) {
								alpha = g_Vars.currentplayer->teleporttime * 16 - 512;
							}

							if (g_Vars.currentplayer->teleporttime == 48) {
								alpha = 255;
							}
						}

						if (alpha) {
							gdl = textConfigureGfxPipeline(gdl);
							gdl = text0f153a34(gdl,
									viGetViewLeft(), viGetViewTop(),
									viGetViewLeft() + viGetViewWidth(),
									viGetViewTop() + viGetViewHeight(), 0xffffff00 | alpha);
							gdl = text0f153780(gdl);
						}
					}
				}

				gdl = scenarioRenderHud(gdl);
				gdl = lvRenderFade(gdl);

				if (g_FrIsValidWeapon) {
					gdl = frRenderHud(gdl);
				}

				gdl = skyRenderOverexposure(gdl);
				gdl = amRender(gdl);
				mtx00016748(1);

				if (g_Vars.currentplayer->menuisactive) {
					gdl = menuRender(gdl);
				}

				mtx00016748(g_Vars.currentplayerstats->scale_bg2gfx);

				if (g_Vars.mplayerisrunning) {
					gdl = mpRenderModalText(gdl);
				}

				if (g_Vars.currentplayer->dostartnewlife) {
					playerStartNewLife();
				}
			}

			artifactsTick();

			if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
					&& playerHasSharedViewport()
					&& g_Vars.currentplayernum != 0) {
				gdl = savedgdl;
			}
		} // end of player loop
	} // end of stage if-statements

	if (g_Vars.autocutplaying && g_Vars.autocutfinished) {
		g_Vars.autocutplaying = false;
		g_Vars.autocutfinished = false;

		if (g_Vars.autocutgroupskip) {
			g_Vars.autocutgroupcur = -1;
			g_Vars.autocutgroupleft = 0;
		}

		if (g_Vars.autocutgroupcur < 0 && g_Vars.autocutgroupleft <= 0) {
			mainChangeToStage(STAGE_TITLE);
		}
	}

	// Advance the cutscenes when autoplaying
	if (!g_Vars.autocutplaying && g_Vars.autocutgroupcur >= 0 && g_Vars.autocutgroupleft > 0) {
		hudmsgRemoveAll();
		g_Vars.autocutnum = g_Cutscenes[g_Vars.autocutgroupcur].scene;
		g_MissionConfig.iscoop = false;
		g_Vars.mplayerisrunning = false;
		g_Vars.normmplayerisrunning = false;
		g_Vars.bondplayernum = 0;
		g_Vars.coopplayernum = -1;
		g_Vars.antiplayernum = -1;
		g_MissionConfig.isanti = false;
		setNumPlayers(1);
		titleSetNextMode(TITLEMODE_SKIP);
		g_MissionConfig.difficulty = DIFF_A;
		lvSetDifficulty(DIFF_A);
		g_MissionConfig.stageindex = g_Cutscenes[g_Vars.autocutgroupcur].mission;
		g_MissionConfig.stagenum = g_Cutscenes[g_Vars.autocutgroupcur].stage;
		titleSetNextStage(g_Cutscenes[g_Vars.autocutgroupcur].stage);
		mainChangeToStage(g_Cutscenes[g_Vars.autocutgroupcur].stage);
		g_Vars.autocutgroupleft--;

		if (g_Vars.autocutgroupleft > 0) {
			g_Vars.autocutgroupcur++;
		} else {
			g_Vars.autocutgroupcur = -1;
		}
	}

	gDPSetScissor(gdl++, G_SC_NON_INTERLACE, 0, 0, viGetWidth(), viGetHeight());

	if (videoGetDisplayFPS()) {
		gdl = lvRenderFPS(gdl);
	}

	return gdl;
}

uint32_t g_CutsceneTime240_60 = 0;

void lvUpdateSoloHandicaps(void)
{
	if (g_Vars.antiplayernum >= 0) {
		if (g_Difficulty == DIFF_A) {
			g_CctvWaitScale = 2;
			g_CctvDamageRxScale = 2;
			g_AutogunAccuracyScale = 0.5f;
			g_AutogunDamageTxScale = 0.5f;
			g_AutogunDamageRxScale = 2;
			g_EnemyAccuracyScale = 0.5f;
			g_PlayerDamageRxScale = 0.35f;
			g_PlayerDamageTxScale = 4;
			g_ExplosionDamageTxScale = 0.25f;
			g_AutoAimScale = 1.5f;
			g_AmmoQuantityScale = 3;
			g_AttackWalkDurationScale = 0.2f;
		} else if (g_Difficulty == DIFF_SA) {
			g_CctvWaitScale = 2;
			g_CctvDamageRxScale = 1.5f;
			g_AutogunAccuracyScale = 0.5f;
			g_AutogunDamageTxScale = 0.5f;
			g_AutogunDamageRxScale = 1.5f;
			g_EnemyAccuracyScale = 0.6f;
			g_PlayerDamageRxScale = 0.5f;
			g_PlayerDamageTxScale = 3;
			g_ExplosionDamageTxScale = 0.25f;
			g_AutoAimScale = 1.1f;
			g_AmmoQuantityScale = 2.5f;
			g_AttackWalkDurationScale = 0.5f;
		} else {
			g_CctvWaitScale = 2;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 0.5f;
			g_AutogunDamageTxScale = 0.5f;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 0.7f;
			g_PlayerDamageRxScale = 0.65f;
			g_PlayerDamageTxScale = 2;
			g_ExplosionDamageTxScale = 0.25f;
			g_AutoAimScale = 0.75f;
			g_AmmoQuantityScale = 2;
			g_AttackWalkDurationScale = 1;
		}
	} else if (g_Vars.coopplayernum >= 0) {
		if (g_Difficulty == DIFF_A) {
			g_CctvWaitScale = 2;
			g_CctvDamageRxScale = 2;
			g_AutogunAccuracyScale = 0.5f;
			g_AutogunDamageTxScale = 0.5f;
			g_AutogunDamageRxScale = 2;
			g_EnemyAccuracyScale = 0.6f;
			g_PlayerDamageRxScale = 0.5f;
			g_PlayerDamageTxScale = 2;
			g_ExplosionDamageTxScale = 0.25f;
			g_AutoAimScale = 1.5f;
			g_AmmoQuantityScale = 2;
			g_AttackWalkDurationScale = 0.2f;
		} else if (g_Difficulty == DIFF_SA) {
			g_CctvWaitScale = 1;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 0.75f;
			g_AutogunDamageTxScale = 1;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 0.75f;
			g_PlayerDamageRxScale = 1;
			g_PlayerDamageTxScale = 1;
			g_ExplosionDamageTxScale = 1;
			g_AutoAimScale = 0.75f;
			g_AmmoQuantityScale = 1.5f;
			g_AttackWalkDurationScale = 0.5f;
		} else {
			g_CctvWaitScale = 1;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 1;
			g_AutogunDamageTxScale = 1.5f;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 1.5f;
			g_PlayerDamageRxScale = 1.5f;
			g_PlayerDamageTxScale = 1;
			g_ExplosionDamageTxScale = 1.5f;
			g_AutoAimScale = 0.2f;
			g_AmmoQuantityScale = 1;
			g_AttackWalkDurationScale = 1;
		}
	} else {
		if (g_Difficulty == DIFF_A) {
			float totalhealth;
			float frac = 1;

			if (g_Vars.coopplayernum < 0 && g_Vars.antiplayernum < 0) {
				totalhealth = playerGetHealthFrac() + playerGetShieldFrac();

				if (totalhealth <= 0.125f) {
					frac = 0.5f;
				} else if (totalhealth <= 0.6f) {
					frac = (totalhealth - 0.125f) * 0.5f / 0.47500002384186f + 0.5f;
				}
			}

			g_CctvWaitScale = 2;
			g_CctvDamageRxScale = 2;
			g_AutogunAccuracyScale = 0.5f * frac;
			g_AutogunDamageTxScale = 0.5f * frac;
			g_AutogunDamageRxScale = 2;
			g_EnemyAccuracyScale = 0.6f;
			g_PlayerDamageRxScale = 0.5f * frac;
			g_PlayerDamageTxScale = 2;
			g_ExplosionDamageTxScale = 0.25f * frac;
			g_AutoAimScale = 1.5f;
			g_AmmoQuantityScale = 2;
			g_AttackWalkDurationScale = 0.2f;
		} else if (g_Difficulty == DIFF_SA) {
			g_CctvWaitScale = 1;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 0.75f;
			g_AutogunDamageTxScale = 0.75f;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 0.8f;
			g_PlayerDamageRxScale = 0.6f;
			g_PlayerDamageTxScale = 1;
			g_ExplosionDamageTxScale = 0.75f;
			g_AutoAimScale =  0.75f;
			g_AmmoQuantityScale = 1.5f;
			g_AttackWalkDurationScale = 0.5f;
		} else if (g_Difficulty == DIFF_PA) {
			g_CctvWaitScale = 1;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 1;
			g_AutogunDamageTxScale = 1;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 1.175f;
			g_PlayerDamageRxScale = 1;
			g_PlayerDamageTxScale = 1;
			g_ExplosionDamageTxScale = 1;
			g_AutoAimScale = 0.2f;
			g_AmmoQuantityScale = 1;
			g_AttackWalkDurationScale = 1;
		} else if (g_Difficulty == DIFF_PD) {
			g_CctvWaitScale = 1;
			g_CctvDamageRxScale = 1;
			g_AutogunAccuracyScale = 1;
			g_AutogunDamageTxScale = 1;
			g_AutogunDamageRxScale = 1;
			g_EnemyAccuracyScale = 1.1f;
			g_PlayerDamageRxScale = 1;
			g_PlayerDamageTxScale = 1;
			g_ExplosionDamageTxScale = 1;
			g_AutoAimScale = 1;
			g_AmmoQuantityScale = 1;
			g_AttackWalkDurationScale = 1;
		}
	}
}

void lvUpdateCutsceneTime(void)
{
	if (g_Vars.in_cutscene) {
		g_CutsceneTime240_60 += g_Vars.lvupdate60;
		return;
	}

	g_CutsceneTime240_60 = 0;
}

int lvGetSlowMotionType(void)
{
	if (g_Vars.normmplayerisrunning) {
		if (g_MpSetup.options & MPOPTION_SLOWMOTION_ON) {
			return SLOWMOTION_ON;
		}
		if (g_MpSetup.options & MPOPTION_SLOWMOTION_SMART) {
			return SLOWMOTION_SMART;
		}
	} else {
		if (cheatIsActive(CHEAT_SLOMO)) {
			return SLOWMOTION_ON;
		}
	}

	return SLOWMOTION_OFF;
}

void lvTick(void)
{
	int j;
	int i;

	lvCheckPauseStateChanged();

	if (g_Vars.pakstocheck) {
		paksTick();
	}

	if (g_Vars.joydisableframestogo > 0) {
		g_Vars.joydisableframestogo--;
	} else if (g_Vars.joydisableframestogo == 0) {
		joyUnlockCyclicPolling();

		if (g_Vars.stagenum == STAGE_TITLE
				|| g_Vars.stagenum == STAGE_BOOTPAKMENU
				|| g_Vars.stagenum == STAGE_CREDITS) {
			g_Vars.paksneededforgame = 0;
		} else {
			g_Vars.paksneededforgame = 0x1f;
			pakEnableRumbleForAllPlayers();
		}

		g_Vars.joydisableframestogo = -1;
	}

	for (j = 0; j < PLAYERCOUNT(); j++) {
		g_Vars.players[j]->hands[HAND_LEFT].hasdotinfo = false;
		g_Vars.players[j]->hands[HAND_RIGHT].hasdotinfo = false;
	}

	if (lvIsPaused()) {
		g_Vars.lvupdate240 = 0;
	} else if (mpIsPaused()) {
		g_Vars.lvupdate240 = 0;

		for (j = 0; j < PLAYERCOUNT(); j++) {
			g_Vars.players[j]->joybutinhibit = 0xffffefff;
		}
	} else {
		int slowmo = lvGetSlowMotionType();
		g_Vars.lvupdate240 = g_Vars.diffframe240;

		if (slowmo == SLOWMOTION_ON) {
			if (g_Vars.speedpillon == false || g_Vars.in_cutscene) {
				if (g_Vars.lvupdate240 > LV_SLOMO_TICK_CAP) {
					g_Vars.lvupdate240 = LV_SLOMO_TICK_RATE;
				}
			}
		} else if (slowmo == SLOWMOTION_SMART) {
			// Smart slow motion - activates if an enemy chr is nearby
			if (g_Vars.speedpillon == false || g_Vars.in_cutscene) {
				if (g_Vars.mplayerisrunning) {
					bool foundnearbychr = false;
					int playernum;

					// Check if another player is in a nearby room
					for (playernum = 0; playernum < PLAYERCOUNT() && !foundnearbychr; playernum++) {
						if (g_Vars.players[playernum]->isdead == false) {
							RoomNum *rooms = g_Vars.players[playernum]->prop->rooms;
							int r;

							for (r = 0; rooms[r] != -1 && !foundnearbychr; r++) {
								int otherplayernum;
								for (otherplayernum = 0; otherplayernum < PLAYERCOUNT(); otherplayernum++) {
									if (playernum != otherplayernum
											&& g_Vars.players[otherplayernum]->isdead == false
											&& bgRoomIsOnPlayerScreen(rooms[r], otherplayernum)) {
										foundnearbychr = true;
									}
								}
							}
						}
					}

					if (foundnearbychr) {
						if (g_Vars.lvupdate240 > LV_SLOMO_TICK_CAP) {
							g_Vars.lvupdate240 = LV_SLOMO_TICK_RATE;
						}
					} else {
						if (g_Vars.lvupdate240 > TICKS(8)) {
							g_Vars.lvupdate240 = TICKS(8);
						}
					}
				} else {
					if (g_Vars.lvupdate240 > LV_SLOMO_TICK_CAP) {
						g_Vars.lvupdate240 = LV_SLOMO_TICK_RATE;
					}
				}
			}
		} else {
			// Slow motion settings are off
			if (g_Vars.speedpillon && g_Vars.in_cutscene == false) {
				if (g_Vars.lvupdate240 > LV_SLOMO_TICK_CAP) {
					g_Vars.lvupdate240 = LV_SLOMO_TICK_RATE;
				}
			}
		}
	}

	g_Vars.lvupdate60 = g_Vars.lvupdate240 + g_Vars.lvupdate240rem;
	g_Vars.lvupdate240rem = g_Vars.lvupdate60 & 3;
	g_Vars.lvupdate60 >>= 2;

	if (g_Vars.lvupdate240 > 0) {
		g_Vars.lvframenum++;
	}

	g_Vars.lvupdate60f = g_Vars.lvupdate240 * 0.25f;
	g_Vars.lvframe60 += g_Vars.lvupdate60;
	g_Vars.lvframe240 += g_Vars.lvupdate240;
	g_Vars.lvupdate60frealprev = g_Vars.lvupdate60freal;
	g_Vars.lvupdate60freal = PALUPF(g_Vars.lvupdate60f);

	bgunTickBoost();
	hudmsgsTick();

	if ((joyGetButtonsPressedThisFrame(0, 0xffffffff) != 0
				|| joyGetStickX(0) > 10
				|| joyGetStickX(0) < -10
				|| joyGetStickY(0) > 10
				|| joyGetStickY(0) < -10
				|| joyGetButtonsPressedThisFrame(1, 0xffffffff) != 0
				|| joyGetStickX(1) > 10
				|| joyGetStickX(1) < -10
				|| joyGetStickY(1) > 10
				|| joyGetStickY(1) < -10
				|| joyGetButtonsPressedThisFrame(2, 0xffffffff) != 0
				|| joyGetStickX(2) > 10
				|| joyGetStickX(2) < -10
				|| joyGetStickY(2) > 10
				|| joyGetStickY(2) < -10
				|| joyGetButtonsPressedThisFrame(3, 0xffffffff) != 0
				|| joyGetStickX(3) > 10
				|| joyGetStickX(3) < -10
				|| joyGetStickY(3) > 10
				|| joyGetStickY(3) < -10) && g_IsTitleDemo) {
		if (g_Vars.stagenum != STAGE_TITLE) {
			titleSetNextMode(TITLEMODE_SKIP);
			mainChangeToStage(STAGE_TITLE);
		}

		g_IsTitleDemo = false;
	}

	if (g_Vars.stagenum < STAGE_TITLE && !g_IsTitleDemo && !g_Vars.in_cutscene) {
		if (joyGetButtons(0, 0xffffffff) == 0
				&& joyGetStickX(0) < 10
				&& joyGetStickX(0) > -10
				&& joyGetStickY(0) < 10
				&& joyGetStickY(0) > -10
				&& joyGetButtons(1, 0xffffffff) == 0
				&& joyGetStickX(1) < 10
				&& joyGetStickX(1) > -10
				&& joyGetStickY(1) < 10
				&& joyGetStickY(1) > -10
				&& joyGetButtons(2, 0xffffffff) == 0
				&& joyGetStickX(2) < 10
				&& joyGetStickX(2) > -10
				&& joyGetStickY(2) < 10
				&& joyGetStickY(2) > -10
				&& joyGetButtons(3, 0xffffffff) == 0
				&& joyGetStickX(3) < 10
				&& joyGetStickX(3) > -10
				&& joyGetStickY(3) < 10
				&& joyGetStickY(3) > -10) {
			g_TitleIdleTime60 += g_Vars.diffframe60;
		} else {
			g_TitleIdleTime60 = 0;
		}
	} else {
		g_TitleIdleTime60 = 0;
	}

	g_NumReasonsToEndMpMatch = 0;

	// Handle MP match ending
	if (g_Vars.normmplayerisrunning && g_Vars.stagenum < STAGE_TITLE) {
		if (g_MpTimeLimit60 > 0) {
			int elapsed = g_StageTimeElapsed60;
			int nexttime = g_Vars.lvupdate60 + g_StageTimeElapsed60;
			int warntime = TICKS(g_MpTimeLimit60) - TICKS(3600);

			// Show HUD message at one minute remaining
			if (elapsed < warntime && nexttime >= warntime) {
				int i;

				for (i = 0; i < PLAYERCOUNT(); i++) {
					setCurrentPlayerNum(i);
					hudmsgCreate(langGet(L_MISC_068), HUDMSGTYPE_DEFAULT); // "One minute left."
				}
			}

			if (elapsed < TICKS(g_MpTimeLimit60) && nexttime >= TICKS(g_MpTimeLimit60)) {
				// Match is ending due to time limit reached
				mainEndStage();
			}

			// Sound alarm at 10 seconds remaining
			if (nexttime >= TICKS(g_MpTimeLimit60) - TICKS(600)
					&& g_MiscAudioHandle == NULL
					&& !lvIsPaused()
					&& nexttime < TICKS(g_MpTimeLimit60)) {
				snd00010718(&g_MiscAudioHandle, 0, AL_VOL_FULL, AL_PAN_CENTER, SFX_ALARM_DEFAULT, 1, 1, -1, true);
			}
		}

		if (g_Vars.lvupdate240 != 0) {
			int numdying = 0;

			for (i = 0; i < PLAYERCOUNT(); i++) {
				if (g_Vars.players[i]->isdead) {
					if (g_Vars.players[i]->redbloodfinished == false
							|| g_Vars.players[i]->deathanimfinished == false
							|| g_Vars.players[i]->colourfadetimemax60 >= 0) {
						numdying++;
					}
				}
			}

			for (i = 0; i < g_MpNumChrs; i++) {
				if (g_MpAllChrPtrs[i]->actiontype == ACT_DIE) {
					numdying++;
				}
			}

			if (g_MpScoreLimit > 0) {
				struct ranking rankings[MAX_MPCHRS];
				int count = mpGetPlayerRankings(rankings);

				for (i = 0; i < count; i++) {
					if (rankings[i].score >= g_MpScoreLimit) {
						g_NumReasonsToEndMpMatch++;
					}
				}
			}

			if (g_MpTeamScoreLimit > 0) {
				struct ranking rankings[MAX_MPCHRS];
				int count = mpGetTeamRankings(rankings);

				for (i = 0; i < count; i++) {
					if (rankings[i].score >= g_MpTeamScoreLimit) {
						g_NumReasonsToEndMpMatch++;
					}
				}
			}

			if (g_NumReasonsToEndMpMatch > 0 && numdying == 0) {
				mainEndStage();
			}
		}
	}

	g_StageTimeElapsed60 += g_Vars.lvupdate60;
	g_StageTimeElapsed1f = g_StageTimeElapsed60 / TICKS(60.0f);

	viSetUseZBuf(true);

	if (g_Vars.stagenum == STAGE_TITLE) {
		titleTick();
		musicTick();
	} else if (g_Vars.stagenum == STAGE_BOOTPAKMENU) {
		setCurrentPlayerNum(0);
		menuTick();
		musicTick();
		pakExecuteDebugOperations();
	} else if (g_Vars.stagenum == STAGE_CREDITS) {
		musicTick();
	} else {
		lvUpdateCutsceneTime();
		vtxstoreTick();
		lvUpdateSoloHandicaps();
		roomsTick();
		skyTick();
		casingsTick();
		shardsTick();
		sparksTick();
		wallhitsTick();

		if (g_WeatherActive) {
			weatherTick();
		}

		if (g_NbombsActive) {
			nbombsTick();
		}

		lvUpdateMiscSfx();
		sndTick();
		pakExecuteDebugOperations();
		lightingTick();
		modelmgrPrintCounts();
		boltbeamsTick();
		amTick();
		menuTick();
		scenarioTick();

		if (!g_MainIsEndscreen) {
			propsTick();
		}

		musicTick();
		propsTickPadEffects();

		if (mainGetStageNum() == STAGE_CITRAINING) {
			struct trainingdata *trainingdata = dtGetData();

			if ((g_Vars.currentplayer->prop->rooms[0] < ROOM_DISH_HOLO1 || g_Vars.currentplayer->prop->rooms[0] > ROOM_DISH_HOLO4)
					&& g_Vars.currentplayer->prop->rooms[0] != ROOM_DISH_FIRINGRANGE
					&& (trainingdata == NULL || trainingdata->intraining == false)) {
				chrUnsetStageFlag(NULL, STAGEFLAG_CI_IN_TRAINING);
			}

			frTick();

			if (g_Vars.lvupdate240 != 0) {
				dtTick();
				htTick();
			}
		}
	}
}

void lvTickPlayer(void)
{
	float xdiff;
	float zdiff;

	playerTick();



	xdiff = g_Vars.currentplayer->prop->pos.x - g_Vars.currentplayer->bondprevpos.x;
	zdiff = g_Vars.currentplayer->prop->pos.z - g_Vars.currentplayer->bondprevpos.z;

	g_Vars.currentplayerstats->distance += sqrtf(xdiff * xdiff + zdiff * zdiff);
}

void lvStop(void)
{
	paksStop(true);

	if (g_MiscAudioHandle && sndGetState(g_MiscAudioHandle)) {
		audioStop(g_MiscAudioHandle);
	}

	chrmgrStop();
	explosionsStop();
	smokeStop();
	shardsStop();
	propsStop();
	objsStop();
	weatherStop();
	objectivesStop();
	bgunStop();
	psStop();
	musicStop();
	hudmsgsStop();

	if (g_Vars.stagenum < STAGE_TITLE) {
		bgStop();
	}

	func00033dd8();

	if (g_FileState == FILESTATE_CHANGINGAGENT) {
		menuPlaySound(MENUSOUND_EXPLOSION);
		g_FileState = FILESTATE_UNSELECTED;
	}
	menuStop();
}

void lvCheckPauseStateChanged(void)
{
	uint32_t paused = mpIsPaused();

	if (paused != g_LvlIsPausedMP) {
		if (paused) {
			pakDisableRumbleForAllPlayers();
		} else {
			pakEnableRumbleForAllPlayers();
		}
	}

	g_LvlIsPausedMP = paused;
}

void lvSetPaused(bool paused)
{
	if (paused) {
		pakDisableRumbleForAllPlayers();
		snd0000fe20();
	} else {
		snd0000fe50();
		pakEnableRumbleForAllPlayers();
	}

	g_IsLvlPaused = paused;
}

bool lvIsPaused(void)
{
	return g_IsLvlPaused;
}

int lvGetDifficulty(void)
{
	return g_Difficulty;
}

void lvSetDifficulty(int difficulty)
{
	if (difficulty < DIFF_A || difficulty > DIFF_PD) {
		difficulty = DIFF_A;
	}

	g_Difficulty = difficulty;
}

void lvSetMpTimeLimit60(uint32_t limit)
{
	g_MpTimeLimit60 = limit;
}

void lvSetMpScoreLimit(uint32_t limit)
{
	g_MpScoreLimit = limit;
}

void lvSetMpTeamScoreLimit(uint32_t limit)
{
	g_MpTeamScoreLimit = limit;
}

float lvGetStageTimeInSeconds(void)
{
	return g_StageTimeElapsed1f;
}

int lvGetStageTime60(void)
{
	return g_StageTimeElapsed60;
}