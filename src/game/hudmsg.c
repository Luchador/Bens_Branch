#include <ultra64.h>
#include <math.h>
#include <stdio.h>
#include "constants.h"
#include "constants.h"
#include "game/propsnd.h"
#include "game/weaponutils.h"
#include "game/player.h"
#include "game/savebuffer.h"
#include "game/hudmsg.h"
#include "game/menugfx.h"
#include "game/playermgr.h"
#include "game/textutils.h"
#include "game/lv.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "game/propobj.h"
#include "bss.h"
#include "lib/lib_317f0.h"
#include "lib/memp.h"
#include "lib/snd.h"
#include "string.h"
#include "lib/vi.h"
#include "data.h"
#include "gfx.h"
#include "types.h"
#include "string.h"

uint32_t g_NextHudMessageId;

uint8_t g_HudmsgsActive = 0;

uint32_t g_HudmsgColours[] = {
	/* 0*/ 0x00ff0000, // green
	/* 1*/ 0x9999ff00, // pastel blue
	/* 2*/ 0xffffff00, // white
	/* 3*/ 0xff777700, // pastel red
	/* 4*/ 0xffff5500, // yellow
	/* 5*/ 0x00ff0000, // green
	/* 6*/ 0xcccccc00, // gray
	/* 7*/ 0xff888800, // pastel red
	/* 8*/ 0xffaa5500, // orange
	/* 9*/ 0x55aaff00, // sky blue
	/*10*/ 0xaa55ff00, // purple
};

int g_HudPaddingY = 10;
int g_HudPaddingX = 24;
int g_NumHudMessages = 0;
struct hudmessage *g_HudMessages = NULL;

struct hudmsgtype g_HudmsgTypes[] = {
	/* 0*/ { 1, 1, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ff0000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_BOTTOM,        0, 0, 80  },
	/* 1*/ { 0, 1, 0, &g_CharsHandelGothicMd, &g_FontHandelGothicMd, 0x00ff0000, 0x000000a0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_YMIDDLE,       0, 0, 120 },
	/* 2*/ { 0, 0, 1, &g_CharsHandelGothicMd, &g_FontHandelGothicMd, 0xff000000, 0xffffffa0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_YMIDDLE,       0, 0, 120 },
	/* 3*/ { 0, 1, 0, &g_CharsHandelGothicMd, &g_FontHandelGothicMd, 0x00ff0000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_BOTTOM,        0, 0, 120 },
	/* 4*/ { 1, 1, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ffc000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_BOTTOM,        0, 0, 40  },
	/* 5*/ { 0, 0, 0, &g_CharsHandelGothicMd, &g_FontHandelGothicMd, 0x00ff0000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_TOP,           0, 0, 120 },
	/* 6*/ { 1, 0, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ff0000, 0x000000a0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_TOP,           0, 0, 120 },
	/* 7*/ { 1, 1, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ff0000, 0x000000a0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_TOP,           0, 0, -1  },
	/* 8*/ { 1, 1, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ffc000, 0x000000a0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_BOTTOM,        0, 0, 500 },
	/* 9*/ { 1, 1, 0, &g_CharsHandelGothicXs, &g_FontHandelGothicXs, 0x00ff0000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_BOTTOM,        0, 0, 120 },
	/*10*/ { 1, 1, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ff0000, 0x000000a0, HUDMSGALIGN_LEFT,    HUDMSGALIGN_BOTTOM,        0, 0, 240 },
	/*11*/ { 0, 0, 0, &g_CharsHandelGothicSm, &g_FontHandelGothicSm, 0x00ff0000, 0x000000a0, HUDMSGALIGN_XMIDDLE, HUDMSGALIGN_BELOWVIEWPORT, 0, 0, 120 },
};

uint8_t hudmsgsAreActive(void)
{
	return g_HudmsgsActive;
}

int hudmsgIsZoomRangeVisible(void)
{
	return optionsGetShowZoomRange(g_Vars.currentplayerstats->mpindex)
		&& (PLAYERCOUNT() == 1
				|| !g_Vars.mplayerisrunning
				|| g_Vars.coopplayernum >= 0
				|| g_Vars.antiplayernum >= 0)
		&& currentPlayerGetSight() == SIGHT_ZOOM
		&& g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY
		&& g_Vars.currentplayer->cameramode != CAMERAMODE_THIRDPERSON;
}

Gfx *hudmsgRenderMissionTimer(Gfx *gdl, uint32_t alpha)
{
	int x;
	int y;
	int viewleft;
	int timery;
	char buffer[24];
	uint32_t textcolour;
	int playercount;
	int playernum;
	int16_t viewtop;
	int16_t viewheight;

	textcolour = alpha;

	viewleft = viGetViewLeft();
	viewtop = viGetViewTop();
	viewheight = viGetViewHeight();
	playercount = PLAYERCOUNT();
	playernum = g_Vars.currentplayernum;

	timery = viewheight;
	timery += viewtop;
	timery -= g_HudPaddingY;
	timery -= 8;

	// @bug: There is no check for playercount >= 2 in the next two statements.
	// Because of this, in 1 player the timer is drawn out of place when the
	// screen split option is vertical and either the countdown timer is visible
	// or a zoomable weapon is in use.
	if ((optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) && countdownTimerIsVisible()) {
		timery -= 8;
	}

	if ((optionsGetScreenSplit() == SCREENSPLIT_VERTICAL || playercount >= 3) && hudmsgIsZoomRangeVisible()) {
		timery -= 8;
	}

	if (playercount == 2) {
		if ((optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && playernum == 0)) {
			timery += 10;
		} else {
			timery += 2;
		}
	} else if (playercount >= 3) {
		if (playernum < 2) {
			timery += 10;
		} else {
			timery += 2;
		}
	}

	// If this is a second player with their viewport on the right side of the
	// screen, move the timer left a bit as the safe zone doesn't need to be
	// considered.
	if (playercount == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) && playernum == 1) {
		viewleft -= 14;
	} else if (playercount >= 3 && (playernum & 1) == 1) {
		viewleft -= 14;
	}

	textcolour = textcolour * 160 / 255;
	textcolour |= 0x00ff0000;

	formatTime(buffer, playerGetMissionTime(), TIMEPRECISION_HUNDREDTHS);

	x = viewleft + g_HudPaddingX + 3;
	y = timery;

	if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
		gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeL);
	}

	gdl = textRender(gdl, &x, &y, buffer, g_CharsNumeric, g_FontNumeric, textcolour, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

	return gdl;
}

