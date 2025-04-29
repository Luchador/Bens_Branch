#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/activemenu.h"
#include "game/bondbike.h"
#include "game/bondgrab.h"
#include "game/bondmove.h"
#include "game/bondwalk.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/footstep.h"
#include "game/menuutils.h"
#include "game/mtxutils.h"
#include "game/chr.h"
#include "game/prop.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/weaponutils.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/bondcutscene.h"
#include "game/bondhead.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/lv.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "lib/joy.h"
#include "lib/lib_17ce0.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "input.h"
#include "video.h"

static void bgunProcessQuickDetonate(struct movedata *data, uint32_t c1buttons, uint32_t c1buttonsthisframe, uint32_t buttons1, uint32_t buttons2) {
	if ((((c1buttons & (buttons1)) && (c1buttonsthisframe & (buttons2)))
			|| ((c1buttons & (buttons2)) && (c1buttonsthisframe & (buttons1))))
			&& bgunGetWeaponNum(HAND_RIGHT) == WEAPON_REMOTEMINE) {
		data->detonating = true;
		data->weaponbackoffset = 0;
		data->weaponforwardoffset = 0;
		data->btapcount = 0;
		// prevent the previous slotnum
		// from causing Jo to switch weapons
		g_AmMenus[g_AmIndex].slotnum = 4;
		amClose();
		g_Vars.currentplayer->invdowntime = -2;
		g_Vars.currentplayer->usedowntime = -2;
	}
}

static void bgunProcessInputAltButton(struct movedata *data, int8_t contpad, int i)
{
	int buttons = joyGetButtonsOnSample(i, contpad, 0xffffffff);
	if (buttons & (BUTTON_ALTMODE)) {
		if (g_Vars.currentplayer->altdowntime >= -1) {
			if (buttons & (Z_TRIG)
					&& g_Vars.currentplayer->altdowntime >= 0
					&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, true, false, true) != USETIMER_CONTINUE) {
				g_Vars.currentplayer->altdowntime = -3;
			}
			if (g_Vars.currentplayer->altdowntime != -4) {
				if (g_Vars.currentplayer->altdowntime <= 0) {
					g_Vars.currentplayer->altdowntime++;
				}
			}
		} else  {
			if (g_Vars.currentplayer->altdowntime == -2) {
				bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, false, false, true);
				g_Vars.currentplayer->altdowntime = -4;
			}
		}
	} else if (buttons & (BUTTON_CANCEL_USE | BUTTON_ACCEPT_USE)) {
		if (g_Vars.currentplayer->altdowntime >= -1) {
			if (buttons & (Z_TRIG)
					&& g_Vars.currentplayer->altdowntime >= 0
					&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, true, false, true) != USETIMER_CONTINUE) {
				g_Vars.currentplayer->altdowntime = -3;
			}
		}
	} else {
		// Released L
		if (g_Vars.currentplayer->altdowntime != 0) {
			const bool trigpressed = (g_Vars.currentplayer->altdowntime == -3);
			int result = bgunConsiderToggleGunFunction(g_Vars.currentplayer->altdowntime, trigpressed, false, true);
			if (result == USETIMER_STOP) {
				g_Vars.currentplayer->altdowntime = -1;
			} else if (result == USETIMER_REPEAT) {
				g_Vars.currentplayer->altdowntime = -2;
			}
		}
		g_Vars.currentplayer->altdowntime = 0;
		bgunDisallowInvertFunc();
	}
}

void bmoveSetControlDef(uint32_t controldef)
{
	g_Vars.currentplayer->controldef = controldef;
}

void bmoveSetAutoMoveCentreEnabled(bool enabled)
{
	g_Vars.currentplayer->automovecentreenabled = enabled;
}

bool bmoveIsAutoMoveCentreEnabled(void)
{
	return g_Vars.currentplayer->automovecentreenabled;
}

void bmoveSetAutoAimY(bool enabled)
{
	g_Vars.currentplayer->autoyaimenabled = enabled;
}

bool bmoveIsAutoAimYEnabled(void)
{
	if (!g_Vars.normmplayerisrunning) {
		return g_Vars.currentplayer->autoyaimenabled;
	}

	if (g_MpSetup.options & MPOPTION_NOAUTOAIM) {
		return false;
	}

	return optionsGetAutoAim(g_Vars.currentplayerstats->mpindex);
}

bool bmoveIsAutoAimYEnabledForCurrentWeapon(void)
{
	struct weaponfunc *func = currentPlayerGetWeaponFunction(0);

	if (func) {
		if (func->flags & FUNCFLAG_NOAUTOAIM) {
			return false;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			return true;
		}
	}

	return bmoveIsAutoAimYEnabled();
}

bool bmoveIsInSightAimMode(void)
{
	return g_Vars.currentplayer->insightaimmode;
}

void bmoveUpdateAutoAimYProp(struct prop *prop, float autoaimy)
{
	if (g_Vars.currentplayer->autoyaimtime60 >= 0) {
		g_Vars.currentplayer->autoyaimtime60 -= g_Vars.lvupdate60;
	}

	if (prop != g_Vars.currentplayer->autoyaimprop) {
		if (g_Vars.currentplayer->autoyaimtime60 < 0) {
			g_Vars.currentplayer->autoyaimtime60 = TICKS(30);
			g_Vars.currentplayer->autoyaimprop = prop;
		} else {
			return;
		}
	}

	g_Vars.currentplayer->autoaimy = autoaimy;
}

void bmoveSetAutoAimX(bool enabled)
{
	g_Vars.currentplayer->autoxaimenabled = enabled;
}

bool bmoveIsAutoAimXEnabled(void)
{
	if (!g_Vars.normmplayerisrunning) {
		return g_Vars.currentplayer->autoxaimenabled;
	}

	if (g_MpSetup.options & MPOPTION_NOAUTOAIM) {
		return false;
	}

	return optionsGetAutoAim(g_Vars.currentplayerstats->mpindex);
}

bool bmoveIsAutoAimXEnabledForCurrentWeapon(void)
{
	struct weaponfunc *func = currentPlayerGetWeaponFunction(0);

	if (func) {
		if (func->flags & FUNCFLAG_NOAUTOAIM) {
			return false;
		}

		if ((func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
			return true;
		}
	}

	return bmoveIsAutoAimXEnabled();
}

void bmoveUpdateAutoAimXProp(struct prop *prop, float autoaimx)
{
	if (g_Vars.currentplayer->autoxaimtime60 >= 0) {
		g_Vars.currentplayer->autoxaimtime60 -= g_Vars.lvupdate60;
	}

	if (prop != g_Vars.currentplayer->autoxaimprop) {
		if (g_Vars.currentplayer->autoxaimtime60 < 0) {
			g_Vars.currentplayer->autoxaimtime60 = TICKS(30);
			g_Vars.currentplayer->autoxaimprop = prop;
		} else {
			return;
		}
	}

	g_Vars.currentplayer->autoaimx = autoaimx;
}

struct prop *bmoveGetHoverbike(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		return g_Vars.currentplayer->hoverbike;
	}

	return NULL;
}

struct prop *bmoveGetGrabbedProp(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		return g_Vars.currentplayer->grabbedprop;
	}

	return NULL;
}

void bmoveGrabProp(struct prop *prop)
{
	struct defaultobj *obj = prop->obj;

	if ((obj->hidden & OBJHFLAG_MOUNTED) == 0 && (obj->hidden & OBJHFLAG_GRABBED) == 0) {
		g_Vars.currentplayer->grabbedprop = prop;
		bgrabInit();
	}
}

void bmoveSetMode(uint32_t movemode)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabExit();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeExit();
	}

	if (movemode == MOVEMODE_BIKE) {
		bbikeInit();
	} else if (movemode == MOVEMODE_GRAB) {
		bgrabInit();
	} else if (movemode == MOVEMODE_CUTSCENE) {
		bcutsceneInit();
	} else if (movemode == MOVEMODE_WALK) {
		bwalkInit();
	}
}

void bmoveSetModeForAllPlayers(uint32_t movemode)
{
	uint32_t prevplayernum = g_Vars.currentplayernum;
	int i;

	for (i = 0; i < PLAYERCOUNT(); i++) {
		setCurrentPlayerNum(i);
		bmoveSetMode(movemode);
	}

	setCurrentPlayerNum(prevplayernum);
}

void bmoveHandleActivate(void)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeHandleActivate();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabHandleActivate();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkHandleActivate();
	}
}

void bmoveApplyMoveData(struct movedata *data)
{
	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeApplyMoveData(data);
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabApplyMoveData(data);
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkApplyMoveData(data);
	}
}

void bmoveUpdateSpeedTheta(void)
{
	switch (g_Vars.currentplayer->bondmovemode) {
	case MOVEMODE_GRAB:
		bgrabUpdateSpeedTheta();
		break;

	case MOVEMODE_BIKE:
	case MOVEMODE_WALK:
	default:
		// No update needed for these modes
		break;
	}
}

float bmoveGetSpeedVertaLimit(float value)
{
	if (value > 0) {
		return (viGetFovY() * value * -0.7f) / 60.0f;
	}

	if (value < 0) {
		return (viGetFovY() * -value * 0.7f) / 60.0f;
	}

	return 0;
}

void bmoveUpdateSpeedVerta(float value)
{
	float mult = viGetFovY() / 60.0f;
	float limit = bmoveGetSpeedVertaLimit(value);

	if (value > 0) {
		if (g_Vars.currentplayer->speedverta > 0) {
			g_Vars.currentplayer->speedverta -= 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedverta -= 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedverta < limit) {
			g_Vars.currentplayer->speedverta = limit;
		}
	} else if (value < 0) {
		if (g_Vars.currentplayer->speedverta < 0) {
			g_Vars.currentplayer->speedverta += 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedverta += 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedverta > limit) {
			g_Vars.currentplayer->speedverta = limit;
		}
	} else {
		if (g_Vars.currentplayer->speedverta > limit) {
			g_Vars.currentplayer->speedverta -= 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedverta < limit) {
				g_Vars.currentplayer->speedverta = limit;
			}
		} else {
			g_Vars.currentplayer->speedverta += 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedverta > limit) {
				g_Vars.currentplayer->speedverta = limit;
			}
		}
	}
}

float bmoveGetSpeedThetaControlLimit(float value)
{
	if (value > 0) {
		return (viGetFovY() * value * -0.7f) / 60.0f;
	}

	if (value < 0) {
		return (viGetFovY() * -value * 0.7f) / 60.0f;
	}

	return 0;
}

void bmoveUpdateSpeedThetaControl(float value)
{
	float mult = viGetFovY() / 60.0f;
	float limit = bmoveGetSpeedThetaControlLimit(value);

	if (value > 0) {
		if (g_Vars.currentplayer->speedthetacontrol > 0) {
			g_Vars.currentplayer->speedthetacontrol -= 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedthetacontrol -= 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedthetacontrol < limit) {
			g_Vars.currentplayer->speedthetacontrol = limit;
		}
	} else if (value < 0) {
		if (g_Vars.currentplayer->speedthetacontrol < 0.0f) {
			g_Vars.currentplayer->speedthetacontrol += 0.05f * g_Vars.lvupdate60freal * mult;
		} else {
			g_Vars.currentplayer->speedthetacontrol += 0.0125f * g_Vars.lvupdate60freal * mult;
		}

		if (g_Vars.currentplayer->speedthetacontrol > limit) {
			g_Vars.currentplayer->speedthetacontrol = limit;
		}
	} else {
		if (g_Vars.currentplayer->speedthetacontrol > limit) {
			g_Vars.currentplayer->speedthetacontrol -= 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedthetacontrol < limit) {
				g_Vars.currentplayer->speedthetacontrol = limit;
			}
		} else {
			g_Vars.currentplayer->speedthetacontrol += 0.05f * g_Vars.lvupdate60freal * mult;

			if (g_Vars.currentplayer->speedthetacontrol > limit) {
				g_Vars.currentplayer->speedthetacontrol = limit;
			}
		}
	}
}

