#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/bondgun.h"
#include "game/weaponutils.h"
#include "game/utils.h"
#include "game/tex.h"
#include "game/savebuffer.h"
#include "game/sight.h"
#include "game/textutils.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "game/lang.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "game/debug.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include <math.h>
#include "video.h"

#define SIGHT_COLOUR ((PLAYER_EXTCFG().crosshairhealth >= CROSSHAIR_HEALTH_ON_GREEN) ? sightGetCrosshairHealthColor(g_Vars.currentplayer->bondhealth, g_Vars.currentplayer->prop->chr->cshield * 0.125f) : PLAYER_EXTCFG().crosshaircolour)
#define SIGHT_SCALE PLAYER_EXTCFG().crosshairsize

static uint32_t sightGetCrosshairHealthColor(float health, float shield)
{
	const float ratio = MAX(0.0f, MIN(health + shield, 2.0f));

	int red = 0;
	int green = 0;
	int blue = 0;
	if (ratio < 0.2f) {
		// Red (critical health level)
		red = 255;
		green = 0;
		blue = 0;
	} else if (ratio < 0.6f) {
		// Red-yellow
		red = 255;
		green = 255 * ((ratio - 0.2f) / 0.4f);
		blue = 0;
	} else if (ratio < 1.0f) {
		if (PLAYER_EXTCFG().crosshairhealth == CROSSHAIR_HEALTH_ON_GREEN) {
			// Yellow-green
			red = 255 * ((ratio - 0.6f) / 0.4f);
			green = 255;
			blue = 0;
		} else {
			// Yellow-white
			red = 255;
			green = 255;
			blue = 255 * ((ratio - 0.6f) / 0.4f);
		}
	} else {
		if (PLAYER_EXTCFG().crosshairhealth == CROSSHAIR_HEALTH_ON_GREEN) {
			// Green-cyan (overheal via shield)
			red = 0;
			green = 255;
			blue = 255 * (ratio - 1.0f);
		} else {
			// White-green (overheal via shield)
			red = 255 * (2.0f - ratio);
			green = 255;
			blue = 255 * (2.0f - ratio);
		}
	}

	return (red << 24) + (green << 16) + (blue << 8) + (PLAYER_EXTCFG().crosshaircolour & 0xff);
}

static inline float sightGetScaleX(void)
{
	return (videoGetAspect() / SCREEN_ASPECT);
}

static inline int sightGetAdjustedX(const float x)
{
	const float cx = (x - (float)(SCREEN_WIDTH_LO / 2)) * sightGetScaleX();
	return roundf((float)(SCREEN_WIDTH_LO / 2) + cx);
}

/**
 * Return true if the prop is considered friendly (blue sight).
 */
bool sightIsPropFriendly(struct prop *prop)
{
	if (prop == NULL) {
		prop = g_Vars.currentplayer->lookingatprop.prop;
	}

	if (prop == NULL) {
		return false;
	}

	if (prop->type != PROPTYPE_CHR && prop->type != PROPTYPE_PLAYER) {
		return false;
	}

	if (g_Vars.coopplayernum >= 0 && prop->type == PROPTYPE_PLAYER) {
		return true;
	}

	if (g_Vars.antiplayernum >= 0 && prop->type == PROPTYPE_PLAYER) {
		return false;
	}

	if (g_Vars.normmplayerisrunning == false
			&& prop->chr
			&& (prop->chr->hidden2 & CHRH2FLAG_BLUESIGHT)) {
		return true;
	}

	return chrCompareTeams(g_Vars.currentplayer->prop->chr, prop->chr, COMPARE_FRIENDS);
}

/**
 * Return true if the given prop can be added to the target list.
 */
bool sightCanTargetProp(struct prop *prop, int max)
{
	int i;

	for (i = 0; i < max; i++) {
		if (prop == g_Vars.currentplayer->trackedprops[i].prop) {
			return false;
		}
	}

	if (prop->type == PROPTYPE_CHR) {
		return true;
	}

	if (prop->type == PROPTYPE_PLAYER) {
		return true;
	}

	if ((prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_WEAPON || prop->type == PROPTYPE_DOOR)
			&& prop->obj && (prop->obj->flags3 & OBJFLAG3_REACTTOSIGHT)) {
		return true;
	}

	if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_ROCKETLAUNCHER) {
		return true;
	}

	return false;
}

/**
 * Return true if the sight should change colour when aiming at the given prop.
 */
bool sightIsReactiveToProp(struct prop *prop)
{
	if (prop->obj == NULL) {
		return false;
	}

	if (prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_WEAPON || prop->type == PROPTYPE_DOOR) {
		struct defaultobj *obj = prop->obj;

		if (g_Vars.stagenum == STAGE_CITRAINING
				&& (obj->modelnum == MODEL_COMHUB || obj->modelnum == MODEL_CIHUB || obj->modelnum == MODEL_TARGET)) {
			return true;
		}

		if (objGetDestroyedLevel(obj) > 0) {
			return false;
		}
	} else if (prop->type == PROPTYPE_CHR) {
		struct chrdata *chr = prop->chr;

		if (chr && chr->race == RACE_EYESPY) {
			struct eyespy *eyespy = chrToEyespy(chr);

			if (!eyespy || !eyespy->deployed) {
				return false;
			}
		}
	}

	return true;
}

int sightFindFreeTargetIndex(int max)
{
	int i;

	for (i = 0; i < max; i++) {
		if (g_Vars.currentplayer->trackedprops[i].prop == NULL) {
			return i;
		}
	}

	return -1;
}

void func0f0d7364(void)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
		g_Vars.currentplayer->trackedprops[i].prop = NULL;
	}
}

