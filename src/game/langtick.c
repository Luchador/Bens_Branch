#include <ultra64.h>
#include "constants.h"
#include "game/lang.h"
#include "game/room.h"
#include "lib/main.h"
#include "bss.h"
#include "data.h"
#include "types.h"

extern s32 g_JpnMaxCacheItems;

void langTick(void)
{
	s32 i;
	if (g_Jpn) {
		for (i = 0; i != MAX_JPN_CACHE_ITEMS(); i++) {
			if (g_JpnCacheCacheItems[i].ttl) {
				g_JpnCacheCacheItems[i].ttl--;
			}
		}
	}
}