/**
 * Calculate the lookahead angle.
 *
 * The return value is the intended vertical angle to look at.
 * 90 = straight up
 * 0 = horizontal
 * -90 = straight down
 */
float bmoveCalculateLookahead(void)
{
	float result = -4.0f;
	float sp160 = 400.0f;
	float ground = g_Vars.currentplayer->vv_ground;
	struct coord sp150;
	float ymax;
	float ymin;
	float radius;
	float angles[5];
	bool populated[5];
	int numpopulated = 0;
	uint16_t flags = 0;
	struct coord sp100;
	struct coord spf0;
	RoomNum spe0[8];
	int i;
	float angle;
	float value;
	struct coord spbc;
	struct coord spb0;
	RoomNum spa0[8];
	RoomNum sp90[8];
	RoomNum sp80[8];
	int j;
	float sp78;
	int indextoremove;
	float angletoremove;

	if (g_Vars.currentplayer->inlift) {
		return result;
	}

	playerGetBbox(g_Vars.currentplayer->prop, &radius, &ymax, &ymin);

	sp100.x = g_Vars.currentplayer->bond2.unk00.x;
	sp100.y = g_Vars.currentplayer->bond2.unk00.y;
	sp100.z = g_Vars.currentplayer->bond2.unk00.z;

	spf0.x = g_Vars.currentplayer->prop->pos.x;
	spf0.y = g_Vars.currentplayer->prop->pos.y - 30;
	spf0.z = g_Vars.currentplayer->prop->pos.z;

	portalTraceLineThroughRooms(&g_Vars.currentplayer->prop->pos, &spf0,
			g_Vars.currentplayer->prop->rooms, spe0, NULL, 0);

	sp150.x = sp100.x * 400 + spf0.x;
	sp150.y = sp100.y * 400 + spf0.y;
	sp150.z = sp100.z * 400 + spf0.z;

	if (cdExamLos08(&spf0, spe0, &sp150,
				CDTYPE_BG | CDTYPE_CLOSEDDOORS,
				GEOFLAG_FLOOR1 | GEOFLAG_FLOOR2 | GEOFLAG_WALL | GEOFLAG_BLOCK_SIGHT) == CDRESULT_COLLISION) {
		cdGetPos(&sp150);
		flags = cdGetGeoFlags();

		sp160 = sqrtf((sp150.x - spf0.x) * (sp150.x - spf0.x)
				+ (sp150.y - spf0.y) * (sp150.y - spf0.y)
				+ (sp150.z - spf0.z) * (sp150.z - spf0.z));
	}

	if (sp160 > 60.0f || (flags & GEOFLAG_FLOOR1)) {
		for (i = 0; i < ARRAYCOUNT(populated); i++) {
			populated[i] = false;
			value = (i + 1) * sp160 * 0.2f;

			spbc.x = sp100.x * value + spf0.x;
			spbc.y = sp100.y * value + spf0.y;
			spbc.z = sp100.z * value + spf0.z;

			portalTraceLineThroughRooms(&spf0, &spbc, spe0, spa0, NULL, 0);

			spb0.x = spbc.x;
			spb0.y = spbc.y - 400;
			spb0.z = spbc.z;

			portalTraceLineThroughRooms(&spbc, &spb0, spa0, sp90, sp80, 7);

			if ( cdFindFloorRoomYColourFlagsAtPos(&spbc, sp80, &sp78, NULL, NULL) > 0
					&& sp78 - ground < 200
					&& sp78 - ground > -200) {
				angle = atan2f(sp78 - g_Vars.currentplayer->vv_ground, value);
				angle = (angle * 360) / M_TAU + -4;

				if (angle >= 180) {
					angle -= 360;
				}

				if (angle >= -50 && angle <= 40) {
					populated[i] = true;
					angles[i] = angle;
					numpopulated++;
					ground = sp78;
				}
			}
		}

		for (i = 0; i < numpopulated - 1; i++) {
			indextoremove = -1;

			for (j = 0; j < ARRAYCOUNT(populated); j++) {
				if (populated[j]) {
					if (indextoremove < 0) {
						indextoremove = j;
						angletoremove = angles[j];
					} else if (i & 1) {
						if (angletoremove > angles[j]) {
							indextoremove = j;
							angletoremove = angles[j];
						}
					} else {
						if (angletoremove < angles[j]) {
							indextoremove = j;
							angletoremove = angles[j];
						}
					}
				}
			}

			if (indextoremove >= 0) {
				populated[indextoremove] = false;
			}
		}

		for (i = 0; i < ARRAYCOUNT(populated); i++) {
			if (populated[i]) {
				result = angles[i];
				break;
			}
		}
	}

	if (result > 0.0f) {
		result *= 0.86666667461395f;
	}

	return result;
}

void bmoveResetMoveData(struct movedata *data)
{
	data->canswivelgun = 0;
	data->canmanualaim = 0;
	data->triggeron = false;
	data->btapcount = 0;
	data->canlookahead = false;
	data->unk14 = 0;
	data->cannaturalturn = false;
	data->cannaturalpitch = false;
	data->digitalstepforward = false;
	data->digitalstepback = false;
	data->digitalstepleft = false;
	data->digitalstepright = false;
	data->weaponbackoffset = 0;
	data->weaponforwardoffset = 0;
	data->unk50 = 0;
	data->aiming = false;
	data->zooming = false;
	data->crouchdown = false;
	data->crouchup = false;
	data->rleanleft = false;
	data->rleanright = false;
	data->detonating = false;
	data->canautoaim = false;
	data->farsighttempautoseek = false;
	data->eyesshut = false;
	data->unk30 = 0;
	data->unk34 = 0;
	data->speedvertadown = 0;
	data->speedvertaup = 0;
	data->aimturnleftspeed = 0;
	data->aimturnrightspeed = 0;
	data->zoomoutfovpersec = 0;
	data->zoominfovpersec = 0;
	data->invertpitch = !optionsGetForwardPitch(g_Vars.currentplayerstats->mpindex);
	data->disablelookahead = false;
	data->c1stickxsafe = 0;
	data->c1stickysafe = 0;
	data->c1stickxraw = 0;
	data->c1stickyraw = 0;
	data->analogturn = 0;
	data->analogpitch = 0;
	data->analogstrafe = 0;
	data->analogwalk = 0;
	data->alt1tapcount = 0;
	data->freelookdx = 0.0f;
	data->freelookdy = 0.0f;
	data->analoglean = 0.0f;
}

/**
 * Called with these arguments:
 * 0, 0, 0, 1 = tickmode 6
 * 0, 0, 0, 1 = eyespy
 * 0, 0, 0, 1 = teleportstate 3
 * 0, 0, 0, 1 = slayerrocket
 * 0, 0, 0, 1 = tickmode normal without control
 * 1, 1, ?, 0 = tickmode normal with control
 * 1, 1, ?, 0 = tickmodes 0 and 5
 * 0, 0, 0, 1 = tickmode mpswirl
 * 0, 0, 0, 1 = tickmode warp
 * 1, 1, 0, 1 = autowalk
 */