void sightTick(bool sighton)
{
	struct trackedprop *trackedprop;
	uint8_t newtracktype;
	int i;
	int index;
	struct invaimsettings *gunsettings = gsetGetAimSettings(&g_Vars.currentplayer->hands[0].gset);
	struct weaponfunc *func = weaponGetFunctionById(g_Vars.currentplayer->hands[0].gset.weaponnum,
			g_Vars.currentplayer->hands[0].gset.weaponfunc);

	g_Vars.currentplayer->sighttimer240 += g_Vars.lvupdate240;

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->targetset); i++) {
		if (g_Vars.currentplayer->targetset[i] > TICKS(512)) {
			if (g_Vars.currentplayer->targetset[i] < 1024 - g_Vars.lvupdate240) {
				g_Vars.currentplayer->targetset[i] += g_Vars.lvupdate240;
			} else {
				g_Vars.currentplayer->targetset[i] = TICKS(1020);
			}
		} else {
			if (g_Vars.currentplayer->targetset[i] < 516 - g_Vars.lvupdate240) {
				g_Vars.currentplayer->targetset[i] += g_Vars.lvupdate240;
			} else {
				g_Vars.currentplayer->targetset[i] = TICKS(512);
			}
		}
	}

	newtracktype = gunsettings->tracktype;

	if (gsetHasFunctionFlags(&g_Vars.currentplayer->hands[0].gset, FUNCFLAG_THREATDETECTOR)) {
		newtracktype = SIGHTTRACKTYPE_THREATDETECTOR;
	}

	if (func && (func->type & 0xff) == INVENTORYFUNCTYPE_MELEE) {
		newtracktype = SIGHTTRACKTYPE_NONE;
	}

	if (newtracktype != g_Vars.currentplayer->sighttracktype) {
		if (newtracktype == SIGHTTRACKTYPE_THREATDETECTOR) {
			for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
				g_Vars.currentplayer->trackedprops[i].prop = NULL;
			}
		}

		g_Vars.currentplayer->sighttracktype = newtracktype;

		switch (newtracktype) {
		case SIGHTTRACKTYPE_NONE:
		case SIGHTTRACKTYPE_DEFAULT:
		case SIGHTTRACKTYPE_BETASCANNER:
		case SIGHTTRACKTYPE_ROCKETLAUNCHER:
		case SIGHTTRACKTYPE_FOLLOWLOCKON:
			break;
		}
	}

	if (sighton && g_Vars.currentplayer->lastsighton == false && newtracktype != SIGHTTRACKTYPE_THREATDETECTOR) {
		for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
			g_Vars.currentplayer->trackedprops[i].prop = NULL;
		}
	}

	for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
		trackedprop = &g_Vars.currentplayer->trackedprops[i];

		if (trackedprop->prop && !sightIsReactiveToProp(trackedprop->prop)) {
			trackedprop->prop = NULL;
		}
	}

	trackedprop = &g_Vars.currentplayer->lookingatprop;

	if (trackedprop->prop && !sightIsReactiveToProp(trackedprop->prop)) {
		trackedprop->prop = NULL;
	}

	switch (g_Vars.currentplayer->sighttracktype) {
	case SIGHTTRACKTYPE_DEFAULT:
	case SIGHTTRACKTYPE_BETASCANNER:
		// Conditionally copy lookingatprop to trackedprops[0], overwriting anything that's there
		if (sighton) {
			if (g_Vars.currentplayer->lookingatprop.prop) {
				if (g_Vars.currentplayer->lookingatprop.prop != g_Vars.currentplayer->trackedprops[0].prop) {
					struct sndstate *handle;

					handle = snd00010718(NULL, 0, AL_VOL_FULL, AL_PAN_CENTER, SFX_0007, 1, 1, -1, true);

					trackedprop = &g_Vars.currentplayer->trackedprops[0];

					trackedprop->prop = g_Vars.currentplayer->lookingatprop.prop;
					trackedprop->x1 = g_Vars.currentplayer->lookingatprop.x1;
					trackedprop->y1 = g_Vars.currentplayer->lookingatprop.y1;
					trackedprop->x2 = g_Vars.currentplayer->lookingatprop.x2;
					trackedprop->y2 = g_Vars.currentplayer->lookingatprop.y2;

					g_Vars.currentplayer->targetset[0] = 0;
				}
			} else {
				g_Vars.currentplayer->trackedprops[0].prop = NULL;
			}
		}
		break;
	case SIGHTTRACKTYPE_ROCKETLAUNCHER:
		// Conditionally copy lookingatprop to trackedprops[0], but only if that slot is empty
		if (sighton && g_Vars.currentplayer->lookingatprop.prop
				&& sightCanTargetProp(g_Vars.currentplayer->lookingatprop.prop, 1)) {
			index = sightFindFreeTargetIndex(1);

			if (index >= 0) {
				struct sndstate *handle;

				handle = snd00010718(NULL, 0, AL_VOL_FULL, AL_PAN_CENTER, SFX_0007, 1, 1, -1, 1);

				trackedprop = &g_Vars.currentplayer->trackedprops[index];

				trackedprop->prop = g_Vars.currentplayer->lookingatprop.prop;
				trackedprop->x1 = g_Vars.currentplayer->lookingatprop.x1;
				trackedprop->y1 = g_Vars.currentplayer->lookingatprop.y1;
				trackedprop->x2 = g_Vars.currentplayer->lookingatprop.x2;
				trackedprop->y2 = g_Vars.currentplayer->lookingatprop.y2;

				g_Vars.currentplayer->targetset[index] = 0;
			}
		}
		break;
	case SIGHTTRACKTYPE_FOLLOWLOCKON:
		// Conditionally copy lookingatprop to any trackedprops slot, but only if the slot is empty
		if (sighton && g_Vars.currentplayer->lookingatprop.prop
				&& sightCanTargetProp(g_Vars.currentplayer->lookingatprop.prop, 4)) {
			index = sightFindFreeTargetIndex(4);

			if (index >= 0) {
				struct sndstate *handle;

				handle = snd00010718(NULL, 0, AL_VOL_FULL, AL_PAN_CENTER, SFX_0007, 1, 1, -1, 1);

				trackedprop = &g_Vars.currentplayer->trackedprops[index];

				trackedprop->prop = g_Vars.currentplayer->lookingatprop.prop;
				trackedprop->x1 = g_Vars.currentplayer->lookingatprop.x1;
				trackedprop->y1 = g_Vars.currentplayer->lookingatprop.y1;
				trackedprop->x2 = g_Vars.currentplayer->lookingatprop.x2;
				trackedprop->y2 = g_Vars.currentplayer->lookingatprop.y2;

				g_Vars.currentplayer->targetset[index] = 0;
			}
		}
		break;
	case SIGHTTRACKTYPE_NONE:
	case SIGHTTRACKTYPE_THREATDETECTOR:
		break;
	}

	g_Vars.currentplayer->lastsighton = sighton;
}