Gfx *hudmsgRenderZoomRange(Gfx *gdl, uint32_t alpha)
{
	int viewtop;
	int viewleft;
	int viewhalfwidth;
	int viewheight;
	float zoominfovy;
	float zoomfov;
	int playercount;
	float curzoom;
	float maxzoom;
	char text[24];
	int weaponnum;
	int texty;
	int x;
	int y;
	int textwidth;
	int textheight;
	int x2;
	int y2;
	uint32_t colour;

	colour = (alpha * 0xa0 / 255) | 0x00ff0000;
	viewtop = viGetViewTop();
	viewleft = viGetViewLeft();
	viewhalfwidth = (viGetViewWidth()) >> 1;
	viewheight = viGetViewHeight();
	texty = viewheight + viewtop - 1;
	maxzoom = 1.0f;
	weaponnum = g_Vars.currentplayer->hands[0].gset.weaponnum;
	playercount = PLAYERCOUNT();

	texty -= 17;

	if (countdownTimerIsVisible()) {
		texty -= 8;
	}

	if (playercount == 2) {
		if ((optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && g_Vars.currentplayernum == 0)) {
			texty += 10;
		} else {
			texty += 2;
		}
	} else if (playercount >= 3) {
		if (g_Vars.currentplayernum < 2) {
			texty += 10;
		} else {
			texty += 2;
		}
	}

	// Left side - current zoom level
	zoomfov = currentPlayerGetGunZoomFov();
	zoominfovy = g_Vars.currentplayer->zoominfovy;

	if (zoomfov == 0.0f || zoomfov == 60.0f) {
		if (weaponnum == WEAPON_SNIPERRIFLE) {
			curzoom = 1.0f;
		} else {
			return gdl;
		}
	} else {
		maxzoom = PLAYER_DEFAULT_FOV / zoomfov;
		curzoom = maxzoom - 1.0f / (zoomfov / zoominfovy) + 1;
	}

	sprintf(text, "%s%s%4.2fX", "", "", curzoom);
	textMeasure(&textheight, &textwidth, text, g_CharsNumeric, g_FontNumeric, 0);

	x = viewleft + viewhalfwidth - textwidth - 5;
	y = texty;
	x2 = x + textwidth;
	y2 = y + textheight;

	if (playercount < 2 || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL)) {
		gSPSetExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);
	}

	gdl = text0f1538e4(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, colour, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);

	// Divider
	sprintf(text, "/");
	textMeasure(&textheight, &textwidth, text, g_CharsNumeric, g_FontNumeric, 0);

	x = viewleft + viewhalfwidth - (textwidth >> 1);
	y = texty;
	x2 = x + textwidth;
	y2 = y + textheight;

	gdl = text0f1538e4(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, colour, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);

	// Right side - max zoom level
	sprintf(text, "%s%s%4.2fX", "", "", maxzoom);
	textMeasure(&textheight, &textwidth, text, g_CharsNumeric, g_FontNumeric, 0);

	x = viewleft + viewhalfwidth + 5;
	y = texty;
	x2 = x + textwidth;
	y2 = y + textheight;

	gdl = text0f1538e4(gdl, &x, &y, &x2, &y2);
	gdl = textRender(gdl, &x, &y, text, g_CharsNumeric, g_FontNumeric, colour, 0x000000a0, viGetWidth(), viGetHeight(), 0, 0);

	gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_CENTER_EXT);

	return gdl;
}

Gfx *hudmsgRenderBox(Gfx *gdl, int x1, int y1, int x2, int y2, float bgopacity, uint32_t bordercolour, float textopacity)
{
	float f0;
	float f20;
	float f22;

	g_HudmsgsActive = true;

	f0 = sinf(90 * bgopacity * M_PI / 180.0f);
	f22 = (x2 - x1) * 0.5f;
	f20 = (y2 - y1) * 0.5f;

	if (f0 < 0.5f) {
		f20 = 0.0f;
		f22 *= f0 + f0;
	} else {
		f20 *= (f0 - 0.5f) + (f0 - 0.5f);
	}

	gdl = savebufferSetCustomProjection(gdl);

	gdl = menugfxDrawFilledRect(gdl, x1, y1, x2, y1 + 1, bordercolour, bordercolour);
	gdl = menugfxDrawFilledRect(gdl, x1, y2, x2, y2 + 1, bordercolour, bordercolour);
	gdl = menugfxDrawFilledRect(gdl, x1, y1 + 1, x1 + 1, y2, bordercolour, bordercolour);
	gdl = menugfxDrawFilledRect(gdl, x2, y1, x2 + 1, y2 + 1, bordercolour, bordercolour);

	gdl = savebufferSetup2DRender(gdl);

	if (textopacity > 0.0f) {
		float width = (x1 + x2) * 0.5f;
		float height = (y1 + y2) * 0.5f;

		gdl = text0f153a34(gdl,
				(int)((width - f22) + 1.0f),
				(height - f20) + 1.0f,
				(int)(width + f22),
				height + f20,
				128.0f * textopacity);
	}

	return gdl;
}

int hudmsgCalcXMargin(int *arg0, int arg1)
{
	int viewwidth = g_Vars.currentplayer->viewwidth;
	int result = 0;

	*arg0 = 24;

	if (PLAYERCOUNT() == 2 && optionsGetScreenSplit() == SCREENSPLIT_VERTICAL) {
		result -= *arg0 * 2 / 3;

		if (g_Vars.currentplayernum == 0) {
			*arg0 /= 3;
		} else {
			*arg0 /= 6;
		}
	}

	result = result + viewwidth - *arg0 - arg1 - 11;

	if (PLAYERCOUNT() == 1) {
		result -= 16;
	}

	return result;
}

