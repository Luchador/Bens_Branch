#include <ultra64.h>
#include <math.h>
#include "constants.h"
#include "../lib/naudio/n_sndp.h"
#include "game/chraction.h"
#include "game/dlights.h"
#include "game/menuutils.h"
#include "game/nbomb.h"
#include "game/chr.h"
#include "game/chraction.h"
#include "game/prop.h"
#include "game/objectives.h"
#include "game/mtxutils.h"
#include "game/tex.h"
#include "game/camera.h"
#include "game/savebuffer.h"
#include "game/bg.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/snd.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/lib_317f0.h"
#include "data.h"
#include "types.h"
#include "platform.h"

int16_t g_TCoordOffset; // Animates the textures going around the sphere
bool firsthalf;
struct nbomb g_Nbombs[6];

bool g_NbombsActive = false;
float sphereradius = 100;

// Ben's comment: if (firsthalf && vertices[i].t == 0) fixes texture seam when atan2f(src.x, src.z) returns 0
#define MAKEVERTEX(i, src) \
	vertices[i].x = src.x * sphereradius; \
	vertices[i].y = src.y * sphereradius; \
	vertices[i].z = src.z * sphereradius; \
	vertices[i].s = src.y * 256.0f * 32.0f; \
	vertices[i].t = atan2f(src.x, src.z) / M_TAU * 256.0f * 32.0f; \
	vertices[i].colour = 0; \
\
	if (firsthalf && vertices[i].t == 0) { \
	} \
\
	vertices[i].t += g_TCoordOffset; // Ben's comment: scrolls the T coord around the sphere but honestly I can't see much difference when this is commented out

Gfx *nbombCreateSphereSegment(Gfx *gdl, struct coord *arg1, struct coord *arg2, struct coord *arg3, uint8_t arg4, uint8_t arg5, uint8_t arg6, uint8_t arg7, int8_t depth)
{
	struct coord sp7c;
	struct coord sp70;
	struct coord sp64;
	float dist;
	Vtx *vertices;

	sp7c.x = arg2->x + arg1->x;
	sp7c.y = arg2->y + arg1->y;
	sp7c.z = arg2->z + arg1->z;

	dist = sqrtf(sp7c.f[0] * sp7c.f[0] + sp7c.f[1] * sp7c.f[1] + sp7c.f[2] * sp7c.f[2]);

	sp7c.x /= dist;
	sp7c.y /= dist;
	sp7c.z /= dist;

	sp70.x = arg3->x + arg2->x;
	sp70.y = arg3->y + arg2->y;
	sp70.z = arg3->z + arg2->z;

	dist = sqrtf(sp70.f[0] * sp70.f[0] + sp70.f[1] * sp70.f[1] + sp70.f[2] * sp70.f[2]);

	sp70.x /= dist;
	sp70.y /= dist;
	sp70.z /= dist;

	sp64.x = arg1->x + arg3->x;
	sp64.y = arg1->y + arg3->y;
	sp64.z = arg1->z + arg3->z;

	dist = sqrtf(sp64.f[0] * sp64.f[0] + sp64.f[1] * sp64.f[1] + sp64.f[2] * sp64.f[2]);

	sp64.x /= dist;
	sp64.y /= dist;
	sp64.z /= dist;

	vertices = gfxAllocateVertices(3);

	MAKEVERTEX(0, sp7c);
	MAKEVERTEX(1, sp70);
	MAKEVERTEX(2, sp64);

	gSPVertex(gdl++, (uintptr_t)(vertices), 3, arg7);

	if (depth == 0) {
		gSPTri4(gdl++,
				arg4, arg7, arg7 + 2,
				arg5, arg7 + 1, arg7,
				arg6, arg7 + 2, arg7 + 1,
				arg7, arg7 + 1, arg7 + 2);
	} else {
		gdl = nbombCreateSphereSegment(gdl, arg1, &sp7c, &sp64, arg4, arg7, arg7 + 2, arg7 + 3, depth - 1);
		gdl = nbombCreateSphereSegment(gdl, arg2, &sp70, &sp7c, arg5, arg7 + 1, arg7, arg7 + 3, depth - 1);
		gdl = nbombCreateSphereSegment(gdl, arg3, &sp64, &sp70, arg6, arg7 + 2, arg7 + 1, arg7 + 3, depth - 1);
		gdl = nbombCreateSphereSegment(gdl, &sp7c, &sp70, &sp64, arg7, arg7 + 1, arg7 + 2, arg7 + 3, depth - 1);
	}

	return gdl;
}