/**
 * Calculate the position of one border of a target box.
 *
 * The arguments here are named for a left border,
 * but can be called for any of the four edges.
 */
int sightCalculateBoxBound(int targetx, int viewleft, int timeelapsed, int timeend)
{
	int value;

	if (timeelapsed > timeend) {
		timeelapsed = timeend;
	}

	value = (targetx - viewleft) * timeelapsed;

	return viewleft + value / timeend;
}

/**
 * Draw a red (or blue) box around the given trackedprop.
 *
 * textid can be:
 * 0 to have no label
 * 1 to label it as "0"
 * 2 to label it as "1"
 * ...
 * 6 to label it as "5"
 * 7 or above to treat textid as a proper language text ID.
 */
Gfx *sightDrawTargetBox(Gfx *gdl, struct trackedprop *trackedprop, int textid, int time)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();
	int viewright = viewleft + viewwidth - 1;
	int viewbottom = viewtop + viewheight - 1;
	uint32_t colour;
	int boxleft;
	int boxright;
	int boxtop;
	int boxbottom;
	bool textonscreen = true;

	if (time > TICKS(512)) {
		time = TICKS(512);
	}

	boxleft = sightCalculateBoxBound(trackedprop->x1, viewleft, time, TICKS(80));
	boxtop = sightCalculateBoxBound(trackedprop->y1, viewtop, time, TICKS(80));
	boxright = sightCalculateBoxBound(trackedprop->x2, viewright, time, TICKS(80));
	boxbottom = sightCalculateBoxBound(trackedprop->y2, viewbottom, time, TICKS(80));

	if (trackedprop->prop) {
		colour = sightIsPropFriendly(trackedprop->prop) ? 0x000ff60 : 0xff000060;

		gdl = textSetPrimColour(gdl, colour);

		// Left
		if (boxleft >= viewleft && boxleft <= viewright && boxtop <= viewbottom && boxbottom >= viewtop) {
			gfx_HUD_Rectangle(gdl++,
					boxleft, (boxtop > viewtop ? boxtop : viewtop),
					boxleft, (boxbottom < viewbottom ? boxbottom : viewbottom));
		}

		// Right
		if (boxright >= viewleft && boxright <= viewright && boxtop <= viewbottom && boxbottom >= viewtop) {
			gfx_HUD_Rectangle(gdl++,
					boxright, (boxtop > viewtop ? boxtop : viewtop),
					boxright, (boxbottom < viewbottom ? boxbottom : viewbottom));
		} else {
			textonscreen = false;
		}

		// Top
		if (boxtop >= viewtop && boxtop <= viewbottom && boxleft <= viewright && boxright >= viewleft) {
			gfx_HUD_Rectangle(gdl++,
					(boxleft > viewleft ? boxleft : viewleft), boxtop,
					(boxright < viewright ? boxright : viewright), boxtop);
		} else {
			textonscreen = false;
		}

		// Bottom
		if (boxbottom >= viewtop && boxbottom <= viewbottom && boxleft <= viewright && boxright >= viewleft) {
			gfx_HUD_Rectangle(gdl++,
					(boxleft > viewleft ? boxleft : viewleft), boxbottom,
					(boxright < viewright ? boxright : viewright), boxbottom);
		}

		gdl = textSetCCCustom02(gdl);

		if (textid != 0 && textonscreen) {
			int x = boxright + 3;
			int y = boxtop + 3;

			if (textid < 7) {
				char label[] = {'1', '\n', '\0'};

				// textid 1 writes '0'
				label[0] = textid + 0x2f;

				gdl = textRender(gdl, &x, &y, label, g_CharsNumeric, g_FontNumeric, 0x00ff00a0, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);
			} else {
				char *text = langGet(textid);
				gdl = textRender(gdl, &x, &y, text, g_CharsHandelGothicXs, g_FontHandelGothicXs, 0x00ff00a0, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);
			}
		}
	}

	return gdl;
}

Gfx *sightDrawAimer(Gfx *gdl, int x, int y, int radius, int cornergap, uint32_t colour)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();
	int viewright = viewleft + viewwidth - 1;
	int viewbottom = viewtop + viewheight - 1;

	gdl = textSetPrimColour(gdl, SIGHT_COLOUR);

	x = sightGetAdjustedX(x);
	gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, -2, -2);

	// Draw the lines that span most of the viewport
	if (PLAYERCOUNT() == 1) {
		gdl += gfx_HUD_Rectangle_EXT(gdl, viewleft + 48, y, x - radius + 2, y);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + radius - 2, y, viewright - 49, y);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x, viewtop + 10, x, y - radius + 2);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x, y + radius - 2, x, viewbottom - 10);
	} else {
		gdl += gfx_HUD_Rectangle_EXT(gdl, viewleft, y, x - radius + 2, y);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + radius - 2, y, viewright, y);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x, viewtop, x, y - radius + 2);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x, y + radius - 2, x, viewbottom);
	}

	gdl = textSetCCCustom02(gdl);
	gdl = textSetPrimColour(gdl, colour);

	// Draw the box
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y - radius, x - radius, y + radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + radius, y - radius, x + radius, y + radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y - radius, x + radius, y - radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y + radius, x + radius, y + radius);

	// Go over the corners a second time
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y - radius, x - radius, y - cornergap);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y + cornergap, x - radius, y + radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + radius, y - radius, x + radius, y - cornergap);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + radius, y + cornergap, x + radius, y + radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y - radius, x - cornergap, y - radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + cornergap, y - radius, x + radius, y - radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - radius, y + radius, x - cornergap, y + radius);
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + cornergap, y + radius, x + radius, y + radius);

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);

	gdl = textSetCCCustom02(gdl);

	return gdl;
}

