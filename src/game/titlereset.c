#include <ultra64.h>
#include "constants.h"
#include "game/getitle.h"
#include "game/title.h"
#include "bss.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

void titleReset(void)
{
	g_TitleMode = -1;
	g_TitleDelayedTimer = 2;
	g_TitleDelayedMode = -1;

	var8009cca0 = mempAlloc(TITLE_ALLOCSIZE, MEMPOOL_STAGE);

	getitleReset();
}
