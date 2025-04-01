#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ultra64.h>
#include <PR/ultrasched.h>
#include "lib/sched.h"
#include "lib/vars.h"
#include "constants.h"
#include "game/cheats.h"
#include "game/debug.h"
#include "game/file.h"
#include "game/lang.h"
#include "game/race.h"
#include "game/body.h"
#include "game/smoke.h"
#include "game/tex.h"
#include "game/challenge.h"
#include "game/title.h"
#include "game/objectives.h"
#include "game/endscreen.h"
#include "game/playermgr.h"
#include "game/textutils.h"
#include "game/gfxmemory.h"
#include "game/lang.h"
#include "game/lv.h"
#include "game/timing.h"
#include "game/music.h"
#include "game/zbuf.h"
#include "game/mplayer/mplayer.h"
#include "game/pak.h"
#include "game/splat.h"
#include "game/utils.h"
#include "bss.h"
#include "lib/audiomgr.h"
#include "lib/args.h"
#include "lib/rzip.h"
#include "lib/vi.h"
#include "lib/dma.h"
#include "lib/joy.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/mema.h"
#include "lib/model.h"
#include "lib/anim.h"
#include "lib/rdp.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"
#include "system.h"
#include "video.h"

extern uint8_t *g_MempHeap;
extern uint32_t g_MempHeapSize;

void rngSetSeed(uint32_t seed);

bool g_AcceptCMDParams = false;
int g_StageNum = STAGE_TITLE;
uint32_t g_MainMemaHeapSize = 1024 * 300;
bool g_MainIsEndscreen = false;
int g_MainChangeToStageNum = -1;

