#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/mainmenu.h"
#include "game/music.h"
#include "game/options.h"
#include "bss.h"
#include "data.h"
#include "types.h"

uint8_t g_InGameSubtitles = 1;
uint8_t g_CutsceneSubtitles = 0;
int g_ScreenSize = SCREENSIZE_FULL;
int g_ScreenRatio = SCREENRATIO_NORMAL;
uint8_t g_ScreenSplit = SCREENSPLIT_HORIZONTAL;

int optionsGetControlMode(int mpchrnum)
{
	return g_PlayerConfigsArray[mpchrnum].controlmode;
}

void optionsSetControlMode(int mpchrnum, int mode)
{
	g_PlayerConfigsArray[mpchrnum].controlmode = mode;
}

int optionsGetContpadNum1(int mpchrnum)
{
	return g_PlayerConfigsArray[mpchrnum].contpad1;
}

int optionsGetContpadNum2(int mpchrnum)
{
	return g_PlayerConfigsArray[mpchrnum].contpad2;
}

int optionsGetForwardPitch(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_FORWARDPITCH) != 0;
}

int optionsGetAutoAim(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_AUTOAIM) != 0;
}

int optionsGetLookAhead(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_LOOKAHEAD) != 0;
}

int optionsGetAimControl(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_AIMCONTROL) != 0;
}

int optionsGetSightOnScreen(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_SIGHTONSCREEN) != 0;
}

int optionsGetAmmoOnScreen(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_AMMOONSCREEN) != 0;
}

int optionsGetShowGunFunction(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_SHOWGUNFUNCTION) != 0;
}

int optionsGetAlwaysShowTarget(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_ALWAYSSHOWTARGET) != 0;
}

int optionsGetShowZoomRange(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_SHOWZOOMRANGE) != 0;
}

int optionsGetPaintball(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_PAINTBALL) != 0;
}

int optionsGetShowMissionTime(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_SHOWMISSIONTIME) != 0;
}

uint8_t optionsGetInGameSubtitles(void)
{
	return g_InGameSubtitles;
}

uint8_t optionsGetCutsceneSubtitles(void)
{
	return g_CutsceneSubtitles;
}

int optionsGetHeadRoll(int mpchrnum)
{
	return (g_PlayerConfigsArray[mpchrnum].options & OPTION_HEADROLL) != 0;
}

void optionsSetForwardPitch(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_FORWARDPITCH;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_FORWARDPITCH;
	}
}

void optionsSetAutoAim(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_AUTOAIM;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_AUTOAIM;
	}
}

void optionsSetLookAhead(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_LOOKAHEAD;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_LOOKAHEAD;
	}
}

void optionsSetAimControl(int mpchrnum, int index)
{
	if (index) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_AIMCONTROL;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_AIMCONTROL;
	}
}

void optionsSetSightOnScreen(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_SIGHTONSCREEN;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_SIGHTONSCREEN;
	}
}

void optionsSetAmmoOnScreen(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_AMMOONSCREEN;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_AMMOONSCREEN;
	}
}

void optionsSetShowGunFunction(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_SHOWGUNFUNCTION;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_SHOWGUNFUNCTION;
	}
}

void optionsSetAlwaysShowTarget(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_ALWAYSSHOWTARGET;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_ALWAYSSHOWTARGET;
	}
}

void optionsSetShowZoomRange(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_SHOWZOOMRANGE;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_SHOWZOOMRANGE;
	}
}

void optionsSetPaintball(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_PAINTBALL;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_PAINTBALL;
	}
}

void optionsSetShowMissionTime(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_SHOWMISSIONTIME;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_SHOWMISSIONTIME;
	}
}

void optionsSetInGameSubtitles(int enable)
{
	g_InGameSubtitles = enable;
}

void optionsSetCutsceneSubtitles(int enable)
{
	g_CutsceneSubtitles = enable;
}

void optionsSetHeadRoll(int mpchrnum, bool enable)
{
	if (enable) {
		g_PlayerConfigsArray[mpchrnum].options |= OPTION_HEADROLL;
	} else {
		g_PlayerConfigsArray[mpchrnum].options &= ~OPTION_HEADROLL;
	}
}

int optionsGetEffectiveScreenSize(void)
{
	if (g_MenuData.root == MENUROOT_TRAINING) {
		g_MpPlayerNum = 0;

		if (g_Menus[g_MpPlayerNum].curdialog && g_GamePaused) {
			return SCREENSIZE_FULL;
		}
	}

	if (g_Menus[g_MpPlayerNum].curdialog) {
		if (g_Menus[g_MpPlayerNum].curdialog->definition == &g_CiControlStylePlayer2MenuDialog
				|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_CiControlStyleMenuDialog
				|| g_Menus[g_MpPlayerNum].curdialog->definition == &g_SoloMissionControlStyleMenuDialog) {
			return SCREENSIZE_FULL;
		}
	}

	if (PLAYERCOUNT() >= 2 || g_MenuData.root == MENUROOT_MPSETUP) {
		return SCREENSIZE_FULL;
	}

	return g_ScreenSize;
}

int optionsGetScreenSize(void)
{
	return g_ScreenSize;
}

void optionsSetScreenSize(int size)
{
	g_ScreenSize = size;
}

int optionsGetScreenRatio(void)
{
	return g_ScreenRatio;
}

void optionsSetScreenRatio(int ratio)
{
	g_ScreenRatio = SCREENRATIO_NORMAL;
}

uint8_t optionsGetScreenSplit(void)
{
	return g_ScreenSplit;
}

void optionsSetScreenSplit(uint8_t split)
{
	g_ScreenSplit = split;
}

uint16_t optionsGetMusicVolume(void)
{
	return musicGetVolume();
}

void optionsSetMusicVolume(uint16_t volume)
{
	musicSetVolume(volume);
}
