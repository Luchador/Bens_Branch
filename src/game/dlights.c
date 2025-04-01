#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/dlights.h"
#include "game/debug.h"
#include "game/gfxmemory.h"
#include "game/propsnd.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/player.h"
#include "game/smoke.h"
#include "game/sparks.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/zbuf.h"
#include "game/mplayer/scenarios.h"
#include "game/portal.h"
#include "game/propobj.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/snd.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_17ce0.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#include "platform.h"

int *var8009cad0;
int *var8009cad8;
int g_NumPortals;
int var8009cae0;
int var8009cae4;
float (*var8009cae8)(int roomnum, float mult, int portalnum1, int portalnum2); // function pointer
uint8_t var8009caec;
uint8_t var8009caed;
uint8_t var8009caee;
uint8_t g_NVChrHighlight;
uint8_t g_NVChrBrightness;

struct var80061420 *var80061420 = NULL;
uint32_t var80061424 = 0x00000000;
struct coord *var80061428 = NULL;
uint16_t **var8006142c = NULL;
uint16_t **var80061430 = NULL;
float *var80061434 = NULL;
bool *g_IsPortalClosed = NULL;
float var8006143c = 50;
uint32_t var80061444 = 1;
uint32_t var80061448 = 0x00000000;
bool g_IsSwitchingGoggles = false;
uint32_t var80061450 = 0x00000000;
uint32_t var80061454 = 0xffffffff;
int g_LightsPrevTickMode = 0;

uint32_t roomGetUpperAndLowerPortals(int portalnum1, int portalnum2)
{
	if (portalnum1 != portalnum2) {
		int upper = (portalnum1 > portalnum2) ? portalnum1 : portalnum2;
		int lower = (portalnum1 < portalnum2) ? portalnum1 : portalnum2;

		return var80061430[upper][lower];
	}

	return 0;
}

struct light *roomGetLight(int roomnum, int lightnum)
{
	return (struct light *)&g_BgLightsFileData[(g_Rooms[roomnum].lightindex + lightnum) * 0x22];
}

uint8_t roomGetFinalBrightness(int roomnum)
{
	int brightness = g_Rooms[roomnum].br_flash + g_Rooms[roomnum].br_settled_regional;

	if (brightness > 255) {
		brightness = 255;
	}

	if (brightness < 0) {
		brightness = 0;
	}

	return brightness;
}

uint8_t roomGetFinalBrightnessForPlayer(int roomnum)
{
	int brightness = g_Rooms[roomnum].br_flash;

	if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
		brightness += var8009caec;
	} else {
		brightness += g_Rooms[roomnum].br_settled_regional;
	}

	if (brightness > 255) {
		brightness = 255;
	}

	if (brightness < 0) {
		brightness = 0;
	}

	return brightness;
}

uint8_t func0f000b18(uint32_t arg0)
{
	return 255;
}

uint8_t roomGetSettledRegionalBrightnessForPlayer(int roomnum)
{
	uint32_t brightness;

	if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
		return var8009caec;
	}

	if (g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) {
		brightness = g_Rooms[roomnum].br_settled_regional;
	} else {
		brightness = 255;
	}

	return brightness;
}

uint8_t roomGetSettledLocalBrightness(int room)
{
	return g_Rooms[room].br_settled_local & 0xff;
}

int roomGetFlashBrightness(int roomnum)
{
	if (g_Rooms[roomnum].br_flash > 255) {
		return 255;
	}

	if (g_Rooms[roomnum].br_flash < 0) {
		return 0;
	}

	return (g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) ? g_Rooms[roomnum].br_flash : 0;
}

float roomGetLightOpCurFrac(int roomnum)
{
	return g_Rooms[roomnum].lightop_cur_frac;
}

float roomGetFinalBrightnessFrac(int roomnum)
{
	float frac = (g_Rooms[roomnum].br_flash + g_Rooms[roomnum].br_settled_regional) / (1.0f / 255.0f);

	if (frac > 1) {
		frac = 1;
	}

	if (frac < 0) {
		frac = 0;
	}

	return frac;
}

float roomGetSettledRegionalBrightnessFrac(int roomnum)
{
	return g_Rooms[roomnum].br_settled_regional / 255.0f;
}

float roomGetSettledLocalBrightnessFrac(int roomnum)
{
	return g_Rooms[roomnum].br_settled_local / 255.0f;
}

/**
 * The resulting position is not a world position. It is relative to the room.
 */
bool lightGetBboxCentre(int roomnum, uint32_t lightnum, struct coord *pos)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	int i;
	light += lightnum;

	pos->x = 0;
	pos->y = 0;
	pos->z = 0;

	for (i = 0; i < ARRAYCOUNT(light->bbox); i++) {
		pos->x += light->bbox[i].x;
		pos->y += light->bbox[i].y;
		pos->z += light->bbox[i].z;
	}

	pos->x *= 0.25f;
	pos->y *= 0.25f;
	pos->z *= 0.25f;

	return true;
}

bool lightIsHealthy(int roomnum, int lightnum)
{
	bool healthy;

	if (roomnum && roomGetLight(roomnum, lightnum)->healthy) {
		healthy = true;
	} else {
		healthy = false;
	}

	return healthy;
}

bool lightIsVulnerable(int roomnum, int lightnum)
{
	struct light *light = roomGetLight(roomnum, lightnum);

	return light->vulnerable;
}

bool lightIsOn(int roomnum, int lightnum)
{
	bool on;

	if (roomnum && roomGetLight(roomnum, lightnum)->on) {
		on = true;
	} else {
		on = false;
	}

	return on;
}

void roomSetFlashBrightness(int roomnum, int value)
{
	g_Rooms[roomnum].br_flash = value;
}

void lightGetDirection(int roomnum, uint32_t lightnum, struct coord *dir)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	light += lightnum;

	dir->x = light->dirx;
	dir->y = light->diry;
	dir->z = light->dirz;
}

void roomSetDefaults(struct room *room)
{
	room->br_light_min = 0;
	room->br_light_max = 255;
	room->br_light_each = 0;
	room->br_settled_local = 128;
	room->br_flash = 0;
	room->br_settled_regional = 0;
	room->lightop = LIGHTOP_NONE;
	room->flags &= ~(ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_LIGHTS_DIRTY | ROOMFLAG_RENDERALWAYS | ROOMFLAG_BRIGHTNESS_CALCED);
	room->volume = 1;
	room->surfacearea = 1;
	room->lightop_cur_frac = 1;
	room->lightop_to_frac = 0;
	room->lightop_from_frac = 0;
	room->lightop_duration240 = 0;
}

Gfx *lightsSetForRoom(Gfx *gdl, RoomNum roomnum)
{
	Lights1 *lights = gfxAllocate(sizeof(Lights1));

	uint8_t brightness = roomGetFinalBrightnessForPlayer(roomnum);
	uint8_t a0 = (uint32_t)(brightness * 0.5882353f);

	lights->a.l.col[0] = a0;
	lights->a.l.col[1] = a0;
	lights->a.l.col[2] = a0;
	lights->a.l.colc[0] = a0;
	lights->a.l.colc[1] = a0;
	lights->a.l.colc[2] = a0;

	lights->l[0].l.col[0] = brightness;
	lights->l[0].l.col[1] = brightness;
	lights->l[0].l.col[2] = brightness;
	lights->l[0].l.colc[0] = brightness;
	lights->l[0].l.colc[1] = brightness;
	lights->l[0].l.colc[2] = brightness;
	lights->l[0].l.dir[0] = 0x4d;
	lights->l[0].l.dir[1] = 0x4d;
	lights->l[0].l.dir[2] = 0x2e;

	gSPSetLights1(gdl++, (*lights));

	gSPLookAtX(gdl++, &camGetLookAt()->l[0]);
	gSPLookAtY(gdl++, &camGetLookAt()->l[1]);

	return gdl;
}