void hudmsgsHideByChannel(int channelnum)
{
	int i;

	for (i = 0; i < g_NumHudMessages; i++) {
		if (g_HudMessages[i].state != HUDMSGSTATE_FREE && g_HudMessages[i].channelnum == channelnum) {
			g_HudMessages[i].flags |= HUDMSGFLAG_FORCEOFF;
			break;
		}
	}
}

void hudmsgsReset(void)
{
	int i;

	g_NumHudMessages = g_Vars.mplayerisrunning ? 20 : 8;
	g_HudMessages = mempAlloc(ALIGN64(sizeof(struct hudmessage) * g_NumHudMessages), MEMPOOL_STAGE);

	for (i = 0; i < g_NumHudMessages; i++) {
		g_HudMessages[i].state = HUDMSGSTATE_FREE;
	}

	g_NextHudMessageId = 0;
}

void hudmsgRemoveAll(void)
{
	int i;

	for (i = 0; i < g_NumHudMessages; i++) {
		g_HudMessages[i].state = HUDMSGSTATE_FREE;
	}
}

int hudmsgGetNext(int refid)
{
	int bestid = -1;
	int bestindex = -1;
	int i;

	// Finding the smallest ID that is greater than refid
	for (i = 0; i < g_NumHudMessages; i++) {
		if (g_HudMessages[i].state && g_HudMessages[i].id > refid) {
			if (bestid < 0 || g_HudMessages[i].id < bestid) {
				bestindex = i;
				bestid = g_HudMessages[i].id;
			}
		}
	}

	return bestindex;
}

void hudmsgCreate(char *text, int type)
{
	hudmsgCreateFromArgs(text, type,
			g_HudmsgTypes[type].unk00,
			g_HudmsgTypes[type].unk01,
			g_HudmsgTypes[type].unk02,
			g_HudmsgTypes[type].unk04,
			g_HudmsgTypes[type].unk08,
			g_HudmsgTypes[type].colour,
			g_HudmsgTypes[type].unk10,
			g_HudmsgTypes[type].alignh,
			g_HudmsgTypes[type].unk16,
			g_HudmsgTypes[type].alignv,
			g_HudmsgTypes[type].unk18,
			-1, 0);
}

void hudmsgCreateWithFlags(char *text, int type, uint32_t flags)
{
	hudmsgCreateFromArgs(text, type,
			g_HudmsgTypes[type].unk00,
			g_HudmsgTypes[type].unk01,
			g_HudmsgTypes[type].unk02,
			g_HudmsgTypes[type].unk04,
			g_HudmsgTypes[type].unk08,
			g_HudmsgTypes[type].colour,
			g_HudmsgTypes[type].unk10,
			g_HudmsgTypes[type].alignh,
			g_HudmsgTypes[type].unk16,
			g_HudmsgTypes[type].alignv,
			g_HudmsgTypes[type].unk18,
			-1, flags);
}

void hudmsgCreateWithColour(char *text, int type, uint8_t colournum)
{
	g_HudmsgTypes[type].colour = g_HudmsgColours[colournum];

	hudmsgCreateFromArgs(text, type,
			g_HudmsgTypes[type].unk00,
			g_HudmsgTypes[type].unk01,
			g_HudmsgTypes[type].unk02,
			g_HudmsgTypes[type].unk04,
			g_HudmsgTypes[type].unk08,
			g_HudmsgTypes[type].colour,
			g_HudmsgTypes[type].unk10,
			g_HudmsgTypes[type].alignh,
			g_HudmsgTypes[type].unk16,
			g_HudmsgTypes[type].alignv,
			g_HudmsgTypes[type].unk18,
			-1, 0);
}

void hudmsgCreateWithDuration(char *text, int type, struct hudmsgtype *config, int duration60)
{
	hudmsgCreateFromArgs(text, type,
			config->unk00,
			config->unk01,
			config->unk02,
			config->unk04,
			config->unk08,
			config->colour,
			config->unk10,
			config->alignh,
			config->unk16,
			config->alignv,
			config->unk18,
			duration60, HUDMSGFLAG_NOCHANNEL);
}

/**
 * Create a hudmsg that is tied to the given audio channel. When the audio
 * finishes the hudmsg is removed.
 *
 * This function is used for both in-game subtitles and cutscene subtitles.
 * If a cutscene is in progress, the function forces the type to cutscene.
 * This allows the caller to specify the type as in-game unconditionally
 * and it will do the right thing.
 *
 * For cutscene subtitles, a dynamic width is used which means the source text
 * has to be re-wrapped. There is also a limit of two lines at a time.
 *
 * The source text is split into individual messages. These splits are
 * usually at the end of each sentence, but they also occur after commas and
 * semi-colons.
 *
 * Each message is wrapped and appended to an accumulator. Every time the
 * accumulator would exceed two lines, the accumulator is queued as a hudmsg
 * and then cleared prior to appending the new message.
 *
 * Each hudmsg is assigned a duration according to its character length relative
 * to the entire string and the audio duration.
 */
