#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/utils.h"
#include "game/tex.h"
#include "game/textutils.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "data.h"
#include "gfx.h"
#include "types.h"

uint32_t g_RadarX;
uint32_t g_RadarY;

bool g_RadarYIndicatorsEnabled = true;

uint32_t g_TeamColours[] = {
	0xff000000, // Red
	0xffff0000, // Yellow
	0x0000ff00, // Blue
	0xff00ff00, // Magenta
	0x00ffff00, // Cyan
	0xff885500, // Orange
	0x8800ff00, // Pink
	0x88445500, // Brown
};

uint32_t var80087ce4[] = {
	0xf801f801,
	0xffc1ffc1,
	0x003f003f,
	0xf83ff83f,
	0x07ff07ff,
	0xfc55fc55,
	0xfc63fc63,
	0x8a158a15,
};

void radarSetYIndicatorsEnabled(bool enable)
{
	g_RadarYIndicatorsEnabled = enable;
}

Gfx *radarRenderBackground(Gfx *gdl, struct textureconfig *tconfig, int screenX, int screenY, int textureScale)
{
    float screenPos[2];
    float scaleFactors[2];

    // Setup simple (non-perspective) rendering state
    gfx_Set_Texture_Persp(gdl++, G_TP_NONE);
    gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
    gfx_Set_Texture_LOD(gdl++, G_TL_TILE);
    gfx_Set_Texture_Filter(gdl++, G_TF_POINT);
    gfx_Set_Texture_LUT(gdl++, G_TT_NONE);
    gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
    gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	RGBA primColor = {0, 0, 0, 0};
    gfx_Set_Prim_Color(gdl++, primColor);

    // Fill background rectangle
    gfx_Fill_Rectangle(gdl++, screenX, screenY, (screenX + tconfig->width), (screenY + tconfig->width));

    // Set texture rendering parameters
    screenPos[0] = screenX;
    screenPos[1] = screenY;
    scaleFactors[0] = textureScale;
    scaleFactors[1] = textureScale;

    texSelect(&gdl, tconfig, 2, 0, 0, 1, NULL);

	RGBA radarColor = {0, 255, 0, 40};
    gfx_Set_Env_Color(gdl++, radarColor);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_ENVIRONMENT, G_CCMUX_0,    // Color 0
		G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_ENVIRONMENT, G_ACMUX_0,    // Alpha 0
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0,                   // Color 1 (unused)
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0);                  // Alpha 1 (unused)

    // Render the radar background texture
    utilsRenderScreenTexture(&gdl, screenPos, scaleFactors, tconfig->width, tconfig->height, false, false, false);

    // Restore standard rendering state
    gfx_Set_Texture_Persp(gdl++, G_TP_PERSP);
    gfx_Set_Alpha_Compare(gdl++, G_AC_NONE);
    gfx_Set_Texture_LOD(gdl++, G_TL_LOD);
    gfx_Set_Texture_LUT(gdl++, G_TT_NONE);

    return gdl;
}

int radarGetTeamIndex(int team)
{
	int index = 0;

	if (team & 1) {
		index = 0;
	} else if (team & 0x02) {
		index = 1;
	} else if (team & 0x04) {
		index = 2;
	} else if (team & 0x08) {
		index = 3;
	} else if (team & 0x10) {
		index = 4;
	} else if (team & 0x20) {
		index = 5;
	} else if (team & 0x40) {
		index = 6;
	} else if (team & 0x80) {
		index = 7;
	}

	return index;
}

