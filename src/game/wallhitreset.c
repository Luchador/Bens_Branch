#include <ultra64.h>
#include <stdbool.h>
#include "constants.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

uint16_t *g_WallhitCountsPerRoom;
int g_WallhitsMax;
uint32_t g_WallhitsNumSettled;
uint32_t g_WallhitsNumFree;
uint32_t g_WallhitsNumUsed;
uint32_t g_WallhitsNumBloodSettled;
uint32_t g_WallhitsNumNonbloodSettled;
int g_MinPropWallhits;
uint32_t g_MaxPropWallhits;
int g_MinBgWallhitsPerRoom;
int g_MaxBgWallhitsPerRoom;
int g_WallhitsCriticalSpareLimit;
int g_WallhitsGoalSpareLimit;
float g_WallhitTargetBloodRatio;

/**
 * Initialises an array of room numbers and a linked list of structs.
 *
 * Related to blood splats, bullet holes and scorch marks.
 * With this function nopped they do not appear.
 */
void wallhitReset(void)
{
	int type = 2;
	int i;

	g_WallhitsMax = 10000;
	g_MinPropWallhits = 50;
	g_MaxPropWallhits = 1200;
	g_MinBgWallhitsPerRoom = 10;
	g_MaxBgWallhitsPerRoom = 1200;
	g_WallhitsCriticalSpareLimit = 25;
	g_WallhitsGoalSpareLimit = 40;
	g_WallhitTargetBloodRatio = 0.5f;
	g_WallhitCountsPerRoom = NULL;
	g_WallhitsNumSettled = 0;
	g_WallhitsNumFree = 0;
	g_WallhitsNumUsed = 0;
	g_WallhitsNumBloodSettled = 0;
	g_WallhitsNumNonbloodSettled = 0;

	if (g_Vars.stagenum >= STAGE_TITLE) {
		g_WallhitsMax = 0;
	}

	if (g_WallhitsMax == 0) {
		g_Wallhits = NULL;
	} else {
		// Allocate an array of s16 room numbers followed by a bunch of structs
		uint32_t numberssize;
		uint32_t structssize;
		void *ptr;

		structssize = g_WallhitsMax * sizeof(struct wallhit);
		structssize += 0xf;
		structssize &= ~0xf;

		numberssize = g_Vars.roomcount * 2;
		numberssize += 0xf;
		numberssize &= ~0xf;

		ptr = mempAlloc(structssize + numberssize, MEMPOOL_STAGE);

		g_WallhitCountsPerRoom = ptr;
		g_Wallhits = (struct wallhit *)((uintptr_t)ptr + numberssize);
		g_FreeWallhits = NULL;
		g_ActiveWallhits = 0;

		// Initialise structs
		for (i = 0; i < g_WallhitsMax; i++) {
			g_Wallhits[i].timermax = 0;
			g_Wallhits[i].timercur = 0;
			g_Wallhits[i].createdframe = 0;
			g_Wallhits[i].inuse = false;
			g_Wallhits[i].roomnum = -1;
			g_Wallhits[i].chrprop = NULL;
			g_Wallhits[i].objprop = NULL;

			g_WallhitsNumFree++;

			g_Wallhits[i].globalnext = g_FreeWallhits;
			g_FreeWallhits = &g_Wallhits[i];
		}

		// Initialise room numbers
		for (i = 0; i < g_Vars.roomcount; i++) {
			g_WallhitCountsPerRoom[i] = 0;
		}
	}
}