Gfx *lightsSetDefault(Gfx *gdl)
{
	static Lights1 var80061460 = gdSPDefLights1(0x96, 0x96, 0x96, 0xff, 0xff, 0xff, 0x4d, 0x4d, 0x2e);

	gSPSetLights1(gdl++, var80061460);

	gSPLookAtX(gdl++, &camGetLookAt()->l[0]);
	gSPLookAtY(gdl++, &camGetLookAt()->l[1]);

	return gdl;
}

void roomInitLights(int roomnum)
{
	struct room *room = &g_Rooms[roomnum];
	struct light *light;
	int i;

	bgGetRoomBrightnessRange(roomnum, &room->br_light_min, &room->br_light_max);

	room->br_light_min = room->br_light_min / 4;

	if (room->numlights) {
		room->br_light_each = (float)(room->br_light_max - room->br_light_min) / (float)room->numlights;
	} else {
		room->br_light_each = 0;
	}

	if (room->br_light_min > 255) {
		room->br_light_min = 255;
	}

	if (room->br_light_max > 255) {
		room->br_light_max = 255;
	}

	room->br_base = room->br_light_min;

	if (room->numlights == 0) {
		room->br_base += (room->br_light_max - room->br_light_min) * 4 / 5;
	}

	switch (g_Vars.stagenum) {
	case STAGE_EXTRACTION:
	case STAGE_DEFECTION:
		if (roomnum == 0x003d) { // near the top comms hub
			room->br_base = 2;
		}
		break;
	}

	if (((g_StageIndex == STAGEINDEX_INFILTRATION || g_StageIndex == STAGEINDEX_RESCUE || g_StageIndex == STAGEINDEX_ESCAPE || g_StageIndex == STAGEINDEX_MAIANSOS)
				&& roomnum == 0x000f) // freight elevator shaft
			|| ((g_StageIndex == STAGEINDEX_DEFECTION || g_StageIndex == STAGEINDEX_EXTRACTION || g_StageIndex == STAGEINDEX_MBR)
				&& roomnum == 0x0001) // moon
			|| ((g_StageIndex == STAGEINDEX_SKEDARRUINS || g_StageIndex == STAGEINDEX_WAR)
				&& roomnum == 0x0002) // fake sky
			|| (g_StageIndex == STAGEINDEX_ATTACKSHIP
				&& roomnum == 0x0071)) { // planet
		room->flags |= ROOMFLAG_RENDERALWAYS;
	} else {
		room->flags &= ~ROOMFLAG_RENDERALWAYS;
	}

	room->flags |= ROOMFLAG_LIGHTS_DIRTY;

	light = (struct light *)&g_BgLightsFileData[(uint32_t)g_Rooms[roomnum].lightindex * 0x22];

	for (i = 0; i < room->numlights; i++) {
		{
			light->brightness = g_Rooms[roomnum].br_light_each;
			light->sparkable = true;
			light->healthy = true;
			light->on = true;
			light->sparking = false;
			light->vulnerable = true;

			switch (g_Vars.stagenum) {
			case STAGE_CITRAINING:
			case STAGE_DEFENSE:
				if (roomnum == 0x000a) { // firing range
					light->vulnerable = false;
				}
				break;
			case STAGE_DEFECTION:
			case STAGE_EXTRACTION:
				if (roomnum == 0x003e && (i == 0 || i == 1)) { // top comms hub
					light->vulnerable = false;
				}
				break;
			}
		}

		light++;
	}
}

bool lightsHandleHit(struct coord *gunpos, struct coord *hitpos, int roomnum)
{
	int i;
	float f2;
	struct coord spa4;
	struct coord sp98;
	struct coord sp8c;
	struct light *light;

	if (roomnum == 0 || g_Rooms[roomnum].numlights == 0) {
		return false;
	}

	spa4.x = gunpos->x - g_BgRooms[roomnum].pos.x;
	spa4.y = gunpos->y - g_BgRooms[roomnum].pos.y;
	spa4.z = gunpos->z - g_BgRooms[roomnum].pos.z;

	sp98.x = hitpos->x - g_BgRooms[roomnum].pos.x;
	sp98.y = hitpos->y - g_BgRooms[roomnum].pos.y;
	sp98.z = hitpos->z - g_BgRooms[roomnum].pos.z;

	sp8c.x = sp98.x - spa4.x;
	sp8c.y = sp98.y - spa4.y;
	sp8c.z = sp98.z - spa4.z;

	f2 = 2.0f / sqrtf(sp8c.x * sp8c.x + sp8c.y * sp8c.y + sp8c.z * sp8c.z);

	sp8c.x *= f2;
	sp8c.y *= f2;
	sp8c.z *= f2;

	sp98.x += sp8c.x;
	sp98.y += sp8c.y;
	sp98.z += sp8c.z;

	light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];

	for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
		if (light->healthy && light->vulnerable) {
			if (utilsRayIntersectsTriangleS16(&light->bbox[0], &light->bbox[1], &light->bbox[3], NULL, &spa4, &sp98, &sp8c, 0, 0)
					|| utilsRayIntersectsTriangleS16(&light->bbox[1], &light->bbox[2], &light->bbox[3], NULL, &spa4, &sp98, &sp8c, 0, 0)) {
				struct coord soundpos;

				soundpos.x = light->bbox[0].x;
				soundpos.y = light->bbox[0].y;
				soundpos.z = light->bbox[0].z;

				roomSetLightBroken(roomnum, i);
				psCreate(0, 0, SFX_HIT_GLASS, -1, -1, PSFLAG_0400, 0, PSTYPE_NONE, &soundpos, -1.0f, 0, roomnum, -1.0f, -1.0f, -1.0f);
				return true;
			}
		}

		light++;
	}

	return false;
}

void roomSetLightsFaulty(int roomnum, int chance)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	int i;

	if (g_Rooms[roomnum].numlights) {
		for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
			if ((rngRandom() % 100) < chance) {
				light->healthy = false;
				light->on = false;
			}

			light++;
		}
	}

	g_Rooms[roomnum].br_base = 50;
	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