Gfx *radarDrawDot(Gfx *gdl, struct prop *prop, struct coord *dist, uint32_t colour1, uint32_t colour2, bool swapcolours)
{
	int x;
	int y;
	uint32_t shiftamount;
	float sqdist;
	float spcc;

	spcc = (atan2f(dist->x, dist->z) * 180.0f) / M_PI + g_Vars.currentplayer->vv_theta + 180.0f;
	sqdist = sqrtf(dist->z * dist->z + dist->x * dist->x) * (1.0f / 250.0f);

	if (sqdist < 16.0f) {
		shiftamount = 0;
	} else {
		sqdist = 16.0f;
		shiftamount = 1;
	}

	x = g_RadarX + (int)(sinf(spcc * 0.017453292384744f) * sqdist);
	y = g_RadarY + (int)(cosf(spcc * 0.017453292384744f) * sqdist);

	if (swapcolours) {
		if (prop == g_Vars.currentplayer->prop) {
			// Box
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 2, x + 1, y + 3);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 1, x + 2, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 1, y + 1, x + 0, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 0, x + 1, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y > 250) {
			// Up triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 1, x + 2, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 0, x + 1, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y < -250) {
			// Down triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 2, x + 2, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 1, x + 1, y + 2);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 1, x + 1, y + 0);
			gfx_Fill_Rectangle(gdl++, x - 1, y + 0, x + 0, y + 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else {
			// Dot
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 2, y + 2);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 1, y + 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		}
	} else {
		if (prop == g_Vars.currentplayer->prop) {
			// Box
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 2, x + 1, y + 3);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 1, x + 2, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 1, y + 1, x + 0, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 0, x + 1, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y > 250) {
			// Up triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 1, x + 2, y + 2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 1, y - 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 0, x + 1, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 0, y + 0);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else if (g_RadarYIndicatorsEnabled && dist->y < -250) {
			// Down triangle
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 3, y - 2, x + 2, y + 1);
			gfx_Fill_Rectangle(gdl++, x - 2, y + 1, x + 1, y + 2);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 1, x + 1, y + 0);
			gfx_Fill_Rectangle(gdl++, x - 1, y + 0, x + 0, y + 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		} else {
			// Dot
			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour2);
			gfx_Fill_Rectangle(gdl++, x - 2, y - 2, x + 2, y + 2);
			gdl = textSetCCPrimColorTexAlpha(gdl);

			gdl = textSetPrimColour(gdl, (0xff >> shiftamount) + colour1);
			gfx_Fill_Rectangle(gdl++, x - 1, y - 1, x + 1, y + 1);
			gdl = textSetCCPrimColorTexAlpha(gdl);
		}
	}

	return gdl;
}

Gfx *radarRender(Gfx *gdl)
{
	int playercount;
	int playernum;
	struct textureconfig *tconfig;
	struct coord pos;
	uint32_t colour;
	int i;

	tconfig = g_TexRadarConfigs;
	playernum = g_Vars.currentplayernum;
	playercount = PLAYERCOUNT();

	if (g_Vars.mplayerisrunning) {
		if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_NORADAR)) {
			return gdl;
		}

		if ((g_PlayerConfigsArray[g_Vars.currentplayerstats->mpindex].base.displayoptions & 0x00000004) == 0) {
			return gdl;
		}
	} else if ((g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_RTRACKER) == 0) {
		if (!g_MissionConfig.iscoop || !g_Vars.coopradaron) {
			return gdl;
		}
	}

	if (g_Vars.currentplayer->mpmenuon || g_Vars.currentplayer->isdead) {
		return gdl;
	}

	g_RadarX = (viGetViewLeft() + viGetViewWidth()) - 41;

	if (playercount == 2) {
		if (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
			if (playernum == 0) {
				g_RadarX += 16;
			}
		} else {
			g_RadarX -= 7;
		}
	} else if (playercount >= 3) {
		if ((playernum & 1) == 0) {
			g_RadarX += 7;
		} else {
			g_RadarX -= 7;
		}
	}

	g_RadarY = viGetViewTop() + 26;

	if (playercount == 2) {
		if (optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && playernum == 1) {
			g_RadarY -= 8;
		}
		if (optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL) {
			gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeR);
		}
	} else if (playercount >= 3) {
		if (playernum >= 2) {
			g_RadarY -= 8;
		} else {
			g_RadarY -= 2;
		}
	} else {
		gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeR);
		gfx_Set_Subpixel_Offset_EXT(gdl++, -2, 2);
	}

	gdl = radarRenderBackground(gdl, tconfig, g_RadarX, g_RadarY, 0x10);

	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE,
		G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_PRIMITIVE,
		G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_PRIMITIVE);
	gfx_Set_Render_Mode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	// Draw dots for human players
	gfx_Set_Subpixel_Offset_EXT(gdl++, 0, 0);
	if (!(g_MpSetup.options & MPOPTION_NOPLAYERONRADAR)) {
	for (i = 0; i < playercount; i++) {
		if (i != playernum) {
			if (g_Vars.players[i]->isdead == false
					&& (g_Vars.players[i]->prop->chr->hidden & CHRHFLAG_CLOAKED) == 0
					&& scenarioRadarChr(&gdl, g_Vars.players[i]->prop) == false) {
				pos.x = g_Vars.players[i]->prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = g_Vars.players[i]->prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = g_Vars.players[i]->prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
					int index = g_PlayerConfigsArray[g_Vars.playerstats[i].mpindex].base.team;
					colour = g_TeamColours[index];
				} else {
					colour = 0x00ff0000;
				}

				gdl = radarDrawDot(gdl, g_Vars.players[i]->prop, &pos, colour, 0, 0);
			}
		}
	}
	}

	// Draw dots for coop AI buddies
	if (!g_Vars.normmplayerisrunning && g_MissionConfig.iscoop) {
		for (i = 0; i < g_Vars.numaibuddies && i < ARRAYCOUNT(g_Vars.aibuddies); i++) {
			struct prop *prop = g_Vars.aibuddies[i];

			if (prop
					&& prop->type == PROPTYPE_CHR
					&& prop->chr
					&& prop->chr->actiontype != ACT_DIE
					&& prop->chr->actiontype != ACT_DEAD) {
				pos.x = prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				gdl = radarDrawDot(gdl, prop, &pos, 0x00ff0000, 0, 0);
			}
		}
	}

	// Draw dots for MP simulants
	if (g_Vars.normmplayerisrunning && !(g_MpSetup.options & MPOPTION_NOPLAYERONRADAR)) {
		for (i = 0; i < g_BotCount; i++) {
			if (!chrIsDead(g_MpBotChrPtrs[i])
					&& (g_MpBotChrPtrs[i]->hidden & CHRHFLAG_CLOAKED) == 0
					&& scenarioRadarChr(&gdl, g_MpBotChrPtrs[i]->prop) == false) {
				pos.x = g_MpBotChrPtrs[i]->prop->pos.x - g_Vars.currentplayer->prop->pos.x;
				pos.y = g_MpBotChrPtrs[i]->prop->pos.y - g_Vars.currentplayer->prop->pos.y;
				pos.z = g_MpBotChrPtrs[i]->prop->pos.z - g_Vars.currentplayer->prop->pos.z;

				if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
					colour = g_TeamColours[radarGetTeamIndex(g_MpBotChrPtrs[i]->team)];
				} else {
					colour = 0x00ff0000;
				}

				gdl = radarDrawDot(gdl, g_MpBotChrPtrs[i]->prop, &pos, colour, 0, 0);
			}
		}
	}

	gdl = scenarioRadarExtra(gdl);

	// Draw dots for r-tracked props
	if (g_Vars.currentplayer->devicesactive & ~g_Vars.currentplayer->devicesinhibit & DEVICE_RTRACKER) {
		gdl = radarRenderRTrackedProps(gdl);
	}

	// Draw dot for the current player
	if (scenarioRadarChr(&gdl, g_Vars.currentplayer->prop) == false) {
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		if (g_Vars.normmplayerisrunning && (g_MpSetup.options & MPOPTION_TEAMSENABLED)) {
			int index = g_PlayerConfigsArray[g_Vars.playerstats[playernum].mpindex].base.team;
			colour = g_TeamColours[index];
		} else {
			colour = 0x00ff0000;
		}

		gdl = radarDrawDot(gdl, g_Vars.currentplayer->prop, &pos, colour, 0, 0);
	}

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

	return gdl;
}