void bmoveProcessInput(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2)
{
	struct movedata movedata;
	int controlmode;
	int weaponnum;
	bool canmanualzoom;
	int result;
	uint32_t c1buttons;
	uint32_t c1buttonsthisframe;
	uint32_t c1allowedbuttons;
	uint32_t c1inhibitedbuttons;
	uint32_t aimonhist[20];
	uint32_t aimoffhist[20];
	int numsamples;
	float tmp;
	float fVar25;
	int8_t shootpad;
	int8_t aimpad;
	uint32_t aimallowedbuttons;
	uint32_t shootallowedbuttons;
	int8_t c2stickx;
	uint32_t c2buttons;
	uint32_t c2buttonsthisframe;
	int i;
	int tmpc2sticky;
	uint32_t c2allowedbuttons;
	int tmpc2stickx;
	int c2sticky;
	uint32_t shootbuttons;
	uint32_t aimbuttons;
	uint32_t invbuttons;
	bool zoomout;
	bool zoomin;
	float increment;
	float savedverta;
	float noiseradius;
	float zoomfov;
	float eraserfov;
	struct coord spa0;
	float crosspos[2];
	float lookahead;
	int8_t contpad1;
	int8_t contpad2;
	int8_t c1stickx;
	int8_t c1sticky;
	uint32_t inhibitedbuttons;
	bool offbike;
	bool cancycleweapons;
	float increment2;
	float newverta;
	const float mlookscale = g_Vars.lvupdate240 ? (4.f / (float)g_Vars.lvupdate240) : 4.f;
	const bool allowmlook = (g_Vars.currentplayernum == 0) && (allowc1x || allowc1y);
	bool allowmcross = false;

	controlmode = optionsGetControlMode(g_Vars.currentplayerstats->mpindex);
	weaponnum = bgunGetWeaponNum(HAND_RIGHT);
	canmanualzoom = weaponHasAimFlag(weaponnum, INVAIMFLAG_MANUALZOOM);
	contpad1 = optionsGetContpadNum1(g_Vars.currentplayerstats->mpindex);

	c1stickx = allowc1x ? joyGetStickX(contpad1) : 0;
	c1sticky = allowc1y ? joyGetStickY(contpad1) : 0;
	c2stickx = allowc1x ? (int8_t) joyGetRStickX(contpad1) : 0;
	c2sticky = allowc1y ? (int8_t) joyGetRStickY(contpad1) : 0;

	c1buttons = allowc1buttons ? joyGetButtons(contpad1, 0xffffffff) : 0;
	c1buttonsthisframe = allowc1buttons ? joyGetButtonsPressedThisFrame(contpad1, 0xffffffff) : 0;

	c1allowedbuttons = 0xffffffff;

	if (g_Vars.currentplayer->joybutinhibit & 0xffffffff) {
		inhibitedbuttons = g_Vars.currentplayer->joybutinhibit & 0xffffffff;
		c1allowedbuttons = ~inhibitedbuttons;
		inhibitedbuttons = joyGetButtons(contpad1, 0xffffffff) & inhibitedbuttons;
		c1buttons &= ~inhibitedbuttons;
		c1buttonsthisframe &= ~inhibitedbuttons;
		g_Vars.currentplayer->joybutinhibit = (g_Vars.currentplayer->joybutinhibit & 0x0) | inhibitedbuttons;
	}

	numsamples = joyGetNumSamples();
	bmoveResetMoveData(&movedata);

	if (c1stickx < -5) {
		movedata.c1stickxsafe = c1stickx + 5;
	} else if (c1stickx > 5) {
		movedata.c1stickxsafe = c1stickx - 5;
	} else {
		movedata.c1stickxsafe = 0;
	}

	if (c1sticky < -5) {
		movedata.c1stickysafe = c1sticky + 5;
	} else if (c1sticky > 5) {
		movedata.c1stickysafe = c1sticky - 5;
	} else {
		movedata.c1stickysafe = 0;
	}

	movedata.c1stickxraw = c1stickx;
	movedata.c1stickyraw = c1sticky;

	// These are zeroed further down conditionally on control style
	movedata.analogturn = movedata.c1stickxsafe;
	movedata.analogstrafe = movedata.c1stickxsafe;
	movedata.analogpitch = movedata.c1stickysafe;
	movedata.analogwalk = movedata.c1stickysafe;

	if (allowmlook) {
		inputMouseGetScaledDelta(&movedata.freelookdx, &movedata.freelookdy);
		allowmcross = (PLAYER_EXTCFG().mouseaimmode == MOUSEAIM_CLASSIC) &&
			(movedata.freelookdx || movedata.freelookdy || g_Vars.currentplayer->swivelpos[0] || g_Vars.currentplayer->swivelpos[1]);
		if (movedata.invertpitch) {
			movedata.freelookdy = -movedata.freelookdy;
		}
	}
	// always pause with ESC
	if (allowc1buttons && g_Vars.currentplayer->isdead == false && g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		if (inputKeyJustPressed(VK_ESCAPE)) {
			c1buttonsthisframe |= START_BUTTON;
		}
	}

	// Pausing
	if (g_Vars.currentplayer->isdead == false) {
		if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED && (c1buttonsthisframe & START_BUTTON)) {
			if (g_Vars.mplayerisrunning == false) {
				if (g_Vars.lvframenum > 15) {
					playerPause(MENUROOT_MAINMENU);
				}
			} else {
				mpPushPauseDialog();
			}
		}
	} else {
		if (g_Vars.mplayerisrunning) {
			if (PLAYERCOUNT() == 1) {
				if (mpIsPaused() && (c1buttonsthisframe & START_BUTTON) && g_MpSetup.paused != MPPAUSEMODE_GAMEOVER) {
					mpSetPaused(MPPAUSEMODE_UNPAUSED);
				}
			} else {
				if (mpIsPaused() && (c1buttonsthisframe & START_BUTTON)) {
					mpPushPauseDialog();
				}
			}
		}
	}

	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		if (g_Vars.currentplayer->isdead == false) {
			if (controlmode == CONTROLMODE_23 || controlmode == CONTROLMODE_24 || controlmode == CONTROLMODE_22 || controlmode == CONTROLMODE_21) {
				// 2.1: ctrl1 stick = walk/turn, z = fire, ctrl2 stick = look/strafe, z = aim
				// 2.2: ctrl1 stick = look,      z = fire, ctrl2 stick = walk/strafe, z = aim
				// 2.3: ctrl1 stick = walk/turn, z = aim,  ctrl2 stick = look/strafe, z = fire
				// 2.4: ctrl1 stick = look,      z = aim,  ctrl2 stick = walk/strafe, z = fire
				contpad2 = (int8_t) optionsGetContpadNum2(g_Vars.currentplayerstats->mpindex);
				c2stickx = (int8_t) joyGetStickX(contpad2);
				c2sticky = (joyGetStickY(contpad2) << 24) >> 24;
				c2buttons = joyGetButtons(contpad2, 0xffffffff);
				c2buttonsthisframe = joyGetButtonsPressedThisFrame(contpad2, 0xffffffff);

				tmpc2stickx = c2stickx;
				tmpc2sticky = c2sticky;

				c2allowedbuttons = 0xffffffff;

				// NOTE: joybutinhibit used to store two copies of the 16-bit inhibited mask for some reason
				//       now it only stores one mask because it is 32 bits in size
				if (g_Vars.currentplayer->joybutinhibit) {
					inhibitedbuttons = g_Vars.currentplayer->joybutinhibit;
					c2allowedbuttons = ~inhibitedbuttons;
					inhibitedbuttons = joyGetButtons(contpad2, 0xffffffff) & inhibitedbuttons;
					c2buttons &= ~inhibitedbuttons;
					c2buttonsthisframe &= ~inhibitedbuttons;
					g_Vars.currentplayer->joybutinhibit |= inhibitedbuttons;
				}

				if (ignorec2) {
					c2stickx = 0;
					c2buttons = 0;
					tmpc2stickx = 0;
					tmpc2sticky = 0;
					c2buttonsthisframe = 0;
				}

				if (tmpc2stickx < -5) {
					tmpc2stickx += 5;
				} else if (tmpc2stickx > 5) {
					tmpc2stickx -= 5;
				} else {
					tmpc2stickx = 0;
				}

				if (tmpc2sticky < -5) {
					tmpc2sticky += 5;
				} else if (tmpc2sticky > 5) {
					tmpc2sticky -= 5;
				} else {
					tmpc2sticky = 0;
				}

				if (controlmode == CONTROLMODE_21 || controlmode == CONTROLMODE_23) {
					movedata.analogstrafe = tmpc2stickx;
					movedata.analogpitch = tmpc2sticky;
				} else {
					movedata.analogstrafe = tmpc2stickx;
					movedata.analogwalk = tmpc2sticky;
				}

				c2sticky = tmpc2sticky;

				if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
					movedata.digitalstepforward = false;
					movedata.digitalstepback = false;
					movedata.analogstrafe = 0;
					movedata.analogwalk = g_Vars.currentplayer->autocontrol_y;
					movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
					movedata.analogpitch = 0;
					movedata.freelookdx = 0.0f;
					movedata.freelookdy = 0.0f;
				}

				if (controlmode == CONTROLMODE_21 || controlmode == CONTROLMODE_22) {
					aimpad = contpad2;
					shootpad = contpad1;
					aimallowedbuttons = c2allowedbuttons;
					shootallowedbuttons = c1allowedbuttons;
				} else {
					aimpad = contpad1;
					shootpad = contpad2;
					aimallowedbuttons = c1allowedbuttons;
					shootallowedbuttons = c2allowedbuttons;
				}

				if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
					for (i = 0; i < numsamples; i++) {
						aimonhist[i] = allowc1buttons && joyGetButtonsOnSample(i, aimpad, aimallowedbuttons & Z_TRIG);
						aimoffhist[i] = !aimonhist[i];
					}

					if (numsamples > 0) {
						g_Vars.currentplayer->insightaimmode = aimonhist[numsamples - 1];
					}
				}

				if (!lvIsPaused()) {
					// Handle aiming
					if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) != AIMCONTROL_HOLD) {
						for (i = 0; i < numsamples; i++) {
							if (allowc1buttons && joyGetButtonsPressedOnSample(i, aimpad, aimallowedbuttons & Z_TRIG)) {
								g_Vars.currentplayer->insightaimmode = !g_Vars.currentplayer->insightaimmode;
							}

							aimonhist[i] = g_Vars.currentplayer->insightaimmode;
							aimoffhist[i] = !aimonhist[i];
						}
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
						g_Vars.currentplayer->insightaimmode = true;
					}

					movedata.canswivelgun = !g_Vars.currentplayer->insightaimmode;
					movedata.canmanualaim = g_Vars.currentplayer->insightaimmode;
					movedata.canautoaim = !g_Vars.currentplayer->insightaimmode;
					movedata.digitalstepforward = false;
					movedata.digitalstepback = false;
					movedata.digitalstepleft = false;
					movedata.digitalstepright = false;
					movedata.canlookahead = !g_Vars.currentplayer->insightaimmode;
					movedata.unk14 = 1;
					movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;
					movedata.cannaturalpitch = !g_Vars.currentplayer->insightaimmode;

					// Handle turning while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw > 60) {
						movedata.speedvertadown = (movedata.c1stickyraw - 60) / 10.0f;

						if (movedata.speedvertadown > 1) {
							movedata.speedvertadown = 1;
						}
					} else {
						movedata.speedvertadown = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw < -60) {
						movedata.speedvertaup = (-60 - movedata.c1stickyraw) / 10.0f;

						if (movedata.speedvertaup > 1) {
							movedata.speedvertaup = 1;
						}
					} else {
						movedata.speedvertaup = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw < -60) {
						movedata.aimturnleftspeed = (-60 - movedata.c1stickxraw) / 10.0f;

						if (movedata.aimturnleftspeed > 1) {
							movedata.aimturnleftspeed = 1;
						}
					} else {
						movedata.aimturnleftspeed = 0;
					}

					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw > 60) {
						movedata.aimturnrightspeed = (movedata.c1stickxraw - 60) / 10.0f;

						if (movedata.aimturnrightspeed > 1) {
							movedata.aimturnrightspeed = 1;
						}
					} else {
						movedata.aimturnrightspeed = 0;
					}

					// Handle weapon switching
					if (allowc1buttons) {
						if (g_Vars.currentplayer->invdowntime < -2) {
							g_Vars.currentplayer->invdowntime += numsamples;

							if (g_Vars.currentplayer->invdowntime > -3) {
								g_Vars.currentplayer->invdowntime = 0;
							}
						} else {
							for (i = 0; i < numsamples; i++) {
								if (controlmode == CONTROLMODE_PC) {
									if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & BUTTON_WPNFORWARD)) {
										movedata.weaponforwardoffset++;
										g_Vars.currentplayer->invdowntime = -1;
									} else if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & BUTTON_WPNBACK)) {
										movedata.weaponbackoffset++;
										g_Vars.currentplayer->invdowntime = -1;
									}
									continue;
								}
								if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & A_BUTTON)
										|| joyGetButtonsOnSample(i, contpad2, c2allowedbuttons & A_BUTTON)) {
									if (g_Vars.currentplayer->invdowntime > -2) {
										if (joyGetButtonsPressedOnSample(i, shootpad, shootallowedbuttons & Z_TRIG)) {
											movedata.weaponbackoffset++;
											g_Vars.currentplayer->invdowntime = -1;
										}

										if (g_Vars.currentplayer->invdowntime > -1
												&& joyGetButtonsOnSample(i, shootpad, shootallowedbuttons & Z_TRIG) == 0) {
											if (g_Vars.currentplayer->invdowntime > TICKS(15)) {
												amOpen();
												g_Vars.currentplayer->invdowntime = -1;
											} else {
												g_Vars.currentplayer->invdowntime += g_Vars.lvupdate60;
											}
										}
									}
								} else {
									if (g_Vars.currentplayer->invdowntime > 0 &&
											(!allowc1buttons || joyGetButtonsOnSample(i, shootpad, shootallowedbuttons & Z_TRIG) == 0)) {
										movedata.weaponforwardoffset++;
									}

									g_Vars.currentplayer->invdowntime = 0;
								}
							}
						}
					}

					// Handle B button activation
					if (allowc1buttons && controlmode != CONTROLMODE_PC) {
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & B_BUTTON)
									|| joyGetButtonsOnSample(i, contpad2, c2allowedbuttons & B_BUTTON)) {
								if (g_Vars.currentplayer->usedowntime >= -1) {
									if (joyGetButtonsPressedOnSample(i, shootpad, shootallowedbuttons & Z_TRIG)
											&& g_Vars.currentplayer->usedowntime > -1
											&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, true, false, 0) != USETIMER_CONTINUE) {
										g_Vars.currentplayer->usedowntime = -3;
									}

									if (g_Vars.currentplayer->usedowntime > -1) {
										if (g_Vars.currentplayer->usedowntime > TICKS(25)) {
											result = bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);

											if (result == USETIMER_STOP) {
												g_Vars.currentplayer->usedowntime = -1;
											} else if (result == USETIMER_REPEAT) {
												g_Vars.currentplayer->usedowntime = -2;
											} else {
												g_Vars.currentplayer->usedowntime++;
											}
										} else {
											g_Vars.currentplayer->usedowntime++;
										}
									}
								} else if (g_Vars.currentplayer->usedowntime >= -2) {
									bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);
								}
							} else {
								// Released B - activate or reload
								if (g_Vars.currentplayer->usedowntime > 0) {
									movedata.btapcount++;
								}

								g_Vars.currentplayer->usedowntime = 0;
								bgunDisallowInvertFunc();
							}
						}
					}

					// Handle manual zoom in and out (sniper, farsight and horizon scanner)
					if (canmanualzoom && g_Vars.currentplayer->insightaimmode) {
						if (c2sticky < 0) {
							movedata.zoomoutfovpersec = -c2sticky / 70.0f;

							if (movedata.zoomoutfovpersec > 1) {
								movedata.zoomoutfovpersec = 1;
							}

							movedata.zoomoutfovpersec = movedata.zoomoutfovpersec + movedata.zoomoutfovpersec;
						}

						if (c2sticky > 0) {
							movedata.zoominfovpersec = c2sticky / 70.0f;

							if (movedata.zoominfovpersec > 1) {
								movedata.zoominfovpersec = 1;
							}

							movedata.zoominfovpersec = movedata.zoominfovpersec + movedata.zoominfovpersec;
						}
					}

					// Handle crouch and uncrouch
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (!canmanualzoom && aimonhist[i]) {
								if (joyGetStickYOnSample(i, contpad2) > 30 && joyGetStickYOnSampleIndex(i, contpad2) <= 30) {
									if (movedata.crouchdown) {
										movedata.crouchdown--;
									} else {
										movedata.crouchup++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}

								if (joyGetStickYOnSample(i, contpad2) < -30 && joyGetStickYOnSampleIndex(i, contpad2) >= -30) {
									if (movedata.crouchup) {
										movedata.crouchup--;
									} else {
										movedata.crouchdown++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}
							}

							if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
								if (aimonhist[i]) {
									if (g_Vars.currentplayer->aimtaptime > -1) {
										g_Vars.currentplayer->aimtaptime++;
									}
								} else {
									if (g_Vars.currentplayer->aimtaptime > 0
											&& g_Vars.currentplayer->aimtaptime < TICKS(15)) {
										if (movedata.crouchdown) {
											movedata.crouchdown--;
										} else {
											movedata.crouchup++;
										}
									}

									g_Vars.currentplayer->aimtaptime = 0;
								}
							}
						}
					}

					// Handle shutting eyes in multiplayer
					if (bmoveGetCrouchPos() == CROUCHPOS_SQUAT
							&& g_Vars.currentplayer->crouchoffset == -90
							&& g_Vars.mplayerisrunning
							&& g_Vars.coopplayernum < 0) {
						movedata.eyesshut = g_Vars.currentplayer->insightaimmode
							&& !canmanualzoom
							&& joyGetStickY(contpad2) < -30;
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT) {
						if (g_Vars.currentplayer->insightaimmode) {
							movedata.unk14 = 0;
						}

						movedata.farsighttempautoseek = g_Vars.currentplayer->insightaimmode
							&& (c2stickx < -30 || c2stickx > 30);
					}

					movedata.rleanleft = false;
					movedata.rleanright = false;
					movedata.analoglean = 0.f;

					// Handle mine detonation
					if ((((c1buttons & A_BUTTON) && (c1buttonsthisframe & B_BUTTON))
								|| ((c1buttons & B_BUTTON) && (c1buttonsthisframe & A_BUTTON))
								|| ((c2buttons & A_BUTTON) && (c2buttonsthisframe & B_BUTTON))
								|| ((c2buttons & B_BUTTON) && (c2buttonsthisframe & A_BUTTON)))
							&& weaponnum == WEAPON_REMOTEMINE) {
						movedata.detonating = true;
						movedata.weaponbackoffset = 0;
						movedata.weaponforwardoffset = 0;
						movedata.btapcount = 0;
						g_Vars.currentplayer->invdowntime = -2;
						g_Vars.currentplayer->usedowntime = -2;
					}
				}

				movedata.aiming = g_Vars.currentplayer->insightaimmode;
				movedata.zooming = g_Vars.currentplayer->insightaimmode;

				if (g_Vars.currentplayer->waitforzrelease
						&& joyGetButtons(shootpad, shootallowedbuttons & Z_TRIG) == 0) {
					g_Vars.currentplayer->waitforzrelease = false;
				}

				if (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_FIRETOACTIVATE)) {
					if (allowc1buttons
							&& joyGetButtonsPressedThisFrame(shootpad, shootallowedbuttons & Z_TRIG)
							&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
						movedata.btapcount++;
					}
				} else {
					movedata.triggeron = g_Vars.currentplayer->waitforzrelease == false
						&& allowc1buttons
						&& joyGetButtons(shootpad, shootallowedbuttons & Z_TRIG)
						&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED
						&& (c1buttons & A_BUTTON) == 0
						&& (c2buttons & A_BUTTON) == 0;
				}

				movedata.disablelookahead = true;
			} else {
				// 1.x or PC control style
				if (controlmode == CONTROLMODE_PC) {
					shootbuttons = Z_TRIG;
					aimbuttons = R_TRIG;
					invbuttons = A_BUTTON;
				} else if (controlmode == CONTROLMODE_13 || controlmode == CONTROLMODE_14) {
					shootbuttons = A_BUTTON;
					aimbuttons = Z_TRIG;
					invbuttons = L_TRIG | R_TRIG;
				} else {
					shootbuttons = Z_TRIG;
					aimbuttons = L_TRIG | R_TRIG;
					invbuttons = A_BUTTON;
				}

				if (controlmode == CONTROLMODE_PC) {
					if (!g_Vars.currentplayer->insightaimmode) {
						movedata.analogstrafe = c2stickx;
						movedata.analogwalk = c2sticky;
						movedata.unk14 = (c2stickx || c2sticky);
					} else {
						movedata.analogstrafe = 0.f;
						movedata.analogwalk = 0.f;
					}
				}

				if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
					for (i = 0; i < numsamples; i++) {
						aimonhist[i] = allowc1buttons && joyGetButtonsOnSample(i, contpad1, aimbuttons & c1allowedbuttons);
						aimoffhist[i] = !aimonhist[i];
					}

					if (numsamples > 0) {
						g_Vars.currentplayer->insightaimmode = aimonhist[numsamples - 1];
					}
				}

				if (!lvIsPaused()) {
					// Handle aiming
					if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) != AIMCONTROL_HOLD) {
						for (i = 0; i < numsamples; i++) {
							if (allowc1buttons && joyGetButtonsPressedOnSample(i, contpad1, aimbuttons & c1allowedbuttons)) {
								g_Vars.currentplayer->insightaimmode = !g_Vars.currentplayer->insightaimmode;
							}

							aimonhist[i] = g_Vars.currentplayer->insightaimmode;
							aimoffhist[i] = !aimonhist[i];
						}
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
						g_Vars.currentplayer->insightaimmode = true;
					}

					movedata.canswivelgun = !g_Vars.currentplayer->insightaimmode;
					movedata.canmanualaim = g_Vars.currentplayer->insightaimmode;
					movedata.canautoaim = !g_Vars.currentplayer->insightaimmode;

					// On N64 control schemes the d-pad does the same thing as the C buttons
					uint32_t slmask, srmask, sumask, sdmask;
					if (controlmode == CONTROLMODE_PC) {
						sumask = U_CBUTTONS;
						sdmask = D_CBUTTONS;
						slmask = L_CBUTTONS;
						srmask = R_CBUTTONS;
					} else {
						sumask = U_JPAD | U_CBUTTONS;
						sdmask = D_JPAD | D_CBUTTONS;
						slmask = L_JPAD | L_CBUTTONS;
						srmask = R_JPAD | R_CBUTTONS;
					}

					if (controlmode == CONTROLMODE_12 || controlmode == CONTROLMODE_14 || controlmode == CONTROLMODE_PC) {
						// Handle side stepping
						if (g_Vars.currentplayer->insightaimmode == false) {
							if (allowc1buttons) {
								movedata.digitalstepleft = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & slmask);
								movedata.digitalstepright = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & srmask);
							}
						} else {
							// This doesn't appear to be r-leaning.
							// R-leaning still works when these are commented.
							if (c1buttons & slmask) {
								movedata.unk30 = 1;
							}

							if (c1buttons & srmask) {
								movedata.unk34 = 1;
							}
						}

						movedata.digitalstepforward = !g_Vars.currentplayer->insightaimmode && (c1buttons & sumask);
						movedata.digitalstepback = !g_Vars.currentplayer->insightaimmode && (c1buttons & sdmask);
						movedata.canlookahead = (controlmode == CONTROLMODE_PC) && !g_Vars.currentplayer->insightaimmode && (c2stickx || c2sticky);
						movedata.cannaturalpitch = !g_Vars.currentplayer->insightaimmode;
						movedata.speedvertadown = 0;
						movedata.speedvertaup = 0;
						movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;

						if (controlmode == CONTROLMODE_PC) {
							if ((g_Vars.currentplayer->devicesactive & DEVICE_EYESPY) || g_Vars.currentplayer->visionmode > 1 || g_Vars.tickmode != 1) {
								movedata.analogturn = 0;
								movedata.analogpitch = 0;
								movedata.analogstrafe = 0;
								movedata.analogwalk = 0;
								movedata.analoglean = 0.f;
							}
							if (PLAYER_EXTCFG().mouseaimmode == MOUSEAIM_LOCKED || bgunGetWeaponNum(HAND_RIGHT) == WEAPON_HORIZONSCANNER) {
								movedata.cannaturalpitch = movedata.cannaturalpitch || (movedata.freelookdy != 0.0f);
								movedata.cannaturalturn = movedata.cannaturalturn  || (movedata.freelookdx != 0.0f);
							}
						}

						if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
							movedata.digitalstepforward = (g_Vars.currentplayer->autocontrol_y > 0);
							movedata.digitalstepback = (g_Vars.currentplayer->autocontrol_y < 0);
							movedata.analogstrafe = 0;
							movedata.analogwalk = 0;
							movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
							movedata.analogpitch = 0;
							movedata.freelookdx = 0.0f;
							movedata.freelookdy = 0.0f;
							movedata.analoglean = 0.f;
						}
					} else {
						// 1.1 or 1.3
						if (c1buttons & (L_JPAD | L_CBUTTONS)) {
							movedata.unk30 = 1;
						}

						if (c1buttons & (R_JPAD | R_CBUTTONS)) {
							movedata.unk34 = 1;
						}

						if (!g_Vars.currentplayer->insightaimmode && allowc1buttons) {
							movedata.digitalstepleft = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (L_JPAD | L_CBUTTONS));
							movedata.digitalstepright = joyCountButtonsOnSpecificSamples(aimoffhist, contpad1, c1allowedbuttons & (R_JPAD | R_CBUTTONS));
						}

						movedata.digitalstepforward = false;
						movedata.digitalstepback = false;
						movedata.canlookahead = !g_Vars.currentplayer->insightaimmode;
						movedata.cannaturalpitch = false;

						// Looking up/down
						if (!g_Vars.currentplayer->insightaimmode && (c1buttons & (U_JPAD | U_CBUTTONS))) {
							movedata.speedvertadown = 1;
						}

						if (!g_Vars.currentplayer->insightaimmode && (c1buttons & (D_JPAD | D_CBUTTONS))) {
							movedata.speedvertaup = 1;
						}

						movedata.cannaturalturn = !g_Vars.currentplayer->insightaimmode;
						movedata.unk14 = 0;

						if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
							movedata.analogstrafe = 0;
							movedata.analogwalk = g_Vars.currentplayer->autocontrol_y;
							movedata.analogturn = g_Vars.currentplayer->autocontrol_x;
							movedata.analogpitch = 0;
							movedata.freelookdx = 0.0f;
							movedata.freelookdy = 0.0f;
							movedata.analoglean = 0.f;
						}
					}

					// Handle looking up/down while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw > 60) {
						movedata.speedvertadown = (movedata.c1stickyraw - 60) / 10.0f;

						if (movedata.speedvertadown > 1) {
							movedata.speedvertadown = 1;
						}
					} else if (g_Vars.currentplayer->insightaimmode && movedata.c1stickyraw < -60) {
						movedata.speedvertaup = (-60 - movedata.c1stickyraw) / 10.0f;

						if (movedata.speedvertaup > 1) {
							movedata.speedvertaup = 1;
						}
					}

					// Handle looking left/right while aiming
					if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw < -60) {
						movedata.aimturnleftspeed = (-60 - movedata.c1stickxraw) / 10.0f;

						if (movedata.aimturnleftspeed > 1) {
							movedata.aimturnleftspeed = 1;
						}
					} else if (g_Vars.currentplayer->insightaimmode && movedata.c1stickxraw > 60) {
						movedata.aimturnrightspeed = (movedata.c1stickxraw - 60) / 10.0f;

						if (movedata.aimturnrightspeed > 1) {
							movedata.aimturnrightspeed = 1;
						}
					}

					// Handle turning and looking up/down via mouselook when aiming
					if (g_Vars.currentplayer->insightaimmode && allowmcross && bgunGetWeaponNum(HAND_RIGHT) != WEAPON_HORIZONSCANNER) {
						if (g_Vars.currentplayer->swivelpos[0] > 0.9f) {
							movedata.aimturnrightspeed = (g_Vars.currentplayer->swivelpos[0] - 0.9f) / 0.1f;
							movedata.aimturnleftspeed = 0.f;
						} else if (g_Vars.currentplayer->swivelpos[0] < -0.9f) {
							movedata.aimturnleftspeed = (g_Vars.currentplayer->swivelpos[0] - -0.9f) / -0.1f;
							movedata.aimturnrightspeed = 0.f;
						}
						float vertaup = 0.f, vertadown = 0.f;
						if (g_Vars.currentplayer->swivelpos[1] > 0.9f) {
							vertaup = (g_Vars.currentplayer->swivelpos[1] - 0.9f) / 0.1f;
						} else if (g_Vars.currentplayer->swivelpos[1] < -0.9f) {
							vertadown = (g_Vars.currentplayer->swivelpos[1] - -0.9f) / -0.1f;
						}
						// Uninvert pitch if needed
						if (movedata.invertpitch) {
							movedata.speedvertaup = vertadown;
							movedata.speedvertadown = vertaup;
						} else {
							movedata.speedvertaup = vertaup;
							movedata.speedvertadown = vertadown;
						}
					} else {
						// Reset mouse aim position when not mouse aiming
						g_Vars.currentplayer->swivelpos[0] = 0.f;
						g_Vars.currentplayer->swivelpos[1] = 0.f;
					}

					// Handle A button
					if (allowc1buttons) {
						if (g_Vars.currentplayer->invdowntime < -2) {
							g_Vars.currentplayer->invdowntime += numsamples;

							if (g_Vars.currentplayer->invdowntime > -3) {
								g_Vars.currentplayer->invdowntime = 0;
							}
						} else {
							for (i = 0; i < numsamples; i++) {
								if (controlmode == CONTROLMODE_PC) {
									if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & BUTTON_WPNFORWARD)) {
										movedata.weaponforwardoffset++;
										g_Vars.currentplayer->invdowntime = -1;
									} else if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & BUTTON_WPNBACK)) {
										movedata.weaponbackoffset++;
										g_Vars.currentplayer->invdowntime = -1;
									}
									continue;
								}
								if (joyGetButtonsOnSample(i, contpad1, invbuttons & c1allowedbuttons)) {
									if (g_Vars.currentplayer->invdowntime > -2) {
										if (joyGetButtonsPressedOnSample(i, contpad1, shootbuttons & c1allowedbuttons)) {
											movedata.weaponbackoffset++;
											g_Vars.currentplayer->invdowntime = -1;
										}

										if (g_Vars.currentplayer->invdowntime >= 0 && joyGetButtonsOnSample(i, contpad1, shootbuttons & c1allowedbuttons) == 0) {
											// Holding A and haven't pressed Z
											if (g_Vars.currentplayer->invdowntime > TICKS(15)) {
												amOpen();
												g_Vars.currentplayer->invdowntime = -1;
											} else {
												g_Vars.currentplayer->invdowntime += g_Vars.lvupdate60;
											}
										}
									}
								} else {
									// Wasn't holding A on this sample
									if (g_Vars.currentplayer->invdowntime > 0 &&
											(!allowc1buttons || joyGetButtonsOnSample(i, contpad1, shootbuttons & c1allowedbuttons) == 0)) {
										// But was on previous sample, so cycle weapon
										movedata.weaponforwardoffset++;
									}

									g_Vars.currentplayer->invdowntime = 0;
								}
							}
						}
					}

					// Handle B and use-like button
					const uint32_t usemask = (controlmode == CONTROLMODE_PC) ?
						(B_BUTTON | BUTTON_CANCEL_USE | BUTTON_ACCEPT_USE) :
						B_BUTTON;
					if (allowc1buttons) {
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & usemask)) {
								if (g_Vars.currentplayer->usedowntime >= -1) {
									if (controlmode != CONTROLMODE_PC) {
										if (joyGetButtonsPressedOnSample(i, contpad1, shootbuttons & c1allowedbuttons)
												&& g_Vars.currentplayer->usedowntime >= 0
												&& bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, true, false, 0) != USETIMER_CONTINUE) {
											g_Vars.currentplayer->usedowntime = -3;
										}
									}

									if (g_Vars.currentplayer->usedowntime >= 0) {
										if (g_Vars.currentplayer->usedowntime > TICKS(25)) {
											int result = (controlmode == CONTROLMODE_PC) ?
												USETIMER_CONTINUE :
												bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);
											if (result == USETIMER_STOP) {
												g_Vars.currentplayer->usedowntime = -1;
											} else if (result == USETIMER_REPEAT) {
												g_Vars.currentplayer->usedowntime = -2;
											} else {
												g_Vars.currentplayer->usedowntime++;
											}
										} else {
											g_Vars.currentplayer->usedowntime++;
										}
									}
								} else {
									if ((controlmode != CONTROLMODE_PC) && g_Vars.currentplayer->usedowntime >= -2) {
										bgunConsiderToggleGunFunction(g_Vars.currentplayer->usedowntime, false, false, 0);
									}
								}
							} else {
								// Released B
								if (g_Vars.currentplayer->usedowntime > 0) {
									movedata.btapcount++;
								}

								g_Vars.currentplayer->usedowntime = 0;
								bgunDisallowInvertFunc();
							}
						}
					}

					if (controlmode == CONTROLMODE_PC && allowc1buttons) {
						// handle L button : alt switching
						for (i = 0; i < numsamples; i++) {
							bgunProcessInputAltButton(&movedata, contpad1, i);
						}

						// Handle ALT1 / MI Reload Hack
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & BUTTON_RELOAD)) {
								movedata.alt1tapcount++;
							}
						}

						// Handle radial menu (D-Down)
						for (i = 0; i < numsamples; i++) {
							if (joyGetButtonsOnSample(i, contpad1, c1allowedbuttons & BUTTON_RADIAL)) {
								if (g_Vars.currentplayer->amdowntime < -2) {
									g_Vars.currentplayer->amdowntime += numsamples;

									if (g_Vars.currentplayer->amdowntime > -3) {
										g_Vars.currentplayer->amdowntime = 0;
									}
								} else {
									if (g_Vars.currentplayer->amdowntime >= 0) {
										if (joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & BUTTON_RADIAL)) {
											amOpen();
											g_Vars.currentplayer->amdowntime = -1;
										} else {
											g_Vars.currentplayer->amdowntime++;
										}
									}
								}
							} else {
								g_Vars.currentplayer->amdowntime = 0;
							}
						}

						// Handle xbla-style crouch cycling
						const int oldcrouchpos = g_Vars.currentplayer->crouchpos;
						for (i = 0; i < numsamples; i++) {
							// handle 1964GEPD style crouch setting
							int crouchsample;
							if (PLAYER_EXTCFG().crouchmode & CROUCHMODE_TOGGLE) {
								// press to toggle crouch position
								crouchsample = joyGetButtonsPressedOnSample(i, contpad1, 0xffffffff) & BUTTON_CROUCH_CYCLE;
								if (crouchsample) {
									if (g_Vars.currentplayer->crouchpos <= 0) {
										g_Vars.currentplayer->crouchpos = CROUCHPOS_STAND;
									} else {
										g_Vars.currentplayer->crouchpos--;
									}
								}
								crouchsample = joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons) & BUTTON_HALF_CROUCH;
								if (crouchsample) {
									if (g_Vars.currentplayer->crouchpos == CROUCHPOS_DUCK) {
										g_Vars.currentplayer->crouchpos = CROUCHPOS_STAND;
									} else {
										g_Vars.currentplayer->crouchpos = CROUCHPOS_DUCK;
									}
								}
								crouchsample = joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons) & BUTTON_FULL_CROUCH;
								if (crouchsample) {
									if (g_Vars.currentplayer->crouchpos == CROUCHPOS_SQUAT) {
										g_Vars.currentplayer->crouchpos = CROUCHPOS_STAND;
									} else {
										g_Vars.currentplayer->crouchpos = CROUCHPOS_SQUAT;
									}
								}
							} else if (PLAYER_EXTCFG().crouchmode == CROUCHMODE_HOLD) {
								// hold to crouch
								crouchsample = joyGetButtonsOnSample(i, contpad1, c1allowedbuttons) & (BUTTON_FULL_CROUCH | BUTTON_HALF_CROUCH);
								if (!crouchsample) {
									g_Vars.currentplayer->crouchpos = CROUCHPOS_STAND;
								} else if (crouchsample & BUTTON_FULL_CROUCH) {
									g_Vars.currentplayer->crouchpos = CROUCHPOS_SQUAT;
								} else if (crouchsample & BUTTON_HALF_CROUCH) {
									g_Vars.currentplayer->crouchpos = CROUCHPOS_DUCK;
								}
							}
						}
						// prevent uncrouching if we don't fit
						while (g_Vars.currentplayer->crouchpos > oldcrouchpos && !bwalkCanUncrouch()) {
							g_Vars.currentplayer->crouchpos--;
						}
					}

					// Handle manual zoom in and out (sniper, farsight and horizon scanner)
					if (canmanualzoom && g_Vars.currentplayer->insightaimmode) {
						increment = 1;
						zoomout = c1buttons & sdmask;
						zoomin = c1buttons & sumask;

						// @bug? Should this be HAND_RIGHT?
						// Ben's comment: when fixed the zoom feels painfully slow. I'm just going to leave out this check entirely.
						/*if (bgunGetWeaponNum(HAND_LEFT) == WEAPON_FARSIGHT) {
							increment = 0.5f;
						}*/

						if (zoomout) {
							movedata.zoomoutfovpersec = increment;
						}

						if (zoomin) {
							movedata.zoominfovpersec = increment;
						}

						if (controlmode == CONTROLMODE_PC) {
							if (c2sticky < 0) {
								movedata.zoomoutfovpersec = -c2sticky / 70.0f;

								if (movedata.zoomoutfovpersec > 1) {
									movedata.zoomoutfovpersec = 1;
								}

								movedata.zoomoutfovpersec = movedata.zoomoutfovpersec + movedata.zoomoutfovpersec;
							}
							if (c2sticky > 0) {
								movedata.zoominfovpersec = c2sticky / 70.0f;

								if (movedata.zoominfovpersec > 1) {
									movedata.zoominfovpersec = 1;
								}

								movedata.zoominfovpersec = movedata.zoominfovpersec + movedata.zoominfovpersec;
							}
						}
					}

					// Handle C-button and analog crouch and uncrouch, if enabled
					if (allowc1buttons && (controlmode != CONTROLMODE_PC || (PLAYER_EXTCFG().crouchmode & CROUCHMODE_ANALOG))) {
						for (i = 0; i < numsamples; i++) {
							if (!canmanualzoom && aimonhist[i]) {
								bool goUp = joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & sumask);
								if (controlmode == CONTROLMODE_PC) {
									goUp = goUp || ((joyGetRStickYOnSample(i, contpad1) > 30 && joyGetRStickYOnSampleIndex(i, contpad1) <= 30));
								}
								if (goUp) {
									if (movedata.crouchdown) {
										movedata.crouchdown--;
									} else {
										movedata.crouchup++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}

								bool goDn = joyGetButtonsPressedOnSample(i, contpad1, c1allowedbuttons & sdmask);
								if (controlmode == CONTROLMODE_PC) {
									goDn = goDn || ((joyGetRStickYOnSample(i, contpad1) < -30 && joyGetRStickYOnSampleIndex(i, contpad1) >= -30));
								}
								if (goDn) {
									if (movedata.crouchup) {
										movedata.crouchup--;
									} else {
										movedata.crouchdown++;
									}

									g_Vars.currentplayer->aimtaptime = -1;
								}
							}

							if (optionsGetAimControl(g_Vars.currentplayerstats->mpindex) == AIMCONTROL_HOLD) {
								if (aimonhist[i]) {
									if (g_Vars.currentplayer->aimtaptime >= 0) {
										g_Vars.currentplayer->aimtaptime++;
									}
								} else {
									// Released aim
									if (g_Vars.currentplayer->aimtaptime > 0 && g_Vars.currentplayer->aimtaptime < TICKS(15)) {
										// Was only a tap, so uncrouch
										if (movedata.crouchdown) {
											movedata.crouchdown--;
										} else {
											movedata.crouchup++;
										}
									}

									g_Vars.currentplayer->aimtaptime = 0;
								}
							}
						}
					}

					// Handle shutting eyes in multiplayer
					if (bmoveGetCrouchPos() == CROUCHPOS_SQUAT
							&& g_Vars.currentplayer->crouchoffset == -90
							&& g_Vars.mplayerisrunning
							&& g_Vars.coopplayernum <= -1) {
						movedata.eyesshut = g_Vars.currentplayer->insightaimmode
							&& !canmanualzoom
							&& joyGetButtons(contpad1, c1allowedbuttons & sdmask);
					}

					if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT) {
						movedata.farsighttempautoseek = g_Vars.currentplayer->insightaimmode && (c1buttons & (srmask | slmask));
						if (controlmode == CONTROLMODE_PC && g_Vars.currentplayer->insightaimmode) {
								movedata.unk14 = 1;
								movedata.analogstrafe = c2stickx;
						}
					} else {
						movedata.rleanleft = g_Vars.currentplayer->insightaimmode && (c1buttons & slmask);
						movedata.rleanright = g_Vars.currentplayer->insightaimmode && (c1buttons & srmask);
						if (controlmode == CONTROLMODE_PC && g_Vars.currentplayer->insightaimmode) {
							movedata.analoglean = c2stickx / 127.f;
						}
					}

					// Handle mine detonation
					if (controlmode != CONTROLMODE_PC) {
						if ((((c1buttons & invbuttons) && (c1buttonsthisframe & B_BUTTON))
								|| ((c1buttons & B_BUTTON) && (c1buttonsthisframe & invbuttons)))
								&& weaponnum == WEAPON_REMOTEMINE) {
							movedata.detonating = true;
							movedata.weaponbackoffset = 0;
							movedata.weaponforwardoffset = 0;
							movedata.btapcount = 0;
							g_Vars.currentplayer->invdowntime = -2;
							g_Vars.currentplayer->usedowntime = -2;
						}
					} else {
						bgunProcessQuickDetonate(&movedata, c1buttons, c1buttonsthisframe, (BUTTON_CANCEL_USE | BUTTON_ACCEPT_USE), (BUTTON_WPNBACK | BUTTON_RADIAL | BUTTON_RELOAD));
					}
				}

				movedata.aiming = g_Vars.currentplayer->insightaimmode;
				movedata.zooming = g_Vars.currentplayer->insightaimmode;

				if (g_Vars.currentplayer->waitforzrelease
						&& (c1buttons & shootbuttons) == 0) {
					g_Vars.currentplayer->waitforzrelease = false;
				}

				if (weaponHasFlag(bgunGetWeaponNum(HAND_RIGHT), WEAPONFLAG_FIRETOACTIVATE)) {
					if ((c1buttonsthisframe & shootbuttons)
							&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
						movedata.btapcount++;
					}
				} else {
					movedata.triggeron = g_Vars.currentplayer->waitforzrelease == false
						&& (c1buttons & shootbuttons)
						&& g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED;
					if (controlmode != CONTROLMODE_PC) {
						movedata.triggeron = movedata.triggeron && ((c1buttons & invbuttons) == 0);
					}
				}

				if (controlmode == CONTROLMODE_12 || controlmode == CONTROLMODE_14 || controlmode == CONTROLMODE_PC) {
					movedata.disablelookahead = true;
				}
			} // end 1.x
		}
	}

	g_Vars.currentplayer->bondactivateorreload = 0;

	int usereloads = (controlmode != CONTROLMODE_PC);
	usereloads = usereloads || PLAYER_EXTCFG().usereloads;
	if (controlmode == CONTROLMODE_PC && movedata.alt1tapcount) {
		g_Vars.currentplayer->bondactivateorreload = g_Vars.currentplayer->bondactivateorreload | JO_ACTION_RELOAD;
	}
	if (movedata.btapcount) {
		g_Vars.currentplayer->activatetimelast = g_Vars.currentplayer->activatetimethis;
		g_Vars.currentplayer->activatetimethis = g_Vars.lvframe60;
		if (!usereloads) {
			g_Vars.currentplayer->bondactivateorreload = g_Vars.currentplayer->bondactivateorreload | JO_ACTION_ACTIVATE;
		} else {
			g_Vars.currentplayer->bondactivateorreload = movedata.btapcount ?
				(g_Vars.currentplayer->bondactivateorreload | JO_ACTION_ACTIVATE | JO_ACTION_RELOAD) : 0;
		}

		bmoveHandleActivate();
	}

	if (!movedata.invertpitch) {
		savedverta = movedata.speedvertadown;
		movedata.analogpitch = -movedata.analogpitch;
		movedata.c1stickyraw = -movedata.c1stickyraw;
		movedata.speedvertadown = movedata.speedvertaup;
		movedata.speedvertaup = savedverta;
	}

	bgunTickGameplay(movedata.triggeron);

	if (g_Vars.bondvisible && (bgunIsFiring(HAND_RIGHT) || bgunIsFiring(HAND_LEFT))) {
		noiseradius = 0;

		if (bgunIsFiring(HAND_RIGHT) && bgunGetNoiseRadius(HAND_RIGHT) > noiseradius) {
			noiseradius = bgunGetNoiseRadius(HAND_RIGHT);
		}

		if (bgunIsFiring(HAND_LEFT) && bgunGetNoiseRadius(HAND_LEFT) > noiseradius) {
			noiseradius = bgunGetNoiseRadius(HAND_LEFT);
		}

		chrsCheckForNoise(noiseradius);
	}

	bgunSetSightVisible(GUNSIGHTREASON_NOTAIMING, movedata.aiming);

	if (movedata.zoomoutfovpersec > 0) {
		currentPlayerZoomOut(movedata.zoomoutfovpersec);
	}

	if (movedata.zoominfovpersec > 0) {
		currentPlayerZoomIn(movedata.zoominfovpersec);
	}

	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED && !g_MainIsEndscreen) {
		zoomfov = PLAYER_DEFAULT_FOV;

		// FarSight in secondary function
		if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT
				&& g_Vars.currentplayer->insightaimmode
				&& (movedata.farsighttempautoseek || g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY)
				&& g_Vars.currentplayer->autoeraserdist > 0) {
			eraserfov = camGetEraserFOV(500.0f / g_Vars.currentplayer->autoeraserdist);

			if (eraserfov > PLAYER_DEFAULT_FOV) {
				eraserfov = PLAYER_DEFAULT_FOV;
			}

			if (eraserfov < ADJUST_ZOOM_FOV(2)) {
				eraserfov = ADJUST_ZOOM_FOV(2);
			}

			g_Vars.currentplayer->gunzoomfovs[1] = eraserfov;

			mtx4TransformVec(camGetPlayerWorldToScreenMtx(), &g_Vars.currentplayer->autoerasertarget->pos, &spa0);

			camProjectWithZoomAndAspect(&spa0, crosspos, eraserfov, g_Vars.currentplayer->c_perspaspect);

			if (crosspos[0] < (camGetScreenLeft() + camGetScreenWidth() * 0.5f) - 20.0f) {
				movedata.aimturnleftspeed = 0.25f;
			} else if (crosspos[0] > camGetScreenLeft() + camGetScreenWidth() * 0.5f + 20.0f) {
				movedata.aimturnrightspeed = 0.25f;
			}

			if (crosspos[1] < (camGetScreenTop() + camGetScreenHeight() * 0.5f) - 20.0f) {
				movedata.speedvertaup = 0.25f;
			} else if (crosspos[1] > camGetScreenTop() + camGetScreenHeight() * 0.5f + 20.0f) {
				movedata.speedvertadown = 0.25f;
			}
		}

		if (movedata.zooming) {
			zoomfov = currentPlayerGetGunZoomFov();
		}

		if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_AR34
				&& g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY) {
			zoomfov = currentPlayerGetGunZoomFov();
		}

		if (zoomfov <= 0) {
			zoomfov = PLAYER_DEFAULT_FOV;
		}

		playerTweenFovY(zoomfov);
		playerUpdateZoom();
	}

	bmoveApplyMoveData(&movedata);

	// Speed boost
	// After 3 seconds of holding forward at max speed, apply boost multiplier.
	// The multiplier starts at 1 and reaches 1.25 after about 0.1 seconds.
	if (g_Vars.currentplayer->speedmaxtime60 >= TICKS(180)) {
		if (g_Vars.currentplayer->speedboost < 1.25f) {
			g_Vars.currentplayer->speedboost += 0.01f * g_Vars.lvupdate60freal;
		}

		if (g_Vars.currentplayer->speedboost > 1.25f) {
			g_Vars.currentplayer->speedboost = 1.25f;
		}
	} else {
		if (g_Vars.currentplayer->speedboost > 1) {
			g_Vars.currentplayer->speedboost -= 0.01f * g_Vars.lvupdate60freal;
		}

		if (g_Vars.currentplayer->speedboost < 1) {
			g_Vars.currentplayer->speedboost = 1;
		}
	}

	// Look ahead
	if (g_Vars.currentplayer->pausemode == PAUSEMODE_UNPAUSED) {
		lookahead = -4;

		offbike = g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK
			|| g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB;

		if (g_Vars.currentplayer->lookaheadcentreenabled) {
			if (g_Vars.lvframenum != g_Vars.currentplayer->lookaheadframe
					&& g_Vars.currentplayernum == (g_Vars.lvframenum & 3)) {
				g_Vars.currentplayer->cachedlookahead = bmoveCalculateLookahead();
			}

			lookahead = g_Vars.currentplayer->cachedlookahead;
		}

		if (g_Vars.currentplayer->movecentrerelease
				&& movedata.analogwalk < 40 && movedata.analogwalk > -40) {
			g_Vars.currentplayer->movecentrerelease = false;
		}

		if (offbike) {
			if (movedata.speedvertadown > 0 || movedata.speedvertaup > 0) {
				g_Vars.currentplayer->docentreupdown = false;
				g_Vars.currentplayer->prevupdown = true;
				g_Vars.currentplayer->automovecentre = false;
			} else {
				if (movedata.disablelookahead) {
					g_Vars.currentplayer->automovecentre = false;
				} else if (g_Vars.currentplayer->automovecentreenabled) {
					if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
						g_Vars.currentplayer->automovecentre = true;
					}

					if (g_Vars.currentplayer->automovecentre
							&& (g_Vars.currentplayer->vv_verta > lookahead + 5.0f || g_Vars.currentplayer->vv_verta < lookahead + -10.0f)
							&& g_Vars.currentplayer->movecentrerelease == false) {
						g_Vars.currentplayer->docentreupdown = true;
					}
				} else if (g_Vars.currentplayer->fastmovecentreenabled
						&& movedata.canlookahead
						&& (movedata.analogwalk > 60 || movedata.analogwalk < -60)
						&& (g_Vars.currentplayer->vv_verta > lookahead + 5.0f || g_Vars.currentplayer->vv_verta < lookahead + -10.0f)
						&& g_Vars.currentplayer->movecentrerelease == false) {
					g_Vars.currentplayer->docentreupdown = true;
				}

				g_Vars.currentplayer->prevupdown = false;
			}
		}

		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
			g_Vars.currentplayer->docentreupdown = false;
		}

		if (g_Vars.currentplayer->docentreupdown) {
			if (offbike) {
				// Determine direction for lookahead increment
				increment2 = (g_Vars.currentplayer->speedverta * g_Vars.currentplayer->speedverta * 0.5f) / 0.05f;

				if (g_Vars.currentplayer->vv_verta > lookahead + increment2) {
					bmoveUpdateSpeedVerta(1);
				} else if (g_Vars.currentplayer->vv_verta < lookahead - increment2) {
					bmoveUpdateSpeedVerta(-1);
				} else {
					bmoveUpdateSpeedVerta(0);
				}

				// Calculate new verta
				newverta = g_Vars.currentplayer->vv_verta + (g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal + g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal);

				if (g_Vars.currentplayer->vv_verta > lookahead && newverta > lookahead) {
					g_Vars.currentplayer->vv_verta = newverta;
				} else if (g_Vars.currentplayer->vv_verta < lookahead && newverta < lookahead) {
					g_Vars.currentplayer->vv_verta = newverta;
				} else {
					g_Vars.currentplayer->vv_verta = lookahead;
					g_Vars.currentplayer->speedverta = 0;

					if (g_Vars.currentplayer->prevupdown == false) {
						g_Vars.currentplayer->docentreupdown = false;
					}
				}
			}
		} else {
			if (movedata.cannaturalpitch) {
				tmp = viGetFovY() / 60.0f;
				fVar25 = movedata.analogpitch / 70.0f;

				if (fVar25 > 1) {
					fVar25 = 1;
				} else if (fVar25 < -1) {
					fVar25 = -1;
				}

				if (fVar25 >= 0) {
					fVar25 *= fVar25;
				} else {
					fVar25 *= -fVar25;
				}

				fVar25 += movedata.freelookdy * mlookscale;

				g_Vars.currentplayer->speedverta = -fVar25 * tmp;
			} else if (movedata.speedvertadown > 0) {
				bmoveUpdateSpeedVerta(movedata.speedvertadown);

				if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
					g_Vars.currentplayer->movecentrerelease = true;
				}
			} else if (movedata.speedvertaup > 0) {
				bmoveUpdateSpeedVerta(-movedata.speedvertaup);

				if (movedata.canlookahead && (movedata.analogwalk > 60 || movedata.analogwalk < -60)) {
					g_Vars.currentplayer->movecentrerelease = true;
				}
			} else {
				bmoveUpdateSpeedVerta(0);
			}

			g_Vars.currentplayer->vv_verta += g_Vars.currentplayer->speedverta * g_Vars.lvupdate60freal * 3.5f;
		}
	}

	if (movedata.cannaturalturn) {
		tmp = viGetFovY() / 60.0f;
		fVar25 = movedata.analogturn / 70.0f;

		if (fVar25 > 1) {
			fVar25 = 1;
		} else if (fVar25 < -1) {
			fVar25 = -1;
		}

		if (fVar25 >= 0) {
			fVar25 *= fVar25;
		} else {
			fVar25 *= -fVar25;
		}

		fVar25 += movedata.freelookdx * mlookscale;

		g_Vars.currentplayer->speedthetacontrol = fVar25 * tmp;
	} else if (movedata.aimturnleftspeed > 0) {
		bmoveUpdateSpeedThetaControl(movedata.aimturnleftspeed);
	} else if (movedata.aimturnrightspeed > 0) {
		bmoveUpdateSpeedThetaControl(-movedata.aimturnrightspeed);
	} else {
		bmoveUpdateSpeedThetaControl(0);
	}

	g_Vars.currentplayer->speedtheta = g_Vars.currentplayer->speedthetacontrol;
	bmoveUpdateSpeedTheta();

	if (movedata.detonating) {
		g_Vars.currentplayer->hands[HAND_RIGHT].mode = HANDMODE_NONE;
		g_Vars.currentplayer->hands[HAND_RIGHT].modenext = HANDMODE_NONE;
		playerActivateRemoteMineDetonator(g_Vars.currentplayernum);
	}

	cancycleweapons = true;

	if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
		cancycleweapons = false;
	}

	if (g_Vars.lvframenum < 10) {
		cancycleweapons = false;
	}

	if (cancycleweapons) {
		while (movedata.weaponbackoffset-- > 0) {
			bgunCycleBack();
		}

		while (movedata.weaponforwardoffset-- > 0) {
			bgunCycleForward();
		}
	}

	if (g_Vars.currentplayer->unk1c64) {
		g_Vars.currentplayer->unk1c64 = 0;
	} else if (movedata.canswivelgun) {
		float x;
		float y;

		bgunSetAimType(0);

		if ((    movedata.canautoaim
				 && (bmoveIsAutoAimXEnabledForCurrentWeapon() || bmoveIsAutoAimYEnabledForCurrentWeapon())
				 && g_Vars.currentplayer->autoxaimprop
				 && g_Vars.currentplayer->autoyaimprop
				 && weaponHasAimFlag(weaponnum, INVAIMFLAG_AUTOAIM))
				|| (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_CMP150 && g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY)) {
			// Auto aim - move crosshair towards target
			int followlockon = false;

			if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_CMP150
					&& g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponfunc == FUNC_SECONDARY) {
				followlockon = true;
			}

			if (g_Vars.currentplayer->autoaimdamp > 0.963f) {
				g_Vars.currentplayer->autoaimdamp -= 0.00031999943894334f * g_Vars.lvupdate60freal;
			}

			if (g_Vars.currentplayer->autoaimdamp < 0.963f) {
				g_Vars.currentplayer->autoaimdamp = 0.963f;
			}

			x = g_Vars.currentplayer->autoaimx;
			y = g_Vars.currentplayer->autoaimy;

			if (followlockon) {
				bgunSwivel(x, y, 0.915f, 0.915f);
			} else {
				bgunSwivelWithDamp(x, y, g_Vars.currentplayer->autoaimdamp);
			}
		} else {
			// This code moves the crosshair as the player turns and makes
			// it return to the centre when not affected by anything else.
			if (g_Vars.currentplayer->autoaimdamp < (0.979f)) {
				g_Vars.currentplayer->autoaimdamp += (0.00031999943894334f) * g_Vars.lvupdate60freal;
			}

			if (g_Vars.currentplayer->autoaimdamp > (0.979f)) {
				g_Vars.currentplayer->autoaimdamp = (0.979f);
			}

			float xscale, yscale;
			if (movedata.freelookdx || movedata.freelookdy) {
				xscale = PLAYER_EXTCFG().crosshairsway * 0.20f;
				yscale = PLAYER_EXTCFG().crosshairsway * 0.30f;
			} else {
				xscale = yscale = PLAYER_EXTCFG().crosshairsway;
			}
			x = g_Vars.currentplayer->speedtheta * 0.3f * xscale + g_Vars.currentplayer->gunextraaimx;
			y = -g_Vars.currentplayer->speedverta * 0.1f * yscale + g_Vars.currentplayer->gunextraaimy;

			bgunSwivelWithDamp(x, y, 0.963f);
		}
	} else if (movedata.canmanualaim) {
		// Adjust crosshair's position on screen
		// when holding aim and moving stick
		bgunSetAimType(0);
		if (allowmcross) {
			// joystick is inactive, move crosshair using the mouse
			const float xcoeff = 320.f / 1080.f;
			const float ycoeff = 240.f / 1080.f;
			const float xscale = (PLAYER_EXTCFG().mouseaimspeedx * xcoeff) / g_Vars.currentplayer->aspect;
			const float yscale = PLAYER_EXTCFG().mouseaimspeedy * ycoeff;
			float x = g_Vars.currentplayer->swivelpos[0] + movedata.freelookdx * xscale;
			float y = g_Vars.currentplayer->swivelpos[1] + movedata.freelookdy * yscale;
			x = (x < -1.f) ? -1.f : ((x > 1.f) ? 1.f : x);
			y = (y < -1.f) ? -1.f : ((y > 1.f) ? 1.f : y);
			g_Vars.currentplayer->swivelpos[0] = x;
			g_Vars.currentplayer->swivelpos[1] = y;
			bgunSwivelWithDamp(x, y, 0.01f);
			return;
		}
		bgunSwivelWithoutDamp((movedata.c1stickxraw * 0.65f) / 80.0f, (movedata.c1stickyraw * 0.65f) / 80.0f);
	}
}