Gfx *sightDrawDefault(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	int radius;
	int cornergap;
	uint32_t colour;
	int x = (int) crossx;
	int y = crossy;
	struct trackedprop *trackedprop;
	int i;

	static int sight = 0;
	static int identifytimer = 0;

	gdl = textConfigureGfxPipeline(gdl);

	switch (g_Vars.currentplayer->sighttracktype) {
	case SIGHTTRACKTYPE_NONE:
		// SIGHTTRACKTYPE_NONE is used for unarmed, but this appears to be
		// unreachable. The aimer is never drawn when unarmed.
		if (sighton) {
			colour = SIGHT_COLOUR;
			radius = 8;
			cornergap = 5;
			gdl = sightDrawAimer(gdl, x, y, radius, cornergap, colour);
		}
		break;
	case SIGHTTRACKTYPE_DEFAULT:
		// For most guns, render the aimer if holding R
		if (sighton) {
			if (g_Vars.currentplayer->lookingatprop.prop == NULL) {
				colour = SIGHT_COLOUR;
				radius = 8;
				cornergap = 5;
			} else {
				colour = sightIsPropFriendly(NULL) ? 0x0000ff60 : 0xff000060;
				radius = 6;
				cornergap = 3;
			}

			switch (sight) {
			case 0:
				gdl = sightDrawAimer(gdl, x, y, radius, cornergap, colour);
				break;
			}
		}
		break;
	case SIGHTTRACKTYPE_BETASCANNER:
		// An unused sight target. When holding R, it flashes the text
		// "Identify" and draws a red box around the targetted prop.
		if (sighton) {
			int textx;
			int texty;

			if (g_Vars.currentplayer->lookingatprop.prop == NULL) {
				colour = SIGHT_COLOUR;
				radius = 8;
				cornergap = 5;
			} else {
				colour = sightIsPropFriendly(NULL) ? 0x0000ff60 : 0xff000060;
				radius = 6;
				cornergap = 3;
			}

			textx = 135;
			texty = 200;

			identifytimer += g_Vars.lvupdate240;

			if (identifytimer & 0x80) {
				// "Identify"
				gdl = textRender(gdl, &textx, &texty, langGet(L_MISC_439),
						g_CharsHandelGothicXs, g_FontHandelGothicXs, 0x00ff00a0, 0x000000a0,
						viGetWidth(), viGetHeight(), 0, 0);
			}

			gdl = sightDrawAimer(gdl, x, y, radius, cornergap, colour);

			if (g_Vars.currentplayer->lookingatprop.prop) {
				gdl = sightDrawTargetBox(gdl, &g_Vars.currentplayer->lookingatprop, 1, g_Vars.currentplayer->targetset[0]);
			}
		}
		break;
	case SIGHTTRACKTYPE_ROCKETLAUNCHER:
		for (i = 0; i < 1; i++) {
			trackedprop = &g_Vars.currentplayer->trackedprops[i];

			if (trackedprop->prop) {
				gdl = sightDrawTargetBox(gdl, trackedprop, 0, g_Vars.currentplayer->targetset[i]);
			}
		}

		if (sighton) {
			if (g_Vars.currentplayer->lookingatprop.prop == NULL) {
				colour = SIGHT_COLOUR;
				radius = 8;
				cornergap = 5;
			} else {
				colour = sightIsPropFriendly(NULL) ? 0x0000ff60 : 0xff000060;
				radius = 6;
				cornergap = 3;
			}

			gdl = sightDrawAimer(gdl, x, y, radius, cornergap, colour);
		}
		break;
	case SIGHTTRACKTYPE_FOLLOWLOCKON:
	case SIGHTTRACKTYPE_THREATDETECTOR:
		for (i = 0; i < ARRAYCOUNT(g_Vars.currentplayer->trackedprops); i++) {
			trackedprop = &g_Vars.currentplayer->trackedprops[i];

			if (trackedprop->prop) {
				if (g_Vars.currentplayer->sighttracktype == SIGHTTRACKTYPE_THREATDETECTOR) {
					struct defaultobj *obj = trackedprop->prop->obj;
					struct weaponobj *weapon;
					uint32_t textid = 0;

					// @dangerous: There is no check here to see if the prop
					// type is obj. However, it's likely that only objs can be
					// in the cmdfollowprops list at this point, so it's
					// probably OK.
					if (obj && obj->type == OBJTYPE_AUTOGUN
							&& (obj->flags2 & (OBJFLAG2_AICANNOTUSE | OBJFLAG2_AUTOGUN_MALFUNCTIONING1)) == 0) {
						textid = L_GUN_215; // "AUTOGUN"
					}

					weapon = trackedprop->prop->weapon;

					if (weapon && weapon->base.type == OBJTYPE_WEAPON) {
						switch (weapon->weaponnum) {
						case WEAPON_GRENADE:
							// "PROXY" and "TIMED"
							textid = (weapon->gunfunc == FUNC_SECONDARY) ? L_GUN_212 : L_GUN_213;
							break;
						case WEAPON_NBOMB:
							// "PROXY" and "IMPACT"
							textid = (weapon->gunfunc == FUNC_SECONDARY) ? L_GUN_212 : L_GUN_216;
							break;
						case WEAPON_TIMEDMINE:
							textid = L_GUN_213; // "TIMED"
							break;
						case WEAPON_PROXIMITYMINE:
							textid = L_GUN_212; // "PROXY"
							break;
						case WEAPON_REMOTEMINE:
							textid = L_GUN_214; // "REMOTE"
							break;
						case WEAPON_DRAGON:
							if (weapon->gunfunc == FUNC_SECONDARY) {
								textid = L_GUN_212; // "PROXY"
							}
							break;
						}
					}

					gdl = sightDrawTargetBox(gdl, trackedprop, textid, g_Vars.currentplayer->targetset[i]);
				} else {
					// CMP150-tracked prop
					gdl = sightDrawTargetBox(gdl, trackedprop, i + 2, g_Vars.currentplayer->targetset[i]);
				}
			}
		}

		if (sighton) {
			if (g_Vars.currentplayer->lookingatprop.prop == NULL) {
				colour = SIGHT_COLOUR;
				radius = 8;
				cornergap = 5;
			} else {
				colour = sightIsPropFriendly(NULL) ? 0x0000ff60 : 0xff000060;
				radius = 6;
				cornergap = 3;
			}

			gdl = sightDrawAimer(gdl, x, y, radius, cornergap, colour);
		}
		break;
	}

	gdl = text0f153780(gdl);

	return gdl;
}