void hudmsgCreateAsSubtitle(char *srctext, int type, uint8_t colourindex, int audiochannelnum)
{
	int audioduration60;
	struct hudmsgtype *config;

	audioduration60 = psGetDuration60(audiochannelnum);

	if (type == HUDMSGTYPE_INGAMESUBTITLE) {
		if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
			if (!optionsGetCutsceneSubtitles()) {
				return;
			}

			type = HUDMSGTYPE_CUTSCENESUBTITLE;
		} else if (!optionsGetInGameSubtitles()) {
			return;
		}
	}

	config = &g_HudmsgTypes[type];
	config->colour = g_HudmsgColours[colourindex];

	if (g_Vars.tickmode == TICKMODE_CUTSCENE && audioduration60 >= 0) {
		char puncchars[] = { '.', ';', '!', '?', ',' };
		uint16_t srclen;
		int sp4a8;
		int wrapwidth;
		char accum[250];
		char prewrap[250];
		char postwrap[250];
		char msg[250];
		int msglen;
		bool split;
		int accumlen;
		int linecount;
		float time60perchar;
		int i;
		int j;
		bool append;
		bool foundpunctuation;

		srclen = strlen(srctext);
		wrapwidth = hudmsgCalcXMargin(&sp4a8, config->unk16);

		accumlen = 0;
		i = 0;
		time60perchar = (float)audioduration60 / srclen;

		// These two loops both work with the i iterator.
		// The inner loop increments i and is looking for places to split the
		// text, while the outer loop iterates once per split until the srctext
		// has been completely scanned.
		while (srctext[i] != '\0') {
			msglen = 0;
			foundpunctuation = false;
			split = false;

			while (srctext[i] != '\0' && (!foundpunctuation || !split || i > srclen - 10)) {
				// Check if the current char is punctuation
				for (j = 0; j < ARRAYCOUNT(puncchars); j++) {
					if (puncchars[j] == srctext[i]) {
						foundpunctuation = true;
					}
				}

				// Avoid splitting in the middle of trailing dots,
				// and also avoid splitting after "Dr." or "Mr."
				if (foundpunctuation && srctext[i] == '.') {
					if (srctext[i + 1] == '.') {
						foundpunctuation = false;
					}

					if (i >= 2) {
						if ((srctext[i - 2] == 'D' || srctext[i - 2] == 'd')
								&& (srctext[i - 1] == 'r' || srctext[i - 1] == 'R')) {
							foundpunctuation = false;
						}

						if ((srctext[i - 2] == 'M' || srctext[i - 2] == 'm')
								&& (srctext[i - 1] == 'r' || srctext[i - 1] == 'R')) {
							foundpunctuation = false;
						}
					}
				}

				// Copy the character from srctext to msg, except:
				// - if it's a space at the start of the string
				// - if it's a consecutive space
				// - if it's a line break (sometimes copy a space instead)
				if (msglen < 249) {
					bool ignore = false;

					if (srctext[i] == ' ') {
						if (msglen == 0) {
							ignore = true;
						} else if (msg[msglen - 1] == ' ') {
							ignore = true;
						}
					}

					if (srctext[i] == '\n') {
						ignore = true;

						if (msglen != 0 && msg[msglen - 1] != ' ' && srctext[i + 1] != ' ') {
							msg[msglen] = ' ';
							msglen++;
						}
					}

					if (foundpunctuation && srctext[i] == ' ') {
						split = true;
					}

					if (!ignore) {
						msg[msglen] = srctext[i];
						msglen++;
					}
				}

				if (1);

				i++;
			} // end of inner loop

			// At this point the string in msg is a single sentence,
			// free of line breaks. It still needs to be wrapped.

			// Make sure msg ends in a space
			if (msglen > 0 && msg[msglen - 1] != ' ') {
				msg[msglen] = ' ';
				msglen++;
			}

			// Rebuild prewrap by concatenating the accumulator and msg.
			// prewrap will be everything that's been read so far and has yet to
			// be queued.
			for (j = 0; j < accumlen; j++) {
				prewrap[j] = accum[j];
			}

			for (j = 0; j < msglen; j++) {
				prewrap[j + accumlen] = msg[j];
			}

			prewrap[accumlen + msglen] = '\n';
			prewrap[accumlen + msglen + 1] = '\0';

			// Apply text wrapping to prewrap
			textWrap(wrapwidth, prewrap, postwrap, g_CharsHandelGothicSm, g_FontHandelGothicSm);

			// Next, count the number of lines in the wrapped message.
			// If it's more than two, send the accumulator out as a hudmsg and
			// then put msg in the accumulator. Otherwise, just append msg to
			// the accumulator.

			// Note that these strings always end in a line break, so counting
			// the line breaks is the same as counting visual lines
			linecount = 0;

			for (j = 0; postwrap[j] != '\0'; j++) {
				if (postwrap[j] == '\n') {
					linecount++;
				}
			}

			append = true;

			if (linecount >= 3) {
				if (accumlen == 0) {
					// Nothing is in the accumulator, so just queue the message
					msg[msglen] = '\n';
					msglen++;

					msg[msglen] = '\0';

					hudmsgCreateWithDuration(msg, type, config, msglen * time60perchar);
					append = false;
				} else {
					// Queue the accumulator and then clear it.
					// The current message will be copied into the accumulator
					// for the next iteration.
					accum[accumlen] = '\n';
					accumlen++;

					accum[accumlen] = '\0';

					hudmsgCreateWithDuration(accum, type, config, accumlen * time60perchar);
					accumlen = 0;
				}
			}

			if (append) {
				for (j = 0; j < msglen; j++) {
					accum[accumlen + j] = msg[j];
				}

				accumlen += msglen;
			}

			msg[msglen] = '\0';
		} // end of outer loop

		// If there's anything remaining in the accumulator, queue it
		if (accumlen != 0) {
			accum[accumlen] = '\n';
			accumlen++;

			accum[accumlen] = '\0';

			hudmsgCreateWithDuration(accum, type, config, accumlen * time60perchar);
		}
	} else {
		hudmsgCreateFromArgs(srctext, type, config->unk00, config->unk01, config->unk02,
				config->unk04, config->unk08, config->colour, config->unk10, config->alignh,
				config->unk16, config->alignv, config->unk18, audiochannelnum, 0);
	}
}

void hudmsgCreateFromArgsWithoutFlags(char *text, int type, int conf00, int conf01, int conf02, struct fontchar **conf04, struct font **conf08, uint32_t textcolour, uint32_t shadowcolour, uint32_t alignh, int conf16, uint32_t alignv, int conf18, int arg14)
{
	hudmsgCreateFromArgs(text, type,
			conf00,
			conf01,
			conf02,
			conf04,
			conf08,
			textcolour,
			shadowcolour,
			alignh,
			conf16,
			alignv,
			conf18,
			arg14, 0);
}