void roomSetLightBroken(int roomnum, int lightnum)
{
	struct light *light = roomGetLight(roomnum, lightnum);
	light->healthy = false;
	light->on = false;

	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

void lightsReset(void)
{
	if (var80061444) {
		func0f004c6c();
	}
}

void func0f001c0c(void)
{
	int i;
	int sp68;
	int table1size;
	int table2size;
	int table3size;
	int table4size;
	int sp54;
	uint8_t *ptr;
	uint8_t *s5;
	uint8_t *sp48;
	int *sp44;
	int j;

	lightsCalculateRoomDimensions();

	for (g_NumPortals = 0; g_BgPortals[g_NumPortals].verticesoffset != 0; g_NumPortals++);

	if (g_NumPortals == 0) {
		return;
	}

	table1size = align16(g_Vars.roomcount * 4);
	table2size = align16(g_NumPortals * 4);
	table3size = align16(g_Vars.roomcount * 4);
	table4size = align16((uint32_t)var8009cae0 * (uint32_t)var8009cae0);
#ifdef PLATFORM_64BIT
	sp68 = align16(g_Vars.roomcount * 8 * 2);
#else
	sp68 = align16(g_Vars.roomcount * 8);
#endif

	mempGetStageFree();

	/**
	 * This lighting initialisation needs to build temporary tables in memory.
	 * The memp system allows freeing of the most recent allocation *only*,
	 * so it's not easy to build these tables and dispose of them afterwards.
	 *
	 * Instead of using memp, it uses the z-buffer as a temporary scratch space.
	 * This is safe because there are no RDP tasks in progress at this point.
	 * The game waits for them to complete before loading the stage.
	 *
	 * Note that for some stages the z-buffer allocation must be higher than the
	 * lo-res size to support the lighting needs here.
	 */
	ptr = zbufGetAllocation();

	var80061434 = (float *)ptr;
	ptr += table1size;

	g_IsPortalClosed = (bool *)ptr;
	ptr += table2size;

	sp44 = (int *)(ptr);
	ptr += table3size;

	sp48 = (uint8_t *)ptr;
	ptr += table4size;

	s5 = (uint8_t *)ptr;

	var80061420 = mempAlloc(sp68, MEMPOOL_STAGE);

	for (i = 0; i < g_NumPortals; i++) {
		if (PORTAL_IS_CLOSED(i)) {
			g_IsPortalClosed[i] = false;
		} else {
			g_IsPortalClosed[i] = true;
		}
	}

	if (g_Vars.stagenum == STAGE_EXTRACTION || g_Vars.stagenum == STAGE_DEFECTION) {
		g_IsPortalClosed[98] = false;
		g_IsPortalClosed[100] = false;
	}

	func0f00215c(sp48);

	for (i = 1, table3size = 0; i < g_Vars.roomcount; i++) {
		sp44[i] = utilCompressZeroRuns((void *)(i * var8009cae0 + sp48), g_Vars.roomcount, (void *)(&s5[i * var8009cae0]), 1);
		table3size += align4(sp44[i]);
	}

	ptr = mempAlloc(align16(table3size), MEMPOOL_STAGE);

	sp68 += align16(table3size);

	sp54 = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		int size = align4(sp44[i]);

		var80061420[i].unk00 = ptr;

		ptr += size;
		sp54 += size;

		for (j = 0; j < sp44[i]; j++) {
			var80061420[i].unk00[j] = *(&s5[i * var8009cae0] + j);
		}
	}

	table3size = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		sp44[i] = utilCompressZeroRuns((void *)(sp48 + i), g_Vars.roomcount, (void *)(&s5[i * var8009cae0]), var8009cae0);

		table3size += align4(sp44[i]);
	}

	ptr = mempAlloc(align16(table3size), MEMPOOL_STAGE);

	align16(table3size);

	for (i = 1; i < g_Vars.roomcount; i++) {
		var80061420[i].unk04 = ptr;

		ptr += align4(sp44[i]);

		for (j = 0; j < sp44[i]; j++) {
			var80061420[i].unk04[j] = *(&s5[i * var8009cae0] + j);
		}
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags |= ROOMFLAG_ONSCREEN;
	}

	lightingTick();

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags &= ~ROOMFLAG_ONSCREEN;
	}
}

float func0f002334(int roomnum, float mult, int portalnum1, int portalnum2);

void func0f00215c(uint8_t *arg0)
{
	int i;
	int j;

	var8009cae8 = &func0f002334;

	var8006143c = 50.0f;
	var8009cae4 = 20;

	for (i = 1; i < g_Vars.roomcount; i++) {
		uint8_t *ptr = &arg0[i * var8009cae0];

		func0f00259c(i);

		for (j = 0; j < 1; j++) {
			ptr[j] = 0;
		}

		for (j = 1; j < g_Vars.roomcount; j++) {
			if (var80061434[i] < var80061434[j]) {
				var80061434[j] = var80061434[i];
			}

			ptr[j] = var80061434[j];
		}
	}
}

float func0f002334(int roomnum, float mult, int portalnum1, int portalnum2)
{
	float surfacearea = 0;
	float result;

	if (portalnum1 != -1) {
		surfacearea = bgCalculatePortalSurfaceArea(portalnum1);
	}

	result = (bgCalculatePortalSurfaceArea(portalnum2) / (g_Rooms[roomnum].surfacearea - surfacearea)) * mult;
	return result;
}

void lightsCalculateRoomDimensions(void)
{
	int i;
	int j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		bool valid = true;

		g_Rooms[i].volume = 1.0f;
		g_Rooms[i].surfacearea = 1.0f;

		for (j = 0; j < 3; j++) {
			float diff = g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j];

			if (diff > 0.0f) {
				g_Rooms[i].volume *= (g_Rooms[i].bbmax[j] - g_Rooms[i].bbmin[j]) / 100.0f;
			} else {
				valid = false;
			}
		}

		g_Rooms[i].volume += 1.0f;

		if (g_Rooms[i].volume > 60.0f) {
			g_Rooms[i].volume = 60.0f;
		}

		if (valid) {
			float xdiff = g_Rooms[i].bbmax[0] - g_Rooms[i].bbmin[0];
			float ydiff = g_Rooms[i].bbmax[1] - g_Rooms[i].bbmin[1];
			float zdiff = g_Rooms[i].bbmax[2] - g_Rooms[i].bbmin[2];

			if (!(xdiff > 0)) {
				xdiff = -xdiff;
			}

			if (!(ydiff > 0)) {
				ydiff = -ydiff;
			}

			if (!(zdiff > 0)) {
				zdiff = -zdiff;
			}

			g_Rooms[i].surfacearea = 2.0f * (xdiff * ydiff + xdiff * zdiff + ydiff * zdiff);
		} else {
			g_Rooms[i].surfacearea = 20000000.0f;
		}
	}
}

void func0f00259c(int roomnum)
{
	int i;
	float sp58;
	float f20 = 0.0f;

	for (i = 0; i < g_Vars.roomcount; i++) {
		var80061434[i] = 0.0f;
	}

	var80061434[roomnum] = sqrtf(g_Rooms[roomnum].volume) * 255.0f;

	if (g_Rooms[roomnum].numportals != 0) {
		func0f002844(roomnum, var80061434[roomnum], 0, -1);
	}

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		f20 += bgCalculatePortalSurfaceArea(g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i]);
	}

	sp58 = (g_Rooms[roomnum].surfacearea - f20) / g_Rooms[roomnum].surfacearea;

	for (i = 1; i < g_Vars.roomcount; i++) {
		var80061434[i] *= 3.0f / sqrtf(g_Rooms[i].volume);
	}

	if (var80061434[roomnum] > 255.0f) {
		var80061434[roomnum] = 255.0f;
	}

	if (sp58 < 0.1f) {
		g_Rooms[roomnum].br_light_min = g_Rooms[roomnum].br_light_max >> 1;
		var80061434[roomnum] = g_Rooms[roomnum].br_light_max;
	}
}