Gfx *sightDrawClassic(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	struct textureconfig *tconfig = &g_TexGeCrosshairConfigs[0];
	float spc4[2];
	float spbc[2];
	int x = crossx;
	int y = crossy + 1; // Plus one, to align with the laser sight.
	int x1;
	int x2;
	int y1;
	int y2;

	const int halfw = roundf((float)(tconfig->width >> 1) * (SCREEN_ASPECT / videoGetAspect()));

	if (!sighton) {
		return gdl;
	}

	gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
	gfx_Set_Texture_Filter(gdl++, G_TF_POINT);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
	RGBA color = {0, 0, 0, 0};
	gfx_Set_Prim_Color(gdl++, color);

	x1 = x - halfw;
	y1 = y - (tconfig->height >> 1);
	x2 = x + halfw;
	y2 = y + (tconfig->height >> 1);

	gfx_Fill_Rectangle(gdl++, x1, y1, x2, y2);

	spc4[0] = x;
	spc4[1] = y;

	spbc[0] = halfw;
	spbc[1] = tconfig->height >> 1;

	texSelect(&gdl, tconfig, 2, 0, 0, 1, NULL);

	RGBA envColor = {255, 255, 255, 127};
	gfx_Set_Env_Color(gdl++, envColor);

	gDPSetCombineMode(gdl++, G_CC_CUSTOM_00, G_CC_CUSTOM_00);

	utilsRenderScreenTexture(&gdl, spc4, spbc, tconfig->width, tconfig->height, 0, 0, 0, false);
	
	gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
	gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
	gfx_Set_Texture_LOD(gdl++, G_TL_LOD);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Texture_Convert(gdl++, G_TC_FILT);
	gfx_Set_Texture_LUT(gdl++, G_TT_NONE);

	return gdl;
}

Gfx *sightDrawType2(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	return sightDrawClassic(gdl, sighton, crossx, crossy);
}

#define COLOUR_LIGHTRED 0xff555564
#define COLOUR_DARKRED  0xff0000b2
#define COLOUR_GREEN    0x55ff5564
#define COLOUR_DARKBLUE 0x0000ff60

#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

Gfx *sightDrawSkedarTriangle(Gfx *gdl, int x, int y, int dir, uint32_t colour)
{
	int points[6];
	Vtx *vertices = gfxAllocateVertices(3);
	Col *colours = gfxAllocateColours(2);

	switch (dir) {
	case DIR_UP:
		points[0] = x;
		points[1] = y;
		points[2] = x + 5;
		points[3] = y + 7;
		points[4] = x - 5;
		points[5] = y + 7;
		break;
	case DIR_DOWN:
		points[0] = x;
		points[1] = y;
		points[2] = x + 5;
		points[3] = y - 7;
		points[4] = x - 5;
		points[5] = y - 7;
		break;
	case DIR_LEFT:
		points[0] = x;
		points[1] = y;
		points[2] = x + 7;
		points[3] = y - 5;
		points[4] = x + 7;
		points[5] = y + 5;
		break;
	case DIR_RIGHT:
		points[0] = x;
		points[1] = y;
		points[2] = x - 7;
		points[3] = y - 5;
		points[4] = x - 7;
		points[5] = y + 5;
		break;
	default:
		return gdl;
	}

	vertices[0].x = points[0] * 10;
	vertices[0].y = points[1] * 10;
	vertices[0].z = -10;
	vertices[1].x = points[2] * 10;
	vertices[1].y = points[3] * 10;
	vertices[1].z = -10;
	vertices[2].x = points[4] * 10;
	vertices[2].y = points[5] * 10;
	vertices[2].z = -10;

#ifndef PLATFORM_N64
	// Center-align Skedar tris
	for (int i = 0; i < 3; ++i) {
		vertices[i].x -= 2;
		vertices[i].y += 2;
	}
#endif

	// @bug: This also needs to check for COLOUR_LIGHTRED because the caller can
	// use two shades of red. The second colour is used when zeroing the sight
	// in on a new target. Because of this bug, targeting an ally with the
	// Mauler or Reaper will show a red crosshair while it's still zeroing.
	if (colour == COLOUR_DARKRED && sightIsPropFriendly(NULL)) {
		colour = COLOUR_DARKBLUE;
	}

#define RGBA(r, g, b, a) (((r) & 0xff) << 24 | ((g) & 0xff) << 16 | ((b) & 0xff) << 8 | ((a) & 0xff))

	colours[0].word = PD_BE32(colour);
	colours[1].word = PD_BE32(RGBA((colour >> 24) & 0xff, (colour >> 16) & 0xff, (colour >> 8) & 0xff, 0x08));

	vertices[0].colour = 0;
	vertices[1].colour = 4;
	vertices[2].colour = 4;

	gfx_Color(gdl++, colours, 2);
	gSPVertex(gdl++, vertices, 3, 0);
	gfx_Tri1(gdl++, 0, 1, 2);

	return gdl;
}