void hudmsgCalculatePosition(struct hudmessage *msg)
{
	int x;
	int y;
	int viewleft = g_Vars.players[msg->playernum]->viewleft;
	int viewtop = g_Vars.players[msg->playernum]->viewtop;
	int viewwidth = g_Vars.players[msg->playernum]->viewwidth;
	int viewheight = g_Vars.players[msg->playernum]->viewheight;
	int v0;

	int offset = (msg->alignh == HUDMSGALIGN_XMIDDLE) ? 10 : 0;

	if (PLAYERCOUNT() >= 3) {
		viewwidth -= offset;

		if (g_Vars.currentplayernum == 0 || g_Vars.currentplayernum == 2) {
			viewleft += offset;
		}
	}

	if (PLAYERCOUNT() == 2 && (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL)) {
		{
			viewwidth -= offset;

			if (g_Vars.currentplayernum == 0) {
				viewleft += offset;
			}
		}
	}

	switch (msg->alignh) {
	case HUDMSGALIGN_SCREENLEFT:
		x = msg->xmargin;
		break;
	case HUDMSGALIGN_LEFT:
		v0 = (g_InCutscene && !g_MainIsEndscreen) ? 24 : msg->xmarginextra;

		x = viewleft + v0 + msg->xmargin + 3;

		if (PLAYERCOUNT() == 2
				&& (optionsGetScreenSplit() == SCREENSPLIT_VERTICAL)
				&& (!g_InCutscene || g_MainIsEndscreen)) {
				if (msg->playernum == 0) {
					x += 15;
				} else if (msg->playernum == 1) {
					x += 4;
				}
		} else if (PLAYERCOUNT() >= 3) {
			if ((msg->playernum % 2) == 0) {
				x--;
			} else {
				x -= 16;
			}
		}
		break;
	case HUDMSGALIGN_RIGHT:
		x = viewleft + viewwidth - msg->width - msg->xmargin - 57;
		break;
	case HUDMSGALIGN_XMIDDLE:
		x = (viewwidth - msg->width) / 2 + viewleft + msg->xmargin;
		break;
	default:
		x = msg->xmargin;
		break;
	}

	switch (msg->alignv) {
	case HUDMSGALIGN_SCREENTOP:
		y = msg->ymargin;
		break;
	case HUDMSGALIGN_TOP:
		y = viewtop + msg->ymargin + 13;
		break;
	case HUDMSGALIGN_BOTTOM:
		y = viewtop + viewheight - msg->height - msg->ymargin - 14;

		if (PLAYERCOUNT() == 2 && (g_InCutscene == false || g_MainIsEndscreen)) {
			if ((optionsGetScreenSplit() != SCREENSPLIT_VERTICAL && msg->playernum == 0)) {
				y += 8;
			} else {
				y += 3;
			}
		} else if (PLAYERCOUNT() >= 3) {
			if (msg->playernum <= 1) {
				y += 8;
			} else {
				y += 3;
			}
		}
		break;
	case HUDMSGALIGN_YMIDDLE:
		//y = (viewheight - msg->height) / 2 + viewtop + msg->ymargin;
		y = (viewheight - msg->height) / 2 + viewtop + msg->ymargin + 50; //Move "Objective Completed/Failed" message down so it does't block the center of the screen
		break;
	case HUDMSGALIGN_BELOWVIEWPORT:
		y = viewtop + viewheight - (msg->height / 2) + 18;
		break;
	default:
		y = msg->ymargin;
		break;
	}

	msg->x = x;
	msg->y = y;
}

