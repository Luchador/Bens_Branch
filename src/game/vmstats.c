#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/vm.h"
#include "data.h"
#include "types.h"

void vmPrintStatsIfEnabled(void)
{
	char buffer[80];
	g_VmNumTlbMisses = 0;
	g_VmNumPageMisses = 0;
	g_VmNumPageReplaces = 0;
}