Gfx *radarRenderRTrackedProps(Gfx *gdl)
{
	struct prop *prop = g_Vars.activeprops;
	struct coord *playerpos = &g_Vars.currentplayer->prop->pos;
	struct defaultobj *obj;
	struct chrdata *chr;
	struct coord dist1;
	struct coord dist2;

	while (prop) {
		switch (prop->type) {
		case PROPTYPE_OBJ:
		case PROPTYPE_DOOR:
		case PROPTYPE_WEAPON:
			obj = prop->obj;

			if ((obj->flags3 & OBJFLAG3_RTRACKED_YELLOW) ||
					(cheatIsActive(CHEAT_RTRACKER) && (obj->flags3 & OBJFLAG3_RTRACKED_BLUE))) {
				dist1.x = prop->pos.x - playerpos->x;
				dist1.y = prop->pos.y - playerpos->y;
				dist1.z = prop->pos.z - playerpos->z;

				gdl = radarDrawDot(gdl, prop, &dist1,
						(obj->flags3 & OBJFLAG3_RTRACKED_YELLOW) ? 0xffff0000 : 0x0000ff00,
						0, 0);
			}
			break;
		case PROPTYPE_CHR:
			chr = prop->chr;

			if (chr && chr->rtracked
					&& chr->actiontype != ACT_DIE
					&& chr->actiontype != ACT_DEAD
					&& (chr->hidden & CHRHFLAG_CLOAKED) == 0) {
				dist2.x = prop->pos.x - playerpos->x;
				dist2.y = prop->pos.y - playerpos->y;
				dist2.z = prop->pos.z - playerpos->z;
				gdl = radarDrawDot(gdl, prop, &dist2, 0xff000000, 0, 0);
			}
			break;
		}

		prop = prop->next;
	}

	return gdl;
}