void bmoveFindEnteredRoomsByPos(struct player *player, struct coord *mid, RoomNum *rooms)
{
	struct coord bbmin;
	struct coord bbmax;
	float eyeheight = g_Vars.players[playermgrGetPlayerNumByProp(player->prop)]->vv_eyeheight;
	float headheight = g_Vars.players[playermgrGetPlayerNumByProp(player->prop)]->vv_headheight;

	bbmin.x = mid->x - 50;
	bbmin.y = mid->y - player->crouchheight - eyeheight - 10;
	bbmin.z = mid->z - 50;

	bbmax.x = mid->x + 50;
	bbmax.y = mid->y - player->crouchheight - eyeheight + headheight + 10;
	bbmax.z = mid->z + 50;

	bgFindEnteredRooms(&bbmin, &bbmax, rooms, 7, false);
}

void bmoveFindEnteredRooms(struct player *player, RoomNum *rooms)
{
	bmoveFindEnteredRoomsByPos(player, &player->prop->pos, rooms);
}

void bmoveUpdateRooms(struct player *player)
{
	propDeregisterRooms(player->prop);
	bmoveFindEnteredRooms(player, player->prop->rooms);
	propRegisterRooms(player->prop);
}

void bmoveDampenVelocity(struct coord *arg0)
{
	if (arg0->f[0] || arg0->f[2]) {
		float hypotenuse = sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2]);
		int i;

		if (hypotenuse > 1.5f) {
			arg0->x *= 1.5f / hypotenuse;
			arg0->z *= 1.5f / hypotenuse;
			hypotenuse = 1.5f;
		}

		for (i = 0; i < 3; i++) {
			if (hypotenuse > 0.0001f) {
				if (arg0->f[i] != 0) {
					if (arg0->f[i] > 0) {
						arg0->f[i] -= (1.0f / 30.0f) * g_Vars.lvupdate60freal * arg0->f[i] / hypotenuse;

						if (arg0->f[i] < 0) {
							arg0->f[i] = 0;
						}
					} else if (arg0->f[i] < 0) {
						arg0->f[i] -= (1.0f / 30.0f) * g_Vars.lvupdate60freal * arg0->f[i] / hypotenuse;

						if (arg0->f[i] > 0) {
							arg0->f[i] = 0;
						}
					}
				}
			} else {
				arg0->f[i] = 0;
			}
		}
	}
}