void func0f002844(int roomnum, float arg1, int arg2, int portalnum)
{
	int i;
	int otherroomnum = -1;

	if (portalnum != -1) {
		if (roomnum == g_BgPortals[portalnum].roomnum1) {
			otherroomnum = (int) g_BgPortals[portalnum].roomnum2;
		} else {
			otherroomnum = (int) g_BgPortals[portalnum].roomnum1;
		}
	}

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		int iterportalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		int iterroomnum;

		if (g_IsPortalClosed[iterportalnum]) {
			if (roomnum == g_BgPortals[iterportalnum].roomnum1) {
				iterroomnum = g_BgPortals[iterportalnum].roomnum2;
			} else {
				iterroomnum = g_BgPortals[iterportalnum].roomnum1;
			}

			if (iterroomnum != otherroomnum) {
				float f0 = var8009cae8(roomnum, arg1, portalnum, iterportalnum);

				if (f0 > var8006143c && arg2 < var8009cae4) { // f0 > 50.0 &&  arg2 < 20
					var80061434[roomnum] -= f0;
					var80061434[iterroomnum] += f0;

					if (var80061434[roomnum] < 0.0f) {
						var80061434[roomnum] = 0.0f;
					}

					func0f002844(iterroomnum, f0, arg2 + 1, iterportalnum);
				}
			}
		}
	}
}

void func0f002a98(void)
{
	int i;

	var8009cae0 = align4(g_Vars.roomcount);
	g_LightsPrevTickMode = 0;
	g_Vars.remakewallhitvtx = 0;

	for (i = 1; i < g_Vars.roomcount; i++) {
		roomSetDefaults(&g_Rooms[i]);
		roomInitLights(i);
	}

	var80061420 = NULL;
}

void roomSetLightsOn(int roomnum, int enable)
{
	struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[roomnum].lightindex * 0x22];
	int i;

	if (g_Rooms[roomnum].numlights) {
		for (i = 0; i < g_Rooms[roomnum].numlights; i++) {
			if (light->healthy) {
				light->on = enable;
			}

			light++;
		}
	}

	if (enable) {
		g_Rooms[roomnum].flags &= ~ROOMFLAG_LIGHTSOFF;
	} else {
		g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTSOFF;
	}

	g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
}

void roomSetLightOp(int roomnum, int operation, uint8_t br_to, uint8_t br_from, uint8_t duration60)
{
	if (cheatIsActive(CHEAT_PERFECTDARKNESS) == false) {
		g_Rooms[roomnum].lightop = operation;

		switch (operation) {
		case LIGHTOP_SET:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			break;
		case LIGHTOP_SETRANDOM:
			g_Rooms[roomnum].lightop_to_frac = br_to;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = duration60;
			break;
		case LIGHTOP_TRANSITION:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = duration60 * 4;
			break;
		case LIGHTOP_SINELOOP:
			g_Rooms[roomnum].lightop_to_frac = br_to * 0.01f;
			g_Rooms[roomnum].lightop_from_frac = br_from * 0.01f;
			g_Rooms[roomnum].lightop_duration240 = duration60 * 4.0f;
			g_Rooms[roomnum].lightop_timer240 = 0;
			break;
		case LIGHTOP_HIGHLIGHT:
			break;
		}
	}
}

bool lightTickBroken(int roomnum, int lightnum)
{
	struct light *light = (struct light *)(g_BgLightsFileData + ((g_Rooms[roomnum].lightindex + lightnum) * 0x22));

	if (!light->sparkable) {
		return false;
	}

	if (light->sparking) {
		if ((rngRandom() % 8) == 0) {
			light->sparking = false;
		} else if ((rngRandom() % 2) == 0) {
			struct coord spc8;
			struct coord spbc;
			struct coord spb0;
			struct coord spa4;
			struct coord sp98;
			struct coord sp8c;
			struct coord sp80;
			struct coord centre;
			float rand1 = 2.0f * RANDOMFRAC() - 1.0f; // range -1 to 1
			float rand2 = 2.0f * RANDOMFRAC() - 1.0f; // range -1 to 1
			int sparktype = -1;
			RoomNum smokerooms[2];
			struct bgroom *room;

			spc8.x = light->bbox[1].x - light->bbox[0].x;
			spc8.y = light->bbox[1].y - light->bbox[0].y;
			spc8.z = light->bbox[1].z - light->bbox[0].z;

			spc8.x = rand1 * spc8.x;
			spc8.y = rand1 * spc8.y;
			spc8.z = rand1 * spc8.z;

			spbc.x = light->bbox[2].x - light->bbox[0].x;
			spbc.y = light->bbox[2].y - light->bbox[0].y;
			spbc.z = light->bbox[2].z - light->bbox[0].z;

			spbc.x = rand2 * spbc.x;
			spbc.y = rand2 * spbc.y;
			spbc.z = rand2 * spbc.z;

			// @bug? These all use x
			sp98.x = light->bbox[0].x + spc8.x + spbc.x;
			sp98.y = light->bbox[0].x + spc8.y + spbc.y;
			sp98.z = light->bbox[0].x + spc8.z + spbc.z;

			sp8c.x = light->dirx;
			sp8c.y = light->diry;
			sp8c.z = light->dirz;

			sp80.x = -sp8c.x;
			sp80.y = -sp8c.y;
			sp80.z = -sp8c.z;

			utilsNormalizeVector(&sp98, &spa4, 1546, "dlights.c");

			spa4.x += sp80.x;
			spa4.y += sp80.y;
			spa4.z += sp80.z;

			utilsNormalizeVector(&spa4, &spa4, 1548, "dlights.c");

			room = (void *) (roomnum * sizeof(struct bgroom));

			switch (rngRandom() % 4) {
			case 0:
				if (roomnum && roomnum && roomnum);
				sparktype = SPARKTYPE_LIGHT1;
				break;
			case 1:
				sparktype = SPARKTYPE_LIGHT2;
				break;
			case 2:
				sparktype = SPARKTYPE_LIGHT3;
				break;
			case 3:
				sparktype = SPARKTYPE_LIGHT4;
				break;
			}

			lightGetBboxCentre(roomnum, lightnum, &centre);

			room = (void *) ((uint8_t *) g_BgRooms + (uintptr_t) room);
			centre.x += room->pos.x;
			centre.y += room->pos.y;
			centre.z += room->pos.z;

			sparksCreate(roomnum, NULL, &centre, &spa4, &sp8c, sparktype);

			if ((rngRandom() % 4) == 0) {
				smokerooms[0] = roomnum;
				smokerooms[1] = -1;

				smokeCreateSimple(&centre, smokerooms, SMOKETYPE_BULLETIMPACT);
			}

			roomFlashLighting(roomnum, 64, 80);
			psCreate(NULL, NULL, psGetRandomSparkSound(), -1, -1, PSFLAG_0400, 0, PSTYPE_FOOTSTEP, &centre, -1.0f, 0, roomnum, -1.0f, -1.0f, -1.0f);
			return true;
		}
	} else {
		if ((rngRandom() % 80) == 0) {
			light->sparking = true;
		}
	}

	return false;
}

void lightingTick(void)
{
	int i;

	roomsTickLighting();

	if (g_Vars.remakewallhitvtx) {
		wallhitsRecolour();

		g_Vars.remakewallhitvtx = false;

		for (i = 1; i < g_Vars.roomcount; i++) {
			g_Rooms[i].flags &= ~ROOMFLAG_NEEDRESHADE;
		}
	}
}