void hudmsgCreateFromArgs(char *text, int type, int conf00, int conf01, int conf02,
		struct fontchar **conf04, struct font **conf08,
		uint32_t textcolour, uint32_t glowcolour,
		uint32_t alignh, int conf16, uint32_t alignv, int conf18, int arg14, uint32_t flags)
{
	int j;
	struct hudmessage *msg;
	int hash = 0;
	int i;
	int index;
	int textwidth;
	int textheight;
	int xmarginaextra;
	int wrapwidth;
	char stacktext[400];
	int writeindex;

	if (type == HUDMSGTYPE_INGAMESUBTITLE && !optionsGetInGameSubtitles()) {
		return;
	}

	for (j = 0; text[j] != '\0'; j++) {
		hash = hash + text[j];
	}

	if ((flags & HUDMSGFLAG_ONLYIFALIVE) == 0 || !g_Vars.currentplayer->isdead) {
		if ((flags & HUDMSGFLAG_ALLOWDUPES) == 0) {
			// Check for duplicate messages
			int dupeofindex = -1;

			for (index = 0; index < g_NumHudMessages; index++) {
				if (g_HudMessages[index].state != HUDMSGSTATE_FREE
						&& g_HudMessages[index].state != HUDMSGSTATE_FADINGOUT
						&& g_HudMessages[index].playernum == g_Vars.currentplayernum
						&& g_HudMessages[index].hash == hash) {
					dupeofindex = index;
				}
			}

			if (dupeofindex >= 0) {
				return;
			}
		}

		// Find an unused index for the new message
		for (index = 0; index < g_NumHudMessages; index++) {
			if (g_HudMessages[index].state == HUDMSGSTATE_FREE) {
				break;
			}
		}

		if (index >= g_NumHudMessages
				&& (type == HUDMSGTYPE_OBJECTIVECOMPLETE
					|| type == HUDMSGTYPE_OBJECTIVEFAILED
					|| type == HUDMSGTYPE_INGAMESUBTITLE)) {
			// Out of space - Check if an existing message can be replaced
			index = hudmsgGetNext(-1);

			while (index >= 0) {
				if (g_HudMessages[index].state == HUDMSGSTATE_QUEUED) {
					if (g_HudMessages[index].type == HUDMSGTYPE_DEFAULT
							|| g_HudMessages[index].type == HUDMSGTYPE_3
							|| g_HudMessages[index].type == HUDMSGTYPE_4) {
						// Good to replace this one
						break;
					}
				}

				// Can't replace - try and find another
				index = hudmsgGetNext(g_HudMessages[index].id);
			}
		}

		if (index >= 0 && index < g_NumHudMessages) {
			xmarginaextra = 0;
			msg = &g_HudMessages[index];
			wrapwidth = hudmsgCalcXMargin(&xmarginaextra, conf16);
			textMeasure(&textheight, &textwidth, text, *conf04, *conf08, 0);
			if (textwidth > wrapwidth)
			{
				i = 0;
				writeindex = 0;

				while (i < 400 && text[i] != '\0') {
					if (text[i] != '\n') {
						stacktext[writeindex++] = text[i];
					}

					i++;
				}

				stacktext[writeindex++] = '\n';
				stacktext[writeindex++] = '\0';

				textWrap(wrapwidth, stacktext, msg->text, *conf04, *conf08);
				textMeasure(&textheight, &textwidth, msg->text, *conf04, *conf08, 0);
			} else {
				strncpy(msg->text, text, 399);
				msg->text[399] = '\0';
			}

			msg->flags = flags;
			msg->playernum = g_Vars.currentplayernum;
			msg->type = type;
			msg->id = g_NextHudMessageId++;
			msg->state = HUDMSGSTATE_QUEUED;
			msg->timer = 0;
			msg->boxed = conf00;
			msg->allowfadein = conf01;
			msg->flash = conf02;
			msg->font1 = *conf04;
			msg->font2 = *conf08;
			msg->textcolour = textcolour;
			msg->glowcolour = glowcolour;
			msg->alignh = alignh;
			msg->alignv = alignv;
			msg->width = textwidth;
			msg->height = textheight;
			msg->xmarginextra = xmarginaextra;
			msg->xmargin = conf16;
			msg->ymargin = conf18;
			msg->hash = hash;

			hudmsgCalculatePosition(msg);

			if (flags & HUDMSGFLAG_NOCHANNEL) {
				msg->showduration = TICKS(arg14);
				msg->channelnum = -1;
			} else {
				msg->showduration = TICKS(g_HudmsgTypes[type].duration);
				msg->channelnum = arg14;
			}
		}
	}
}

void hudmsgsTick(void)
{
	int k;
	int previd;
	bool show;
	struct hudmessage *msg;
	int prevplayernum;
	int i;
	int j;
	int index;
	bool hide;
	float fadeintime;
	float fadeouttime;

	g_HudmsgsActive = false;

	prevplayernum = g_Vars.currentplayernum;

	for (k = 0; k < g_NumHudMessages; k++) {
		if (g_HudMessages[k].state != HUDMSGSTATE_FREE) {
			if (g_Vars.tickmode == TICKMODE_CUTSCENE) {
				for (j = 0; j < g_NumHudMessages; j++) {
					if (k != j
							&& g_HudMessages[j].state != HUDMSGSTATE_FREE
							&& g_HudMessages[j].hash == g_HudMessages[k].hash) {
						g_HudMessages[j].state = HUDMSGSTATE_FREE;
					}
				}
			}

			setCurrentPlayerNum(g_HudMessages[k].playernum);
			hudmsgCalculatePosition(&g_HudMessages[k]);
		}
	}

	setCurrentPlayerNum(prevplayernum);

	previd = -1; \
	while (true) {
		index = hudmsgGetNext(previd);

		if (index < 0) {
			break;
		}

		msg = &g_HudMessages[index];
		previd = msg->id;

		if (msg->channelnum >= 0) {
			msg->opacity = psGetSubtitleOpacity(msg->channelnum);
		} else {
			msg->opacity = 0xff;
		}

		if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE && g_Vars.tickmode != TICKMODE_CUTSCENE) {
			msg->state = HUDMSGSTATE_FREE;
			msg->timer = 0;
		}

		switch (msg->state) {
		case HUDMSGSTATE_QUEUED:
			if (msg->flags & HUDMSGFLAG_DELAY) {
				msg->timer++;

				if (msg->timer > 3) {
					msg->flags &= ~HUDMSGFLAG_DELAY;
				}
			} else
			{
				show = true;

				if (g_Vars.players[msg->playernum]->isdead) {
					show = false;
				}

				if (show) {
					// Check if any other message is occupying our space
					for (i = 0; i < g_NumHudMessages; i++) {
						if (g_HudMessages[i].state != HUDMSGSTATE_FREE
								&& g_HudMessages[i].state != HUDMSGSTATE_QUEUED
								&& g_HudMessages[i].x + g_HudMessages[i].width >= msg->x
								&& g_HudMessages[i].x <= msg->x + msg->width
								&& g_HudMessages[i].y + g_HudMessages[i].height >= msg->y
								&& g_HudMessages[i].y <= msg->y + msg->height) {
							show = false;

							// Consider booting the previous message out earlier
							if (g_HudMessages[i].type == msg->type
									&& msg->boxed
									&& g_HudMessages[i].boxed
									&& g_HudMessages[i].state == HUDMSGSTATE_FADINGOUT) {
								g_HudMessages[i].state = HUDMSGSTATE_FREE;
								g_HudMessages[i].timer = 0;
								msg->state = HUDMSGSTATE_FADINGIN;
								msg->timer = 0;
							}

							break;
						}
					}
				}

				if (show) {
					if (msg->boxed) {
						msg->state = HUDMSGSTATE_CHOOSETRANSITION;
					} else if (msg->allowfadein) {
						msg->state = HUDMSGSTATE_FADINGIN;
					} else {
						msg->state = HUDMSGSTATE_ONSCREEN;
					}

					msg->timer = 0;

					if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
						msg->state = HUDMSGSTATE_ONSCREEN;
					}
				}
			}
			break;
		case HUDMSGSTATE_CHOOSETRANSITION:
			if (msg->boxed && msg->allowfadein) {
				msg->state = HUDMSGSTATE_FADINGIN;
			} else {
				msg->state = HUDMSGSTATE_ONSCREEN;
			}

			if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
				msg->state = HUDMSGSTATE_ONSCREEN;
			}

			msg->timer = 0;
			break;
		case HUDMSGSTATE_FADINGIN:
			if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
				// Cutscene subtitles appear immediately
				msg->state = HUDMSGSTATE_ONSCREEN;
				msg->timer = 0;
			} else {
				// Most HUD messages play a swish sound effect
				if (msg->timer == 0
						&& !lvIsPaused()
						&& !mpIsPaused()
						&& msg->type != HUDMSGTYPE_CUTSCENESUBTITLE
						&& msg->type != HUDMSGTYPE_INGAMESUBTITLE
						&& PLAYERCOUNT() == 1) {
					sndStart(var80095200, SFX_HUDMSG, NULL, -1, -1, -1, -1, -1);
				}

				fadeintime = (sqrtf(msg->width * msg->width + msg->height * msg->height) + 132) / 7.0f;

				msg->timer += g_Vars.lvupdate60;

				if (msg->timer >= (int)fadeintime || msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
					msg->state = HUDMSGSTATE_ONSCREEN;
					msg->timer = 0;
				}
			}
			break;
		case HUDMSGSTATE_ONSCREEN:
			msg->timer += g_Vars.lvupdate60;

			hide = false;

			// Subtitles have an audio channel number and are hidden when the audio stops
			if (msg->channelnum >= 0) {
				if (psIsChannelFree(msg->channelnum)) {
					hide = true;
				} else if (msg->flags & HUDMSGFLAG_FORCEOFF) {
					msg->flags &= ~HUDMSGFLAG_FORCEOFF;
					hide = true;
				}
			} else if (msg->timer >= msg->showduration && msg->showduration != -1) {
				hide = true;
			}

			if (hide) {
				if (msg->boxed) {
					msg->state = HUDMSGSTATE_FADINGOUT;
				} else {
					msg->state = HUDMSGSTATE_FREE;
				}

				msg->timer = 0;
			}
			break;
		case HUDMSGSTATE_FADINGOUT:
			fadeouttime = (sqrtf(msg->width * msg->width + msg->height * msg->height) + 92) / 7.0f;

			msg->timer += g_Vars.lvupdate60;

			if (msg->timer >= (int)fadeouttime) {
				msg->state = HUDMSGSTATE_FREE;
				msg->timer = 0;
			}
			break;
		case HUDMSGSTATE_FREE:
			break;
		}
	}
}