Gfx *nbombCreateSphere(Gfx *gdl, int depth)
{
	Vtx *vertices;
	struct coord sp5c[] = {
		{ 0,  0,  1  },
		{ 1,  0,  0  },
		{ 0,  0,  -1 },
		{ -1, 0,  0  },
		{ 0,  1,  0  },
		{ 0,  -1, 0  },
	};

	firsthalf = false;

	vertices = gfxAllocateVertices(6);

	// Make one half of the sphere
	MAKEVERTEX(0, sp5c[0]);
	MAKEVERTEX(1, sp5c[1]);
	MAKEVERTEX(2, sp5c[2]);
	MAKEVERTEX(3, sp5c[3]);
	MAKEVERTEX(4, sp5c[4]);
	MAKEVERTEX(5, sp5c[5]);

	gSPVertex(gdl++, (uintptr_t)(vertices), 6, 0);

	gdl = nbombCreateSphereSegment(gdl, &sp5c[0], &sp5c[4], &sp5c[1], 0, 4, 1, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[1], &sp5c[4], &sp5c[2], 1, 4, 2, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[1], &sp5c[5], &sp5c[0], 1, 5, 0, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[2], &sp5c[5], &sp5c[1], 2, 5, 1, 6, depth);

	firsthalf = true;

	vertices = gfxAllocateVertices(6);

	// Make the other half of the sphere
	MAKEVERTEX(0, sp5c[0]);
	MAKEVERTEX(1, sp5c[1]);
	MAKEVERTEX(2, sp5c[2]);
	MAKEVERTEX(3, sp5c[3]);
	MAKEVERTEX(4, sp5c[4]);
	MAKEVERTEX(5, sp5c[5]);

	gSPVertex(gdl++, (uintptr_t)(vertices), 6, 0);

	gdl = nbombCreateSphereSegment(gdl, &sp5c[2], &sp5c[4], &sp5c[3], 2, 4, 3, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[3], &sp5c[4], &sp5c[0], 3, 4, 0, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[3], &sp5c[5], &sp5c[2], 3, 5, 2, 6, depth);
	gdl = nbombCreateSphereSegment(gdl, &sp5c[0], &sp5c[5], &sp5c[3], 0, 5, 3, 6, depth);

	return gdl;
}

void nbombReset(struct nbomb *nbomb)
{
	nbomb->age240 = 0;
	nbomb->radius = 0;
	nbomb->spawnframe240 = g_Vars.lvframe240;
}

/**
 * If nbomb->age240 is 0 to 310, return 127
 * If nbomb->age240 is 311 to 349, return a scaled number between 127 and 0
 * If nbomb->age240 is 350+, return 0
 */
int nbombCalculateAlpha(struct nbomb *nbomb)
{
	int alpha = 127;

	if (nbomb->age240 > TICKS(310)) {
		if (nbomb->age240 < TICKS(350)) {
			alpha = (TICKS(350) * 127 - nbomb->age240 * 127) / TICKS(40);
		} else {
			alpha = 0;
		}
	}

	return alpha;
}

/**
 * Allocate and populate a graphics display list for an nbomb dome.
 *
 * The gdl sets up a single vertex but has no colour or triangles.
 */