void bmove0f0cba88(float *a, float *b, struct coord *c, float mult1, float mult2)
{
	if (c->x != 0 || c->z != 0) {
		bmoveDampenVelocity(c);
		*a = c->z * mult2 + -c->x * mult1;
		*b = -c->x * mult2 - c->z * mult1;
	} else {
		*a = 0;
		*b = 0;
	}
}

void bmoveUpdateMoveInitSpeed(struct coord *newpos)
{
	if (g_Vars.currentplayer->moveinitspeed.x != 0) {
		if (g_Vars.currentplayer->moveinitspeed.x < 0.001f && g_Vars.currentplayer->moveinitspeed.x > -0.001f) {
			g_Vars.currentplayer->moveinitspeed.x = 0;
		} else {
			g_Vars.currentplayer->moveinitspeed.x *= 0.9f;
			newpos->x += g_Vars.currentplayer->moveinitspeed.x * g_Vars.lvupdate60freal;
		}
	}

	if (g_Vars.currentplayer->moveinitspeed.z != 0) {
		if (g_Vars.currentplayer->moveinitspeed.z < 0.001f && g_Vars.currentplayer->moveinitspeed.z > -0.001f) {
			g_Vars.currentplayer->moveinitspeed.z = 0;
		} else {
			g_Vars.currentplayer->moveinitspeed.z *= 0.9f;
			newpos->z += g_Vars.currentplayer->moveinitspeed.z * g_Vars.lvupdate60freal;
		}
	}
}