void hudmsgsSetOn(uint32_t reason)
{
	g_Vars.currentplayer->hudmessoff &= ~reason;
}

void hudmsgsSetOff(uint32_t reason)
{
	g_Vars.currentplayer->hudmessoff |= reason;
}

void hudmsgsRemoveForDeadPlayer(int playernum)
{
	int i;

	for (i = 0; i < g_NumHudMessages; i++) {
		if (g_HudMessages[i].state
				&& g_HudMessages[i].playernum == playernum
				&& (g_HudMessages[i].flags & HUDMSGFLAG_ONLYIFALIVE)) {
			g_HudMessages[i].state = HUDMSGSTATE_FREE;
			g_HudMessages[i].timer = 0;
		}
	}
}

Gfx *hudmsgsRender(Gfx *gdl)
{
	struct hudmessage *msg;
	int i;
	uint32_t textcolour;
	uint32_t glowcolour;
	float sin;
	int x;
	int y;
	int timerthing = 255;
	int spdc = true;
	const int playercount = PLAYERCOUNT();

	gdl = textConfigureGfxPipeline(gdl);

	if ((g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0)
			&& g_InCutscene
			&& g_MainIsEndscreen == false
			&& g_Vars.currentplayernum == 0) {
		spdc = false;
	}

	for (i = 0; i < g_NumHudMessages; i++) {
		msg = &g_HudMessages[i];

		if (!msg->opacity) {
			continue;
		}

		if (msg->state == HUDMSGSTATE_FREE
				|| msg->state == HUDMSGSTATE_QUEUED
				|| (spdc && g_Vars.currentplayernum != msg->playernum)) {
			continue;
		}

		if (msg->flash) {
			int alpha;
			sin = sinf((msg->timer * M_PI) / 60.0f);

			if (sin < 0.0f) {
				sin = -sin;
			}

			alpha = 192.0f * sin;

			textcolour = (msg->textcolour & 0xffffff00) + alpha;
			glowcolour = msg->glowcolour;
		} else {
			textcolour = msg->textcolour | 0xa0;
			glowcolour = msg->glowcolour;
		}

		if (msg->opacity != 255) {
			uint32_t textalpha = textcolour & 0xff;
			uint32_t glowalpha = glowcolour & 0xff;

			textalpha = (msg->opacity * textalpha) / 255;
			glowalpha = (msg->opacity * glowalpha) / 255;

			textcolour = (textcolour & 0xffffff00) + (textalpha & 0xff);
			glowcolour = (glowcolour & 0xffffff00) + (glowalpha & 0xff);
		}

		x = msg->x;
		y = msg->y;

		if (msg->type == HUDMSGTYPE_INGAMESUBTITLE && playerIsHealthVisible()) {
			y += (int)(16.0f * playerGetHealthBarHeightFrac());
		}

		const bool doaspectfix = (playercount < 2) || (playercount == 2 && optionsGetScreenSplit() == SCREENSPLIT_HORIZONTAL);
		if (doaspectfix && msg->state >= HUDMSGSTATE_FADINGIN) {
			if (msg->alignh == HUDMSGALIGN_SCREENLEFT || msg->alignh == HUDMSGALIGN_LEFT) {
				gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeL);
			} else if (msg->alignh == HUDMSGALIGN_RIGHT) {
				gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, g_HudAlignModeR);
			} else {
				gfx_Extra_Geometry_Mode_EXT(gdl++, G_ASPECT_MODE_EXT, G_ASPECT_CENTER_EXT);
			}
		}

		if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
			gfx_Set_Scissor(gdl++,
					(x - 4), 0,
					(x + msg->width + 3), viGetBufHeight());
		}

		switch (msg->state) {
		case HUDMSGSTATE_FREE:
		case HUDMSGSTATE_QUEUED:
			break;
		case HUDMSGSTATE_FADINGIN:
			{
				uint32_t bordercolour = msg->textcolour | 0x40;
				float tmp;
				float spc0;

				if (msg->opacity != 255) {
					uint32_t alpha = (msg->opacity * (bordercolour & 0xff)) / 255;
					bordercolour = (bordercolour & 0xffffff00) + (alpha & 0xff);
				}

				spc0 = (sqrtf(msg->width * msg->width + msg->height * msg->height) + 132.0f) / 7.0f;

				if (spc0 > 30.0f) {
					spc0 = 30.0f;
				}

				spc0 = msg->timer / spc0;

				if (spc0 > 1.0f) {
					spc0 = 1.0f;
				}

				if (spc0 < 0.0f) {
					spc0 = 0.0f;
				}

				tmp = msg->timer * 7.0f;

				textSetDiagonalBlend(x, y, tmp, DIAGMODE_FADEIN);

				if (msg->boxed) {
					gdl = hudmsgRenderBox(gdl, x - 3, y - 3, x + msg->width + 2, y + msg->height + 2, 1.0f, bordercolour, spc0);

					if (spc0 > 0) {
						gdl = textRenderProjected(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, viGetWidth(), viGetHeight(), 0, 0);
					}
				} else {
					gdl = text0f153a34(gdl, x, y, x + msg->width, y + msg->height, 0);
					gdl = textRender(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, glowcolour, viGetWidth(), viGetHeight(), 0, 0);
				}

				if (msg->alignv == 6) {
					timerthing = 0;
				}

				textResetBlends();
			}
			break;
		case HUDMSGSTATE_ONSCREEN:
			if (msg->boxed) {
				uint32_t bordercolour = msg->textcolour | 0x40;

				if (msg->opacity != 255) {
					uint32_t alpha = (msg->opacity * (bordercolour & 0xff)) / 255;
					bordercolour = (bordercolour & 0xffffff00) + (alpha & 0xff);
				}

				gdl = hudmsgRenderBox(gdl, x - 3, y - 3, x + msg->width + 2, y + msg->height + 2, 1.0f, bordercolour, 1.0f);

				gdl = textRenderProjected(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, viGetWidth(), viGetHeight(), 0, 0);
			} else {
				gdl = text0f153a34(gdl, x, y, x + msg->width, y + msg->height, 0);

				gdl = textRender(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, glowcolour, viGetWidth(), viGetHeight(), 0, 0);
			}
			if (msg->alignv == 6) {
				timerthing = 0;
			}
			break;
		case HUDMSGSTATE_FADINGOUT:
			{
				uint32_t bordercolour;
				uint32_t stack;
				float spa8 = (sqrtf(msg->width * msg->width + msg->height * msg->height) + 92.0f) / 7.0f;
				float tmp;

				bordercolour = msg->textcolour | 0x40;

				if (msg->opacity != 255) {
					uint32_t alpha = (msg->opacity * (bordercolour & 0xff)) / 255;
					bordercolour = (bordercolour & 0xffffff00) + (alpha & 0xff);
				}

				tmp = (spa8 - msg->timer) * 7.0f;

				textSetDiagonalBlend(x + msg->width, y + msg->height, tmp, DIAGMODE_FADEOUT);

				if (spa8 > 30.0f) {
					spa8 = 30.0f;
				}

				spa8 = msg->timer / spa8;

				if (spa8 > 1.0f) {
					spa8 = 1.0f;
				}

				if (msg->boxed) {
					gdl = hudmsgRenderBox(gdl, x - 3, y - 3, x + msg->width + 2, y + msg->height + 2, 1.0f, bordercolour, 1.0f - spa8);

					if (spa8 < 1.0f) {
						gdl = textRenderProjected(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, viGetWidth(), viGetHeight(), 0, 0);
					}
				} else {
					gdl = text0f153a34(gdl, x, y, x + msg->width, y + msg->height, 0);

					gdl = textRender(gdl, &x, &y, msg->text, msg->font1, msg->font2, textcolour, glowcolour, viGetWidth(), viGetHeight(), 0, 0);
				}

				if (msg->alignv == 6) {
					timerthing = 0;
				}

				textResetBlends();
			}
			break;
		}

		gSPClearExtraGeometryModeEXT(gdl++, G_ASPECT_MODE_EXT);

		if (msg->type == HUDMSGTYPE_CUTSCENESUBTITLE) {
			gfx_Set_Scissor(gdl++,
					viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth(), viGetViewTop() + viGetViewHeight());
		}
	}

	if (timerthing) {
		if (optionsGetShowMissionTime(g_Vars.currentplayerstats->mpindex)
				&& g_Vars.normmplayerisrunning == false
				&& g_Vars.stagenum != STAGE_CITRAINING
				&& g_Vars.currentplayer->cameramode != CAMERAMODE_EYESPY
				&& g_Vars.currentplayer->cameramode != CAMERAMODE_THIRDPERSON) {
			gdl = hudmsgRenderMissionTimer(gdl, timerthing);
		}

		if (hudmsgIsZoomRangeVisible()) {
			gdl = hudmsgRenderZoomRange(gdl, timerthing);
		}

		gdl = countdownTimerRender(gdl);
	}

	gdl = textSetPerspAndLOD(gdl);

	return gdl;
}

void hudmsgsStop(void)
{
	int i;

	for (i = 0; i < g_NumHudMessages; i++) {
		g_HudMessages[i].state = HUDMSGSTATE_FREE;
	}
}