Gfx *sightDrawSkedar(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();
	int viewright = viewleft + viewwidth - 1;
	int viewbottom = viewtop + viewheight - 1;
	int paddingy = viewheight / 4;
	int paddingx = viewwidth / 4;
	int x = (int) (crossx);
	int trix1;
	int trix2;
	int y = crossy;
	int triy2;
	int triy1;
	uint32_t colour;
	uint8_t dir;
	bool hasprop = g_Vars.currentplayer->lookingatprop.prop != NULL;
	float frac;

	if (!sighton) {
		return gdl;
	}

	if (!hasprop) {
		g_Vars.currentplayer->sighttimer240 = 0;
	}

#ifndef PLATFORM_N64
	x = sightGetAdjustedX(x);
	gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
#endif

	gdl = savebufferSetCustomProjection(gdl);

	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Geometry_Mode(gdl++, G_SHADE | G_SHADING_SMOOTH);
	gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	trix1 = x;
	triy1 = y;
	trix2 = x;
	triy2 = y;

	if (hasprop && g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
		frac = g_Vars.currentplayer->sighttimer240 / TICKS(48.0f);
	}

	// Outer top triangle
	if (!hasprop) {
		colour = COLOUR_LIGHTRED;

		if (x < viewleft + paddingx) {
			// Aiming far left
			dir = DIR_LEFT;
			trix1 = viewleft + paddingx;
		} else if (x > viewright - paddingx) {
			// Aiming far right
			dir = DIR_RIGHT;
			trix1 = viewright - paddingx;
		} else {
			// Aiming within the bounds
			dir = DIR_DOWN;
			colour = COLOUR_GREEN;
		}

		triy1 = viewtop + paddingy;
	} else {
		if (g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
			// Zeroing on a prop
			colour = COLOUR_LIGHTRED;
			dir = DIR_DOWN;
			triy1 = (y - viewtop - paddingy - 2) * frac + viewtop + paddingy;
		} else {
			// Zeroed on a prop
			colour = COLOUR_DARKRED;
			dir = DIR_DOWN;
			triy1 = y - 2;
		}
	}

	gdl = sightDrawSkedarTriangle(gdl, trix1, triy1, dir, colour);

	// Outer bottom triangle
	if (!hasprop) {
		colour = COLOUR_LIGHTRED;

		if (dir == DIR_DOWN) {
			colour = COLOUR_GREEN;
			dir = DIR_UP;
		}

		triy1 = viewbottom - paddingy;
	} else {
		if (g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
			// Zeroing on a prop
			colour = COLOUR_LIGHTRED;
			dir = DIR_UP;
			triy1 = (y - viewbottom + paddingy + 2) * frac + viewbottom - paddingy;
		} else {
			// Zeroed on a prop
			colour = COLOUR_DARKRED;
			dir = DIR_UP;
			triy1 = y + 2;
		}
	}

	gdl = sightDrawSkedarTriangle(gdl, trix1, triy1, dir, colour);

	// Outer right triangle
	if (!hasprop) {
		colour = COLOUR_LIGHTRED;

		if (y < viewtop + paddingy) {
			// Aiming far up
			dir = DIR_UP;
			triy2 = viewtop + paddingy;
		} else if (y > viewbottom - paddingy) {
			// Aiming far down
			dir = DIR_DOWN;
			triy2 = viewbottom - paddingy;
		} else {
			// Aiming within the bounds
			dir = DIR_LEFT;
			colour = COLOUR_GREEN;
		}

		trix2 = viewright - paddingx;
	} else {
		if (g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
			// Zeroing on a prop
			colour = COLOUR_LIGHTRED;
			dir = DIR_LEFT;
			trix2 = (x - viewright + paddingx + 2) * frac + viewright - paddingx;
		} else {
			colour = COLOUR_DARKRED;
			// Zeroed on a prop
			dir = DIR_LEFT;
			trix2 = x + 2;
		}
	}

	gdl = sightDrawSkedarTriangle(gdl, trix2, triy2, dir, colour);

	// Outer left triangle
	if (!hasprop) {
		colour = COLOUR_LIGHTRED;

		if (dir == DIR_LEFT) {
			colour = COLOUR_GREEN;
			dir = DIR_RIGHT;
		}

		trix2 = viewleft + paddingx;
	} else {
		if (g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
			// Zeroing on a prop
			colour = COLOUR_LIGHTRED;
			dir = DIR_RIGHT;
			trix2 = (x - viewleft - paddingx - 2) * frac + viewleft + paddingx;
		} else {
			// Zeroed on a prop
			colour = COLOUR_DARKRED;
			dir = DIR_RIGHT;
			trix2 = x - 2;
		}
	}

	gdl = sightDrawSkedarTriangle(gdl, trix2, triy2, dir, colour);

	// Inner triangles
	if (!hasprop || g_Vars.currentplayer->sighttimer240 < TICKS(48)) {
		colour = hasprop ? COLOUR_LIGHTRED : COLOUR_GREEN;

		gdl = sightDrawSkedarTriangle(gdl, x + 0, y - 2, DIR_DOWN, colour);
		gdl = sightDrawSkedarTriangle(gdl, x + 0, y + 2, DIR_UP, colour);
		gdl = sightDrawSkedarTriangle(gdl, x - 2, y + 0, DIR_RIGHT, colour);
		gdl = sightDrawSkedarTriangle(gdl, x + 2, y + 0, DIR_LEFT, colour);
	}

	gdl = savebufferSetup2DRender(gdl);

#ifndef PLATFORM_N64
	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
#endif

	return gdl;
}