struct stageallocation g_StageAllocations8Mb[] = {
	{ STAGE_CITRAINING,    "-ml0 -me0 -mgfx480 -mvtx392 -ma1600"             },
	{ STAGE_DEFECTION,     "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_INVESTIGATION, "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_EXTRACTION,    "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_CHICAGO,       "-ml0 -me0 -mgfx110 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_G5BUILDING,    "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_VILLA,         "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_INFILTRATION,  "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2000" },
	{ STAGE_RESCUE,        "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2000" },
	{ STAGE_ESCAPE,        "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2000" },
	{ STAGE_AIRBASE,       "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_AIRFORCEONE,   "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_CRASHSITE,     "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_PELAGIC,       "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_DEEPSEA,       "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_DEFENSE,       "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_ATTACKSHIP,    "-ml0 -me0 -mgfx440 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_SKEDARRUINS,   "-ml0 -me0 -mgfx110 -mgfxtra320 -mvtx400 -ma2400" },
	{ STAGE_MP_SKEDAR,     "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_RAVINE,     "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_PIPES,      "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_G5BUILDING, "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_SEWERS,     "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_WAREHOUSE,  "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_BASE,       "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_COMPLEX,    "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_TEMPLE,     "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_FELICITY,   "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_AREA52,     "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_GRID,       "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_CARPARK,    "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_RUINS,      "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_FORTRESS,   "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MP_VILLA,      "-ml0 -me0 -mgfx200 -mvtx200 -ma400"            },
	{ STAGE_MBR,           "-ml0 -me0 -mgfx120 -mvtx100 -ma700"            },
	{ STAGE_MAIANSOS,      "-ml0 -me0 -mgfx120 -mvtx100 -ma500"            },
	{ STAGE_WAR,           "-ml0 -me0 -mgfx120 -mvtx98 -ma400"             },
	{ STAGE_DUEL,          "-ml0 -me0 -mgfx120 -mvtx100 -ma700"            },
	{ STAGE_TITLE,         "-ml0 -me0 -mgfx80 -mvtx20 -ma001"              },
	{ 0,                   "-ml0 -me0 -mgfx120 -mvtx98 -ma300"             },
};

bool g_MainIsBooting = true;

void mainInit(void)
{
	dmaInit();
	varsInit();
	joyInit();
	joyReset();

	g_AcceptCMDParams = true;

	viConfigureForLegal();
	viUpdateMode();

	filesInit();

	if (g_AcceptCMDParams) {
		argSetString("          -ml0 -me0 -mgfx100 -mvtx50 -mt700 -ma400");
	}

	mempSetHeap(g_MempHeap, g_MempHeapSize);

	mempResetPool(MEMPOOL_8);
	mempResetPool(MEMPOOL_PERMANENT);
	challengesInit();
	texInit();
	lvInit();
	cheatsInit();
	playermgrInit();
	frametimeInit();
	smokesInit();
	mpInit();
	paksInit();
	animsInit();
	racesInit();
	bodiesInit();
	titleInit();

	modelSetDistanceChecksDisabled(true); // don't use LODs

	g_MainIsBooting = false;
}

void mainProc(void)
{
	mainInit();
	sndInit();

	while (true) {
		mainLoop();
	}
}

/**
 * This function enters an infinite loop which iterates once per stage load.
 * Within this loop is an inner loop which runs very frequently and decides
 * whether to run mainTick on each iteration.
 *
 * NTSC beta checks two shorts at an offset 64MB into the development board
 * and refuses to continue if they are not any of the allowed values.
 * Decomp patches these reads in its build system so it can be played
 * without the development board.
 */
void mainLoop(void)
{
	bool ending = false;
	int index;
	int numplayers;

	filesStop(5);
	mempResetPool(MEMPOOL_5);
	filesStop(5);

	argGetLevel(&g_StageNum);

	if (g_StageNum != STAGE_TITLE) {
		titleSetNextStage(g_StageNum);

		if (g_StageNum < STAGE_TITLE) {
			if (argFindByPrefix(1, "-hard")) {
				lvSetDifficulty(argFindByPrefix(1, "-hard")[0] - '0');
			}
		}
	}

	rngSetSeed(utilsGetCount());

	// Outer loop - this is infinite because ending is never changed
	while (!ending) {
		g_MainIsEndscreen = false;

		if (g_AcceptCMDParams) {
			index = -1;

			if (g_StageNum < STAGE_TITLE && getNumPlayers() >= 2) {
				index = 0; \
				while (g_StageAllocations8Mb[index].stagenum) { \
					if (g_StageNum + 400 == g_StageAllocations8Mb[index].stagenum) { \
						break; \
					} \
					index++;
				}

				if (g_StageAllocations8Mb[index].stagenum == 0) {
					index = -1;
				}
			}

			if (index < 0) {
				index = 0;

				while (g_StageAllocations8Mb[index].stagenum) {
					if (g_StageNum == g_StageAllocations8Mb[index].stagenum) {
						break;
					}

					index++;
				}
			}

			argSetString(g_StageAllocations8Mb[index].string);
			
		}

		mempResetPool(MEMPOOL_7);
		mempResetPool(MEMPOOL_STAGE);
		filesStop(4);

		if (argFindByPrefix(1, "-ma")) {
			g_MainMemaHeapSize = strtol(argFindByPrefix(1, "-ma"), NULL, 0) * 1024;
		}

		memaReset(mempAlloc(g_MainMemaHeapSize, MEMPOOL_STAGE), g_MainMemaHeapSize);
		langInit();
		playermgrReset();

		if (g_StageNum >= STAGE_TITLE) {
			numplayers = 0;
		} else {
			if (argFindByPrefix(1, "-play")) {
				numplayers = strtol(argFindByPrefix(1, "-play"), NULL, 0);
			} else {
				numplayers = 1;
			}

			if (getNumPlayers() >= 2) {
				numplayers = getNumPlayers();
			}
		}

		if (numplayers < 2) {
			g_Vars.bondplayernum = 0;
			g_Vars.coopplayernum = -1;
			g_Vars.antiplayernum = -1;
		} else if (argFindByPrefix(1, "-coop")) {
			g_Vars.bondplayernum = 0;
			g_Vars.coopplayernum = 1;
			g_Vars.antiplayernum = -1;
		} else if (argFindByPrefix(1, "-anti")) {
			g_Vars.bondplayernum = 0;
			g_Vars.coopplayernum = -1;
			g_Vars.antiplayernum = 1;
		}

		playermgrAllocatePlayers(numplayers);

		if (argFindByPrefix(1, "-mpbots")) {
			g_Vars.lvmpbotlevel = 1;
		}

		if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
			g_MpSetup.chrslots = 0x03;
			mpReset();
		} else if (g_Vars.perfectbuddynum) {
			mpReset();
		} else if (g_Vars.mplayerisrunning == false
				&& (numplayers >= 2 || g_Vars.lvmpbotlevel || argFindByPrefix(1, "-play"))) {
			g_MpSetup.chrslots = 1;

			if (numplayers >= 2) {
				g_MpSetup.chrslots |= 1 << 1;
			}

			if (numplayers >= 3) {
				g_MpSetup.chrslots |= 1 << 2;
			}

			if (numplayers >= 4) {
				g_MpSetup.chrslots |= 1 << 3;
			}

			g_MpSetup.stagenum = g_StageNum;
			mpReset();
		}

		gfxReset();
		joyReset();
		zbufReset(g_StageNum);
		lvReset(g_StageNum);
		viReset(g_StageNum);
		frametimeCalculate();

		while (g_MainChangeToStageNum < 0) {
			const int cycles = utilsGetCount() - g_Vars.thisframestartt;
			if (!g_Vars.mininc60 || (cycles >= g_Vars.mininc60 * CYCLES_PER_FRAME - CYCLES_PER_FRAME / 2)) {
				videoStartFrame();
				mainTick();
				schedEndFrame();
			}
			if (g_TickExtraSleep) {
				sysSleep(EXTRA_SLEEP_TIME);
			}
		}

		lvStop();
		mempDisablePool(MEMPOOL_STAGE);
		mempDisablePool(MEMPOOL_7);
		filesStop(4);
		pak0f116994();

		g_StageNum = g_MainChangeToStageNum;
		g_MainChangeToStageNum = -1;
	}
}

void mainTick(void)
{
	Gfx *gdl = NULL;
	Gfx *gdlstart = NULL;
	int i;

	if (g_MainChangeToStageNum < 0) {
		frametimeCalculate();
		joyDebugJoy();

		gdl = gdlstart = gfxGetMasterDisplayList();

		gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0x0000, G_TX_LOADTILE, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
		gDPSetTile(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 0x0100, 6, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);

		lvTick();
		playermgrShuffle();

		if (g_StageNum < STAGE_TITLE) {
			for (i = 0; i < PLAYERCOUNT(); i++) {
				setCurrentPlayerNum(playermgrGetPlayerAtOrder(i));

				if (!titleIsKeepingMode()) {
					viSetViewPosition(g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop);
					viSetFovAspectAndSize(
							g_Vars.currentplayer->fovy, g_Vars.currentplayer->aspect,
							g_Vars.currentplayer->viewwidth, g_Vars.currentplayer->viewheight);
				}

				lvTickPlayer();
			}
		}

		gdl = lvRender(gdl);

		gDPFullSync(gdl++);
		gSPEndDisplayList(gdl++);

		gfxSwapBuffers();
		viUpdateMode();

		// Used in PC port
		rdpCreateTask(gdlstart, gdl, 0);
		memaPrint();
	}
}

void mainEndStage(void)
{
	sndStopNosedive();

	if (!g_MainIsEndscreen) {
		pak0f11c6d0();
		joyDisableTemporarily();

		if (g_Vars.coopplayernum >= 0) {
			int prevplayernum = g_Vars.currentplayernum;
			int i;

			for (i = 0; i < PLAYERCOUNT(); i++) {
				setCurrentPlayerNum(i);
				endscreenPushCoop();
			}

			setCurrentPlayerNum(prevplayernum);
			musicStartMenu();
		} else if (g_Vars.antiplayernum >= 0) {
			int prevplayernum = g_Vars.currentplayernum;
			int i;

			for (i = 0; i < PLAYERCOUNT(); i++) {
				setCurrentPlayerNum(i);
				endscreenPushAnti();
			}

			setCurrentPlayerNum(prevplayernum);
			musicStartMenu();
		} else if (g_Vars.normmplayerisrunning) {
			mpEndMatch();
		} else {
			endscreenPrepare();
			musicStartMenu();
		}
	}

	g_MainIsEndscreen = true;
}

/**
 * Change to the given stage at the end of the current frame.
 */
void mainChangeToStage(int stagenum)
{
	pak0f11c6d0();

	g_MainChangeToStageNum = stagenum;
}

int mainGetStageNum(void)
{
	return g_StageNum;
}

void mainFinalObjectiveCheck(void)
{
	objectivesCheckAll();
	objectivesDisableChecking();
	mainEndStage();
}