void bmoveTick(bool allowc1x, bool allowc1y, bool allowc1buttons, bool ignorec2)
{
	struct chrdata *chr;
	uint8_t foot;
	int sound;
	float xdiff;
	float ydiff;
	float zdiff;
	float distance;

	bmoveProcessInput(allowc1x, allowc1y, allowc1buttons, ignorec2);

	if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
		bbikeTick();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB) {
		bgrabTick();
	} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
		bwalkTick();
	}

	// Update footstep sounds
	if ((g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK || g_Vars.currentplayer->bondmovemode == MOVEMODE_GRAB)
			&& (g_Vars.currentplayer->speedforwards || g_Vars.currentplayer->speedsideways)
			&& (!g_Vars.normmplayerisrunning || PLAYERCOUNT() == 1)) {
		chr = g_Vars.currentplayer->prop->chr;

		if (g_Vars.currentplayer->cameramode == CAMERAMODE_DEFAULT
				&& g_Vars.currentplayer->bdeltapos.y >= -6.0f) {
			xdiff = g_Vars.currentplayer->bondprevpos.x - g_Vars.currentplayer->prop->pos.x;
			ydiff = g_Vars.currentplayer->bondprevpos.y - g_Vars.currentplayer->prop->pos.y;
			zdiff = g_Vars.currentplayer->bondprevpos.z - g_Vars.currentplayer->prop->pos.z;

			foot = 0;
			distance = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

			g_Vars.currentplayer->footstepdist += distance;

			if (g_Vars.currentplayer->footstepdist >= 150.0f) {
				foot = 1;
				g_Vars.currentplayer->footstepdist = 0;
			}

			if (foot) {
				if (g_Vars.currentplayer->foot) {
					chr->footstep = 1;
				} else {
					chr->footstep = 2;
				}

				g_Vars.currentplayer->foot = 1 - g_Vars.currentplayer->foot;

				chr->floortype = g_Vars.currentplayer->floortype;

				sound = footstepChooseSound(chr, distance > 10);

				if (sound != -1) {
					snd00010718(0, 0, AL_VOL_FULL, AL_PAN_CENTER, sound, 1, 1, -1, true);
				}
			}
		}
	}
}