Gfx *sightDrawZoom(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewhalfwidth = (viGetViewWidth()) >> 1;
	int viewhalfheight = viGetViewHeight() >> 1;
	int viewright = viewleft + viewhalfwidth * 2 - 1;
	int viewbottom = viewtop + viewhalfheight * 2 - 1;
	float maxfovy;
	int availableabove;
	int availablebelow;
	int availableleft;
	int availableright;
	float zoominfovy;
	float frac;
	float marginright;
	float margintop;
	float marginbottom;
	float marginleft;
	int cornerwidth;
	int cornerheight;
	int weaponnum;
	uint8_t showzoomrange;

	// The 48, 49 and 10 numbers are padding values. When zoomed in, the left
	// corner will be 48px from the viewport's left edge. The available values
	// are the zoomable range from the padding to the middle of the viewport.
	availableleft = viewhalfwidth - 48;
	availableright = viewhalfwidth - 49;
	availableabove = viewhalfheight - 10;
	availablebelow = viewhalfheight - 10;
	frac = 1.0f;
	weaponnum = g_Vars.currentplayer->hands[HAND_RIGHT].gset.weaponnum;
	cornerwidth = (viewhalfwidth >> 1) - 60;
	cornerheight = (viewhalfheight >> 1) - 22;

	showzoomrange = optionsGetShowZoomRange(g_Vars.currentplayerstats->mpindex)
		&& optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex);

	maxfovy = currentPlayerGetGunZoomFov();
	zoominfovy = g_Vars.currentplayer->zoominfovy;

	if (maxfovy == 0.0f || maxfovy == 60.0f) {
		if (weaponnum != WEAPON_SNIPERRIFLE) {
			showzoomrange = false;
		}
	} else {
		frac = maxfovy / zoominfovy;
	}

	if (showzoomrange) {
		gdl = textConfigureGfxPipeline(gdl);
		gdl = textSetPrimColour(gdl, SIGHT_COLOUR);

		if (frac < 0.2f) {
			cornerwidth *= 0.2f;
			cornerheight *= 0.2f;
		} else {
			cornerwidth *= frac;
			cornerheight *= frac;
		}

		if (PLAYERCOUNT() >= 2) {
			cornerheight *= 2;
		}

		if (cornerwidth < 5) {
			cornerwidth = 5;
		}

		if (cornerheight < 5) {
			cornerheight = 5;
		}

		// Margin is the gap from the viewport edge to the zoom box
		marginleft = viewhalfwidth - availableleft * frac;
		marginright = viewhalfwidth - availableright * frac;
		marginbottom = viewhalfheight - availablebelow * frac;
		margintop = viewhalfheight - availableabove * frac;

		// Center-align the zoom range
		if (frac != 1.0f) {
			viewleft += 1;
			viewright += 1;
			viewbottom += 1;
			viewtop += 1;
		}

#define BOXLEFT   (viewleft + marginleft)
#define BOXRIGHT  (viewright - marginright)
#define BOXBOTTOM (viewbottom - marginbottom)
#define BOXTOP    (viewtop + margintop)

		if (cornerwidth > BOXRIGHT - BOXLEFT) {
			cornerwidth = BOXRIGHT - BOXLEFT;
		}

		if (cornerheight > BOXBOTTOM - BOXTOP) {
			cornerheight = BOXBOTTOM - BOXTOP;
		}

		gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
		gfx_Set_Subpixel_Offset_EXT(gdl++, -2, -2);

		// Top left
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT + 1, BOXTOP, BOXLEFT + cornerwidth - 1, BOXTOP);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXTOP, BOXLEFT, BOXTOP + cornerheight - 1);

		// Top right
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT - cornerwidth + 2, BOXTOP, BOXRIGHT - 1, BOXTOP);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT, BOXTOP, BOXRIGHT, BOXTOP + cornerheight - 1);

		// Bottom left
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT + 1, BOXBOTTOM, BOXLEFT + cornerwidth - 1, BOXBOTTOM);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXBOTTOM - cornerheight + 1, BOXLEFT, BOXBOTTOM);

		// Bottom right
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT - cornerwidth + 2, BOXBOTTOM, BOXRIGHT - 1, BOXBOTTOM);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT, BOXBOTTOM - cornerheight + 1, BOXRIGHT, BOXBOTTOM);

		// Draw over the corners again, but only half as wide/high
		cornerwidth >>= 1;
		cornerheight >>= 1;

		// Top left
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXTOP, BOXLEFT + cornerwidth, BOXTOP);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXTOP, BOXLEFT, BOXTOP + cornerheight);

		// Top right
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT - cornerwidth, BOXTOP, BOXRIGHT, BOXTOP);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT, BOXTOP, BOXRIGHT, BOXTOP + cornerheight);

		// Bottom left
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXBOTTOM, BOXLEFT + cornerwidth, BOXBOTTOM);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXLEFT, BOXBOTTOM - cornerheight, BOXLEFT, BOXBOTTOM);

		// Bottom right
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT - cornerwidth, BOXBOTTOM, BOXRIGHT, BOXBOTTOM);
		gdl += gfx_HUD_Rectangle_EXT(gdl, BOXRIGHT, BOXBOTTOM - cornerheight, BOXRIGHT, BOXBOTTOM);


		gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
		gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);

		gdl = textSetCCCustom02(gdl);
		gdl = text0f153780(gdl);
	}

	gdl = sightDrawDefault(gdl, sighton, crossx, crossy);

	return gdl;
}

Gfx *sightDrawMaian(Gfx *gdl, bool sighton, float crossx, float crossy)
{
	int viewleft = viGetViewLeft();
	int viewtop = viGetViewTop();
	int viewwidth = viGetViewWidth();
	int viewheight = viGetViewHeight();
	int viewright = viewleft + viewwidth - 1;
	int viewbottom = viewtop + viewheight - 1;
	int x = (int)crossx;
	int y = crossy;
	Vtx *vertices;
	Col *colours;
	int inner[4];
	bool hasprop = g_Vars.currentplayer->lookingatprop.prop != NULL;
	uint32_t colour = 0xff000060;

	if (!sighton) {
		return gdl;
	}

	if (sightIsPropFriendly(NULL)) {
		colour = 0x0000ff60;
	}

	x = sightGetAdjustedX(x);
	gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, -2, -2);

	vertices = gfxAllocateVertices(8);
	colours = gfxAllocateColours(2);
	gdl = savebufferSetCustomProjection(gdl);

	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Geometry_Mode(gdl++, G_SHADE | G_SHADING_SMOOTH);
	gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	vertices[0].x = (viewleft + (viewwidth >> 1)) * 10;
	vertices[0].y = (viewtop + 10) * 10;
	vertices[0].z = -10;
	vertices[1].x = (viewleft + (viewwidth >> 1)) * 10;
	vertices[1].y = (viewbottom - 10) * 10;
	vertices[1].z = -10;
	vertices[2].x = (viewleft + 48) * 10;
	vertices[2].y = (viewtop + (viewheight >> 1)) * 10;
	vertices[2].z = -10;
	vertices[3].x = (viewright - 49) * 10;
	vertices[3].y = (viewtop + (viewheight >> 1)) * 10;
	vertices[3].z = -10;

	inner[0] = x + 4;
	inner[1] = x - 4;
	inner[2] = y + 4;
	inner[3] = y - 4;

	vertices[4].x = inner[1] * 10;
	vertices[4].y = inner[3] * 10;
	vertices[4].z = -10;
	vertices[5].x = inner[0] * 10;
	vertices[5].y = inner[3] * 10;
	vertices[5].z = -10;
	vertices[6].x = inner[0] * 10;
	vertices[6].y = inner[2] * 10;
	vertices[6].z = -10;
	vertices[7].x = inner[1] * 10;
	vertices[7].y = inner[2] * 10;
	vertices[7].z = -10;

	// Center-align Maian tris
	for (int i = 0; i < 8; ++i) {
		vertices[i].x -= 2;
		vertices[i].y += 2;
	}

	colours[0].word = PD_BE32(0x00ff000f);
	colours[1].word = PD_BE32(hasprop ? colour : 0x00ff0044);

	vertices[0].colour = 0;
	vertices[1].colour = 0;
	vertices[2].colour = 0;
	vertices[3].colour = 0;
	vertices[4].colour = 4;
	vertices[5].colour = 4;
	vertices[6].colour = 4;
	vertices[7].colour = 4;

	// Draw the main 4 triangles
	gfx_Color(gdl++, colours, 2);
	gSPVertex(gdl++, vertices, 8, 0);
	gfx_Tri4(gdl++, 0, 4, 5, 5, 3, 6, 7, 6, 1, 4, 7, 2);

	gdl = savebufferSetup2DRender(gdl);
	gdl = textSetPrimColour(gdl, SIGHT_COLOUR);

	// Draw border over inner points
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - 4, y - 4, x - 4, y + 4); // left
	gdl += gfx_HUD_Rectangle_EXT(gdl, x + 4, y - 4, x + 4, y + 4); // right
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - 4, y - 4, x + 4, y - 4); // top
	gdl += gfx_HUD_Rectangle_EXT(gdl, x - 4, y + 4, x + 4, y + 4); // bottom

	gdl = textSetCCCustom02(gdl);

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);


	return gdl;
}