void lightsConfigureForPerfectDarknessCutscene(void)
{
	int i;
	int j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];
		g_Rooms[i].lightop = LIGHTOP_SET;
		g_Rooms[i].lightop_to_frac = 0.5f;

		for (j = 0; j < g_Rooms[i].numlights; j++) {
			light->sparkable = rngRandom() % 2 ? true : false;
			light->healthy = true;
			light->on = true;
			light->sparking = false;
			light->vulnerable = true;
			light->brightness = g_Rooms[i].br_light_each;

			light++;
		}
	}
}

void lightsConfigureForPerfectDarknessGameplay(void)
{
	int i;
	int j;

	for (i = 0; i < g_Vars.roomcount; i++) {
		struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];
		g_Rooms[i].lightop = LIGHTOP_SET;
		g_Rooms[i].lightop_to_frac = 0;

		for (j = 0; j < g_Rooms[i].numlights; j++) {
			light->sparkable = rngRandom() % 2 ? true : false;
			light->healthy = false;
			light->on = false;
			light->sparking = false;
			light->vulnerable = false;
			light->brightness = 0;

			light++;
		}
	}
}

void lightsTickPerfectDarkness(void)
{
	if (g_Vars.tickmode != g_LightsPrevTickMode) {
		if (TICKMODE_CUTSCENE == g_Vars.tickmode && TICKMODE_CUTSCENE != g_LightsPrevTickMode) {
			lightsConfigureForPerfectDarknessCutscene();
		} else if (TICKMODE_NORMAL == g_Vars.tickmode && TICKMODE_NORMAL != g_LightsPrevTickMode) {
			lightsConfigureForPerfectDarknessGameplay();
		}

		g_LightsPrevTickMode = g_Vars.tickmode;
	}
}