Gfx *nbombCreateGdl(void)
{
	Vtx *vertices;
#ifdef PLATFORM_64BIT
	uint32_t gdlsizes[] = { 0x0a30*2, 0x0330*2 }; // 1 player, 2+ players
#else
	uint32_t gdlsizes[] = { 0x0a30, 0x0330 }; // 1 player, 2+ players
#endif
	Gfx *gdlstart;
	Gfx *gdl;
	int index = 0;

	if (PLAYERCOUNT() >= 2) {
		index = 1;
	}

	g_TCoordOffset = (int)(g_20SecIntervalFrac * 64.0f * 32.0f * 16.0f) % 0x800; //0x800 = 2048. g_TCoordOffset goes from 0 to 16 over a period of 20 seconds

	gdl = gdlstart = gfxAllocate(gdlsizes[index]);

	texSelect(&gdl, &g_TexGeneralConfigs[10], 2, 1, 2, 1, NULL);

	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetAlphaCompare(gdl++, G_AC_NONE);
	gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
	gSPClearGeometryMode(gdl++, G_CULL_BOTH);
	gDPSetColorDither(gdl++, G_CD_DISABLE);
	gDPSetTextureFilter(gdl++, G_TF_BILERP);
	gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	vertices = gfxAllocateVertices(1);

	vertices[0].z = 0;
	vertices[0].t = 0;
	vertices[0].colour = 0;
	vertices[0].y = vertices[0].z;
	vertices[0].x = vertices[0].z;
	vertices[0].s = vertices[0].t;

	gSPVertex(gdl++, (uintptr_t)(vertices), 1, 0);

	if (index != 0) {
		gdl = nbombCreateSphere(gdl, 1);
	} else {
		gdl = nbombCreateSphere(gdl, 2);
	}

	gSPEndDisplayList(gdl++);

	return gdlstart;
}

struct sndstate *g_NbombAudioHandle = NULL;

