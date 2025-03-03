#include <ultra64.h>
#include "constants.h"
#include "game/body.h"
#include "game/cheats.h"
#include "game/chrai.h"
#include "game/chrutils.h"
#include "game/playerreset.h"
#include "game/setuputils.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "data.h"
#include "types.h"

void bodiesReset(s32 stagenum)
{
	s32 i;

	for (i = 0; g_HeadsAndBodies[i].filenum != 0; i++) {
		g_HeadsAndBodies[i].modeldef = NULL;
	}

	g_RandomBond = rngRandom() % g_NumBondBodies;
}