void roomsTickLighting(void)
{
	int i;
	int numprocessed = 0;
	int j;
	bool wasdirty = false;
	struct light *light;
	float amount;
	int timer240;
	float angle;
	float average;

	if (cheatIsActive(CHEAT_PERFECTDARKNESS)) {
		lightsTickPerfectDarkness();
	}

	if (var80061420 == NULL) {
		return;
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags &= ~ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
	}

	for (i = 1; i < g_Vars.roomcount; i++) {
		// Tick any light operations
		g_Rooms[i].lightop_timer240 -= g_Vars.lvupdate240;

		switch (g_Rooms[i].lightop) {
		case LIGHTOP_SET:
			g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_to_frac;

			if (g_Rooms[i].lightop_cur_frac < 0.0f) {
				g_Rooms[i].lightop_cur_frac = 0.0f;
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			roomSetLightOp(i, LIGHTOP_NONE, 0, 0, 0);
			break;
		case LIGHTOP_SETRANDOM:
			if (g_Rooms[i].lightop_timer240 < 0) {
				if (RANDOMFRAC() * 100.0f < g_Rooms[i].lightop_to_frac) {
					g_Rooms[i].lightop_cur_frac = 1.0f;
				} else {
					g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_from_frac;

					if (g_Rooms[i].lightop_cur_frac < 0.0f) {
						g_Rooms[i].lightop_cur_frac = 0.0f;
					}
				}

				g_Rooms[i].lightop_timer240 = g_Rooms[i].lightop_duration240;
				g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			}
			break;
		case LIGHTOP_TRANSITION:
			if (g_Rooms[i].lightop_timer240 > 0) {
				g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_from_frac;
				g_Rooms[i].lightop_cur_frac += g_Rooms[i].lightop_timer240 / g_Rooms[i].lightop_duration240 * (g_Rooms[i].lightop_to_frac - g_Rooms[i].lightop_from_frac);

				if (g_Rooms[i].lightop_cur_frac < 0.0f) {
					g_Rooms[i].lightop_cur_frac = 0.0f;
				}
			} else {
				roomSetLightOp(i, LIGHTOP_NONE, 0, 0, 0);
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		case LIGHTOP_SINELOOP:
			timer240 = g_Rooms[i].lightop_timer240 > 0 ? g_Rooms[i].lightop_timer240 : -g_Rooms[i].lightop_timer240;

			angle = (timer240 % (int) g_Rooms[i].lightop_duration240) * M_TAU / g_Rooms[i].lightop_duration240;
			average = (g_Rooms[i].lightop_to_frac + g_Rooms[i].lightop_from_frac) * 0.5f;

			g_Rooms[i].lightop_cur_frac = g_Rooms[i].lightop_to_frac + (cosf(angle) + 1.0f) * average;

			if (g_Rooms[i].lightop_cur_frac < 0.0f) {
				g_Rooms[i].lightop_cur_frac = 0.0f;
			}

			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		case LIGHTOP_HIGHLIGHT:
			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
			break;
		}

		if (g_IsSwitchingGoggles) {
			g_Rooms[i].flags |= ROOMFLAG_LIGHTS_DIRTY;
		}

		if (g_Rooms[i].flags & ROOMFLAG_LIGHTS_DIRTY) {
			// Calculate the settled local brightness
			if (g_Rooms[i].numlights != 0) {
				int numlightson = 0;
				struct light *light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];

				for (j = 0; j < g_Rooms[i].numlights; j++) {
					if (light->on) {
						numlightson++;
					}

					light++;
				}

				if (g_Rooms[i].flags & ROOMFLAG_LIGHTSOFF) {
					amount = 2.0f;
				} else if (numlightson != 0) {
					amount = g_Rooms[i].br_base;
				} else {
					amount = (float)g_Rooms[i].br_base / 2;
				}
			} else {
				if (g_Rooms[i].flags & ROOMFLAG_LIGHTSOFF) {
					amount = 2.0f;
				} else {
					amount = g_Rooms[i].br_base;
				}
			}

			amount *= g_Rooms[i].lightop_cur_frac;
			g_Rooms[i].br_settled_local = amount;

			// Add brightness for each light that is on
			light = (struct light *)&g_BgLightsFileData[g_Rooms[i].lightindex * 0x22];

			for (j = 0; j < g_Rooms[i].numlights; j++) {
				if (light->on) {
					amount = g_Rooms[i].lightop_cur_frac * light->brightness;
					g_Rooms[i].br_settled_local += (int)amount;
				}

				light++;
			}

			if (g_Rooms[i].br_settled_local > 255) {
				g_Rooms[i].br_settled_local = 255;
			}

			g_Rooms[i].flags &= ~ROOMFLAG_LIGHTS_DIRTY;

			wasdirty = 1;
		}

		// Tick any flash lighting
		if (g_Rooms[i].br_flash != 0) {
			int increment = g_Vars.lvupdate240 * 2;

			if (var80061420 != NULL) {
				int spa0 = 0;
				int sp9c = 0;

				int ret = untilCompressRoomData(var80061420[i].unk04, &spa0, &sp9c);

				while (ret != -1) {
					if (ret != 0) {
						g_Rooms[sp9c].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
					}

					ret = untilCompressRoomData(var80061420[i].unk04, &spa0, &sp9c);
				}
			}

			if (g_Rooms[i].br_flash > 0) {
				if (increment > g_Rooms[i].br_flash) {
					increment = g_Rooms[i].br_flash;
				}

				g_Rooms[i].br_flash -= increment;
			} else {
				// @bug: In this branch br_flash is zero or negative.
				// Both instances of br_flash should be negated here.
				if (increment < g_Rooms[i].br_flash) {
					increment = g_Rooms[i].br_flash;
				}

				g_Rooms[i].br_flash += increment;
			}

			g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_TEMP;
		}

		g_Rooms[i].flags &= ~ROOMFLAG_LIGHTS_DIRTY;
	}

	// If switching googles, mark all rooms as dirty so their brightness will be
	// recalculated the next time they appear on screen.
	if (g_IsSwitchingGoggles || wasdirty) {
		for (i = 1; i < g_Vars.roomcount; i++) {
			g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_PERM;
		}
	}

	// Calculate settled regional lighting for any rooms that need it
	for (i = 1; i < g_Vars.roomcount; i++) {
		if (i != 0) {
			if ((g_Rooms[i].flags & ROOMFLAG_RENDERALWAYS) || (g_Rooms[i].flags & (ROOMFLAG_ONSCREEN | ROOMFLAG_STANDBY))) {
				if (g_Rooms[i].flags & (ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_BRIGHTNESS_DIRTY_TEMP)) {
					int sum = 0;
					int sp90 = 0;
					int sp8c = 0;

					int ret = untilCompressRoomData(var80061420[i].unk00, &sp90, &sp8c);

					while (ret != -1) {
						if (sp8c != 0) {
							int add = 0;

							if (sp8c) {
								add += (int)((1.0f / 255.0f) * ret * g_Rooms[sp8c].br_settled_local);
							}

							sum += add;
						}

						ret = untilCompressRoomData(var80061420[i].unk00, &sp90, &sp8c);
					}

					if (sum > 255) {
						sum = 255;
					}

					g_Rooms[i].br_settled_regional = sum;
					g_Rooms[i].flags |= ROOMFLAG_BRIGHTNESS_CALCED;
					g_Rooms[i].flags |= ROOMFLAG_NEEDRESHADE;
					g_Rooms[i].flags &= ~(ROOMFLAG_BRIGHTNESS_DIRTY_PERM | ROOMFLAG_BRIGHTNESS_DIRTY_TEMP);

					if (g_Rooms[i].lightop == LIGHTOP_HIGHLIGHT) {
						int r = roomGetFinalBrightnessForPlayer(i);
						int g = r;
						int b = r;

						scenarioHighlightRoom(i, &r, &g, &b);

						g_Rooms[i].highlightfrac_r = r * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_g = g * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_b = b * (1.0f / 255.0f);
					} else {
						g_Rooms[i].highlightfrac_r = roomGetFinalBrightnessForPlayer(i) * (1.0f / 255.0f);
						g_Rooms[i].highlightfrac_g = g_Rooms[i].highlightfrac_r;
						g_Rooms[i].highlightfrac_b = g_Rooms[i].highlightfrac_r;
					}

					numprocessed++;
				}
			}
		}
	}

	if (g_Vars.joydisableframestogo <= 0 && (numprocessed || g_IsSwitchingGoggles)) {
		struct prop *prop = g_Vars.activeprops;

		while (prop) {
			if (prop->type == PROPTYPE_CHR) {
				for (i = 0; prop->rooms[i] != -1; i++) {
					if (g_Rooms[prop->rooms[i]].flags & ROOMFLAG_NEEDRESHADE) {
						if (2 == g_IsSwitchingGoggles) {
							struct chrdata *chr = prop->chr;
							propCalculateShadeColour(chr->prop, chr->nextcol, chr->floorcol);
						} else {
							struct chrdata *chr = prop->chr;
							chr->unk32c_18 = true;
						}
					}
				}
			}

			prop = prop->next;
		}

		g_Vars.remakewallhitvtx = 0xf;
	}

	if (g_IsSwitchingGoggles) {
		g_IsSwitchingGoggles = false;
	}
}

void lightsTick(void)
{
	struct hand *hand1 = &g_Vars.currentplayer->hands[0];
	struct hand *hand2 = &g_Vars.currentplayer->hands[1];

	func0f005bb0();

	if (hand1->flashon || hand2->flashon) {
		roomFlashLighting(g_Vars.currentplayer->prop->rooms[0], 64, 80);
	}
}

/**
 * Set a lighting flash in the given room and its neighbours.
 *
 * The room must not have ROOMFLAG_OUTDOORS.
 */
void roomFlashLighting(int roomnum, int start, int limit)
{
	if (var80061420 && !(g_Rooms[roomnum].flags & ROOMFLAG_OUTDOORS ? 1 : 0)) {
		int value;
		int sp78 = 0;
		int neighbournum = 0;

		value = untilCompressRoomData(var80061420[roomnum].unk04, &sp78, &neighbournum);

		while (value != -1) {
			float increment = value * (1.0f / 255.0f) * start * 5.0f;

			if (start > 0) {
				if (increment > start) {
					increment = start;
				}
			} else {
				if (increment < start) {
					increment = start;
				}
			}

			// @bug: Should be checking neighbournum flags, not roomnum
			//if (!(g_Rooms[roomnum].flags & ROOMFLAG_OUTDOORS ? 1 : 0)) {
			if (!(g_Rooms[neighbournum].flags & ROOMFLAG_OUTDOORS ? 1 : 0)) { // Fix
				roomFlashLocalLighting(neighbournum, increment, limit);
			}

			value = untilCompressRoomData(var80061420[roomnum].unk04, &sp78, &neighbournum);
		}
	}
}

void roomFlashLocalLighting(int roomnum, int increment, int limit)
{
	if (roomnum) {
		if (g_Rooms[roomnum].flags & ROOMFLAG_ONSCREEN) {
			if (increment > 0) {
				// Increasing
				if (g_Rooms[roomnum].br_flash < limit) {
					g_Rooms[roomnum].br_flash += increment;

					if (g_Rooms[roomnum].br_flash > limit) {
						g_Rooms[roomnum].br_flash = limit;
					}
				}
			} else {
				// Decreasing
				if (g_Rooms[roomnum].br_flash > limit) {
					g_Rooms[roomnum].br_flash += increment;

					if (g_Rooms[roomnum].br_flash < limit) {
						g_Rooms[roomnum].br_flash = limit;
					}
				}
			}
		}
	}
}

void roomHighlight(int roomnum)
{
	int i;
	int tmpr;
	int tmpg;
	int tmpb;
	int alpha;
	int red;
	int green;
	int blue;
	int extra;
	int numcolours;
	Col *src;
	Col *dst;
	uint8_t br_settled_regional;
	int max;
	float mult;

	if (g_BgFrameCount != g_Rooms[roomnum].hlupdatedframe && g_Rooms[roomnum].loaded240 != 0) {
		g_Rooms[roomnum].hlupdatedframe = g_BgFrameCount;

		if ((g_Rooms[roomnum].flags & ROOMFLAG_BRIGHTNESS_CALCED) == 0) {
			g_Rooms[roomnum].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_PERM;
		}

		br_settled_regional = roomGetSettledRegionalBrightnessForPlayer(roomnum);
		numcolours = g_Rooms[roomnum].gfxdata->numcolours;
		dst = gfxAllocateColours(numcolours);
		g_Rooms[roomnum].colours = dst;

		extra = g_Rooms[roomnum].br_flash;
		src = (Col *)((uintptr_t)g_Rooms[roomnum].gfxdata->vertices + g_Rooms[roomnum].gfxdata->numvertices * sizeof(Vtx));
		src = (Col *)ALIGN8((uintptr_t)src);

		if (g_Rooms[roomnum].flags & ROOMFLAG_RENDERALWAYS) {
			g_Rooms[roomnum].colours = src;
			return;
		}

		for (i = 0; i < numcolours; i++) {
			// @bug? Why is this looking up vertices using a colour index?
			if (g_Rooms[roomnum].gfxdata->vertices[i].flags & 0x01) {
				dst[i].r = src[i].r;
				dst[i].g = src[i].g;
				dst[i].b = src[i].b;
				//dst[i].a = src[i].a * (1.0f / 255.0f * br_settled_regional); // Ben's comment: this line was causing some walls to become transparent when lights in a room were destroyed
				dst[i].a = src[i].a;
			} else {
				if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
					tmpr = tmpg = tmpb = (src[i].r > src[i].g && src[i].r > src[i].b)
						? src[i].r
						: src[i].g > src[i].b ? src[i].g : src[i].b;
				} else {
					tmpr = src[i].r;
					tmpg = src[i].g;
					tmpb = src[i].b;
				}

				alpha = src[i].a;
				max = tmpr;

				if (tmpg > max) {
					max = tmpg;
				}

				if (tmpb > max) {
					max = tmpb;
				}

				if (max > br_settled_regional) {
					mult = br_settled_regional / (float)max;
				} else {
					mult = 1.0f;
				}

				red = tmpr * mult;
				green = tmpg * mult;
				blue = tmpb * mult;
				max *= mult;

				if (USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) {
					extra = 0;
				} else if (extra + max > 285) {
					extra = 285 - max;
				}

				if (red + green + blue != 0
						|| g_Rooms[roomnum].lightop_cur_frac == 0.0f
						|| (g_Rooms[roomnum].flags & ROOMFLAG_LIGHTSOFF)) {
					red += extra;
					green += extra;
					blue += extra;
				}

				if (g_Rooms[roomnum].lightop == LIGHTOP_HIGHLIGHT) {
					scenarioHighlightRoom(roomnum, &red, &green, &blue);
				}

				if (red > 255) {
					red = 255;
				}

				if (green > 255) {
					green = 255;
				}

				if (blue > 255) {
					blue = 255;
				}

				if (red < 0) {
					red = 0;
				}

				if (green < 0) {
					green = 0;
				}

				if (blue < 0) {
					blue = 0;
				}

				// Sets room color
				dst[i].r = red;
				dst[i].g = green;
				dst[i].b = blue;
				dst[i].a = alpha;
			}
		}
	}
}

