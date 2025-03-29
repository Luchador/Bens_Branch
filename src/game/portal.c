#include <ultra64.h>
#include "constants.h"
#include "game/bondeyespy.h"
#include "game/bondmove.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/inv.h"
#include "game/nbomb.h"
#include "game/title.h"
#include "game/chr.h"
#include "game/body.h"
#include "game/prop.h"
#include "game/propsnd.h"
#include "game/objectives.h"
#include "game/quaternion.h"
#include "game/bondgun.h"
#include "game/gunfx.h"
#include "game/weaponutils.h"
#include "game/utils.h"
#include "game/tex.h"
#include "game/portal.h"
#include "game/healthbar.h"
#include "game/hudmsg.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/filemgr.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/explosions.h"
#include "game/bondview.h"
#include "game/textutils.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/room.h"
#include "game/gfxmemory.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/texdecompress.h"
#include "game/mplayer/ingame.h"
#include "game/mplayer/scenarios.h"
#include "game/radar.h"
#include "game/training.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "game/pak.h"
#include "game/options.h"
#include "game/propobj.h"
#include "game/splat.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"

struct roomacousticdata *g_RoomAcousticData;
struct var8009dd78 var8009dd78[10];
uint16_t *g_PortalXluFracs;
int g_NumPortalXluFracs;

void portalSetXluFrac2(int portalnum, float frac)
{
	if (portalnum >= 0) {
		uint8_t value = (uint32_t)(255 * frac);
		value <<= 0;
		g_PortalXluFracs[portalnum] = (g_PortalXluFracs[portalnum] & 0xff00) | value;
	}
}

void portalSetXluFrac(int portalnum, float frac)
{
	if (portalnum >= 0) {
		uint8_t value = (uint32_t)(15 * frac) & 0xf;
		g_PortalXluFracs[portalnum] = (g_PortalXluFracs[portalnum] & 0xf0ff) | (value << 8);
	}
}

float portalGetXluFrac2(int arg0)
{
	float value = (g_PortalXluFracs[arg0] & 0xff) * 0.0039215688593686f;

	return value;
}

float portalGetXluFrac(int arg0)
{
	float value = ((g_PortalXluFracs[arg0] & 0xf00) >> 8) * 0.06666667f;

	return value;
}

void portal0f0b65a8(int numportals)
{
	if (numportals > 0) {
		g_NumPortalXluFracs = numportals;
		g_PortalXluFracs = mempAlloc(ALIGN16(numportals * 2), MEMPOOL_STAGE);
	} else {
		g_PortalXluFracs = NULL;
	}
}

void portalsReset(void)
{
	if (g_PortalXluFracs) {
		struct prop *prop;
		int i;

		for (i = 0; i < g_NumPortalXluFracs; i++) {
			portalSetXluFrac(i, 1);
			portalSetXluFrac2(i, 1);
		}

		prop = g_Vars.activeprops;

		while (prop) {
			if (prop->type == PROPTYPE_OBJ) {
				struct defaultobj *obj = prop->obj;

				if (obj) {
					if (obj->type == OBJTYPE_TINTEDGLASS) {
						struct tintedglassobj *glass = (struct tintedglassobj *)obj;

						if (glass->portalnum >= 0) {
							portalSetXluFrac(glass->portalnum, 0);
						}
					} else if (obj->type == OBJTYPE_GLASS) {
						struct glassobj *glass = (struct glassobj *)obj;

						if (glass->portalnum >= 0) {
							portalSetXluFrac(glass->portalnum, 0);
						}
					}
				}
			}

			prop = prop->next;
		}

		for (i = 0; i < g_NumPortalXluFracs; i++) {
			portalGetXluFrac(i);
			portalGetXluFrac2(i);
		}
	}
}

void acousticReset(void)
{
	int i;
	int j;
	uint32_t size = ALIGN16(g_Vars.roomcount * sizeof(struct roomacousticdata));
	float range;
	float width;
	float height;
	float depth;
	float halfsurfacearea;

	g_RoomAcousticData = mempAlloc(size, MEMPOOL_STAGE);

	for (i = 0; i < g_Vars.roomcount; i++) {
		bool allgood = true;

		g_RoomAcousticData[i].roomvolume = 1;
		g_RoomAcousticData[i].surfacearea = 1;

		for (j = 0; j < 3; j++) {

			range = g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j];

			if (range > 0) {
				g_RoomAcousticData[i].roomvolume *= (g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j]) / 100;
			} else {
				allgood = false;
			}
		}

		if (allgood) {
			if (g_Rooms[i].bbmin[0] < g_Rooms[i].bbmax[0]) {
				width = g_Rooms[i].bbmax[0] - g_Rooms[i].bbmin[0];
			} else {
				width = -(g_Rooms[i].bbmax[0] - g_Rooms[i].bbmin[0]);
			}

			if (g_Rooms[i].bbmin[1] < g_Rooms[i].bbmax[1]) {
				height = g_Rooms[i].bbmax[1] - g_Rooms[i].bbmin[1];
			} else {
				height = -(g_Rooms[i].bbmax[1] - g_Rooms[i].bbmin[1]);
			}

			if (g_Rooms[i].bbmin[2] < g_Rooms[i].bbmax[2]) {
				depth = g_Rooms[i].bbmax[2] - g_Rooms[i].bbmin[2];
			} else {
				depth = -(g_Rooms[i].bbmax[2] - g_Rooms[i].bbmin[2]);
			}

			halfsurfacearea = width * height + width * depth + height * depth;

			g_RoomAcousticData[i].surfacearea = halfsurfacearea + halfsurfacearea;
		} else {
			g_RoomAcousticData[i].surfacearea = 20000000;
		}
	}

	for (j = 0; j < g_Vars.roomcount; j++) {
		g_RoomAcousticData[j].unk08 = 0;
		g_RoomAcousticData[j].unk04 = g_RoomAcousticData[j].unk08;
	}

	for (j = 0; j < ARRAYCOUNT(var8009dd78); j++) {
		var8009dd78[j].unk00 = -1;
		var8009dd78[j].unk04 = 0;
	}
}
