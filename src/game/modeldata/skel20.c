#include <ultra64.h>
#include "bss.h"
#include "data.h"
#include "types.h"

uint8_t g_Skel20Joints[][2] = {
	{ 0, 0 },
};

struct skeleton g_Skel20 = {
	SKEL_20, ARRAYCOUNT(g_Skel20Joints), g_Skel20Joints,
};
