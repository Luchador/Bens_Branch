#include <stdint.h>
#include "constants.h"
#include "game/timing.h"
#include "game/utils.h"
#include "bss.h"
#include "data.h"
#include "types.h"
#include "system.h"

void frametimeInit(void)
{
	g_Vars.thisframestartt = utilsGetCount();
	g_Vars.prevframestartt = g_Vars.thisframestartt;
}

void frametimeApply(int diffframe60, int diffframe240, int frametime)
{
	g_Vars.prevframestartt = g_Vars.thisframestartt;
	g_Vars.thisframestartt = frametime;

	g_Vars.diffframe60 = diffframe60;
	g_Vars.diffframe60f = diffframe60;
	g_Vars.diffframe60freal = g_Vars.diffframe60f;

	g_Vars.prevframestart240 = g_Vars.thisframestart240;
	g_Vars.thisframestart240 += diffframe240;
	g_Vars.diffframe240 = diffframe240;
	g_Vars.diffframe240f = diffframe240;
	g_Vars.diffframe240freal = g_Vars.diffframe240f;
}

void frametimeCalculate(void)
{
	uint32_t count;
	uint32_t diffframet;
	uint32_t diffframe60;
	uint32_t diffframe240;

	do {
		count = utilsGetCount();
		diffframet = count - g_Vars.thisframestartt;
		g_Vars.diffframet = diffframet;

		diffframe60 = (g_Vars.lostframetime60t + diffframet + CYCLES_PER_FRAME / 2) / CYCLES_PER_FRAME;
		diffframe240 = (g_Vars.lostframetime240t + diffframet + CYCLES_PER_FRAME / 2 / 4) / (CYCLES_PER_FRAME / 4);

		if (g_TickExtraSleep) {
			sysSleep(EXTRA_SLEEP_TIME);
		}
	} while (g_Vars.mininc60 && diffframe60 < g_Vars.mininc60);

	g_Vars.lostframetime60t = g_Vars.lostframetime60t + diffframet - diffframe60 * CYCLES_PER_FRAME;
	g_Vars.lostframetime240t = g_Vars.lostframetime240t + diffframet - diffframe240 * (CYCLES_PER_FRAME / 4);
	g_Vars.mininc60 = g_TickRateDiv;

	frametimeApply(diffframe60, diffframe240, count);
}