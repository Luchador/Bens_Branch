#include "constants.h"
#include "game/chrutils.h"
#include "game/title.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"
#include "video.h"

void chrmgrReset(void)
{
	int i;

	g_ChrAnimSpeed = 1;
	g_SelectedAnimNum = 0;
	g_NextChrnum = 5000;
	g_ChrSlots = NULL;
	g_NumChrSlots = 0;

	g_ShieldHits = mempAlloc(sizeof(struct shieldhit) * 20, MEMPOOL_STAGE);

	for (i = 0; i < 20; i++) {
		g_ShieldHits[i].prop = NULL;
	}

	g_ShieldHitActive = false;
	g_NumChrs = 0;
	g_Chrnums = NULL;
	g_ChrIndexes = NULL;
	var80062960 = mempAlloc(ALIGN16(15 * sizeof(struct var80062960)), MEMPOOL_STAGE);

	for (i = 0; i < ARRAYCOUNT(var8009ccc0); i++) {
		if (!var8009ccc0[i]) {
			var8009ccc0[i] = videoCreateFramebuffer(16, 16, false, false);
		}
	}

	resetSomeStageThings();
}

void chrmgrConfigure(int numchrs)
{
	int i;

	//g_NumChrSlots = PLAYERCOUNT() + numchrs + 10;
	g_NumChrSlots = PLAYERCOUNT() + numchrs + 400; // Ben's comment: allow far more chars
	g_ChrSlots = mempAlloc(ALIGN16(g_NumChrSlots * sizeof(struct chrdata)), MEMPOOL_STAGE);

	for (i = 0; i < g_NumChrSlots; i++) {
		g_ChrSlots[i].chrnum = -1;
		g_ChrSlots[i].model = NULL;
		g_ChrSlots[i].prop = NULL;
	}

	g_NumChrs = 0;
	g_Chrnums = mempAlloc(ALIGN16(g_NumChrSlots * sizeof(g_Chrnums[0])), MEMPOOL_STAGE);
	g_ChrIndexes = mempAlloc(ALIGN16(g_NumChrSlots * sizeof(g_ChrIndexes[0])), MEMPOOL_STAGE);

	for (i = 0; i < g_NumChrSlots; i++) {
		g_Chrnums[i] = -1;
		g_ChrIndexes[i] = -1;
	}
}