Gfx *sightDrawTarget(Gfx *gdl, float crossx, float crossy)
{
	int x = sightGetAdjustedX((int)crossx);
	int y = crossy;

	gdl = textSetPrimColour(gdl, SIGHT_COLOUR);

	gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, -2, -2);
	if (SIGHT_SCALE == 0) {
		// Draw single rectangle to preserve intended opacity
		gdl += gfx_HUD_Rectangle_EXT(gdl++, x, y, x, y);
	} else
	{
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 1 * SIGHT_SCALE, y + 0 * SIGHT_SCALE, x + 3 * SIGHT_SCALE, y + 0 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 1 * SIGHT_SCALE, y + 0 * SIGHT_SCALE, x + 2 * SIGHT_SCALE, y + 0 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x - 3 * SIGHT_SCALE, y + 0 * SIGHT_SCALE, x - 1 * SIGHT_SCALE, y + 0 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x - 2 * SIGHT_SCALE, y + 0 * SIGHT_SCALE, x - 1 * SIGHT_SCALE, y + 0 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 0 * SIGHT_SCALE, y + 1 * SIGHT_SCALE, x + 0 * SIGHT_SCALE, y + 3 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 0 * SIGHT_SCALE, y + 1 * SIGHT_SCALE, x + 0 * SIGHT_SCALE, y + 2 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 0 * SIGHT_SCALE, y - 3 * SIGHT_SCALE, x + 0 * SIGHT_SCALE, y - 1 * SIGHT_SCALE);
		gdl += gfx_HUD_Rectangle_EXT(gdl, x + 0 * SIGHT_SCALE, y - 2 * SIGHT_SCALE, x + 0 * SIGHT_SCALE, y - 1 * SIGHT_SCALE);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);

	gdl = textSetCCCustom02(gdl);

	return gdl;
}

bool sightHasTargetWhileAiming(int sight)
{
	if (sight == SIGHT_DEFAULT || sight == SIGHT_ZOOM) {
		return true;
	}

	return false;
}

/**
 * sighton is true if the player is using the aimer (ie. holding R).
 */
Gfx *sightDraw(Gfx *gdl, bool sighton, int sight)
{
	if (sight);

	if (g_Vars.currentplayer->activemenumode != AMMODE_CLOSED) {
		return gdl;
	}

	if (g_Vars.currentplayer->gunctrl.passivemode) {
		return gdl;
	}

	// Rounding the crosshair positions allow them to more accurately follow the
	// gun's vector. Without this, the mantissa isn't factored in at all (cast
	// to integer), which leads to some awkward behavior, such as the crosshair
	// taking a long time to return to the center of the screen when coming from
	// an up and/or left direction.
	const float crossx = roundf(g_Vars.currentplayer->crosspos[0]);
	const float crossy = roundf(g_Vars.currentplayer->crosspos[1]);

	if (PLAYERCOUNT() >= 2 && g_Vars.coopplayernum < 0 && g_Vars.antiplayernum < 0) {
		sight = SIGHT_DEFAULT;
	}

	if (g_Vars.currentplayer->bondhealth <= 0.0f) {
		// Hide crosshair during death animation
		sight = SIGHT_NONE;
	}

	sightTick(sighton);

	switch (sight) {
	case SIGHT_DEFAULT:
		gdl = sightDrawDefault(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_CLASSIC:
		gdl = sightDrawClassic(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_2:
		gdl = sightDrawType2(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_3:
		gdl = sightDrawDefault(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_SKEDAR:
		gdl = sightDrawSkedar(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_ZOOM:
		gdl = sightDrawZoom(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_MAIAN:
		gdl = sightDrawMaian(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	default:
		gdl = sightDrawDefault(gdl, sighton && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex), crossx, crossy);
		break;
	case SIGHT_NONE:
		break;
	}

	if (sight != SIGHT_NONE && optionsGetSightOnScreen(g_Vars.currentplayerstats->mpindex)) {
		if ((optionsGetAlwaysShowTarget(g_Vars.currentplayerstats->mpindex) && !sighton)
				|| (sighton && sightHasTargetWhileAiming(sight))) {
			gdl = sightDrawTarget(gdl, crossx, crossy);
		}
	}

	return gdl;
}