void bmoveUpdateVerta(void)
{
	while (g_Vars.currentplayer->vv_verta < -180) {
		g_Vars.currentplayer->vv_verta += 360;
	}

	while (g_Vars.currentplayer->vv_verta >= 180) {
		g_Vars.currentplayer->vv_verta -= 360;
	}

	if (g_Vars.currentplayer->vv_verta > 90) {
		g_Vars.currentplayer->vv_verta = 90;
	} else if (g_Vars.currentplayer->vv_verta < -90) {
		g_Vars.currentplayer->vv_verta = -90;
	}

	g_Vars.currentplayer->vv_costheta = cosf(DEG2RAD(g_Vars.currentplayer->vv_theta));
	g_Vars.currentplayer->vv_sintheta = sinf(DEG2RAD(g_Vars.currentplayer->vv_theta));

	g_Vars.currentplayer->vv_verta360 = g_Vars.currentplayer->vv_verta;

	if (g_Vars.currentplayer->vv_verta360 < 0) {
		g_Vars.currentplayer->vv_verta360 += 360;
	}

	g_Vars.currentplayer->vv_cosverta = cosf(DEG2RAD(g_Vars.currentplayer->vv_verta360));
	g_Vars.currentplayer->vv_sinverta = sinf(DEG2RAD(g_Vars.currentplayer->vv_verta360));

	g_Vars.currentplayer->bond2.unk00.x = -g_Vars.currentplayer->vv_sintheta;
	g_Vars.currentplayer->bond2.unk00.y = 0;
	g_Vars.currentplayer->bond2.unk00.z = g_Vars.currentplayer->vv_costheta;

	if (g_Vars.currentplayer->prop) {
		struct chrdata *chr = g_Vars.currentplayer->prop->chr;

		if (chr && chr->model) {
			chrSetLookAngle(chr, DEG2RAD(360 - g_Vars.currentplayer->vv_theta));
		}
	}
}