Gfx *nbombRender(Gfx *gdl, struct nbomb *nbomb, Gfx *subgdl)
{
	float divider = 2048;
	Mtxf *mtx;
	Mtxf spc8;
	Mtxf sp88;
	Mtxf sp48;
	struct coord sp3c;
	uint32_t colour;
	Col *colours;

	mtx = gfxAllocateMatrix();
	sphereradius = 2000.0f;
	colour = nbombCalculateAlpha(nbomb);

	colours = gfxAllocateColours(2);
	colours[0].word = PD_BE32(colour);
	colours[1].word = PD_BE32(0xffffff00);

	gSPColor(gdl++, (uintptr_t)(colours), 2);

	sp3c.x = 0;
	sp3c.y = 0;
	sp3c.z = -100;

	mtx4LoadIdentity(&sp48);
	mtx4LoadTranslation(&nbomb->pos, &sp48);

	sp3c.x = 0;
	sp3c.y = nbomb->unk14 / divider * M_TAU;
	sp3c.z = 0;

	mtx4LoadRotation(&sp3c, &sp88);
	mtxScaleRotationPart(nbomb->radius / 2000.0f, &sp88);
	mtx4MultMtx4(&sp48, &sp88, &spc8);

	mtxApplyAffineTransformInPlace(camGetWorldToScreenMtxf(), &spc8);
	mtxF2L(&spc8, mtx);

	gSPMatrix(gdl++, (uintptr_t)(mtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	gSPDisplayList(gdl++, subgdl);

	return gdl;
}

void nbombClearAllNBombs(void)
{
	int i;

	g_NbombsActive = false;
	g_NbombAudioHandle = NULL;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		g_Nbombs[i].age240 = -1;

		g_Nbombs[i].audiohandle20 = NULL;
		g_Nbombs[i].audiohandle24 = NULL;
	}
}

void nbombInflictDamage(struct nbomb *nbomb)
{
	int index = 0;
	int16_t propnums[256];
	struct coord bbmin;
	struct coord bbmax;
	RoomNum roomnums[54];
	int16_t *propnumptr;
	int i;
	struct gset gset;

	gset.weaponnum = WEAPON_NBOMB;
	gset.weaponfunc = FUNC_PRIMARY;

	if (g_Vars.lvupdate240 <= 0 || nbomb->age240 > TICKS(350)) {
		return;
	}

	// Find rooms which intersect the nbomb dome's bbox
	bbmin.x = nbomb->pos.f[0] - nbomb->radius;
	bbmin.y = nbomb->pos.f[1] - nbomb->radius;
	bbmin.z = nbomb->pos.f[2] - nbomb->radius;

	bbmax.x = nbomb->pos.f[0] + nbomb->radius;
	bbmax.y = nbomb->pos.f[1] + nbomb->radius;
	bbmax.z = nbomb->pos.f[2] + nbomb->radius;

	for (i = 1; i < g_Vars.roomcount; i++) {
		if (!(bbmax.f[0] < g_Rooms[i].bbmin[0]
				|| bbmin.f[0] > g_Rooms[i].bbmax[0]
				|| bbmax.f[1] < g_Rooms[i].bbmin[1]
				|| bbmin.f[1] > g_Rooms[i].bbmax[1]
				|| bbmax.f[2] < g_Rooms[i].bbmin[2]
				|| bbmin.f[2] > g_Rooms[i].bbmax[2])) {
			if (index < 52) {
				roomnums[index] = i;
				index++;
				roomFlashLighting(i, -38, -180);
			}
		}
	}

	roomnums[index] = -1;

	// Iterate props in the affected rooms and damage any chrs
	roomGetProps(roomnums, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		struct prop *prop = &g_Vars.props[*propnumptr];

		if (prop->timetoregen == 0) {
			if (prop->type == PROPTYPE_CHR || prop->type == PROPTYPE_PLAYER) {
				float xdiff = prop->pos.f[0] - nbomb->pos.f[0];
				float ydiff = prop->pos.f[1] - nbomb->pos.f[1];
				float zdiff = prop->pos.f[2] - nbomb->pos.f[2];

				float dist = sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);

				if (dist < nbomb->radius) {
					struct chrdata *chr = prop->chr;
					if (chr)
					{
						struct coord vector = {0, 0, 0};
						float damage = 0.01f * g_Vars.lvupdate60freal;

						chrDamageByMisc(chr, damage, &vector, &gset, nbomb->ownerprop);
						chr->chrflags |= CHRCFLAG_TRIGGERSHOTLIST;

						if (chr->hidden & CHRHFLAG_CLOAKED) {
							chrUncloak(chr, true);
						}
					}
				}
			}
		}

		propnumptr++;
	}
}

void nbombTick(struct nbomb *nbomb)
{
	if (nbomb->age240 >= 0) {
		int age60;
		int oldage240 = nbomb->age240;
		nbomb->age240 = (g_Vars.lvframe240 - nbomb->spawnframe240) >> 2;
		int increment = nbomb->age240 - oldage240;

		if (nbomb->age240 < TICKS(80)) {
			nbomb->radius = nbomb->age240 / 80.0f;
			nbomb->radius = sqrtf(sqrtf(nbomb->radius));
			nbomb->unk18 = 0;
		} else {
			nbomb->radius = sinf((nbomb->age240 - TICKS(80)) * 0.0523333363235f) * 0.05f + 1.0f;
			nbomb->unk18 = ((nbomb->age240 - TICKS(80)) / 270.0f) * 3.0f;
		}

		nbomb->radius *= 500.0f;

		nbombInflictDamage(nbomb);

		age60 = nbomb->age240 / 4;

		if (age60 > 40) {
			age60 = 40;
		}

		nbomb->unk14 += increment * age60;

		nbomb->unk14 %= 0x800; //0x800 = 2048

		if (nbomb->age240 > 370) { // N-Bomb lifespan (6 seconds)
			nbomb->age240 = -1;
		}
	}
}