void func0f004c6c(void)
{
	int sp44;
	int sp40;
	int sp3c;
	int sp38;
	int sp34;
	int i;
	int j;
	int s4;
	uint8_t *ptr;
	uint8_t *backupptr;

#ifdef PLATFORM_64BIT
	sp44 = align16(0x2000 * 2);
	sp40 = align16(g_NumPortals * 4 * 2);
	sp3c = align16(g_NumPortals * 0xc * 2);
	sp38 = align16(g_NumPortals * 4 * 2);
	sp34 = align16(g_NumPortals * 2 * 2);
#else
	sp44 = align16(0x2000);
	sp40 = align16(g_NumPortals * 4);
	sp3c = align16(g_NumPortals * 0xc);
	sp38 = align16(g_NumPortals * 4);
	sp34 = align16(g_NumPortals * 2);
#endif

	for (i = 0, s4 = sp38; i < g_NumPortals; i++) {
		if (i != 0) {
			s4 += i * 2;
		}
	}

	s4 = align16(s4);
	ptr = mempAlloc(align16(s4), MEMPOOL_STAGE);
	var80061430 = (void *)ptr;

	ptr += sp38;

	for (i = 0; i < g_NumPortals; i++) {
		if (i != 0) {
			var80061430[i] = (void *)ptr;
			ptr += i * 2;
		} else {
			var80061430[i] = 0;
		}
	}

	s4 += sp3c;
	s4 += sp44;
	s4 += sp40;
	s4 += sp38;
	s4 += g_NumPortals * sp34;

	align16((int)s4);

	ptr = mempGetNextStageAllocation();
	var8009cad0 = (void *)ptr;
	ptr += sp44;

	var8009cad8 = (void *)ptr;
	ptr += sp40;

	var8006142c = (void *)ptr;
	ptr += sp38;

	backupptr = ptr;

	ptr += g_NumPortals * sp34;
	var80061428 = (void *)ptr;
	ptr = backupptr;

	s4 = sp38;

	for (i = 0; i < g_NumPortals; i++) {
		var8006142c[i] = (void *)ptr;
		ptr += sp34;
		s4 += sp34;

		for (j = 0; j < g_NumPortals; j++) {
			var8006142c[i][j] = 0x8009;
		}
	}

	for (i = 0; i < g_NumPortals; i++) {
		var8009cad8[i] = portalGetXluFrac(i) > 0.5f;

		portalGetAvgVertexPos(i, &var80061428[i]);
	}

	if (g_Vars.stagenum == STAGE_INVESTIGATION) {
		var8009cad8[0] = 1;
	}

	for (i = 0; i < g_NumPortals; i++) {
		var8006142c[i][i] = 0;
	}

	func0f00505c();

	for (i = 0; i < g_NumPortals; i++) {
		for (j = 0; j < i; j++) {
			uint16_t a = var8006142c[i][j];
			uint16_t b = var8006142c[j][i];

			var80061430[i][j] = a < b ? a : b;
		}
	}
}

void func0f00505c(void)
{
	int j;
	int sp78;
	int i;
	int k;
	int portalnum;
	int portalnum2;
	int roomnum;
	int l;
	uint16_t dist;

	for (i = 0; i < g_NumPortals; i++) {
		for (j = 0, var8009cad0[0] = i, sp78 = 1; j != sp78; j = (j + 1) & 0x7ff) {
			portalnum = var8009cad0[j];

			for (k = 0; k < 2; k++) {
				if (k != 0) {
					roomnum = g_BgPortals[portalnum].roomnum2;
				} else {
					roomnum = g_BgPortals[portalnum].roomnum1;
				}

				for (l = 0; l < g_Rooms[roomnum].numportals; l++) {
					portalnum2 = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + l];

					if (portalnum2 != portalnum && var8009cad8[portalnum2] != 0) {
						if (var8006142c[portalnum][portalnum2] >= 0x8000) {
							float xdiff = var80061428[portalnum].x - var80061428[portalnum2].x;
							float ydiff = var80061428[portalnum].y - var80061428[portalnum2].y;
							float zdiff = var80061428[portalnum].z - var80061428[portalnum2].z;

							float dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

							var8006142c[portalnum][portalnum2] = dist;
							var8006142c[portalnum2][portalnum] = dist;
						}

						dist = (var8006142c[i][portalnum] + var8006142c[portalnum2][portalnum]);

						if (dist <= 5800 && ((i == portalnum) != 0 || dist < var8006142c[i][portalnum2])) {
							var8006142c[i][portalnum2] = dist;
							var8009cad0[sp78] = portalnum2;
							sp78 = (sp78 + 1) & 0x7ff;
						}
					}
				}
			}
		}
	}
}