void bmoveUpdateEyeHeight(struct coord *arg)
{
	float min;
	float mult;

	g_Vars.currentplayer->bond2.unk10.x = arg->x;
	g_Vars.currentplayer->bond2.unk10.y = arg->y;
	g_Vars.currentplayer->bond2.unk10.z = arg->z;

	// Lean down when dead
	if (g_Vars.currentplayer->isdead && g_Vars.currentplayer->bondleandown > 0) {
		g_Vars.currentplayer->bondleandown -= 0.25f;

		if (g_Vars.currentplayer->bondleandown < 0) {
			g_Vars.currentplayer->bondleandown = 0;
		}
	}

	if (g_Vars.currentplayer->vv_verta < 0) {
		g_Vars.currentplayer->bond2.unk10.y += -(1.0f - g_Vars.currentplayer->vv_cosverta) * g_Vars.currentplayer->bondleandown;
	}

	// Small Jo scaling logic
	if (cheatIsActive(CHEAT_SMALLJO)) {
		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_BIKE) {
			mult = g_Vars.currentplayer->bondentert * 0.6f + 0.4f;
		} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK && g_Vars.currentplayer->walkinitmove) {
			mult = (1.0f - g_Vars.currentplayer->walkinitt) * 0.6f + 0.4f;
			g_Vars.currentplayer->bond2.unk10.y += (g_Vars.currentplayer->crouchoffsetreal - g_Vars.currentplayer->crouchoffsetrealsmall) * g_Vars.currentplayer->walkinitt;
		} else if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK) {
			mult = 0.4f;
			g_Vars.currentplayer->bond2.unk10.y += (g_Vars.currentplayer->crouchoffsetreal - g_Vars.currentplayer->crouchoffsetrealsmall);
		} else {
			mult = 0.4f;
		}

		g_Vars.currentplayer->bond2.unk10.y = (g_Vars.currentplayer->bond2.unk10.y - g_Vars.currentplayer->vv_manground) * mult;
		g_Vars.currentplayer->bond2.unk10.y += g_Vars.currentplayer->vv_manground;
	}

	// Make sure head doesn't go below the ground
	min = g_Vars.currentplayer->vv_ground + 10;

	if (g_Vars.currentplayer->bond2.unk10.y < min) {
		g_Vars.currentplayer->bond2.unk10.y = min;
	}
}

void bmoveUpdateHead(float animFrameDelta, float animSpeed, float headTilt, Mtx *targetMatrix, float blendFraction)
{
	float animationRate = 0;
	Mtx headMatrix;             // Final head orientation matrix
	Mtx tempMatrix;             // Used for building intermediate transforms
	float currentQuat[4];       // Quaternion from calculated headMatrix
	float targetQuat[4];        // Quaternion from provided targetMatrix
	float blendedQuat[4];       // Result of slerped quaternion

	if (!g_Vars.currentplayer->isdead) {
		bheadAdjustAnimation(animFrameDelta);

		if (animFrameDelta != 0) {
			animationRate = animSpeed / animFrameDelta;
		} else if (animSpeed == 0) {
			animFrameDelta = 0;
		}
	} else {
		// Player just died, start a new death animation
		if (g_Vars.currentplayer->startnewbonddie) {
			bheadStartDeathAnimation(
				g_DeathAnimations[rngRandom() % g_NumDeathAnimations],
				rngRandom() % 2,
				0,
				1
			);
			g_Vars.currentplayer->startnewbonddie = false;
		}

		// Set fixed speed for death animation
		bheadSetSpeed(0.5f);
		headTilt = 0;
	}

	// Update head animation with computed rate and tilt
	bheadUpdate(animationRate, headTilt);

	// Start with a vertical rotation (pitch/tilt)
	mtx4LoadXRotation(DEG2RAD(360 - g_Vars.currentplayer->vv_verta360), &headMatrix);

	// Optional head roll via look vector and up vector
	if (optionsGetHeadRoll(g_Vars.currentplayerstats->mpindex)) {
		mtxBuildLookAtFromTarget(
			&tempMatrix,
			0, 0, 0,
			-g_Vars.currentplayer->headlook.x, -g_Vars.currentplayer->headlook.y, -g_Vars.currentplayer->headlook.z,
			g_Vars.currentplayer->headup.x, g_Vars.currentplayer->headup.y, g_Vars.currentplayer->headup.z
		);
		mtx4MultMtx4InPlace(&tempMatrix, &headMatrix);
	}

	// Add horizontal rotation (yaw/heading)
	mtx4LoadYRotation(DEG2RAD(360 - g_Vars.currentplayer->vv_theta), &tempMatrix);
	mtx4MultMtx4InPlace(&tempMatrix, &headMatrix);

	// Blend toward the target orientation if one was provided
	if (targetMatrix) {
		quaternion3x3MtxToQuat(&headMatrix, currentQuat);
		quaternion3x3MtxToQuat(targetMatrix, targetQuat);
		quaternionAvoidFlips(currentQuat, targetQuat);
		quaternionSlerp(currentQuat, targetQuat, blendFraction, blendedQuat);
		quaternionToMtx(blendedQuat, &headMatrix);
	}

	// Set head orientation vectors in bond2 struct
	g_Vars.currentplayer->bond2.unk1c.x = headMatrix[2][0];  // Forward vector
	g_Vars.currentplayer->bond2.unk1c.y = headMatrix[2][1];
	g_Vars.currentplayer->bond2.unk1c.z = headMatrix[2][2];

	g_Vars.currentplayer->bond2.unk28.x = headMatrix[1][0];  // Up vector
	g_Vars.currentplayer->bond2.unk28.y = headMatrix[1][1];
	g_Vars.currentplayer->bond2.unk28.z = headMatrix[1][2];
}

void bmoveUpdateHeadNoTargetMtx(float arg0, float arg1, float arg2)
{
	bmoveUpdateHead(arg0, arg1, arg2, NULL, 0);
}

int bmoveGetCrouchPos(void)
{
	return (g_Vars.currentplayer->crouchpos < g_Vars.currentplayer->autocrouchpos)
		? g_Vars.currentplayer->crouchpos
		: g_Vars.currentplayer->autocrouchpos;
}

int bmoveGetCrouchPosByPlayer(int playernum)
{
	return (g_Vars.players[playernum]->crouchpos < g_Vars.players[playernum]->autocrouchpos)
		? g_Vars.players[playernum]->crouchpos
		: g_Vars.players[playernum]->autocrouchpos;
}