void nbombsTick(void)
{
	int i;
	int youngest240 = 20000;
	int volume;

	if (g_Vars.lvupdate240 != 0) {
		g_NbombsActive = false;
	}

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Vars.lvupdate240 != 0 && g_Nbombs[i].age240 >= 0) {
			nbombTick(&g_Nbombs[i]);

			if (g_Nbombs[i].age240 < youngest240) {
				youngest240 = g_Nbombs[i].age240;
			}

			g_NbombsActive = true;
		}
	}

	volume = 0;

	if (youngest240 < TICKS(350)) {
		if (g_Vars.lvupdate240 != 0) {
			if (g_NbombAudioHandle == 0) {
				sndStart(var80095200, SFX_SHIP_HUM, &g_NbombAudioHandle, -1, -1, -1, -1, -1);
			}

			volume = AL_VOL_FULL;

			if (g_NbombAudioHandle) {
				float speed = menuGetSinOscFrac(20) * 0.02f + 0.4f;

				if (youngest240 > TICKS(300)) {
					volume = (1.0f - (float)(youngest240 - TICKS(300)) / 50.0f) * AL_VOL_FULL;
				}

				if (youngest240 >= TICKS(350)) {
					volume = 0;
				}

				audioPostEvent(g_NbombAudioHandle, AL_SNDP_VOL_EVT, volume);
				audioPostEvent(g_NbombAudioHandle, AL_SNDP_PITCH_EVT, *(int *)&speed);
			}
		} else {
			if (g_NbombAudioHandle && sndGetState(g_NbombAudioHandle) != AL_STOPPED) {
				audioStop(g_NbombAudioHandle);
			}
		}
	} else {
		if (g_NbombAudioHandle && sndGetState(g_NbombAudioHandle) != AL_STOPPED) {
			audioStop(g_NbombAudioHandle);
		}
	}

	if (g_Vars.lvupdate240 == 0) {
		for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
			if (g_Nbombs[i].age240 >= 0) {
				if (g_Nbombs[i].audiohandle20 && sndGetState(g_Nbombs[i].audiohandle20) != AL_STOPPED) {
					audioStop(g_Nbombs[i].audiohandle20);
				}

				if (g_Nbombs[i].audiohandle24 && sndGetState(g_Nbombs[i].audiohandle24) != AL_STOPPED) {
					audioStop(g_Nbombs[i].audiohandle24);
				}
			}
		}
	}
}

Gfx *nbombsRender(Gfx *gdl)
{
	int i;
	Gfx *subgdl = NULL;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 >= 0) {
			if (!subgdl) {
				subgdl = nbombCreateGdl();
			}

			gdl = nbombRender(gdl, &g_Nbombs[i], subgdl);
		}
	}

	return gdl;
}

void nbombCreateStorm(struct coord *pos, struct prop *ownerprop)
{
	int oldest240;
	int index;
	int i;

	oldest240 = -1;
	index = 0;

	g_NbombsActive = true;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 == -1
				&& g_Nbombs[i].audiohandle20 == NULL
				&& g_Nbombs[i].audiohandle24 == NULL
				) {
			index = i;
			break;
		}

		if (g_Nbombs[i].age240 > oldest240) {
			index = i;
			oldest240 = g_Nbombs[i].age240;
		}
	}

	nbombReset(&g_Nbombs[index]);

	g_Nbombs[index].pos.x = pos->x;
	g_Nbombs[index].pos.y = pos->y;
	g_Nbombs[index].pos.z = pos->z;
	g_Nbombs[index].age240 = 0;
	g_Nbombs[index].ownerprop = ownerprop;

	// Newer versions only play audio if the handles are null,
	// while ntsc-beta clears the handles then plays them unconditionally.
	if (g_Nbombs[index].audiohandle20 == NULL) {
		sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle20, -1, -1, -1, -1, -1);

		if (g_Nbombs[index].audiohandle20) {
			union audioparam param;
			param.f32 = 0.4f;
			audioPostEvent(g_Nbombs[index].audiohandle20, AL_SNDP_PITCH_EVT, param.s32);
		}
	}

	if (g_Nbombs[index].audiohandle24 == NULL) {
		sndStart(var80095200, SFX_LAUNCH_ROCKET, &g_Nbombs[index].audiohandle24, -1, -1, -1, -1, -1);

		if (g_Nbombs[index].audiohandle24) {
			union audioparam param;
			param.f32 = 0.4f;
			audioPostEvent(g_Nbombs[index].audiohandle24, AL_SNDP_PITCH_EVT, param.s32);
		}
	}
}