float func0f0053d0(int roomnum1, struct coord *pos1, int portalnum1, int roomnum2, struct coord *pos2, int portalnum2, float *arg6)
{
	float sp6c;
	float *sp68;
	float sp64;
	float xdiff;
	float ydiff;
	float zdiff;

	sp6c = 32767.0f;
	sp68 = arg6 ? arg6 : &sp6c;
	sp64 = *sp68;

	xdiff = pos1->x - pos2->x;
	xdiff = xdiff > 0.0f ? xdiff : -xdiff;

	if (xdiff < sp64) {
		zdiff = pos1->z - pos2->z;
		zdiff = zdiff > 0.0f ? zdiff : -zdiff;

		if (zdiff < sp64) {
			ydiff = pos1->y - pos2->y;
			ydiff = ydiff > 0.0f ? ydiff : -ydiff;

			if (ydiff < sp64) {
				float dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

				if (dist < sp64) {
					if (roomnum1 == roomnum2 || portalnum1 == portalnum2) {
						if (dist < *sp68) {
							*sp68 = dist;
						}
					} else {
						float sp50 = roomGetUpperAndLowerPortals(portalnum1, portalnum2);

						if (sp50 < sp64) {
							struct coord sp44;
							float xdiff2;
							float zdiff2;

							portalGetAvgVertexPos(portalnum1, &sp44);
							sp64 -= sp50;

							xdiff2 = sp44.x - pos1->x;
							xdiff2 = xdiff2 > 0.0f ? xdiff2 : -xdiff2;

							if (xdiff2 < sp64) {
								zdiff2 = sp44.z - pos1->z;
								zdiff2 = zdiff2 > 0.0f ? zdiff2 : -zdiff2;

								if (zdiff2 < sp64) {
									float sp38 = sqrtf(xdiff2 * xdiff2 + zdiff2 * zdiff2);

									if (sp38 < sp64) {
										struct coord sp2c;
										float xdiff3;
										float zdiff3;

										portalGetAvgVertexPos(portalnum2, &sp2c);
										sp64 -= sp38;

										xdiff3 = sp2c.x - pos2->x;
										xdiff3 = xdiff3 > 0.0f ? xdiff3 : -xdiff3;

										if (xdiff3 < sp64) {
											zdiff3 = sp2c.z - pos2->z;
											zdiff3 = zdiff3 > 0.0f ? zdiff3 : -zdiff3;

											if (zdiff3 < sp64) {
												float dist3 = sqrtf(xdiff3 * xdiff3 + zdiff3 * zdiff3);

												if (dist3 < sp64) {
													sp64 -= dist3;
													*sp68 -= sp64;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return *sp68;
}

void func0f0056f4(int roomnum1, struct coord *pos1, int roomnum2, struct coord *pos2, int arg4, float *result, int arg6)
{
	float dist;

	if (!var80061444
			|| PLAYERCOUNT() >= 3
			|| roomnum1 == roomnum2
			|| roomnum1 == -1
			|| roomnum2 == -1) {
		float xdist = pos1->x - pos2->x;

		if (!(xdist > 0.0f)) {
			xdist = -xdist;
		}

		if (xdist < *result) {
			float zdist = pos1->z - pos2->z;

			if (!(zdist > 0.0f)) {
				zdist = -zdist;
			}

			if (zdist < *result) {
				float ydist = pos1->y - pos2->y;

				if (!(ydist > 0.0f)) {
					ydist = -ydist;
				}

				if (ydist < *result) {
					dist = sqrtf(xdist * xdist + ydist * ydist + zdist * zdist);

					if (dist < *result) {
						*result = dist;
					}
				}
			}
		}
	} else {
		int portalnum1;
		int portalnum2;
		int i;
		int j;

		for (i = 0; i < g_Rooms[roomnum1].numportals; i++) {
			portalnum1 = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + i];

			for (j = 0; j < g_Rooms[roomnum2].numportals; j++) {
				portalnum2 = g_RoomPortals[g_Rooms[roomnum2].roomportallistoffset + j];

				dist = func0f0053d0(roomnum1, pos1, portalnum1, roomnum2, pos2, portalnum2, result);

				if (dist < *result) {
					*result = dist;
				}
			}
		}
	}
}

void func0f0059fc(int roomnum1, struct coord *pos1, int roomnum2, struct coord *pos2, int arg4, float *result)
{
	int portalnum1;
	int portalnum2;
	int i;
	int j;
	float dist;

	*result = 32767;

	if (roomnum1 == roomnum2) {
		*result = coordsGetDistance(pos1, pos2);
		return;
	}

	for (i = 0; i < g_Rooms[roomnum1].numportals; i++) {
		portalnum1 = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + i];
		if (1);

		for (j = 0; j < g_Rooms[roomnum2].numportals; j++) {
			portalnum2 = g_RoomPortals[g_Rooms[roomnum2].roomportallistoffset + j];
			if (j);
			if (j);

			dist = func0f0053d0(roomnum1, pos1, portalnum1, roomnum2, pos2, portalnum2, NULL);

			if (dist < *result) {
				*result = dist;
			}
		}
	}
}

/**
 * This function:
 * - Controls the sound effects for the night vision and IR scanner.
 * - Sets g_IsSwitchingGoggles if equipping or unequipping NV/IR on this frame.
 * - Updates the player's usinggoggles property.
 */
void func0f005bb0(void)
{
	int brightness = roomGetFinalBrightness(g_Vars.currentplayer->prop->rooms[0]);

	if (((USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER)) && !g_Vars.currentplayer->usinggoggles)
			|| ((!USINGDEVICE(DEVICE_NIGHTVISION) && !USINGDEVICE(DEVICE_IRSCANNER)) && g_Vars.currentplayer->usinggoggles)) {
		g_IsSwitchingGoggles = true;
	}

	g_Vars.currentplayer->usinggoggles = USINGDEVICE(DEVICE_NIGHTVISION) || USINGDEVICE(DEVICE_IRSCANNER);

	if (USINGDEVICE(DEVICE_NIGHTVISION) && !lvIsPaused()) {
		// Play the goggle's hum sound
		if (g_Vars.currentplayer->nvhum == NULL) {
			sndStart(var80095200, SFX_0505, &g_Vars.currentplayer->nvhum, -1, -1, -1.0f, -1, -1);
		}

		if (brightness > 128) {
			// Room is too bright for night vision - play overload sound
			if (g_Vars.currentplayer->nvoverload == NULL) {
				sndStart(var80095200, SFX_01BE, &g_Vars.currentplayer->nvoverload, -1, -1, -1.0f, -1, -1);
			}
		} else {
			// Room is dark enough for night vision - stop overload sound if active
			if (g_Vars.currentplayer->nvoverload != NULL) {
				if (sndGetState(g_Vars.currentplayer->nvoverload) != AL_STOPPED) {
					audioStop(g_Vars.currentplayer->nvoverload);
				}
			}
		}
	} else {
		// Paused or not wearing night vision - stop both sounds
		if (g_Vars.currentplayer->nvhum != NULL) {
			if (sndGetState(g_Vars.currentplayer->nvhum) != NULL) {
				audioStop(g_Vars.currentplayer->nvhum);
			}
		}

		if (g_Vars.currentplayer->nvoverload != NULL) {
			if (sndGetState(g_Vars.currentplayer->nvoverload) != NULL) {
				audioStop(g_Vars.currentplayer->nvoverload);
			}
		}
	}
}