float gasGetDoorFrac(int tagnum)
{
	struct defaultobj *obj = objFindByTagId(tagnum);

	if (obj && obj->prop && obj->type == OBJTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *)obj;
		return door->frac;
	}

	return 0;
}

/**
 * Checks if the player is inside an nbomb storm, and if so renders the black
 * storm texture directly over the screen.
 */
Gfx *nbombRenderOverlay(Gfx *gdl)
{
	bool inside = false;
	struct coord campos;
	int finalalpha = 0;
	int i;
	int16_t t;
	int16_t s;
	bool drawn = false;
	Col *colours;
	Vtx *vertices;
	int16_t viewleft;
	int16_t viewtop;
	int16_t viewright;
	int16_t viewbottom;

	campos.x = g_Vars.currentplayer->cam_pos.x;
	campos.y = g_Vars.currentplayer->cam_pos.y;
	campos.z = g_Vars.currentplayer->cam_pos.z;

	for (i = 0; i < ARRAYCOUNT(g_Nbombs); i++) {
		if (g_Nbombs[i].age240 >= 0 && g_Nbombs[i].age240 <= TICKS(350)) {
			float xdiff = campos.f[0] - g_Nbombs[i].pos.f[0];
			float ydiff = campos.f[1] - g_Nbombs[i].pos.f[1];
			float zdiff = campos.f[2] - g_Nbombs[i].pos.f[2];

			if (sqrtf(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff) < g_Nbombs[i].radius) {
				uint32_t alpha = nbombCalculateAlpha(&g_Nbombs[i]);

				inside = true;

				if (alpha > finalalpha) {
					finalalpha = alpha;
				}
			}
		}
	}

	if (inside) {
		colours = gfxAllocateColours(1);
		vertices = gfxAllocateVertices(4);

		viewleft = viGetViewLeft() * 10;
		viewtop = viGetViewTop() * 10;
		viewright = (int16_t) (viGetViewLeft() + viGetViewWidth()) * 10;
		viewbottom = (int16_t) (viGetViewTop() + viGetViewHeight()) * 10;

		s = (int) (8.0f * g_20SecIntervalFrac * 128.0f * 32.0f) % 2048;
		t = (int16_t) ((int) (campos.f[1] * 8.0f) % 2048) + (int16_t) (2.0f * g_20SecIntervalFrac * 128.0f * 32.0f);

		drawn = true;

		gdl = func0f0d479c(gdl);

		texSelect(&gdl, &g_TexGeneralConfigs[10], 2, 1, 2, true, NULL);

		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_1CYCLE);
		gDPSetAlphaCompare(gdl++, G_AC_NONE);
		gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
		gSPClearGeometryMode(gdl++, G_CULL_BOTH);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetTextureFilter(gdl++, G_TF_BILERP);
		gDPSetRenderMode(gdl++, G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2);
		gDPSetTexturePersp(gdl++, G_TP_PERSP);

		vertices[0].x = viewleft;
		vertices[0].y = viewtop;
		vertices[0].z = -10;

		vertices[1].x = viewright;
		vertices[1].y = viewtop;
		vertices[1].z = -10;

		vertices[2].x = viewright;
		vertices[2].y = viewbottom;
		vertices[2].z = -10;

		vertices[3].x = viewleft;
		vertices[3].y = viewbottom;
		vertices[3].z = -10;

		vertices[0].s = s;
		vertices[0].t = t;
		vertices[1].s = s + 160;
		vertices[1].t = t;
		vertices[2].s = s + 160;
		vertices[2].t = t + 960;
		vertices[3].s = s;
		vertices[3].t = t + 960;

		vertices[0].colour = 0;
		vertices[1].colour = 0;
		vertices[2].colour = 0;
		vertices[3].colour = 0;

		colours[0].word = PD_BE32(finalalpha);

		gSPColor(gdl++, (uintptr_t)(colours), 1);
		gSPVertex(gdl++, (uintptr_t)(vertices), 4, 0);

		gSPTri2(gdl++, 0, 1, 2, 2, 3, 0);
	}

	if (drawn) {
		gdl = func0f0d49c8(gdl);
	}

	return gdl;
}

//The gas rendering for Area 51 Escape is here too
Gfx *gasRender(Gfx *gdl)
{
	bool show = false;
	float alphafrac = 1.0f;
	struct coord campos;
	int16_t layer2t;
	uint32_t alpha;
	int i;
	bool drawn = false;

	const int gasrooms[] = {
		ROOM_LUE_0092,
		ROOM_LUE_0093,
		ROOM_LUE_0094,
		ROOM_LUE_0095,
		ROOM_LUE_0096,
		ROOM_LUE_0097,
		ROOM_LUE_0098,
		ROOM_LUE_0099,
		ROOM_LUE_009A,
		ROOM_LUE_0091,
		ROOM_LUE_008F,
		ROOM_LUE_0090,
	};

	if (g_Vars.stagenum == STAGE_ESCAPE) {
		float intensityfrac = 1.0f;

		campos.x = g_Vars.currentplayer->cam_pos.x;
		campos.y = g_Vars.currentplayer->cam_pos.y;
		campos.z = g_Vars.currentplayer->cam_pos.z;

		for (i = 0; i < ARRAYCOUNT(gasrooms); i++) {
			if (bgRoomContainsCoord(&campos, gasrooms[i])) {
				show = true;
			}
		}

		if (!show) {
			// Outside of the gas rooms list - check distance to abitrary point
			float distance = sqrtf(
					(campos.f[0] - -1473.0f) * (campos.f[0] - -1473.0f) +
					(campos.f[1] - -308.0f) * (campos.f[1] - -308.0f) +
					(campos.f[2] - -13660.0f) * (campos.f[2] - -13660.0f));

			if (distance < 1328.0f) {
				show = true;
				alphafrac = 1.0f - distance / 1328.0f;
				intensityfrac = gasGetDoorFrac(0x32);
			}
		} else {
			if (bgRoomContainsCoord(&campos, 0x91)) {
				// In the small room between the first two doors
				float frac1 = gasGetDoorFrac(0x30);
				float frac2 = gasGetDoorFrac(0x31);

				if (frac2 > frac1) {
					intensityfrac = frac2;
				} else {
					intensityfrac = frac1;
				}

				intensityfrac += 0.2f;
			}
		}

		alphafrac *= intensityfrac;

		if (show && g_Vars.tickmode == TICKMODE_CUTSCENE) {
			if (g_CutsceneCurAnimFrame60 < 2180) {
				show = false;
			} else if (g_CutsceneCurAnimFrame60 < 2600) {
				float tmp = (g_CutsceneCurAnimFrame60 - 2180) / 420.0f;
				alphafrac *= tmp;
			}
		}

		if (show) {
			Col *colours = gfxAllocateColours(1);
			Vtx *vertices = gfxAllocateVertices(8);
			int16_t viewleft = viGetViewLeft() * 10;
			int16_t viewtop = viGetViewTop() * 10;
			int16_t viewright = (int16_t) (viGetViewLeft() + viGetViewWidth()) * 10;
			int16_t viewbottom = (int16_t) (viGetViewTop() + viGetViewHeight()) * 10;
			float lookx = g_Vars.currentplayer->cam_look.x;
			float lookz = g_Vars.currentplayer->cam_look.z;
			float camposx = g_Vars.currentplayer->cam_pos.x;
			float camposz = g_Vars.currentplayer->cam_pos.z;
			float f2;
			float f16;
			float sp78;
			int16_t layer2s;
			int16_t layer1s;
			int16_t layer1t;

			f2 = (camposx + camposz) / 3000.0f;
			f16 = (f2 - (int) f2);

			sp78 = atan2f(-lookx, lookz) / M_TAU;

			layer2s = ((int) (2.0f * ((menuGetSinOscFrac(4.0f) - 0.5f) / 6.0f + sp78 + f16 * 1.5f) * 128.0f * 32.0f) % 2048);
			layer1s = ((int) (2.0f * ((menuGetCosOscFrac(4.0f) - 0.5f) / -9.0f + sp78 + f16) * 128.0f * 32.0f) % 2048);

			layer2t = (int16_t) ((int) (campos.y * 8.0f) % 2048) + (int16_t) (2.0f * g_20SecIntervalFrac * 128.0f * 32.0f);
			layer1t = (int16_t) ((int) (campos.y * 8.0f) % 2048) + (int16_t) (2.0f * g_20SecIntervalFrac * 64.0f * 32.0f);

			drawn = true;

			gdl = func0f0d479c(gdl);

			texSelect(&gdl, &g_TexGeneralConfigs[6], 4, 1, 2, true, NULL);

			gDPPipeSync(gdl++);
			gDPSetCycleType(gdl++, G_CYC_1CYCLE);
			gDPSetAlphaCompare(gdl++, G_AC_NONE);
			gDPSetCombineMode(gdl++, G_CC_MODULATEIA, G_CC_MODULATEIA);
			gSPClearGeometryMode(gdl++, G_CULL_BOTH);
			gDPSetColorDither(gdl++, G_CD_DISABLE);
			gDPSetTextureFilter(gdl++, G_TF_BILERP);
			gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
			gDPSetTexturePersp(gdl++, G_TP_PERSP);

			vertices[0].x = viewleft;
			vertices[0].y = viewtop;
			vertices[0].z = -10;

			vertices[1].x = viewright;
			vertices[1].y = viewtop;
			vertices[1].z = -10;

			vertices[2].x = viewright;
			vertices[2].y = viewbottom;
			vertices[2].z = -10;

			vertices[3].x = viewleft;
			vertices[3].y = viewbottom;
			vertices[3].z = -10;

			vertices[4].x = viewleft;
			vertices[4].y = viewtop;
			vertices[4].z = -10;

			vertices[5].x = viewright;
			vertices[5].y = viewtop;
			vertices[5].z = -10;

			vertices[6].x = viewright;
			vertices[6].y = viewbottom;
			vertices[6].z = -10;

			vertices[7].x = viewleft;
			vertices[7].y = viewbottom;
			vertices[7].z = -10;

			vertices[0].s = layer1s;
			vertices[0].t = layer1t;
			vertices[1].s = layer1s + 960;
			vertices[1].t = layer1t;
			vertices[2].s = layer1s + 960;
			vertices[2].t = layer1t + 640;
			vertices[3].s = layer1s;
			vertices[3].t = layer1t + 640;

			vertices[0].colour = 0;
			vertices[1].colour = 0;
			vertices[2].colour = 0;
			vertices[3].colour = 0;

			vertices[4].s = layer2s;
			vertices[4].t = layer2t;
			vertices[5].s = layer2s + 640;
			vertices[5].t = layer2t;
			vertices[6].s = layer2s + 640;
			vertices[6].t = layer2t + 480;
			vertices[7].s = layer2s;
			vertices[7].t = layer2t + 480;

			vertices[4].colour = 0;
			vertices[5].colour = 0;
			vertices[6].colour = 0;
			vertices[7].colour = 0;

			alpha = 127.0f * alphafrac;

			colours[0].word = PD_BE32(0x3faf1100 | alpha);

			gSPColor(gdl++, (uintptr_t)(colours), 1);
			gSPVertex(gdl++, (uintptr_t)(vertices), 8, 0);

			gSPTri4(gdl++, 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4);
		}
	}

	if (drawn) {
		gdl = func0f0d49c8(gdl);
	}

	return gdl;
}
