#include <ultra64.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "constants.h"
#include "game/artifacts.h"
#include "game/bg.h"
#include "game/bondgun.h"
#include "game/camera.h"
#include "game/chr.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/dyntex.h"
#include "game/env.h"
#include "game/file.h"
#include "game/gfxmemory.h"
#include "game/gfxreplace.h"
#include "game/lv.h"
#include "game/menuutils.h"
#include "game/mtxutils.h"
#include "game/player.h"
#include "game/portal.h"
#include "game/portalconv.h"
#include "game/prop.h"
#include "game/room.h"
#include "game/sky.h"
#include "game/stagetable.h"
#include "game/stars.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "game/textutils.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/lib_17ce0.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/rng.h"
#include "lib/rzip.h"
#include "lib/vi.h"
#include "data.h"
#include "types.h"
#include "preprocess.h"
#include "preprocess/common.h"
#include "system.h"
#include "video.h"
#include "platform.h"

#define BGCMD_END                               0x00
#define BGCMD_PUSH                              0x01
#define BGCMD_POP                               0x02
#define BGCMD_AND                               0x03
#define BGCMD_OR                                0x04
#define BGCMD_NOT                               0x05
#define BGCMD_XOR                               0x06
#define BGCMD_PUSH_CAMINROOMRANGE               0x14
#define BGCMD_SETRESULT_TRUE                    0x1e
#define BGCMD_SETRESULT_IFPORTALINFOV           0x1f
#define BGCMD_IFRESULT_SHOWROOM                 0x20
#define BGCMD_SETRESULT_FALSE                   0x21
#define BGCMD_SETRESULT_TRUEIFTHROUGHPORTAL     0x22
#define BGCMD_SETRESULT_FALSEIFNOTTHROUGHPORTAL 0x23
#define BGCMD_DISABLEROOM                       0x24
#define BGCMD_DISABLEROOMRANGE                  0x25
#define BGCMD_LOADROOM                          0x26
#define BGCMD_LOADROOMRANGE                     0x27
#define BGCMD_SETROOMTESTSDISABLED              0x28
#define BGCMD_PUSH_PORTALISOPEN                 0x29
#define BGCMD_2A                                0x2a
#define BGCMD_BRANCH                            0x50
#define BGCMD_THROW                             0x51
#define BGCMD_CATCH                             0x52
#define BGCMD_IF                                0x5a
#define BGCMD_ELSE                              0x5b
#define BGCMD_ENDIF                             0x5c
#define BGCMD_PORTALARG                         0x64
#define BGCMD_ROOMARG                           0x65

#define BGRESULT_TRUE  0
#define BGRESULT_FALSE 1

#define VTXBATCHTYPE_OPA 0x01
#define VTXBATCHTYPE_XLU 0x02

struct drawslot g_BgDrawSlots[61];
uint8_t *g_BgPrimaryData;
uint32_t var800a4920;
uint32_t g_BgSection3;
struct room *g_Rooms;
uint8_t *g_MpRoomVisibility;
RoomNum g_BgForceOnscreenRooms[350];
int g_BgNumForceOnscreenRooms;
int16_t g_BgUnloadDelay240;
int16_t g_BgUnloadDelay240_2;
RoomNum g_GlareRooms[100];
uintptr_t *g_BgPrimaryData2;
struct bgroom *g_BgRooms;
struct bgportal *g_BgPortals;
struct portalmetric *g_PortalMetrics;
struct bgcmd *g_BgCommands;
uint8_t *g_BgLightsFileData;
float *g_BgStanThings;
int16_t *g_RoomPortals;
int16_t g_BgMinDrawOrder;
int16_t g_BgMaxDrawOrder;
struct drawslotpointer *g_BgDrawSlotsByRoom;
struct portalcamcacheitem *g_PortalCameraCache;
struct bgsnake g_BgSnake;

int g_StageIndex = 1;
uint8_t *var8007fc08 = NULL;

int16_t g_RoomStreamingBoostTimer = 0;
int16_t g_NumRoomLoadsLeftThisFrame = 0; // Number of rooms that are allowed to be loaded in a single frame
int g_NumRoomsWithGlares = 0;
int g_CamRoom = 1;
struct drawslot *g_BgSpecialDrawSlot = &g_BgDrawSlots[60];
int g_BgLoadCandidateTimer240 = 0;
int g_BgNumDrawSlots = 0;
int g_BgNumAttemptedDrawSlots = 0;
int g_BgMostAttemptedDrawSlots = 0;
int g_BgNumRoomLoadCandidates = 0;
uint16_t g_BgFrameCount = 0xfffe;
int g_BgNumPortalCameraCacheItems = 0;

void bgUnpausePropsInRoom(uint32_t roomnum, bool tintedglassonly)
{
	struct prop *prop;
	struct defaultobj *obj;
	int16_t *propnumptr;
	RoomNum rooms[2];
	int16_t propnums[256];

	rooms[0] = roomnum;
	rooms[1] = -1;

	roomGetProps(rooms, propnums, 256);

	propnumptr = propnums;

	while (*propnumptr >= 0) {
		prop = &g_Vars.props[*propnumptr];

		if (!prop->active) {
			if (tintedglassonly) {
				/**
				 * @bug: A missing prop->type check means this is inadvertently
				 * casting other pointer types to obj pointers. By chance this
				 * happens to be harmless.
				 */
				obj = prop->obj;

				if (obj->type == OBJTYPE_TINTEDGLASS) {
					propUnpause(prop);
				}
			} else {
				propUnpause(prop);
			}
		}

		propnumptr++;
	}
}

void bgSetRoomOnscreen(int roomnum, int draworder, struct screenbox *box)
{
	int index;

	if ((g_Rooms[roomnum].flags & ROOMFLAG_DISABLEDBYSCRIPT) == 0) {
		g_Rooms[roomnum].flags |= ROOMFLAG_ONSCREEN;

		if (g_Rooms[roomnum].flags & ROOMFLAG_BBOXHACK) {
			box->xmin = g_BgDrawSlots[60].box.xmin;
			box->ymin = g_BgDrawSlots[60].box.ymin;
			box->xmax = g_BgDrawSlots[60].box.xmax;
			box->ymax = g_BgDrawSlots[60].box.ymax;
		}

		if (g_BgFrameCount == g_BgDrawSlotsByRoom[roomnum].updatedframe) {
			index = g_BgDrawSlotsByRoom[roomnum].slotnum;

			if (draworder > g_BgDrawSlots[index].draworder) {
				g_BgDrawSlots[index].draworder = draworder;

				if (g_BgDrawSlots[index].draworder > g_BgMaxDrawOrder) {
					g_BgMaxDrawOrder = g_BgDrawSlots[index].draworder;
				}

				if (g_BgDrawSlots[index].draworder < g_BgMinDrawOrder) {
					g_BgMinDrawOrder = g_BgDrawSlots[index].draworder;
				}
			}

			bgExpandBox(&g_BgDrawSlots[index].box, box);
		} else {
			index = g_BgNumDrawSlots;

			if (index > 59) {
				index = 59;
			}

			g_BgDrawSlots[index].roomnum = roomnum;
			g_BgDrawSlots[index].draworder = draworder;

			g_BgDrawSlots[index].box.xmin = box->xmin;
			g_BgDrawSlots[index].box.ymin = box->ymin;
			g_BgDrawSlots[index].box.xmax = box->xmax;
			g_BgDrawSlots[index].box.ymax = box->ymax;

			if (g_BgDrawSlots[index].draworder > g_BgMaxDrawOrder) {
				g_BgMaxDrawOrder = g_BgDrawSlots[index].draworder;
			}

			if (g_BgDrawSlots[index].draworder < g_BgMinDrawOrder) {
				g_BgMinDrawOrder = g_BgDrawSlots[index].draworder;
			}

			g_BgDrawSlotsByRoom[roomnum].updatedframe = g_BgFrameCount;
			g_BgDrawSlotsByRoom[roomnum].slotnum = index;

			g_BgNumAttemptedDrawSlots++;

			if (g_BgNumAttemptedDrawSlots < 60) {
				g_BgNumDrawSlots = g_BgNumAttemptedDrawSlots;
			}

			bgUnpausePropsInRoom(roomnum, false);

			if (g_Rooms[roomnum].loaded240 == 0 && g_NumRoomLoadsLeftThisFrame > 0) {
				g_NumRoomLoadsLeftThisFrame--;
				bgLoadRoom(roomnum);
			} else if (g_Rooms[roomnum].loaded240 == 0) {
				g_NumRoomLoadsLeftThisFrame--;
			}
		}
	}
}

void bgGetRoomBrightnessRange(int roomnum, int8_t *min, int8_t *max)
{
	*min = g_BgRooms[roomnum].br_light_min;
	*max = g_BgRooms[roomnum].br_light_max;
}

struct drawslot *bgGetRoomDrawSlot(int roomnum)
{
	int index = 60;

	if (g_BgFrameCount == g_BgDrawSlotsByRoom[roomnum].updatedframe) {
		index = g_BgDrawSlotsByRoom[roomnum].slotnum;
	}

	return &g_BgDrawSlots[index];
}

Gfx *bgRenderXrayData(Gfx *gdl, struct xraydata *xraydata)
{
	Vtx *vertices;
	Col *colours;
	int numgroups;
	int i;
	int count;

	if (xraydata->numtris > 0) {
		vertices = gfxAllocateVertices(xraydata->numvertices);
		colours = gfxAllocateColours(xraydata->numvertices);

		for (i = 0; i < xraydata->numvertices; i++) {
			vertices[i].x = xraydata->vertices[i][0];
			vertices[i].y = xraydata->vertices[i][1];
			vertices[i].z = xraydata->vertices[i][2];
			vertices[i].colour = i << 2;
			colours[i].word = PD_BE32(xraydata->colours[i]);
		}

		count = xraydata->numvertices;
		gfx_Color(gdl++, colours, count);

		count = xraydata->numvertices;
		gSPVertex(gdl++, vertices, count, 0);

		numgroups = (xraydata->numtris - 1) / 4 + 1;

		for (i = xraydata->numtris; i < numgroups * 4; i++) {
			xraydata->tris[i][0] = xraydata->tris[i][1] = xraydata->tris[i][2] = 0;
		}

		for (i = 0; i < numgroups; i++) {
			gfx_Tri4(gdl++,
					xraydata->tris[i * 4 + 0][0], xraydata->tris[i * 4 + 0][1], xraydata->tris[i * 4 + 0][2],
					xraydata->tris[i * 4 + 1][0], xraydata->tris[i * 4 + 1][1], xraydata->tris[i * 4 + 1][2],
					xraydata->tris[i * 4 + 2][0], xraydata->tris[i * 4 + 2][1], xraydata->tris[i * 4 + 2][2],
					xraydata->tris[i * 4 + 3][0], xraydata->tris[i * 4 + 3][1], xraydata->tris[i * 4 + 3][2]);
		}
	}

	xraydata->numtris = 0;
	xraydata->numvertices = 0;

	return gdl;
}

Gfx *bgAddXrayTri(Gfx *gdl, struct xraydata *xraydata, int16_t vertices1[3], int16_t vertices2[3], int16_t vertices3[3], uint32_t colour1, uint32_t colour2, uint32_t colour3)
{
	int16_t sp30[3] = {-1, -1, -1};
	int count = 0;
	int16_t i;

	if (xraydata->numtris >= 64) {
		gdl = bgRenderXrayData(gdl, xraydata);
	}

	for (i = 0; i < xraydata->numvertices && sp30[0] == -1; i++) {
		if (vertices1[0] == xraydata->vertices[i][0]
				&& vertices1[1] == xraydata->vertices[i][1]
				&& vertices1[2] == xraydata->vertices[i][2]) {
			sp30[0] = i;
			count++;
		}
	}

	for (i = 0; i < xraydata->numvertices && sp30[1] == -1; i++) {
		if (vertices2[0] == xraydata->vertices[i][0]
				&& vertices2[1] == xraydata->vertices[i][1]
				&& vertices2[2] == xraydata->vertices[i][2]) {
			sp30[1] = i;
			count++;
		}
	}

	for (i = 0; i < xraydata->numvertices && sp30[2] == -1; i++) {
		if (vertices3[0] == xraydata->vertices[i][0]
				&& vertices3[1] == xraydata->vertices[i][1]
				&& vertices3[2] == xraydata->vertices[i][2]) {
			sp30[2] = i;
			count++;
		}
	}

	if (count < 3) {
		if (xraydata->numvertices - count + 3 > 16) {
			gdl = bgRenderXrayData(gdl, xraydata);

			xraydata->vertices[0][0] = vertices1[0];
			xraydata->vertices[0][1] = vertices1[1];
			xraydata->vertices[0][2] = vertices1[2];
			xraydata->colours[0] = colour1;

			xraydata->vertices[1][0] = vertices2[0];
			xraydata->vertices[1][1] = vertices2[1];
			xraydata->vertices[1][2] = vertices2[2];
			xraydata->colours[1] = colour2;

			xraydata->vertices[2][0] = vertices3[0];
			xraydata->vertices[2][1] = vertices3[1];
			xraydata->vertices[2][2] = vertices3[2];
			xraydata->colours[2] = colour3;

			xraydata->numvertices = 3;

			sp30[0] = 0;
			sp30[1] = 1;
			sp30[2] = 2;
		} else {
			if (sp30[0] == -1) {
				xraydata->vertices[xraydata->numvertices][0] = vertices1[0];
				xraydata->vertices[xraydata->numvertices][1] = vertices1[1];
				xraydata->vertices[xraydata->numvertices][2] = vertices1[2];
				xraydata->colours[xraydata->numvertices] = colour1;

				sp30[0] = xraydata->numvertices;

				xraydata->numvertices++;
			}

			if (sp30[1] == -1) {
				xraydata->vertices[xraydata->numvertices][0] = vertices2[0];
				xraydata->vertices[xraydata->numvertices][1] = vertices2[1];
				xraydata->vertices[xraydata->numvertices][2] = vertices2[2];
				xraydata->colours[xraydata->numvertices] = colour2;

				sp30[1] = xraydata->numvertices;

				xraydata->numvertices++;
			}

			if (sp30[2] == -1) {
				xraydata->vertices[xraydata->numvertices][0] = vertices3[0];
				xraydata->vertices[xraydata->numvertices][1] = vertices3[1];
				xraydata->vertices[xraydata->numvertices][2] = vertices3[2];
				xraydata->colours[xraydata->numvertices] = colour3;

				sp30[2] = xraydata->numvertices;

				xraydata->numvertices++;
			}
		}
	}

	xraydata->tris[xraydata->numtris][0] = sp30[0];
	xraydata->tris[xraydata->numtris][1] = sp30[1];
	xraydata->tris[xraydata->numtris][2] = sp30[2];
	xraydata->numtris++;

	return gdl;
}

void bgChooseXrayVtxColour(bool *inrange, int16_t vertex[3], uint32_t *colour, struct xraydata *xraydata)
{
	float sp2c[3];
	float f12;
	float alphafrac;
	float anglefrac;
	struct player *player = g_Vars.currentplayer;
	float colfrac;

	*inrange = false;

	sp2c[0] = (float) vertex[0] - (float) xraydata->unk000;
	sp2c[0] = sp2c[0] * sp2c[0];

	if (sp2c[0] < xraydata->unk010) {
		sp2c[2] = (float) vertex[2] - (float) xraydata->unk008;
		sp2c[2] = sp2c[2] * sp2c[2];

		if (sp2c[2] < xraydata->unk010) {
			sp2c[1] = (float) vertex[1] - (float) xraydata->unk004;
			sp2c[1] = sp2c[1] * sp2c[1];

			if (sp2c[1] < xraydata->unk010) {
				float dist = sqrtf(sp2c[0] + sp2c[1] + sp2c[2]);

				if (dist < xraydata->unk00c) {
					*inrange = true;

					f12 = dist / xraydata->unk00c;

					if (xraydata->unk014 < f12) {
						alphafrac = 1.0f - (f12 - xraydata->unk014) / (1.0f - xraydata->unk014);
					} else {
						alphafrac = 1.0f;
					}

					if (f12 < xraydata->unk01c) {
						anglefrac = xraydata->unk01c;
						anglefrac = f12 / anglefrac;
						colfrac = sinf((1.0f - anglefrac) * 1.5707964f);

						*colour = (uint32_t)(colfrac * 255.0f) << player->ecol_1
							| (uint32_t)((1.0f - colfrac) * 255.0f) << player->ecol_2
							| (uint32_t)(alphafrac * 128.0f);
					} else {
						anglefrac = (f12 - xraydata->unk01c) / (1.0f - xraydata->unk01c);
						anglefrac = 0.65f * anglefrac + 0.35f;
						colfrac = sinf(anglefrac * 1.5707964f);

						*colour = (uint32_t)(colfrac * 255.0f) << player->ecol_3
							| 0xff << player->ecol_2
							| (uint32_t)(alphafrac * 128.0f);
					}
				}
			}
		}
	}

	if (*inrange == false) {
		*colour = 0x0000ff00;
	}
}

Gfx *bgProcessXrayTri(Gfx *gdl, struct xraydata *xraydata, int16_t arg2[3], int16_t arg3[3], int16_t arg4[3], int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
	int spa4[3];
	int16_t sp9c[3] = {0, 0, 0};
	int sum;
	int16_t sp84[3][3];
	bool inrange[3];
	uint32_t colours[3];
	int sp68 = -1;
	int sp64 = 0;

	if (xraydata->maxEdgeLength > 0) {
		spa4[0] = arg3[0] - arg2[0];
		spa4[1] = arg3[1] - arg2[1];
		spa4[2] = arg3[2] - arg2[2];

		sum = spa4[0] * spa4[0] + spa4[1] * spa4[1] + spa4[2] * spa4[2];

		if (sum > xraydata->maxEdgeLengthSq) {
			sp84[0][0] = (arg3[0] + arg2[0]) / 2;
			sp84[0][1] = (arg3[1] + arg2[1]) / 2;
			sp84[0][2] = (arg3[2] + arg2[2]) / 2;
			sp9c[0] = 1;

			sp64++;
			sp68 = 0;

			bgChooseXrayVtxColour(&inrange[0], sp84[0], &colours[0], xraydata);
		}

		spa4[0] = arg4[0] - arg3[0];
		spa4[1] = arg4[1] - arg3[1];
		spa4[2] = arg4[2] - arg3[2];

		sum = spa4[0] * spa4[0] + spa4[1] * spa4[1] + spa4[2] * spa4[2];

		if (sum > xraydata->maxEdgeLengthSq) {
			sp84[1][0] = (arg4[0] + arg3[0]) / 2;
			sp84[1][1] = (arg4[1] + arg3[1]) / 2;
			sp84[1][2] = (arg4[2] + arg3[2]) / 2;
			sp9c[1] = 1;

			sp64++;
			sp68 = 1;

			bgChooseXrayVtxColour(&inrange[1], sp84[1], &colours[1], xraydata);
		}

		spa4[0] = arg2[0] - arg4[0];
		spa4[1] = arg2[1] - arg4[1];
		spa4[2] = arg2[2] - arg4[2];

		sum = spa4[0] * spa4[0] + spa4[1] * spa4[1] + spa4[2] * spa4[2];

		if (sum > xraydata->maxEdgeLengthSq) {
			sp84[2][0] = (arg2[0] + arg4[0]) / 2;
			sp84[2][1] = (arg2[1] + arg4[1]) / 2;
			sp84[2][2] = (arg2[2] + arg4[2]) / 2;
			sp9c[2] = 1;

			sp64++;
			sp68 = 2;

			bgChooseXrayVtxColour(&inrange[2], sp84[2], &colours[2], xraydata);
		}
	}

	if (sp64 == 0) {
		if (arg8 || arg9 || arg10) {
			return bgAddXrayTri(gdl, xraydata, arg2, arg3, arg4, arg5, arg6, arg7);
		}
	} else {
		bool render;

		if (arg8 || arg9 || arg10) {
			render = true;
		} else {
			uint32_t mask1 = 0;
			uint32_t mask2 = 0;

			render = true;

			mask1 = (arg2[0] < xraydata->unk000) ? 1 : 0;

			if (arg2[1] < xraydata->unk004) {
				mask1 |= 2;
			}

			if (arg2[2] < xraydata->unk008) {
				mask1 |= 4;
			}

			mask2 = (arg3[0] < xraydata->unk000) ? 1 : 0;

			if (arg3[1] < xraydata->unk004) {
				mask2 |= 2;
			}

			if (arg3[2] < xraydata->unk008) {
				mask2 |= 4;
			}

			if (mask1 == mask2) {
				mask2 = (arg4[0] < xraydata->unk000) ? 1 : 0;

				if (arg4[1] < xraydata->unk004) {
					mask2 |= 2;
				}

				if (arg4[2] < xraydata->unk008) {
					mask2 |= 4;
				}

				if (mask1 == mask2) {
					render = false;
				}
			}
		}

		if (render) {
			if (sp64 == 1) {
				if (sp68 == 0) {
					gdl = bgProcessXrayTri(gdl, xraydata, arg2, sp84[0], arg4, arg5, colours[0], arg7, arg8, inrange[0], arg10);
					gdl = bgProcessXrayTri(gdl, xraydata, arg4, sp84[0], arg3, arg7, colours[0], arg6, arg10, inrange[0], arg9);
				} else if (sp68 == 1) {
					gdl = bgProcessXrayTri(gdl, xraydata, arg3, sp84[1], arg2, arg6, colours[1], arg5, arg9, inrange[1], arg8);
					gdl = bgProcessXrayTri(gdl, xraydata, arg2, sp84[1], arg4, arg5, colours[1], arg7, arg8, inrange[1], arg10);
				} else if (sp68 == 2) {
					gdl = bgProcessXrayTri(gdl, xraydata, arg4, sp84[2], arg3, arg7, colours[2], arg6, arg10, inrange[2], arg9);
					gdl = bgProcessXrayTri(gdl, xraydata, arg3, sp84[2], arg2, arg6, colours[2], arg5, arg9, inrange[2], arg8);
				}
			} else if (sp64 == 2) {
				int v0 = 0;

				if (sp9c[1] == 0) {
					v0 = 1;
				}

				if (sp9c[2] == 0) {
					v0 = 2;
				}

				if (v0 == 0) {
					gdl = bgProcessXrayTri(gdl, xraydata, arg4, sp84[2], sp84[1], arg7, colours[2], colours[1], arg10, inrange[2], inrange[1]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg3, sp84[1], sp84[2], arg6, colours[1], colours[2], arg9, inrange[1], inrange[2]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg2, arg3, sp84[2], arg5, arg6, colours[2], arg8, arg9, inrange[2]);
				} else if (v0 == 1) {
					gdl = bgProcessXrayTri(gdl, xraydata, arg2, sp84[0], sp84[2], arg5, colours[0], colours[2], arg8, inrange[0], inrange[2]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg4, sp84[2], sp84[0], arg7, colours[2], colours[0], arg10, inrange[2], inrange[0]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg3, arg4, sp84[0], arg6, arg7, colours[0], arg9, arg10, inrange[0]);
				} else {
					gdl = bgProcessXrayTri(gdl, xraydata, arg3, sp84[1], sp84[0], arg6, colours[1], colours[0], arg9, inrange[1], inrange[0]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg2, sp84[0], sp84[1], arg5, colours[0], colours[1], arg8, inrange[0], inrange[1]);
					gdl = bgProcessXrayTri(gdl, xraydata, arg4, arg2, sp84[1], arg7, arg5, colours[1], arg10, arg8, inrange[1]);
				}
			} else if (sp64 == 3) {
				gdl = bgProcessXrayTri(gdl, xraydata, arg2, sp84[0], sp84[2], arg5, colours[0], colours[2], arg8, inrange[0], inrange[2]);
				gdl = bgProcessXrayTri(gdl, xraydata, arg3, sp84[1], sp84[0], arg6, colours[1], colours[0], arg9, inrange[1], inrange[0]);
				gdl = bgProcessXrayTri(gdl, xraydata, arg4, sp84[2], sp84[1], arg7, colours[2], colours[1], arg10, inrange[2], inrange[1]);
				gdl = bgProcessXrayTri(gdl, xraydata, sp84[0], sp84[1], sp84[2], colours[0], colours[1], colours[2], inrange[0], inrange[1], inrange[2]);
			}
		}
	}

	return gdl;
}

uint32_t g_BgPointerOffset = 0;
bool g_BgCmdStack[20] = {0};
int g_BgCmdStackIndex = 0;
uint32_t g_BgCmdResult = BGRESULT_TRUE;

Gfx *bgRenderGdlInXray(Gfx *gdl, int8_t *readgdl, Vtx *vertices, int16_t arg3[3])
{
	uint8_t *verticesuint8_t = (uint8_t *) vertices;
	struct xraydata xraydata;
	struct stagetableentry *stage = stageGetCurrent();
	int16_t dmemvertices[16][3];
	uint32_t dmemcolours[16];
	bool inrange[16];

	xraydata.unk00c = g_Vars.currentplayer->eraserbgdist;
	xraydata.unk010 = xraydata.unk00c * xraydata.unk00c;
	xraydata.unk018 = xraydata.unk00c * 0.25f;
	xraydata.unk01c = g_Vars.currentplayer->eraserpropdist / xraydata.unk00c;

	if (xraydata.unk01c > 0.7f) {
		xraydata.unk01c = 0.7f;
	}

	xraydata.unk014 = 0.250f;
	xraydata.maxEdgeLength = stage->maxXRayEdgeLength;
	xraydata.maxEdgeLengthSq = xraydata.maxEdgeLength * xraydata.maxEdgeLength;
	xraydata.unk000 = arg3[0];
	xraydata.unk004 = arg3[1];
	xraydata.unk008 = arg3[2];
	xraydata.numtris = 0;
	xraydata.numvertices = 0;

	while (true) {
		if (readgdl[GFX_W0_BYTE(0)] == G_ENDDL) {
			break;
		}

		if (readgdl[GFX_W0_BYTE(0)] == G_MTX) {
			// empty
		} else if (readgdl[GFX_W0_BYTE(0)] == G_VTX) {
			Gfx *cmd = (Gfx *) readgdl;
			int dmemindex = cmd->bytes[GFX_W0_BYTE(1)] & 0xf;
			int numvertices = ((uint32_t) cmd->bytes[GFX_W0_BYTE(1)] >> 4) + 1;
			uint32_t offset = UNSEGADDR(cmd->words.w1) & 0xffffff;

			for (int i = 0; i < numvertices; i++) {
				Vtx *vtx = (Vtx *) (verticesuint8_t + offset);

				dmemvertices[dmemindex + i][0] = vtx->x;
				dmemvertices[dmemindex + i][1] = vtx->y;
				dmemvertices[dmemindex + i][2] = vtx->z;

				bgChooseXrayVtxColour(&inrange[i], dmemvertices[dmemindex + i], &dmemcolours[dmemindex + i], &xraydata);

				offset += sizeof(Vtx);
			}
		} else if (readgdl[GFX_W0_BYTE(0)] == G_TRI1) {
			Gfx *cmd = (Gfx *) readgdl;
			int16_t x = cmd->tri.tri.v[GFX_TRI_VTX(0)] / 10;
			int16_t y = cmd->tri.tri.v[GFX_TRI_VTX(1)] / 10;
			int16_t z = cmd->tri.tri.v[GFX_TRI_VTX(2)] / 10;

			gdl = bgProcessXrayTri(gdl, &xraydata, dmemvertices[x], dmemvertices[y], dmemvertices[z], dmemcolours[x], dmemcolours[y], dmemcolours[z], inrange[x], inrange[y], inrange[z]);
		} else if (readgdl[GFX_W0_BYTE(0)] == G_TRI4) {
			Gfx *cmd = (Gfx *) readgdl;
			int16_t x;
			int16_t y;
			int16_t z;

			x = cmd->tri4.x1;
			y = cmd->tri4.y1;
			z = cmd->tri4.z1;

			gdl = bgProcessXrayTri(gdl, &xraydata, dmemvertices[x], dmemvertices[y], dmemvertices[z], dmemcolours[x], dmemcolours[y], dmemcolours[z], inrange[x], inrange[y], inrange[z]);

			x = cmd->tri4.x2;
			y = cmd->tri4.y2;
			z = cmd->tri4.z2;

			gdl = bgProcessXrayTri(gdl, &xraydata, dmemvertices[x], dmemvertices[y], dmemvertices[z], dmemcolours[x], dmemcolours[y], dmemcolours[z], inrange[x], inrange[y], inrange[z]);

			x = cmd->tri4.x3;
			y = cmd->tri4.y3;
			z = cmd->tri4.z3;

			gdl = bgProcessXrayTri(gdl, &xraydata, dmemvertices[x], dmemvertices[y], dmemvertices[z], dmemcolours[x], dmemcolours[y], dmemcolours[z], inrange[x], inrange[y], inrange[z]);

			x = cmd->tri4.x4;
			y = cmd->tri4.y4;
			z = cmd->tri4.z4;

			gdl = bgProcessXrayTri(gdl, &xraydata, dmemvertices[x], dmemvertices[y], dmemvertices[z], dmemcolours[x], dmemcolours[y], dmemcolours[z], inrange[x], inrange[y], inrange[z]);
		}

		readgdl += sizeof(Gfx);
	}

	gdl = bgRenderXrayData(gdl, &xraydata);

	return gdl;
}

Gfx *bgRenderRoomXrayPass(Gfx *gdl, int roomnum, struct roomblock *block, bool recurse, int16_t arg4[3])
{
	struct player *player = g_Vars.currentplayer;

	if (block == NULL) {
		return gdl;
	}

	switch (block->type) {
	case ROOMBLOCKTYPE_LEAF:
		gdl = bgRenderGdlInXray(gdl, (int8_t *) block->gdl, block->vertices, arg4);

		if (recurse) {
			gdl = bgRenderRoomXrayPass(gdl, roomnum, block->next, true, arg4);
		}
		break;
	case ROOMBLOCKTYPE_PARENT:
		if (block->child != NULL) {
			struct roomblock *child1 = block->child;
			struct roomblock *child2 = child1->next;
			struct coord *coords = block->unk0c;
			struct coord sp34;
			struct coord sp28;
			float sum;

			sp34.x = coords[1].x;
			sp34.y = coords[1].y;
			sp34.z = coords[1].z;

			sp28.x = coords[0].x - player->cam_pos.x;
			sp28.y = coords[0].y - player->cam_pos.y;
			sp28.z = coords[0].z - player->cam_pos.z;

			sum = sp34.f[0] * sp28.f[0] + sp34.f[1] * sp28.f[1] + sp34.f[2] * sp28.f[2];

			if (sum < 0.0f) {
				gdl = bgRenderRoomXrayPass(gdl, roomnum, child1, false, arg4);
				gdl = bgRenderRoomXrayPass(gdl, roomnum, child2, false, arg4);
			} else {
				gdl = bgRenderRoomXrayPass(gdl, roomnum, child2, false, arg4);
				gdl = bgRenderRoomXrayPass(gdl, roomnum, child1, false, arg4);
			}

			if (recurse) {
				gdl = bgRenderRoomXrayPass(gdl, roomnum, block->next, true, arg4);
			}
		}
		break;
	}

	return gdl;
}

/**
 * Render the given room for the purpose of the FarSight or xray scanner.
 */
Gfx *bgRenderRoomInXray(Gfx *gdl, int roomnum)
{
	struct coord sp54;
	struct coord roomoffset;
	int16_t sp40[3];
	struct player *player = g_Vars.currentplayer;

	if (roomnum == 0 || roomnum >= g_Vars.roomcount) {
		return gdl;
	}

	if (g_Rooms[roomnum].loaded240 == 0) {
		if (g_NumRoomLoadsLeftThisFrame > 0) {
			g_NumRoomLoadsLeftThisFrame--;
			bgLoadRoom(roomnum);
		}
	}

	if (g_Rooms[roomnum].loaded240 == 0) {
		g_NumRoomLoadsLeftThisFrame--;
	}

	if (g_Rooms[roomnum].loaded240 == 0) {
		return gdl;
	}

	roomGetPos(roomnum, &roomoffset);

	sp54.x = player->eraserpos.x - roomoffset.x;
	sp54.y = player->eraserpos.y - roomoffset.y;
	sp54.z = player->eraserpos.z - roomoffset.z;

	sp40[0] = sp54.f[0];
	sp40[1] = sp54.f[1];
	sp40[2] = sp54.f[2];

	gdl = roomApplyMtx(gdl, roomnum);
	gdl = bgRenderRoomXrayPass(gdl, roomnum, g_Rooms[roomnum].gfxdata->opablocks, true, sp40);
	gdl = bgRenderRoomXrayPass(gdl, roomnum, g_Rooms[roomnum].gfxdata->xlublocks, true, sp40);

	g_Rooms[roomnum].loaded240 = 1;

	return gdl;
}

Gfx *bgRenderSceneInXray(Gfx *gdl)
{
	RoomNum *roomnumptr;
	RoomNum *room;
	int16_t i;
	int j;
	RoomNum roomnumsbyprop[200];
	struct prop *prop;
	struct prop **ptr;
	int k;

	roomnumptr = roomnumsbyprop;

	for (ptr = g_Vars.onscreenprops; ptr < g_Vars.endonscreenprops; ptr++) {
		*roomnumptr = 0;
		prop = *ptr;

		if (prop) {
			room = prop->rooms;

			while (*room != -1) {
				if (g_Rooms[*room].flags & ROOMFLAG_ONSCREEN) {
					*roomnumptr = *room;
					break;
				}

				room++;
			}
		}

		roomnumptr++;
	}

	gdl = envStopFog(gdl);

	gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH);
	gfx_Set_Geometry_Mode(gdl++, G_SHADE | G_SHADING_SMOOTH);
	gDPSetCombineMode(gdl++, G_CC_SHADE, G_CC_SHADE);
	gfx_Set_Texture_Filter(gdl++, G_TF_BILERP);
	gfx_Set_Cycle_Type(gdl++, G_CYC_1CYCLE);
	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	texSelect(&gdl, NULL, 2, 0, 2, 1, NULL);

	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);

	// Render BG
	gdl = bgScissorToViewport(gdl);

	for (i = g_BgMinDrawOrder; i <= g_BgMaxDrawOrder; i++) {
		for (j = 0; j < g_BgNumDrawSlots; j++) {
			struct drawslot *thing = &g_BgDrawSlots[j];

			if (thing->draworder == i) {
				gdl = bgRenderRoomInXray(gdl, thing->roomnum);
			}
		}
	}

	// Render props
	gdl = bgScissorToViewport(gdl);

	gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	if (g_BgMinDrawOrder); \
	if (g_BgNumDrawSlots); \
	for (i = g_BgMaxDrawOrder; i >= g_BgMinDrawOrder; i--) {
		for (k = 0; k < g_BgNumDrawSlots; k++) {
			struct drawslot *thing = &g_BgDrawSlots[k];

			if (thing->draworder == i) {
				gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

				gdl = bgScissorWithinViewportF(gdl, thing->box.xmin, thing->box.ymin, thing->box.xmax, thing->box.ymax);

				gfx_Matrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

				if (thing->roomnum == -1) {
					gdl = propsRender(gdl, 0, RENDERPASS_XLU, roomnumsbyprop);
				}

				gdl = propsRender(gdl, thing->roomnum, RENDERPASS_XLU, roomnumsbyprop);
			}
		}
	}

	gdl = skyRenderSuns(gdl, true);

	return gdl;
}

Gfx *bgRenderScene(Gfx *gdl)
{
	int stagenum = g_Vars.stagenum;
	int firstroomnum = -1;
	int i;
	int roomnum;
	RoomNum roomnumsbyprop[200];
	struct prop **ptr;
	struct drawslot *thing;
	RoomNum *roomnumptr;
	struct prop *prop;
	int16_t tmp;
	RoomNum *room;
	int16_t roomorder[250]; // 60 to 250
	RoomNum roomnums[250]; // 60 to 250

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		gdl = bgRenderSceneInXray(gdl);
		return gdl;
	}

	// Build an array of all room numbers, and a parallel array that contains
	// their draw order (as defined by portal code).
	for (roomnum = 0; roomnum < g_BgNumDrawSlots; roomnum++) {
		roomorder[roomnum] = g_BgDrawSlots[roomnum].draworder;
		roomnums[roomnum] = roomnum;
	}

	// Sort them by distance ascending
	if (g_BgNumDrawSlots >= 2) {
		do {
			i = false;

			for (roomnum = 0; roomnum < g_BgNumDrawSlots - 1; roomnum++) {
				if (roomorder[roomnum + 1] < roomorder[roomnum]) {
					tmp = roomorder[roomnum];
					roomorder[roomnum] = roomorder[roomnum + 1];
					roomorder[roomnum + 1] = tmp;

					tmp = roomnums[roomnum];
					roomnums[roomnum] = roomnums[roomnum + 1];
					roomnums[roomnum + 1] = tmp;

					i = true;
				}
			}
		} while (i);
	}

	gdl = bgScissorToViewport(gdl);

	// Render special "always on" rooms, such as the Defection moon,
	// Attack Ship planet, and other sky tricks that are implemented as rooms
	if (!USINGDEVICE(DEVICE_NIGHTVISION) && !USINGDEVICE(DEVICE_IRSCANNER)
			&& (stagenum == g_Stages[STAGEINDEX_INFILTRATION].id
				|| stagenum == g_Stages[STAGEINDEX_RESCUE].id
				|| stagenum == g_Stages[STAGEINDEX_ESCAPE].id
				|| stagenum == g_Stages[STAGEINDEX_MAIANSOS].id
				|| stagenum == g_Stages[STAGEINDEX_SKEDARRUINS].id
				|| stagenum == g_Stages[STAGEINDEX_WAR].id
				|| stagenum == g_Stages[STAGEINDEX_DEFECTION].id
				|| stagenum == g_Stages[STAGEINDEX_EXTRACTION].id
				|| stagenum == g_Stages[STAGEINDEX_MBR].id
				|| stagenum == g_Stages[STAGEINDEX_ATTACKSHIP].id)) {
		gdl = envStopFog(gdl);
		gdl = viSetCamNoTranslation(gdl);

		roomnum = -1;

		if (stagenum == g_Stages[STAGEINDEX_INFILTRATION].id
				|| stagenum == g_Stages[STAGEINDEX_RESCUE].id
				|| stagenum == g_Stages[STAGEINDEX_ESCAPE].id
				|| stagenum == g_Stages[STAGEINDEX_MAIANSOS].id) {
			roomnum = 0x0f;
		} else if (stagenum == g_Stages[STAGEINDEX_SKEDARRUINS].id
				|| stagenum == g_Stages[STAGEINDEX_WAR].id) {
			roomnum = 0x02;
		} else if (stagenum == g_Stages[STAGEINDEX_DEFECTION].id
				|| stagenum == g_Stages[STAGEINDEX_EXTRACTION].id
				|| stagenum == g_Stages[STAGEINDEX_MBR].id) {
			roomnum = 0x01;
		} else if (stagenum == g_Stages[STAGEINDEX_ATTACKSHIP].id) {
			roomnum = 0x71;
		}

		if (stagenum == STAGE_DEFECTION
					|| stagenum == STAGE_EXTRACTION
					|| stagenum == STAGE_MBR      // Enable stars in Mr. Blonde's Revenge
					|| stagenum == STAGE_INFILTRATION
					|| stagenum == STAGE_ESCAPE
					|| stagenum == STAGE_ATTACKSHIP) {
			gdl = textConfigureGfxPipeline(gdl);

			gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

			gdl = envStopFog(gdl);
			gdl = starsRender(gdl);
			gdl = text0f153780(gdl);
			gdl = viSetCamNoTranslation(gdl);
		}

		if (roomnum != -1) {
			if (!g_Rooms[roomnum].loaded240) {
				bgLoadRoom(roomnum);
			}

			gdl = bgRenderRoomOpaque(gdl, roomnum);
		}
	}

	gdl = skyRenderSuns(gdl, false);

	// Build an array of room numbers per onscreen prop.
	// For each onscreen prop there is exactly one entry in the roomnumsbyprop array.
	roomnumptr = roomnumsbyprop;

	for (ptr = g_Vars.onscreenprops; ptr < g_Vars.endonscreenprops; ptr++) {
		*roomnumptr = 0;
		prop = *ptr;

		if (prop) {
			room = prop->rooms;

			while (*room != -1) {
				if (g_Rooms[*room].flags & ROOMFLAG_ONSCREEN) {
					*roomnumptr = *room;
					break;
				}

				room++;
			}
		}

		roomnumptr++;
	}

	// Render the opaque passes
	for (i = 0; i < g_BgNumDrawSlots; i++) {
		roomnum = roomnums[i];

		if (firstroomnum < 0) {
			firstroomnum = g_BgDrawSlots[roomnum].roomnum;
			if (firstroomnum);
		}

		thing = &g_BgDrawSlots[roomnum];

		// Render prop opaque components - pre BG pass
		gfx_Matrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
		gdl = envStopFog(gdl);

		if (firstroomnum == thing->roomnum) {
			gdl = propsRender(gdl, 0, RENDERPASS_OPA_PREBG, roomnumsbyprop);
		}

		gdl = propsRender(gdl, thing->roomnum, RENDERPASS_OPA_PREBG, roomnumsbyprop);

		// Render BG opaque components
		gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		gdl = bgScissorWithinViewportF(gdl, thing->box.xmin, thing->box.ymin, thing->box.xmax, thing->box.ymax);
		gdl = envStartFog(gdl);
		gdl = bgRenderRoomOpaque(gdl, thing->roomnum);

		// Render prop opaque components - post BG pass
		gfx_Matrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		gdl = envStopFog(gdl);

		if (firstroomnum == thing->roomnum) {
			gdl = propsRender(gdl, 0, RENDERPASS_OPA_POSTBG, roomnumsbyprop);
		}

		gdl = propsRender(gdl, thing->roomnum, RENDERPASS_OPA_POSTBG, roomnumsbyprop);
	}

	gdl = envStopFog(gdl);
	gdl = bgScissorToViewport(gdl);

	// Render wall hits
	gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	if (g_Vars.currentplayer->visionmode != VISIONMODE_XRAY) {
		for (i = 0; i < g_BgNumDrawSlots; i++) {
			roomnum = roomnums[i];
			gdl = wallhitRenderBgHits(g_BgDrawSlots[roomnum].roomnum, gdl);
		}
	}

	for (i = g_BgNumDrawSlots - 1; i >= 0; i--) {
		roomnum = roomnums[i];

		gfx_Matrix(gdl++, camGetOrthogonalMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		thing = &g_BgDrawSlots[roomnum];

		// Render BG translucent components
		gfx_Clear_Geometry_Mode(gdl++, G_CULL_BOTH); // Ben's comment: this fixes transparent textures like railings from going invisible in levels exported by the Setup Editor, though it doesn't fix their sorting problems
		gdl = bgScissorWithinViewportF(gdl, thing->box.xmin, thing->box.ymin, thing->box.xmax, thing->box.ymax);
		gdl = envStartFog(gdl);
		gdl = bgRenderRoomXlu(gdl, thing->roomnum);

		gfx_Matrix(gdl++, camGetPerspectiveMtxL(), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

		gdl = envStopFog(gdl);

		// Render prop translucent components
		if (firstroomnum == thing->roomnum) {
			gdl = propsRender(gdl, 0, RENDERPASS_XLU, roomnumsbyprop);
		}

		gdl = propsRender(gdl, thing->roomnum, RENDERPASS_XLU, roomnumsbyprop);
	}

	return gdl;
}

Gfx *bgRenderArtifacts(Gfx *gdl)
{
	int i;

	//if (g_Vars.mplayerisrunning == false && g_NumRoomsWithGlares > 0) {
		gdl = artifactsConfigureForGlares(gdl);

		for (i = 0; i < g_NumRoomsWithGlares; i++) {
			gdl = artifactsRenderGlaresForRoom(gdl, g_GlareRooms[i]);
		}

		gdl = artifactsUnconfigureForGlares(gdl);
	//}

	gdl = skyRenderArtifacts(gdl);

	return gdl;
}

void bgLoadFile(void *memaddr, uint32_t offset, uint32_t len)
{
	fileLoadPartToAddr(g_Stages[g_StageIndex].bgfileid, memaddr, offset, len);
}

int bgGetStageIndex(int stagenum)
{
	int index = -1;
	int i;

	for (i = 0; i != ARRAYCOUNT(g_Stages); i++) {
		if (g_Stages[i].id == stagenum) {
			index = i;
		}
	}

	return index;
}

float bgCalculatePortalSurfaceArea(int portalnum)
{
	struct portalvertices *pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum].verticesoffset);
	int count = pvertices->count;
	int i;
	int j;
	float sum = 0.0f;
	float sp90[3];
	float sp84[3];
	float sp78[3];

	for (i = 2; i < count; i++) {
		for (j = 0; j < 3; j++) {
			sp84[j] = pvertices->vertices[i - 1].f[j] - pvertices->vertices[0].f[j];
			sp78[j] = pvertices->vertices[i].f[j] - pvertices->vertices[i - 1].f[j];
		}

		sp90[0] = sp84[1] * sp78[2] - sp84[2] * sp78[1];
		sp90[1] = -sp84[0] * sp78[2] + sp84[2] * sp78[0];
		sp90[2] = sp84[0] * sp78[1] - sp84[1] * sp78[0];

		sum += sqrtf(sp90[0] * sp90[0] + sp90[1] * sp90[1] + sp90[2] * sp90[2]) * 0.5f;
	}

	return sum;
}

/**
 * Extracts and inflates primary data (room/portal/light tables) from the
 * BG file to memory, ensures required textures are loaded, and sets some
 * global pointers to each table within the primary data.
 *
 * -- Overview -----------------------------------------------------------------
 *
 * The structure of a BG File is:
 *
 * - Section 1:
 *   - Primary data (compressed):
 *     - Room table, with pointers to their room gfx data
 *     - Portal table
 *     - Portal command list
 *     - Light table
 *   - For each room, a compressed binary containing gfx data
 * - Section 2 (compressed) - list of texture IDs to load
 * - Section 3 (compressed) - unknown and not read by this function
 *
 * Each section has a header (uncompressed) containing size information:
 *
 * Section 1 header is 0x0c bytes long:
 * - 4 bytes decompressed size of primary data
 * - 4 bytes compressed size of section 1 in its entirety
 * - 4 bytes compressed size of primary data
 *
 * Section 2 and 3 headers are 0x04 bytes long:
 * - 2 bytes decompressed size of data (mask with 0x7fff)
 * - 2 bytes compressed size of data
 *
 * -- Primary Data (compressed) ------------------------------------------------
 *
 * Header is 0x18 bytes long:
 * - 4 bytes null
 * - 4 bytes pointer to room table
 * - 4 bytes pointer to portal table
 * - 4 bytes pointer to portal commands list
 * - 4 bytes pointer to light table
 * - 4 bytes null
 */
void bgReset(int stagenum)
{
	uint8_t *header;
	uint8_t headerbuffer[0x50];
	uint32_t numtextures;
	int16_t *section2;
	int j;
	int i;
	uint32_t primcompsize;
	uint32_t inflatedsize; // used for both primary and section 2
	uint32_t section2compsize;
	uint32_t section2start;
	uint32_t section1compsize;
	uintptr_t scratch;

	g_RoomStreamingBoostTimer = 8;
	g_BgUnloadDelay240 = 120;
	g_BgUnloadDelay240_2 = 120;
	g_StageIndex = bgGetStageIndex(stagenum);

	if (g_StageIndex < 0) {
		g_StageIndex = 0;
	}

	// Copy section 1 header to stack and parse into variables
	header = (uint8_t *)ALIGN16((uintptr_t)headerbuffer);
	bgLoadFile(header, 0, 0x40);
	preprocessBgSection1Header(header, 0x40);
	inflatedsize = *(uint32_t *)&header[0];
	section1compsize = *(uint32_t *)&header[4];
	primcompsize = *(uint32_t *)&header[8];
	g_BgPointerOffset = inflatedsize - primcompsize;
	g_BgPointerOffset -= 0xc; //0xc probably accounts for fixed header data

#ifdef PLATFORM_64BIT
	inflatedsize = romdataFileGetEstimatedSize(inflatedsize, LOADTYPE_BG);
#endif

	inflatedsize = ALIGN16(inflatedsize);

	// Allocate space for the primary bg data
	// An extra 0x8000 or so is given as temporary scratch space
	g_BgPrimaryData = mempAlloc(ALIGN16(inflatedsize + 0x8010), MEMPOOL_STAGE);

	// Set up pointer to scratch space
	scratch = (uintptr_t) g_BgPrimaryData + inflatedsize - primcompsize;
	scratch = ALIGN16(scratch + 0x8000);

	g_LoadType = LOADTYPE_BG;

	// Copy section 1 header + compressed primary to scratch space
	bgLoadFile((uint8_t *) scratch, 0, ALIGN16(primcompsize + 15));

	// Inflate primary data to the start of the buffer
	scratch += 0xc;
	bgInflate((uint8_t *) scratch, g_BgPrimaryData, primcompsize);

	preprocessBgSection1(g_BgPrimaryData, inflatedsize, 0x0f000000);

	// Shrink the allocation (ie. free the scratch space)
	mempRealloc(g_BgPrimaryData, inflatedsize, MEMPOOL_STAGE);

	// Load the section 2 header
	section2start = section1compsize + 0xc;

	bgLoadFile(header, section2start, 0x40);
	preprocessBgSection2Header(header, 0x40);

	inflatedsize = (*(int16_t *) &header[0] & 0x7fff) - 1;
	section2compsize = *(int16_t *) &header[2];
	inflatedsize = (inflatedsize | 0xf) + 1;

	section2 = mempAlloc(inflatedsize + section2compsize, MEMPOOL_STAGE);
	scratch = (uintptr_t) section2 + inflatedsize;

	// Load compressed data from ROM to scratch
	bgLoadFile((uint8_t *) scratch, section2start + 4, ((section2compsize - 1) | 0xf) + 1);

	// Inflate section 2 to the start of the buffer
	bgInflate((uint8_t *) scratch, (uint8_t *) section2, section2compsize);

	// Iterate texture IDs and ensure they're loaded
	inflatedsize = (*(int16_t *) &header[0] & 0x7fff) >> 1;

	for (i = 0; i ^ inflatedsize; i++) {
		texLoadFromTextureNum(section2[i] & 0xffff, NULL);
	}

	// Free section 2
	mempRealloc(section2, 0, MEMPOOL_STAGE);

	g_BgSection3 = section2start + section2compsize + 4;

	var800a4920 = *(uint32_t *)g_BgPrimaryData;

	if (var800a4920 == 0) {
		g_BgPrimaryData2 = (uintptr_t*)g_BgPrimaryData;
		g_BgRooms = (struct bgroom *)(g_BgPrimaryData2[1] + g_BgPrimaryData - 0x0f000000);
		g_Vars.roomcount = 0;

		for (j = 1; g_BgRooms[j].ptr_gfxdata != 0; j++) {
			g_Vars.roomcount++;
		}

		g_BgPortals = (struct bgportal *)(g_BgPrimaryData2[2] + g_BgPrimaryData - 0x0f000000);

		if (g_BgPrimaryData2[3] == 0) {
			g_BgCommands = NULL;
		} else {
			g_BgCommands = (struct bgcmd *)(g_BgPrimaryData2[3] + g_BgPrimaryData - 0x0f000000);
		}

		if (g_BgPrimaryData2[4] == 0) {
			g_BgLightsFileData = NULL;
		} else {
			g_BgLightsFileData = (uint8_t *)(g_BgPrimaryData2[4] + g_BgPrimaryData - 0x0f000000);
		}

		if (g_BgPrimaryData2[5] == 0) {
			g_BgStanThings = NULL;
		} else {
			g_BgStanThings = (float *)(g_BgPrimaryData2[5] + g_BgPrimaryData - 0x0f000000);
		}
	}
}

void bgBuildTables(int stagenum)
{
	int i;
	int j;
	int k;
	uint32_t r;
	uint8_t *header;
	int numportals;
	int index;
	float divisor;
	int16_t lightindex;
	int candportalnum;
	int numportalsthisroom;
	bool swap;
	struct portalvertices *pvertices;
	struct portalmetric *metric;
	int numvertices;
	struct portalmetric tmp;
	uint8_t *scratch;
	uint8_t headerbuffer[0x50];
	int thisneighbournum;
	int offset;
	uint32_t inflatedsize;
	uint32_t section3compsize;
	int16_t *bboxptr;
	uint8_t *section3;
	int16_t *datalenptr;
	uint8_t *numlightsptr;

	g_Rooms = mempAlloc(ALIGN16(g_Vars.roomcount * sizeof(struct room)), MEMPOOL_STAGE);
	g_BgDrawSlotsByRoom = mempAlloc(ALIGN16(g_Vars.roomcount * sizeof(struct drawslotpointer)), MEMPOOL_STAGE);

	for (i = 0; i < g_Vars.roomcount; i++) {
		g_BgDrawSlotsByRoom[i].updatedframe = 0xffff;
		g_BgDrawSlotsByRoom[i].slotnum = 0;
	}

	if (g_Vars.mplayerisrunning) {
		g_MpRoomVisibility = mempAlloc(ALIGN16(g_Vars.roomcount), MEMPOOL_STAGE);

		for (i = 0; i < g_Vars.roomcount; i++) {
			g_MpRoomVisibility[i] = 0;
		}
	}

	for (i = 0; i < g_Vars.roomcount; i++) {
		g_Rooms[i].vtxbatches = NULL;
		g_Rooms[i].numlights = 0;
		g_Rooms[i].lightindex = 0;
		g_Rooms[i].flags = 0;
		g_Rooms[i].unk4d = 0;
		g_Rooms[i].lightop = 0;
		g_Rooms[i].unk4e_04 = 0;
		g_Rooms[i].extra_flags = 0;
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		g_Vars.playerstats[i].scale_bg2gfx = 1.0f;
	}

	if (var800a4920 == 0) {
		numportals = 0;

		for (i = 0; g_BgPortals[i].verticesoffset != 0; i++) {
			numportals++;
		}

		g_BgNumPortalCameraCacheItems = numportals;
		g_PortalCameraCache = mempAlloc(ALIGN16(g_BgNumPortalCameraCacheItems * sizeof(struct portalcamcacheitem)), MEMPOOL_STAGE);

		// Iterate the portals and update their verticesoffset value. In
		// storage, the g_BgPortals array is followed by vertice data, and each
		// portal's verticesoffset value is an index into the vertice data.
		// Here, the unk00 value is being converted to an offset relative to the
		// start of the g_BgPortals array. Start by initialising offset past the
		// end of the portal array, which is the start of the vertice data.
		offset = numportals * sizeof(struct bgportal);
		offset += sizeof(struct bgportal);

		// Because each group of vertices is variable length, the portals can't
		// be iterated in order and have their offset calculated. The vertice
		// data has to be iterated in storage order, then iterate all portals to
		// see if any refer to this index.
		for (i = 1; true; i++) {
			for (j = 0; j < numportals; j++) {
				if (g_BgPortals[j].verticesoffset == i) {
					g_BgPortals[j].verticesoffset = offset;
				}
			}

			pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + offset);

			if (pvertices->count <= 0) {
				break;
			}

			offset += pvertices->count * 12;
			offset += 4;
		}

		// Calculate g_RoomPortals: An array of portal numbers, ordered by room
		// number ascending. Each room struct contains an index into this array
		// where its portal numbers start.
		index = 0;
		g_RoomPortals = mempAlloc(ALIGN16((numportals == 0 ? 1 : numportals) * sizeof(int16_t *)), MEMPOOL_STAGE);

		g_Vars.roomportalrecursionlimit = 0;

		for (i = 0; i < g_Vars.roomcount; i++) {
			numportalsthisroom = 0;

			g_Rooms[i].roomportallistoffset = index;

			for (j = 0; j < numportals; j++) {
				if (i == g_BgPortals[j].roomnum1) {
					g_RoomPortals[index] = j;
					numportalsthisroom++;
					index++;
				}

				if (i == g_BgPortals[j].roomnum2) {
					g_RoomPortals[index] = j;
					numportalsthisroom++;
					index++;
				}
			}


			g_Rooms[i].numportals = numportalsthisroom;

			if (numportalsthisroom > g_Vars.roomportalrecursionlimit) {
				g_Vars.roomportalrecursionlimit = numportalsthisroom;
			}
		}

		// Sort the portal numbers in g_RoomPortals within their room groups.
		// Sorting is done by neighbouring room number ascending.
		//
		// @bug: The k loop doesn't reset to j after doing a swap, which means
		// some items may not be sorted correctly. This isn't a problem if the
		// data on ROM is already sorted, or if they actually don't need to be
		// sorted.
		for (i = 0; i < g_Vars.roomcount; i++) {
			for (j = 0; j < g_Rooms[i].numportals; j++) {
				if (g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum1 == i) {
					thisneighbournum = g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum2;
				} else {
					thisneighbournum = g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum1;
				}

				for (k = j; k < g_Rooms[i].numportals; k++) {
					swap = false;

					if (i == g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + k]].roomnum1) {
						if (g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + k]].roomnum2 < thisneighbournum) {
							swap = true;
						}
					} else {
						if (g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + k]].roomnum1 < thisneighbournum) {
							swap = true;
						}
					}

					if (swap) {
						candportalnum = g_RoomPortals[g_Rooms[i].roomportallistoffset + k];
						g_RoomPortals[g_Rooms[i].roomportallistoffset + k] = g_RoomPortals[g_Rooms[i].roomportallistoffset + j];
						g_RoomPortals[g_Rooms[i].roomportallistoffset + j] = candportalnum;

						if (g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum1 == i) {
							thisneighbournum = g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum2;
						} else {
							thisneighbournum = g_BgPortals[g_RoomPortals[g_Rooms[i].roomportallistoffset + j]].roomnum1;
						}
						k = j; // Fix
					}
				}
			}
		}

		g_PortalMetrics = mempAlloc(ALIGN16(numportals * sizeof(struct portalmetric)), MEMPOOL_STAGE);

		for (i = 0; i < numportals; i++) {
			// Clockwise vertices will cause the normal to point towards the viewer
			tmp.normal.x = 0.0f;
			tmp.normal.y = 0.0f;
			tmp.normal.z = 0.0f;

			pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[i].verticesoffset);

			for (j = 0; j < pvertices->count; j++) {
				struct coord *next = &pvertices->vertices[(j + 1) % pvertices->count];

				tmp.normal.x += (pvertices->vertices[j].y - next->y) * (pvertices->vertices[j].z + next->z);
				tmp.normal.y += (pvertices->vertices[j].z - next->z) * (pvertices->vertices[j].x + next->x);
				tmp.normal.z += (pvertices->vertices[j].x - next->x) * (pvertices->vertices[j].y + next->y);
			}

			divisor = -sqrtf(tmp.normal.f[0] * tmp.normal.f[0] + tmp.normal.f[1] * tmp.normal.f[1] + tmp.normal.f[2] * tmp.normal.f[2]);

			tmp.normal.x /= divisor;
			tmp.normal.y /= divisor;
			tmp.normal.z /= divisor;

			tmp.min = MAXFLOAT;
			tmp.max = MINFLOAT;

			for (j = 0; j < pvertices->count; j++) {
				float value = pvertices->vertices[j].f[0] * tmp.normal.f[0]
					+ pvertices->vertices[j].f[1] * tmp.normal.f[1]
					+ pvertices->vertices[j].f[2] * tmp.normal.f[2];

				if (value < tmp.min) {
					tmp.min = value;
				}

				if (value > tmp.max) {
					tmp.max = value;
				}
			}

			metric = &g_PortalMetrics[i];
			metric->normal.x = tmp.normal.x;
			metric->normal.y = tmp.normal.y;
			metric->normal.z = tmp.normal.z;
			metric->min = tmp.min;
			metric->max = tmp.max;
		}

		portal0f0b65a8(numportals);

		if (g_BgCommands != NULL) {
			for (i = 0; g_BgCommands[i].type != BGCMD_END; i++) {
				if (g_BgCommands[i].type == BGCMD_PORTALARG) {
					g_BgCommands[i].param = bgFindPortalByVertices((void *)((intptr_t)g_BgPrimaryData - 0x0f000000 + g_BgCommands[i].param));
				}
			}
		}

		for (i = 0; i < g_Vars.roomcount; i++) {
			g_Rooms[i].flags = 0;
			g_Rooms[i].snakecount = 0;
			g_Rooms[i].unk07 = 1;
			g_Rooms[i].loaded240 = 0;
			g_Rooms[i].gfxdata = NULL;
			g_Rooms[i].gfxdatalen = -1;
			g_Rooms[i].opawallhits = NULL;
			g_Rooms[i].xluwallhits = NULL;
		}

		roomsReset();

		g_Rooms[0].bbmin[0] = 0.0f;
		g_Rooms[0].bbmin[1] = 0.0f;
		g_Rooms[0].bbmin[2] = 0.0f;
		g_Rooms[0].bbmax[0] = 0.0f;
		g_Rooms[0].bbmax[1] = 0.0f;
		g_Rooms[0].bbmax[2] = 0.0f;

		dyntexReset();

		// Load section 3 of the BG file. To do this, the header of the BG file
		// must be loaded first as it contains the offset to section 3. Then
		// section 3 is loaded and inflated. Data is read out of section 3, then
		// the section 3 allocation is resized to 0, effectively freeing it.

		// Load and read the header
		header = (uint8_t *)ALIGN16((uintptr_t)headerbuffer);
		bgLoadFile(header, g_BgSection3, 0x40);
		preprocessBgSection3Header(header, 0x40);
		inflatedsize = (*(uint16_t *)&header[0] & 0x7fff) - 1;
		section3compsize = *(uint16_t *)&header[2];
		inflatedsize = (inflatedsize | 0xf) + 1;

		// Load and inflate section 3
		section3 = mempAlloc(inflatedsize + section3compsize, MEMPOOL_STAGE);
		scratch = section3 + inflatedsize;

		bgLoadFile(scratch, g_BgSection3 + 4, ((section3compsize - 1) | 0xf) + 1);
		bgInflate(scratch, section3, section3compsize);
		preprocessBgSection3(section3, section3compsize);

		// Section 3 starts with a table of room bounding boxes
		bboxptr = (int16_t *) section3;

		for (r = 1; r < g_Vars.roomcount; r++) {
			// Calculate bounding box
			g_Rooms[r].bbmin[0] = *bboxptr + g_BgRooms[r].pos.x; bboxptr++;
			g_Rooms[r].bbmin[1] = *bboxptr + g_BgRooms[r].pos.y; bboxptr++;
			g_Rooms[r].bbmin[2] = *bboxptr + g_BgRooms[r].pos.z; bboxptr++;
			g_Rooms[r].bbmax[0] = *bboxptr + g_BgRooms[r].pos.x; bboxptr++;
			g_Rooms[r].bbmax[1] = *bboxptr + g_BgRooms[r].pos.y; bboxptr++;
			g_Rooms[r].bbmax[2] = *bboxptr + g_BgRooms[r].pos.z; bboxptr++;

			// Calculate centre
			g_Rooms[r].centre.x = (g_Rooms[r].bbmin[0] + g_Rooms[r].bbmax[0]) / 2.0f;
			g_Rooms[r].centre.y = (g_Rooms[r].bbmin[1] + g_Rooms[r].bbmax[1]) / 2.0f;
			g_Rooms[r].centre.z = (g_Rooms[r].bbmin[2] + g_Rooms[r].bbmax[2]) / 2.0f;

			// Calculate radius
			g_Rooms[r].radius = sqrtf((g_Rooms[r].bbmin[0] - g_Rooms[r].bbmax[0]) * (g_Rooms[r].bbmin[0] - g_Rooms[r].bbmax[0])
					+ (g_Rooms[r].bbmin[1] - g_Rooms[r].bbmax[1]) * (g_Rooms[r].bbmin[1] - g_Rooms[r].bbmax[1])
					+ (g_Rooms[r].bbmin[2] - g_Rooms[r].bbmax[2]) * (g_Rooms[r].bbmin[2] - g_Rooms[r].bbmax[2])) / 2.0f;
		}

		// The next part of section 3 is a list of roomgfxdata sizes.
		// There is one per room and the value needs to be multiplied by 0x10.
		datalenptr = (uint16_t *) bboxptr;

		for (r = 1; r < g_Vars.roomcount; r++) {
			g_Rooms[r].gfxdatalen = ALIGN16(*datalenptr * 0x10 + 0x100);
			datalenptr++;
		}

		// The last part of section 3 is the number of lights per room.
		// This is calculating the index into the lights file where each room's
		// lights start. The light data is already ordered by room, so it can do
		// this easily by adding to the offset of the previous one.
		lightindex = 0;
		numlightsptr = (uint8_t *) datalenptr;

		for (r = 1; r < g_Vars.roomcount; r++) {
			g_Rooms[r].numlights = *numlightsptr;

			if (g_Rooms[r].numlights > 0) {
				g_Rooms[r].lightindex = lightindex;
				lightindex += g_Rooms[r].numlights;
			} else {
				g_Rooms[r].lightindex = -1;
			}

			numlightsptr++;
		}

		// Free the section 3 allocation
		mempRealloc(section3, 0, MEMPOOL_STAGE);

		for (i = 1; i < g_Vars.roomcount; i++) {
			roomInitLights(i);
		}

		// Initialise a table related to lights.
		// j is being reused here as a total light count.
		j = 0;

		for (i = 1; i < g_Vars.roomcount; i++) {
			j += g_Rooms[i].numlights;
		}

		if (j) {
			var800a41a0 = mempAlloc(ALIGN16(j * 3), MEMPOOL_STAGE);

			for (i = 0; i < j; i++) {
				var800a41a0[i * 3 + 0] = 0;
				var800a41a0[i * 3 + 1] = 0;
				var800a41a0[i * 3 + 2] = 0;
			}
		} else {
			var800a41a0 = NULL;
		}

		for (i = 0; g_BgPortals[i].verticesoffset != 0; i++) {
			bgInitPortal(i);
		}

		for (i = 1; i < g_Vars.roomcount; i++) {
			bgInitRoom(i);
		}

		for (i = 1; i < g_Vars.roomcount; i++) {
			bgExpandRoomToPortals(i);
		}

		for (i = 0; g_BgPortals[i].verticesoffset != 0; i++) {
			g_BgPortals[i].flags &= ~PORTALFLAG_CLOSED & 0xff;
		}
	}

	g_NumRoomLoadsLeftThisFrame = 200;

	wallhitReset();
	roomResetLights();
	roomPreprocessVisibility();
}

void bgStop(void)
{
	bgUnloadAllRooms();
}

/**
 * The counter is incremented once per frame per player.
 *
 * The portal camera cache uses this counter as a last-updated timestamp to keep
 * track of whether any cache item is current as of this frame/player or whether
 * it needs to be recalculated.
 *
 * When the counter rolls over, all portal camera cache is cleared to avoid any
 * potential issues with reusing the timestamps.
 */
void bgTickCounter(void)
{
	g_BgFrameCount++;

	if (g_BgFrameCount == 0xffff) {
		g_BgFrameCount = 1;

		bgClearPortalCameraCache();
	}
}

void bgTick(void)
{
	int tickmode;

	g_BgNumForceOnscreenRooms = 0;

	bgTickCounter();

	if (g_Vars.currentplayerindex == 0) {
		bgTickRooms();
	}

	tickmode = g_Vars.tickmode;

	if (tickmode == TICKMODE_NORMAL) {
		g_NumRoomLoadsLeftThisFrame = 4;

		// On the first few frames of gameplay allow 200 rooms to be loaded instead of just 4
		if (g_RoomStreamingBoostTimer)
		{
			g_RoomStreamingBoostTimer--;
			g_NumRoomLoadsLeftThisFrame = 200;
		}
	} else {
		g_RoomStreamingBoostTimer = 8;
		g_NumRoomLoadsLeftThisFrame = 200;
	}

	if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		g_NumRoomLoadsLeftThisFrame = 100;
	}

	g_CamRoom = g_Vars.currentplayer->cam_room;

	bgTickPortals();
}

Gfx *bgRender(Gfx *gdl)
{
	gdl = lightsSetDefault(gdl);

	gfx_Segment(gdl++, SPSEGMENT_BG_DL, (uintptr_t)g_BgPrimaryData);

	gdl = envStartFog(gdl);
	gdl = bgRenderSceneAndLoadCandidate(gdl);
	gdl = bgScissorToViewport(gdl);
	gdl = envStopFog(gdl);

	gfx_Matrix(gdl++, g_CameraPerspectiveMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

	return gdl;
}

Gfx *bgScissorToViewport(Gfx *gdl)
{
	return bgScissorWithinViewport(gdl,
			g_Vars.currentplayer->viewleft,
			g_Vars.currentplayer->viewtop,
			g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth,
			g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight);
}

Gfx *bgScissorWithinViewportF(Gfx *gdl, float viewleft, float viewtop, float viewright, float viewbottom)
{
	gdl = bgScissorWithinViewport(gdl, viewleft, viewtop, (int)ceilf(viewright), (int)ceilf(viewbottom));

	return gdl;
}

Gfx *bgScissorWithinViewport(Gfx *gdl, int viewleft, int viewtop, int viewright, int viewbottom)
{
	if(g_Vars.stagenum != STAGE_AIRBASE)
	{
		const int xmargin = videoGetWidth() / SCREEN_320 - 1;
		const int ymargin = videoGetHeight() / SCREEN_240 - 1;
		if (xmargin > 0) {
			viewleft -= xmargin;
			viewright += xmargin;
		}
		if (ymargin > 0) {
			viewtop -= ymargin;
			viewbottom += ymargin;
		}

		if (viewleft < g_Vars.currentplayer->viewleft) {
			viewleft = g_Vars.currentplayer->viewleft;
		}

		if (viewtop < g_Vars.currentplayer->viewtop) {
			viewtop = g_Vars.currentplayer->viewtop;
		}

		if (viewright > g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth) {
			viewright = g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth;
		}

		if (viewbottom > g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight) {
			viewbottom = g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight;
		}

		gfx_Set_Scissor(gdl++, viewleft, viewtop, viewright, viewbottom);
	}

	return gdl;
}

void bgClearPortalCameraCache(void)
{
	int i;

	for (i = 0; i < g_BgNumPortalCameraCacheItems; i++) {
		g_PortalCameraCache[i].bboxisvalid = -1;
		g_PortalCameraCache[i].updatedframe2 = 0;
		g_PortalCameraCache[i].updatedframe1 = 0;
	}
}

bool bgRoomIntersectsScreenBox(int room, struct screenbox *screen)
{
	int i;
	struct coord roomscreenpos;
	struct coord corner;
	int numbehind = 0;
	int numfar = 0;
	int numleft = 0;
	int numright = 0;
	int numbelow = 0;
	int numabove = 0;

	for (i = 0; i != 8; i++) {
		if (i & 1) {
			corner.x = g_Rooms[room].bbmin[0];
		} else {
			corner.x = g_Rooms[room].bbmax[0];
		}

		if (i & 2) {
			corner.y = g_Rooms[room].bbmin[1];
		} else {
			corner.y = g_Rooms[room].bbmax[1];
		}

		if (i & 4) {
			corner.z = g_Rooms[room].bbmin[2];
		} else {
			corner.z = g_Rooms[room].bbmax[2];
		}

		if (bg3dPosTo2dPos(&corner, &roomscreenpos) == 0) {
			// Corner is behind the camera
			if (g_BgSnake.zrange.far <= -roomscreenpos.z) {
				numfar++;
			}

			if (roomscreenpos.x > screen->xmin) {
				numleft++;
			}

			if (roomscreenpos.x < screen->xmax) {
				numright++;
			}

			if (roomscreenpos.y > screen->ymin) {
				numbelow++;
			}

			if (roomscreenpos.y < screen->ymax) {
				numabove++;
			}

			numbehind++;
		} else {
			// Corner is in front of the camera
			if (g_BgSnake.zrange.far <= -roomscreenpos.z) {
				numfar++;
			}

			if (roomscreenpos.x < screen->xmin) {
				numleft++;
			} else if (roomscreenpos.x > screen->xmax) {
				numright++;
			}

			if (roomscreenpos.y < screen->ymin) {
				numbelow++;
			} else if (roomscreenpos.y > screen->ymax) {
				numabove++;
			}
		}
	}

	if (numbehind == 8
			|| numfar == 8
			|| numleft == 8
			|| numright == 8
			|| numbelow == 8
			|| numabove == 8) {
		return false;
	}

	return true;
}

bool bg3dPosTo2dPos(struct coord *cornerpos, struct coord *screenpos)
{
	Mtx *matrix = camGetPlayerWorldToScreenMtx();

	screenpos->x = cornerpos->x;
	screenpos->y = cornerpos->y;
	screenpos->z = cornerpos->z;

	mtx4TransformVecInPlace(matrix, screenpos);
	camProjectViewToScreenSafe(screenpos, screenpos->f);

	if (screenpos->z > 0) {
		return false;
	}

	return true;
}

bool bgGetPortalScreenBbox(int portalnum, struct screenbox *box)
{
	int len;
	int start;
	int numvalid;
	float sp2e4[2];
	float sp2d4[2][2];
	struct portalthing2 *thing;
	struct portalthing2 things[40];

	if (g_PortalCameraCache[portalnum].updatedframe2 == g_BgFrameCount) {
		box->xmin = g_PortalCameraCache[portalnum].xmin;
		box->ymin = g_PortalCameraCache[portalnum].ymin;
		box->xmax = g_PortalCameraCache[portalnum].xmax;
		box->ymax = g_PortalCameraCache[portalnum].ymax;

		return g_PortalCameraCache[portalnum].bboxisvalid;
	}

	len = portalConvertCoordinates(portalnum, &start, things);

	numvalid = 0;
	thing = &things[start];

	for (int j = 0; j < len; j++) {
		if (thing->coord.z <= 0.0f) {
			camProjectViewToScreenSafe(&thing->coord, sp2e4);

			if (numvalid == 0) {
				sp2d4[0][0] = sp2d4[1][0] = sp2e4[0];
				sp2d4[0][1] = sp2d4[1][1] = sp2e4[1];
			} else {
				if (sp2e4[0] < sp2d4[0][0]) {
					sp2d4[0][0] = sp2e4[0];
				}

				if (sp2d4[1][0] < sp2e4[0]) {
					sp2d4[1][0] = sp2e4[0];
				}

				if (sp2e4[1] < sp2d4[0][1]) {
					sp2d4[0][1] = sp2e4[1];
				}

				if (sp2d4[1][1] < sp2e4[1]) {
					sp2d4[1][1] = sp2e4[1];
				}
			}

			numvalid++;
		}

		thing++;
	}

	if (numvalid == 0) {
		box->xmin = 0;
		box->xmax = 0;
		box->ymin = 0;
		box->ymax = 0;
	} else if (sp2d4[1][0] < sp2d4[0][0] || sp2d4[1][1] < sp2d4[0][1]) {
		struct player *player = g_Vars.currentplayer;
		box->xmin = player->screenxminf;
		box->ymin = player->screenyminf;
		box->xmax = player->screenxmaxf;
		box->ymax = player->screenymaxf;
	} else {
		sp2d4[0][0] -= 0.5f;
		sp2d4[0][1] -= 0.5f;
		sp2d4[1][0] += 0.5f;
		sp2d4[1][1] += 0.5f;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				float value = sp2d4[i][j];

				if (value >= 0.0f) {
					if (value > 32000.0f) {
						box->array[i][j] = 32000;
					} else {
						box->array[i][j] = value;
					}
				} else if (value < -32000.0f) {
					box->array[i][j] = -32000;
				} else {
					box->array[i][j] = value;
				}
			}
		}
	}

	g_PortalCameraCache[portalnum].xmin = box->xmin;
	g_PortalCameraCache[portalnum].ymin = box->ymin;
	g_PortalCameraCache[portalnum].xmax = box->xmax;
	g_PortalCameraCache[portalnum].ymax = box->ymax;
	g_PortalCameraCache[portalnum].bboxisvalid = numvalid;
	g_PortalCameraCache[portalnum].updatedframe2 = g_BgFrameCount;

	return numvalid;
}

bool bgGetBoxIntersection(struct screenbox *a, struct screenbox *b)
{
	a->xmin = a->xmin > b->xmin ? a->xmin : b->xmin;
	a->ymin = a->ymin > b->ymin ? a->ymin : b->ymin;
	a->xmax = b->xmax > a->xmax ? a->xmax : b->xmax;
	a->ymax = b->ymax > a->ymax ? a->ymax : b->ymax;

	if (a->xmin >= a->xmax) {
		a->xmin = a->xmax;
		return false;
	}

	if (a->ymax <= a->ymin) {
		a->ymin = a->ymax;
		return false;
	}

	return true;
}

void bgExpandBox(struct screenbox *a, struct screenbox *b)
{
	a->xmin = a->xmin < b->xmin ? a->xmin : b->xmin;
	a->ymin = a->ymin < b->ymin ? a->ymin : b->ymin;
	a->xmax = a->xmax > b->xmax ? a->xmax : b->xmax;
	a->ymax = a->ymax > b->ymax ? a->ymax : b->ymax;
}

void bgCopyBox(struct screenbox *dst, struct screenbox *src)
{
	dst->xmin = src->xmin;
	dst->ymin = src->ymin;
	dst->xmax = src->xmax;
	dst->ymax = src->ymax;
}

bool bgRoomIsOnscreen(int room)
{
	if (g_Vars.mplayerisrunning) {
		return (g_MpRoomVisibility[room] & 0xf) != 0;
	} else {
		return g_Rooms[room].flags & ROOMFLAG_ONSCREEN;
	}
}

bool bgRoomIsStandby(int room)
{
	if (g_Vars.mplayerisrunning) {
		return (g_MpRoomVisibility[room] & 0xf0) != 0;
	}

	return g_Rooms[room].flags & ROOMFLAG_STANDBY;
}

bool bgRoomIsOnPlayerScreen(int room, uint32_t playernum)
{
	if (g_Vars.mplayerisrunning) {
		return (g_MpRoomVisibility[room] & (1 << playernum)) != 0;
	} else {
		return g_Rooms[room].flags & ROOMFLAG_ONSCREEN;
	}
}

bool bgRoomIsOnPlayerStandby(int room, uint32_t playernum)
{
	if (g_Vars.mplayerisrunning) {
		return (g_MpRoomVisibility[room] & (0x10 << playernum)) != 0;
	} else {
		return g_Rooms[room].flags & ROOMFLAG_STANDBY;
	}
}

int bgFindPortalByVertices(struct portalvertices *target)
{
	int i;
	struct bgportal *portal = g_BgPortals;

	for (i = 0; portal[i].verticesoffset != 0; i++) {
		struct portalvertices *pvertices = (struct portalvertices *)((uintptr_t)portal + portal[i].verticesoffset);

		if (pvertices == target) {
			return i;
		}
	}

	return 0;
}

uint32_t bgInflate(uint8_t *src, uint8_t *dst, uint32_t len)
{
	uint32_t result;
	uint8_t scratch[5120];

	if (rzipIs1173(src)) {
		result = rzipInflate(src, dst, &scratch);
	} else {
		result = len;
		memcpy(dst, src, len);
	}

	return result;
}

Gfx *bgGetNextGdlInBlock(struct roomblock *block, Gfx *start, Gfx *end)
{
	Gfx *tmp;
	while (true) {
		if (block == NULL) {
			return end;
		}

		switch (block->type) {
		case ROOMBLOCKTYPE_LEAF:
			if (block->gdl > start && (block->gdl < end || end == NULL)) {
				end = block->gdl;
			}
			block = block->next;
			break;
		case ROOMBLOCKTYPE_PARENT:
			tmp = bgGetNextGdlInBlock(block->child, start, end);
			block = block->next;
			end = tmp;
			break;
		default:
			return end;
		}
	}

	return end;
}

Gfx *bgGetNextGdlInLayer(int roomnum, Gfx *start, uint32_t types)
{
	struct roomblock *opablocks = g_Rooms[roomnum].gfxdata->opablocks;
	struct roomblock *xlublocks = g_Rooms[roomnum].gfxdata->xlublocks;
	Gfx *opagdl = NULL;
	Gfx *xlugdl = NULL;

	if ((types & VTXBATCHTYPE_OPA) && opablocks) {
		opagdl = bgGetNextGdlInBlock(opablocks, start, NULL);

		if (types == VTXBATCHTYPE_OPA) {
			return opagdl;
		}
	}

	if ((types & VTXBATCHTYPE_XLU) && xlublocks) {
		xlugdl = bgGetNextGdlInBlock(xlublocks, start, NULL);

		if (types == VTXBATCHTYPE_XLU) {
			return xlugdl;
		}
	}

	if (opagdl) {
		if (xlugdl && xlugdl < opagdl) {
			return xlugdl;
		}

		return opagdl;
	}

	return xlugdl;
}

Vtx *bgFindVerticesForGdl(int roomnum, Gfx *gdl)
{
	struct roomblock *block = g_Rooms[roomnum].gfxdata->blocks;
	uintptr_t end = (uintptr_t)g_Rooms[roomnum].gfxdata->vertices;

	while ((uintptr_t)(block + 1) <= end) {
		switch (block->type) {
		case ROOMBLOCKTYPE_LEAF:
			if (gdl == block->gdl) {
				return block->vertices;
			}
			break;
		case ROOMBLOCKTYPE_PARENT:
			if ((uintptr_t)block->unk0c < end) {
				end = (uintptr_t)block->unk0c;
			}
			break;
		}

		block++;
	}

	return NULL;
}

/**
 * The rough steps to load a room are:
 * - Allocate some memory out of mema.
 * - DMA the zipped graphics data from ROM to the right side of the allocation.
 * - Unzip the data to the left side.
 * - Replace offsets within the graphics data with pointers.
 * - Copy the displaylists to the right side of the allocation.
 * - Scan the right-side displaylists, loading textures and rewriting the
 *   displaylists, overwriting the ones on the left side. These rewritten
 *   displaylists may be longer than the originals.
 * - Shrink the mema allocation to just the left side.
 * - Fix pointers due to the resized displaylists.
 * - Do some find/replaces in the displaylist based on environment settings.
 * - Find each batch of vertices and build a bbox for each batch.
 *   These are used for hit detection.
 */
void bgLoadRoom(int roomnum)
{
	int alloclen;
	int inflatedlen;
	uint8_t *allocation;
	int readlen;
	int fileoffset;
	uint8_t *itergdl1;
	uint8_t *itergdl2;
	struct roomblock *block1;
	struct roomblock *block2;
	uint8_t *memaddr;
	uint8_t *gfxblocks[50];
	uint8_t *vtxblocks[50];
	uint8_t *gdlpointers[50];
	int numgdls;
	uintptr_t end1;
	int i;
	int len;
	uintptr_t end2;
	int prev;

	if (roomnum == 0 || roomnum >= g_Vars.roomcount) {
		return;
	}

	if (g_Rooms[roomnum].loaded240) {
		return;
	}

	// Determine how much memory to allocate.
	// It must be big enough to fit the biggest of:
	// 1. The inflated room data and compressed room data
	// 2. The inflated room data and the displaylists a second time.
	if (g_Rooms[roomnum].gfxdatalen > 0) {
		alloclen = g_Rooms[roomnum].gfxdatalen;
	} else {
		// probably never reaches here in practice as all rooms have gfxdatalen
		// alloc 10k and hope for the best
		alloclen = 10240;
	}

#ifdef PLATFORM_64BIT
	alloclen = alloclen * 8; // just to be safe for now, adjust properly later #TODO
#endif

	// allocate room data from heap to not take up mema space
	allocation = malloc(alloclen);

	if (allocation != NULL) {
		dyntexSetCurrentRoom(roomnum);

		// Calculate the file offset and read length
		// of the compressed room data in the BG file
		readlen = ((g_BgRooms[roomnum + 1].ptr_gfxdata - g_BgRooms[roomnum].ptr_gfxdata) + 0xf) & ~0xf;
		fileoffset = (g_BgPrimaryData + g_BgRooms[roomnum].ptr_gfxdata - g_BgPrimaryData) - 0x0f000000;
		fileoffset -= g_BgPointerOffset;

		if (readlen > alloclen) {
			dyntexSetCurrentRoom(-1);
			return;
		}

		// Load the compressed data to the right side of the allocation
		memaddr = allocation + (alloclen - readlen);

		bgLoadFile(memaddr, fileoffset, readlen);

		if (rzipIs1173(memaddr) && readlen + 0x20 > alloclen) {
			dyntexSetCurrentRoom(-1);
			return;
		}

		// Inflate the data to the left side of the allocation
		inflatedlen = bgInflate(memaddr, allocation, g_BgRooms[roomnum + 1].ptr_gfxdata - g_BgRooms[roomnum].ptr_gfxdata);
		inflatedlen = preprocessBgRoom(allocation, inflatedlen, g_BgRooms[roomnum].ptr_gfxdata);

		g_Rooms[roomnum].gfxdata = (struct roomgfxdata *)allocation;

		// Promote offsets to pointers in the gfxdata header
		if (g_Rooms[roomnum].gfxdata->vertices) {
			g_Rooms[roomnum].gfxdata->vertices = (Vtx *) (allocation + ((uintptr_t) g_Rooms[roomnum].gfxdata->vertices - g_BgRooms[roomnum].ptr_gfxdata));
		}

		if (g_Rooms[roomnum].gfxdata->colours) {
			g_Rooms[roomnum].gfxdata->colours = (Col *) (allocation + ((uintptr_t) g_Rooms[roomnum].gfxdata->colours - g_BgRooms[roomnum].ptr_gfxdata));
		}

		if (g_Rooms[roomnum].gfxdata->opablocks) {
			g_Rooms[roomnum].gfxdata->opablocks = (struct roomblock *) (allocation + ((uintptr_t) g_Rooms[roomnum].gfxdata->opablocks - g_BgRooms[roomnum].ptr_gfxdata));
		}

		if (g_Rooms[roomnum].gfxdata->xlublocks) {
			g_Rooms[roomnum].gfxdata->xlublocks = (struct roomblock *) (allocation + ((uintptr_t) g_Rooms[roomnum].gfxdata->xlublocks - g_BgRooms[roomnum].ptr_gfxdata));
		}

		// Promote offsets to pointers in each gfxdata block
		end1 = (uintptr_t)g_Rooms[roomnum].gfxdata->vertices;

		for (block1 = g_Rooms[roomnum].gfxdata->blocks; (intptr_t) (block1 + 1) <= end1; block1++) {
			switch (block1->type) {
			case ROOMBLOCKTYPE_LEAF:
				if (block1->next != NULL) {
					block1->next = (struct roomblock *) (allocation + ((uintptr_t) block1->next - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->gdl != 0) {
					block1->gdl = (Gfx *) (allocation + ((uintptr_t) block1->gdl - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->vertices != 0) {
					block1->vertices = (Vtx *) (allocation + ((uintptr_t) block1->vertices - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->colours != 0) {
					block1->colours = (Col *) (allocation + ((uintptr_t) block1->colours - g_BgRooms[roomnum].ptr_gfxdata));
				}
				break;
			case ROOMBLOCKTYPE_PARENT:
				if (block1->next != NULL) {
					block1->next = (struct roomblock *) (allocation + ((uintptr_t) block1->next - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->gdl != 0) {
					block1->gdl = (Gfx *) (allocation + ((uintptr_t) block1->gdl - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->vertices != 0) {
					block1->vertices = (Vtx *) (allocation + ((uintptr_t) block1->vertices - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if (block1->colours != 0) {
					block1->colours = (Col *) (allocation + ((uintptr_t) block1->colours - g_BgRooms[roomnum].ptr_gfxdata));
				}
				if ((uintptr_t) block1->vertices < end1) {
					end1 = (uintptr_t) block1->vertices;
				}
				break;
			}
		}

		// Calculate the number of vertices and colours
		g_Rooms[roomnum].gfxdata->numvertices = ((uintptr_t) g_Rooms[roomnum].gfxdata->colours - (uintptr_t) g_Rooms[roomnum].gfxdata->vertices) / sizeof(Vtx);
		g_Rooms[roomnum].gfxdata->numcolours = ((uintptr_t) bgGetNextGdlInLayer(roomnum, 0, VTXBATCHTYPE_OPA | VTXBATCHTYPE_XLU) - (uintptr_t) g_Rooms[roomnum].gfxdata->colours) / sizeof(Col);

		// Build arrays of pointers to gfx blocks and vtx blocks
		numgdls = 0;
		itergdl1 = (uint8_t *) bgGetNextGdlInLayer(roomnum, NULL, VTXBATCHTYPE_OPA | VTXBATCHTYPE_XLU);

		while (itergdl1) {
			gfxblocks[numgdls] = (uint8_t *) itergdl1;
			vtxblocks[numgdls] = (uint8_t *) bgFindVerticesForGdl(roomnum, (Gfx *) itergdl1);
			numgdls++;

			itergdl1 = (uint8_t *) bgGetNextGdlInLayer(roomnum, (Gfx *) itergdl1, VTXBATCHTYPE_OPA | VTXBATCHTYPE_XLU);
		}

		gfxblocks[numgdls] = allocation + inflatedlen;

		// Copy gdls to the right-side of the allocation
		// and build a pointer array to them
		texCopyGdls((void *) gfxblocks[0], (void *) (allocation + alloclen - (gfxblocks[numgdls] - gfxblocks[0])), (uintptr_t) (gfxblocks[numgdls] - gfxblocks[0]));

		for (i = 0; i < numgdls + 1; i++) {
			gdlpointers[i] = gfxblocks[i] + (allocation + alloclen - gfxblocks[numgdls]);
		}

		// Load textures by scanning the right-side gdls.
		// texLoadFromGdl is reading from gdlpointers and writing new GBI commands
		// to itergdl2, overwriting the GBI commands that were loaded from the
		// BG file. As these are being processed the gdlpointers pointers are
		// changed to point to the written GBI.
		itergdl2 = gfxblocks[0];

		for (i = 0; i < numgdls; i++) {
			int byteswritten;
			len = gfxblocks[i + 1] - gfxblocks[i];
			byteswritten = texLoadFromGdl((void *) gdlpointers[i], len, (void *) itergdl2, NULL, vtxblocks[i]);
			gdlpointers[i] = itergdl2;

			if (len);

			itergdl2 = (uint8_t *) ALIGN8((uintptr_t) (itergdl2 + byteswritten));
		}

		gdlpointers[numgdls] = itergdl2;

		// Free the right side of the allocation
		prev = g_Rooms[roomnum].gfxdatalen;
		g_Rooms[roomnum].gfxdatalen = ALIGN16(gdlpointers[numgdls] - allocation + 0x20);

		g_Rooms[roomnum].loaded240 = 1;

		// Update gdl pointers in the gfxdata so they point to the ones
		// that have been processed by textLoadFromGdl.
		block2 = g_Rooms[roomnum].gfxdata->blocks;
		end2 = (uintptr_t) g_Rooms[roomnum].gfxdata->vertices;

		while ((intptr_t) (block2 + 1) <= end2) {
			switch (block2->type) {
			case ROOMBLOCKTYPE_LEAF:
				if (block2->gdl) {
					for (i = 0; i < numgdls; i++) {
						memaddr = (uint8_t *) block2->gdl; // reusing var

						if (memaddr == gfxblocks[i]) {
							block2->gdl = (Gfx *) gdlpointers[i];
							break;
						}
					}
				}
				break;
			case ROOMBLOCKTYPE_PARENT:
				if ((uintptr_t) block2->unk0c < end2) {
					end2 = (uintptr_t) block2->unk0c;
				}
				break;
			}

			block2++;
		}

		// Do some find/replaces in the gdls based on environment configuration
		if (g_FogEnabled) {
			gfxReplaceGbiCommandsRecursively(g_Rooms[roomnum].gfxdata->opablocks, 0);
			gfxReplaceGbiCommandsRecursively(g_Rooms[roomnum].gfxdata->xlublocks, 1);
		}

		// Create vertex batches - these are used for hit detection
		bgFindRoomVtxBatches(roomnum);

		g_Rooms[roomnum].flags |= ROOMFLAG_LIGHTS_DIRTY;
		g_Rooms[roomnum].flags |= ROOMFLAG_BRIGHTNESS_DIRTY_PERM;

		g_Rooms[roomnum].colours = NULL;

		dyntexSetCurrentRoom(-1);
	}
}

void bgUnloadRoom(int roomnum)
{
	uint32_t size;

	if (g_Rooms[roomnum].vtxbatches) {
		sysMemFree(g_Rooms[roomnum].vtxbatches);
		g_Rooms[roomnum].vtxbatches = NULL;
	}

	if (g_Rooms[roomnum].gfxdatalen > 0) {
		sysMemFree(g_Rooms[roomnum].gfxdata);
		g_Rooms[roomnum].gfxdata = NULL;
	}

	g_Rooms[roomnum].loaded240 = 0;
}

void bgUnloadAllRooms(void)
{
	int i;

	for (i = 1; i < g_Vars.roomcount; i++) {
		if (g_Rooms[i].loaded240) {
			bgUnloadRoom(i);
		}
	}
}

/**
 * Increase the loaded240 timers for rooms which are no longer visible.
 * If any rooms have reached the timer limit then unload them, but don't unload
 * more than 2 rooms per frame.
 */
void bgTickRooms(void)
{
	int numunloaded = 0;
	int i;

	for (i = 1; i < g_Vars.roomcount; i++) {
		if (g_Rooms[i].loaded240) {
			g_Rooms[i].loaded240++;

			if (g_Rooms[i].loaded240 >= g_BgUnloadDelay240) {
				g_Rooms[i].loaded240 = g_BgUnloadDelay240;
			}

			if (g_Rooms[i].flags & ROOMFLAG_ONSCREEN) {
				g_Rooms[i].loaded240 = 1;
			}

			if (numunloaded < 2 && g_Rooms[i].loaded240 == g_BgUnloadDelay240_2) {
				bgUnloadRoom(i);
				numunloaded++;
			}
		}
	}
}

Gfx *bgRenderRoomPass(Gfx *gdl, int roomnum, struct roomblock *block, bool includetransp)
{
	uintptr_t v0;

	if (block == NULL) {
		return gdl;
	}

	switch (block->type) {
	case ROOMBLOCKTYPE_LEAF:
		if (g_Rooms[roomnum].flags & ROOMFLAG_HASDYNTEX) {
			dyntexTickRoom(roomnum, block->vertices);
		}

		gfx_Segment(gdl++, SPSEGMENT_BG_VTX, (uintptr_t)(block->vertices));

		lightHighlight(roomnum);

		v0 = (uintptr_t)g_Rooms[roomnum].colours;

		if (v0 != 0) {
			uintptr_t addr = ALIGN8((uintptr_t)&g_Rooms[roomnum].gfxdata->vertices[g_Rooms[roomnum].gfxdata->numvertices]);
			v0 += (((uintptr_t)block->colours - addr) >> 2) * 4;
		} else {
			v0 = (uintptr_t)block->colours;
		}

		gfx_Segment(gdl++, SPSEGMENT_BG_COL, (uintptr_t)(v0));

		gSPDisplayList(gdl++, (uintptr_t)(block->gdl));

		if (includetransp) {
			gdl = bgRenderRoomPass(gdl, roomnum, block->next, true); // Render double sided translucent textures
		}
		break;
	case ROOMBLOCKTYPE_PARENT:
		if (block->child != NULL) {
			struct roomblock *sp58;
			struct roomblock *sp54;
			struct coord *coord;
			float sum;
			float sp40[3];
			float sp34[3];

			sp58 = block->child;
			sp54 = sp58->next;
			coord = block->unk0c;

			sp40[0] = coord[1].f[0];
			sp40[1] = coord[1].f[1];
			sp40[2] = coord[1].f[2];
			sp34[0] = coord[0].f[0] - g_Vars.currentplayer->cam_pos.f[0];
			sp34[1] = coord[0].f[1] - g_Vars.currentplayer->cam_pos.f[1];
			sp34[2] = coord[0].f[2] - g_Vars.currentplayer->cam_pos.f[2];

			sum = sp40[0] * sp34[0] + sp40[1] * sp34[1] + sp40[2] * sp34[2];

			if (sum < 0.0f) {
				gdl = bgRenderRoomPass(gdl, roomnum, sp58, false);
				gdl = bgRenderRoomPass(gdl, roomnum, sp54, false);
			} else {
				gdl = bgRenderRoomPass(gdl, roomnum, sp54, false);
				gdl = bgRenderRoomPass(gdl, roomnum, sp58, false);
			}

			if (includetransp) {
				gdl = bgRenderRoomPass(gdl, roomnum, block->next, true);
			}
		}
		break;
	}

	return gdl;
}

/**
 * Render the opaque layer of the room.
 */
Gfx *bgRenderRoomOpaque(Gfx *gdl, int roomnum)
{
	if (g_Rooms[roomnum].loaded240 == 0) {
		return gdl;
	}

	gdl = roomApplyMtx(gdl, roomnum);

	gdl = lightsSetForRoom(gdl, roomnum);
	gdl = bgRenderRoomPass(gdl, roomnum, g_Rooms[roomnum].gfxdata->opablocks, true);
	gdl = lightsSetDefault(gdl);

	g_Rooms[roomnum].loaded240 = 1;

	return gdl;
}

/**
 * Render the transparency layer of the room.
 */
Gfx *bgRenderRoomXlu(Gfx *gdl, int roomnum)
{
	if (roomnum == 0 || roomnum >= g_Vars.roomcount) {
		return gdl;
	}

	if (g_Rooms[roomnum].loaded240) {
		if (g_Rooms[roomnum].gfxdata->xlublocks == NULL) {
			return gdl;
		}

		lightHighlight(roomnum);

		if (g_Rooms[roomnum].gfxdata);
		if (g_Rooms[roomnum].gfxdata);

		gdl = roomApplyMtx(gdl, roomnum);
		gdl = bgRenderRoomPass(gdl, roomnum, g_Rooms[roomnum].gfxdata->xlublocks, true);

		g_Rooms[roomnum].loaded240 = 1;
	} else {
		bgLoadRoom(roomnum);
	}

	return gdl;
}

int bgPopulateVtxBatchType(int roomnum, struct vtxbatch *batches, Gfx *gdl, int batchindex, Vtx *vertices, int type)
{
	int i;
	int j;
	int numvertices;
	Vtx *batchvertices;

	for (i = 0; gdl[i].dma.cmd != G_ENDDL; i++) {
		if (gdl[i].dma.cmd == G_VTX) {
			batches[batchindex].gdl = gdl;
			batches[batchindex].gbicmdindex = i;
			batches[batchindex].type = type;

			for (j = 0; j < 3; j++) {
				batches[batchindex].bbmin.f[j] = 32767.0f;
				batches[batchindex].bbmax.f[j] = -32768.0f;
			}

			numvertices = (((uint32_t)gdl[i].bytes[GFX_W0_BYTE(1)] >> 4) & 0xf) + 1;
			batchvertices = (Vtx *)((uintptr_t)vertices + (UNSEGADDR(gdl[i].words.w1) & 0xffffff));

			for (j = 0; j < numvertices; j++) {
				float x = batchvertices[j].x;
				float y = batchvertices[j].y;
				float z = batchvertices[j].z;

				if (x < batches[batchindex].bbmin.x) {
					batches[batchindex].bbmin.x = x;
				}

				if (y < batches[batchindex].bbmin.y) {
					batches[batchindex].bbmin.y = y;
				}

				if (z < batches[batchindex].bbmin.z) {
					batches[batchindex].bbmin.z = z;
				}

				if (x > batches[batchindex].bbmax.x) {
					batches[batchindex].bbmax.x = x;
				}

				if (y > batches[batchindex].bbmax.y) {
					batches[batchindex].bbmax.y = y;
				}

				if (z > batches[batchindex].bbmax.z) {
					batches[batchindex].bbmax.z = z;
				}
			}

			if (batches[batchindex].bbmin.x == batches[batchindex].bbmax.x) {
				batches[batchindex].bbmax.x++;
			}

			if (batches[batchindex].bbmin.y == batches[batchindex].bbmax.y) {
				batches[batchindex].bbmax.y++;
			}

			if (batches[batchindex].bbmin.z == batches[batchindex].bbmax.z) {
				batches[batchindex].bbmax.z++;
			}

			batches[batchindex].bbmin.x += g_BgRooms[roomnum].pos.x;
			batches[batchindex].bbmin.y += g_BgRooms[roomnum].pos.y;
			batches[batchindex].bbmin.z += g_BgRooms[roomnum].pos.z;

			batches[batchindex].bbmax.x += g_BgRooms[roomnum].pos.x;
			batches[batchindex].bbmax.y += g_BgRooms[roomnum].pos.y;
			batches[batchindex].bbmax.z += g_BgRooms[roomnum].pos.z;

			batchindex++;
		}
	}

	return batchindex;
}

void bgFindRoomVtxBatches(int roomnum)
{
	int i;
	int batchindex = 0;
	int xlucount;
	Gfx *gdl;
	struct vtxbatch *batches;

	if (g_Rooms[roomnum].vtxbatches == NULL) {
		gdl = bgGetNextGdlInLayer(roomnum, NULL, VTXBATCHTYPE_OPA);

		if (gdl != NULL) {
			while (gdl) {
				for (i = 0; gdl[i].dma.cmd != G_ENDDL; i++) {
					// if gSPVertex
					if (gdl[i].dma.cmd == G_VTX) {
						batchindex++;
					}
				}

				gdl = bgGetNextGdlInLayer(roomnum, gdl, VTXBATCHTYPE_OPA);
			}

			xlucount = 0;

			gdl = bgGetNextGdlInLayer(roomnum, NULL, VTXBATCHTYPE_XLU);

			while (gdl) {
				for (i = 0; gdl[i].dma.cmd != G_ENDDL; i++) {
					// if gSPVertex
					if (gdl[i].dma.cmd == G_VTX) {
						xlucount++;
					}
				}

				gdl = bgGetNextGdlInLayer(roomnum, gdl, VTXBATCHTYPE_XLU);
			}

			batchindex += xlucount;
			batches = sysMemAlloc((batchindex * sizeof(struct vtxbatch) + 0xf) & ~0xf);

			if (batches != NULL) {
				gdl = bgGetNextGdlInLayer(roomnum, NULL, VTXBATCHTYPE_OPA);
				batchindex = 0;

				g_Rooms[roomnum].vtxbatches = batches;

				while (gdl) {
					Vtx *vertices = bgFindVerticesForGdl(roomnum, gdl);
					batchindex = bgPopulateVtxBatchType(roomnum, batches, gdl, batchindex, vertices, VTXBATCHTYPE_OPA);
					gdl = bgGetNextGdlInLayer(roomnum, gdl, VTXBATCHTYPE_OPA);
				}

				if (xlucount) {
					gdl = bgGetNextGdlInLayer(roomnum, NULL, VTXBATCHTYPE_XLU);

					while (gdl) {
						Vtx *vertices = bgFindVerticesForGdl(roomnum, gdl);
						batchindex = bgPopulateVtxBatchType(roomnum, batches, gdl, batchindex, vertices, VTXBATCHTYPE_XLU);
						gdl = bgGetNextGdlInLayer(roomnum, gdl, VTXBATCHTYPE_XLU);
					}
				}

				g_Rooms[roomnum].numvtxbatches = (int16_t)batchindex;
			}
		}
	}
}

bool bgTestLineIntersectsIntBbox(struct coord *arg0, struct coord *arg1, int *arg2, int *arg3)
{
	struct coord arg2f;
	struct coord arg3f;

	arg2f.x = arg2[0];
	arg2f.y = arg2[1];
	arg2f.z = arg2[2];

	arg3f.x = arg3[0];
	arg3f.y = arg3[1];
	arg3f.z = arg3[2];

	return bgTestLineIntersectsBbox(arg0, arg1, &arg2f, &arg3f);
}

bool bgTestLineIntersectsBbox(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct coord *arg3)
{
	float f0;
	float f0_2;
	float f2;
	float f2_2;
	float f6;
	float f10;
	float sp34;
	float sp30;
	float f16;
	float f18;
	float f18_2;
	float sp20;
	float f12;
	float f12_2;
	float f14;
	float f14_2;

	// x
	f18 = arg1->x;
	f16 = arg3->x - arg0->x;
	f14 = arg2->x - arg0->x;

	if (f18 < 0.0f) {
		f18 = -f18;
		f14 = -f14;
		f16 = -f16;
	}

	if (f14 < 0.0f && f16 < 0.0f) {
		return false;
	}

	if (f16 < f14) {
		float tmp = f14;
		f14 = f16;
		f16 = tmp;
	}

	// y
	f12 = arg1->y;
	f2 = arg3->y - arg0->y;
	f0 = arg2->y - arg0->y;

	if (f12 < 0.0f) {
		f12 = -f12;
		f0 = -f0;
		f2 = -f2;
	}

	if (f0 < 0.0f && f2 < 0.0f) {
		return false;
	}

	if (f2 < f0) {
		sp20 = f0;
		f0 = f2;
		f2 = sp20;
	}

	f6 = f14 * f12;
	f10 = f0 * f18;

	if (f10 < f6) {
		if (f2 * f18 < f6) {
			return false;
		}

		sp34 = f14;
		sp30 = f18;
	} else {
		if (f16 * f12 < f10) {
			return false;
		}

		sp34 = f0;
		sp30 = f12;
	}

	if (f16 * f12 < f2 * f18) {
		f0_2 = f16;
		f14_2 = f18;
	} else {
		f0_2 = f2;
		f14_2 = f12;
	}

	// z
	f2_2 = arg1->z;
	f12_2 = arg3->z - arg0->z;
	f18_2 = arg2->z - arg0->z;

	if (f2_2 < 0.0f) {
		f2_2 = -f2_2;
		f18_2 = -f18_2;
		f12_2 = -f12_2;
	}

	if (f18_2 < 0.0f && f12_2 < 0.0f) {
		return false;
	}

	if (f12_2 < f18_2) {
		float tmp = f18_2;
		f18_2 = f12_2;
		f12_2 = tmp;
	}

	if (sp34 * f2_2 < f18_2 * sp30) {
		if (f0_2 * f2_2 < f18_2 * f14_2) {
			return false;
		}
	} else {
		if (f12_2 * sp30 < sp34 * f2_2) {
			return false;
		}
	}

	return true;
}

bool bgTestHitOnObj(struct coord *arg0, struct coord *arg1, struct coord *arg2, Gfx *gdl,
		Gfx *gdl2, Vtx *vertices, struct hitthing *hitthing)
{
	int16_t triref = 0;
	int trisremaining = 0;
	bool intersectsbbox = false;
	float *ptr;
	float tmp = 0.0f;
	float sqdist = 0.0f;
	bool hit = false;
	struct coord *point1, *point2, *point3;
	Vtx *vtx;
	Gfx *imggdl = NULL;
	int texturenum;
	float lowestsqdist = MAXFLOAT;
	uintptr_t offset;
	int numvertices;
	Gfx *tri4gdl = NULL;
	int count;
	struct coord min;
	struct coord max;
	struct coord sp8c;
	struct coord sp80;
	int points[3];

	while (true) {
		if (gdl->dma.cmd == G_ENDDL) {
			imggdl = NULL;

			if (gdl2 != NULL) {
				gdl = gdl2;
				gdl2 = NULL;
				continue;
			}
			break;
		} else if (gdl->dma.cmd == G_VTX) {
			ptr = g_TransformedVertices;
			count = gdl->bytes[GFX_W0_BYTE(1)] & 0xf;
			if (gdl->words.w1 & 1) {
				// segmented address
				offset = (UNSEGADDR(gdl->words.w1) & 0xffffff);
			} else {
				// linear address
				offset = gdl->words.w1 - (uintptr_t)vertices;
			}
			numvertices = (((uint32_t) gdl->bytes[GFX_W0_BYTE(1)] >> 4) & 0xf) + 1;
			vtx = (Vtx *)((uintptr_t)vertices + offset);
			vtx -= count;

			ptr[0] = vtx->x;
			ptr[1] = vtx->y;
			ptr[2] = vtx->z;

			min.x = ptr[0];
			max.x = ptr[0];
			min.y = ptr[1];
			max.y = ptr[1];
			min.z = ptr[2];
			max.z = ptr[2];

			ptr += 3;
			vtx++;
			numvertices--;

			while (numvertices > 0) {
				ptr[0] = vtx->x;
				ptr[1] = vtx->y;
				ptr[2] = vtx->z;

				if (ptr[0] < min.x) {
					min.x = ptr[0];
				}

				if (ptr[1] < min.y) {
					min.y = ptr[1];
				}

				if (ptr[2] < min.z) {
					min.z = ptr[2];
				}

				if (ptr[0] > max.x) {
					max.x = ptr[0];
				}

				if (ptr[1] > max.y) {
					max.y = ptr[1];
				}

				if (ptr[2] > max.z) {
					max.z = ptr[2];
				}

				vtx++;
				numvertices--;
				ptr += 3;
			}

			intersectsbbox = true;

			if (arg0->x < min.x) {
				if (arg1->x < min.x) {
					intersectsbbox = false;
				}
			} else if (arg0->x > max.x) {
				if (arg1->x > max.x) {
					intersectsbbox = false;
				}
			}

			if (arg0->y < min.y) {
				if (arg1->y < min.y) {
					intersectsbbox = false;
				}
			} else if (arg0->y > max.y) {
				if (arg1->y > max.y) {
					intersectsbbox = false;
				}
			}

			if (arg0->z < min.z) {
				if (arg1->z < min.z) {
					intersectsbbox = false;
				}
			} else if (arg0->z > max.z) {
				if (arg1->z > max.z) {
					intersectsbbox = false;
				}
			}

			if (intersectsbbox) {
				intersectsbbox = bgTestLineIntersectsBbox(arg0, arg2, &min, &max);
			}
		} else if (gdl->dma.cmd == (int8_t)G_SETTIMG) {
			imggdl = gdl;
		} else {
			if (!intersectsbbox) {
				gdl++;
				continue;
			}

			if (gdl->dma.cmd != G_TRI1 && gdl->dma.cmd != G_TRI4) {
				gdl++;
				continue;
			}

			if (gdl->dma.cmd == G_TRI1) {
				trisremaining = 0;
				triref = 0;
				points[0] = gdl->tri.tri.v[GFX_TRI_VTX(0)] / 10;
				points[1] = gdl->tri.tri.v[GFX_TRI_VTX(1)] / 10;
				points[2] = gdl->tri.tri.v[GFX_TRI_VTX(2)] / 10;
			} else if (gdl->dma.cmd == G_TRI4) {
				tri4gdl = gdl;
				trisremaining = 3;
				triref = 1;
				points[0] = gdl->tri4.x1;
				points[1] = gdl->tri4.y1;
				points[2] = gdl->tri4.z1;
			}

			do {
				if (points[0] == 0 && points[1] == 0 && points[2] == 0) {
					break;
				}

				point1 = (struct coord *) (g_TransformedVertices + points[0] * 3);
				point2 = (struct coord *) (g_TransformedVertices + points[1] * 3);
				point3 = (struct coord *) (g_TransformedVertices + points[2] * 3);

				min.x = point1->x;
				max.x = point1->x;

				if (point2->x < min.x) {
					min.x = point2->x;
				}

				if (point2->x > max.x) {
					max.x = point2->x;
				}

				if (point3->x < min.x) {
					min.x = point3->x;
				}

				if (point3->x > max.x) {
					max.x = point3->x;
				}

				if (!(arg0->x < min.x && arg1->x < min.x) && !(arg0->x > max.x && arg1->x > max.x)) {
					min.z = point1->z;
					max.z = point1->z;

					if (point2->z < min.z) {
						min.z = point2->z;
					}

					if (point2->z > max.z) {
						max.z = point2->z;
					}

					if (point3->z < min.z) {
						min.z = point3->z;
					}

					if (point3->z > max.z) {
						max.z = point3->z;
					}

					if (!(arg0->z < min.z && arg1->z < min.z) && !(arg0->z > max.z && arg1->z > max.z)) {
						min.y = point1->y;
						max.y = point1->y;

						if (point2->y < min.y) {
							min.y = point2->y;
						}

						if (point2->y > max.y) {
							max.y = point2->y;
						}

						if (point3->y < min.y) {
							min.y = point3->y;
						}

						if (point3->y > max.y) {
							max.y = point3->y;
						}

						if (!(arg0->y < min.y && arg1->y < min.y) && !(arg0->y > max.y && arg1->y > max.y)) {
							if (bgTestLineIntersectsBbox(arg0, arg2, &min, &max)
									&& utilsIntersectTest2(point1, point2, point3, NULL, arg0, arg1, arg2, &sp8c, &sp80)) {
								tmp = sp8c.x - arg0->x;
								sqdist = tmp * tmp;

								tmp = sp8c.y - arg0->y;
								sqdist += tmp * tmp;

								tmp = sp8c.z - arg0->z;
								sqdist += tmp * tmp;

								if (sqdist < lowestsqdist) {
									hit = true;

									if (imggdl == NULL
										|| (imggdl->words.w1 & 1)) {
										texturenum = -1;
									} else {
										uintptr_t tmp = (k_ptr_t)(UNSEGADDR(imggdl->words.w1) - 8);
										texturenum = *(int16_t *) tmp;
									}

									lowestsqdist = sqdist;

									hitthing->pos.x = sp8c.x;
									hitthing->pos.y = sp8c.y;
									hitthing->pos.z = sp8c.z;
									hitthing->unk0c.x = sp80.x;
									hitthing->unk0c.y = sp80.y;
									hitthing->unk0c.z = sp80.z;
									hitthing->point1 = &vtx[points[0]];
									hitthing->point2 = &vtx[points[1]];
									hitthing->point3 = &vtx[points[2]];
									hitthing->texturenum = texturenum;
									hitthing->tricmd = gdl;
									hitthing->unk28 = triref;
								}
							}
						}
					}
				}

				trisremaining--;

				if (trisremaining == 2) {
					points[0] = tri4gdl->tri4.x2;
					points[1] = tri4gdl->tri4.y2;
					points[2] = tri4gdl->tri4.z2;
					triref = 2;
				} else if (trisremaining == 1) {
					points[0] = tri4gdl->tri4.x3;
					points[1] = tri4gdl->tri4.y3;
					points[2] = tri4gdl->tri4.z3;
					triref = 3;
				} else if (trisremaining == 0) {
					points[0] = tri4gdl->tri4.x4;
					points[1] = tri4gdl->tri4.y4;
					points[2] = tri4gdl->tri4.z4;
					triref = 1;
				}
			} while (trisremaining >= 0);
		}

		gdl++;
	}

	return hit;
}

bool bgTestHitOnChr(struct model *model, struct coord *arg1, struct coord *arg2, struct coord *arg3,
		Gfx *gdl, Gfx *gdl2, Vtx *vertices, float *sqdistptr, struct hitthing *hitthing)
{
	int16_t triref = 0;
	int i = 0;
	bool intersectsbbox = false;
	int count;
	int spdc = 16;
	int spd8 = 0;
	int numvertices;
	float *ptr;
	bool hit = false;
	float tmp;
	float sqdist;
	Vtx *vtx;
	struct coord *point1;
	struct coord *point2;
	struct coord *point3;
	uint32_t word;
	Gfx *tri4gdl;
	Mtx *mtx = gfxAllocateMatrix();
	mtxIdent(mtx);
	struct coord min;
	struct coord max;
	struct coord sp84;
	struct coord sp78;
	int points[3];

	while (true) {
		if (gdl->dma.cmd == G_ENDDL) {
			if (gdl2 != NULL) {
				gdl = gdl2;
				gdl2 = NULL;
				continue;
			}
			break;
		} else if (gdl->dma.cmd == G_MTX) {
			word = UNSEGADDR(gdl->words.w1) & 0xffffff;
			i = word / sizeof(Mtx);
			mtx = (Mtx*)&model->matrices[i];
		} else if (gdl->dma.cmd == G_VTX) {
			count = (gdl->bytes[GFX_W0_BYTE(1)] & 0xf);
			word = UNSEGADDR(gdl->words.w1) & 0xffffff;
			numvertices = ((uint32_t) gdl->bytes[GFX_W0_BYTE(1)] >> 4) + 1;
			vtx = (Vtx *)((uintptr_t)vertices + word);

			if (count < spdc) {
				spdc = count;
			}

			if (numvertices + count > spd8) {
				spd8 = numvertices + count;
			}

			ptr = &g_TransformedVertices[count * 3];

			while (numvertices > 0) {
				ptr[0] = vtx->x;
				ptr[1] = vtx->y;
				ptr[2] = vtx->z;

				mtx4TransformVecInPlace(mtx, (struct coord *) ptr);

				numvertices--;
				ptr += 3;
				vtx++;
			}

			ptr = &g_TransformedVertices[spdc];

			min.x = ptr[0];
			max.x = ptr[0];
			min.y = ptr[1];
			max.y = ptr[1];
			min.z = ptr[2];
			max.z = ptr[2];

			ptr += 3;

			for (i = spdc; i < spd8; i++) {
				min.x = MIN(min.x, ptr[0]);
				min.y = MIN(min.y, ptr[1]);
				min.z = MIN(min.z, ptr[2]);
			
				max.x = MAX(max.x, ptr[0]);
				max.y = MAX(max.y, ptr[1]);
				max.z = MAX(max.z, ptr[2]);
			
				ptr += 3;
			}

			if ((arg1->x < min.x && arg2->x < min.x)
					|| (arg1->x > max.x && arg2->x > max.x)
					|| (arg1->y < min.y && arg2->y < min.y)
					|| (arg1->y > max.y && arg2->y > max.y)
					|| (arg1->z < min.z && arg2->z < min.z)
					|| (arg1->z > max.z && arg2->z > max.z)) {
				intersectsbbox = false;
			} else {
				intersectsbbox = bgTestLineIntersectsBbox(arg1, arg3, &min, &max);
			}
		} else {
			if (!intersectsbbox) {
				gdl++;
				continue;
			}

			if ((gdl->dma.cmd != G_TRI1 && gdl->dma.cmd != G_TRI4)) {
				gdl++;
				continue;
			}

			if (gdl->dma.cmd == G_TRI1) {
				i = 0;
				triref = 0;
				points[0] = gdl->tri.tri.v[GFX_TRI_VTX(0)] / 10;
				points[1] = gdl->tri.tri.v[GFX_TRI_VTX(1)] / 10;
				points[2] = gdl->tri.tri.v[GFX_TRI_VTX(2)] / 10;
			} else if (gdl->dma.cmd == G_TRI4) {
				tri4gdl = gdl;
				i = 3;
				triref = 1;
				points[0] = gdl->tri4.x1;
				points[1] = gdl->tri4.y1;
				points[2] = gdl->tri4.z1;
			}

			do {
				if (points[0] == 0 && points[1] == 0 && points[2] == 0) {
					break;
				}

				point1 = (struct coord *) (g_TransformedVertices + points[0] * 3);
				point2 = (struct coord *) (g_TransformedVertices + points[1] * 3);
				point3 = (struct coord *) (g_TransformedVertices + points[2] * 3);

				min.x = point1->x;
				max.x = point1->x;

				min.x = MIN(min.x, point2->x);
				max.x = MAX(max.x, point2->x);
				min.x = MIN(min.x, point3->x);
				max.x = MAX(max.x, point3->x);

				if (!(arg1->x < min.x && arg2->x < min.x) && !(arg1->x > max.x && arg2->x > max.x)) {
					min.z = point1->z;
					max.z = point1->z;

					min.z = MIN(min.z, point2->z);
					max.z = MAX(max.z, point2->z);
					min.z = MIN(min.z, point3->z);
					max.z = MAX(max.z, point3->z);

					if (!(arg1->z < min.z && arg2->z < min.z) && !(arg1->z > max.z && arg2->z > max.z)) {
						min.y = point1->y;
						max.y = point1->y;

						min.y = MIN(min.y, point2->y);
						max.y = MAX(max.y, point2->y);
						min.y = MIN(min.y, point3->y);
						max.y = MAX(max.y, point3->y);

						if (!(arg1->y < min.y && arg2->y < min.y) && !(arg1->y > max.y && arg2->y > max.y)) {
							if (bgTestLineIntersectsBbox(arg1, arg3, &min, &max)
									&& utilsIntersectTest2(point1, point2, point3, NULL, arg1, arg2, arg3, &sp84, &sp78)) {
								tmp = sp84.x - arg1->x;
								sqdist = tmp * tmp;

								tmp = sp84.y - arg1->y;
								sqdist += tmp * tmp;

								tmp = sp84.z - arg1->z;
								sqdist += tmp * tmp;

								if (sqdist < *sqdistptr) {
									hit = true;

									*sqdistptr = sqdist;

									hitthing->pos.x = sp84.x;
									hitthing->pos.y = sp84.y;
									hitthing->pos.z = sp84.z;
									hitthing->unk0c.x = sp78.x;
									hitthing->unk0c.y = sp78.y;
									hitthing->unk0c.z = sp78.z;
									hitthing->point1 = &vtx[points[0]];
									hitthing->point2 = &vtx[points[1]];
									hitthing->point3 = &vtx[points[2]];
									hitthing->texturenum = -1;
									hitthing->tricmd = gdl;
									hitthing->unk28 = triref;
								}
							}
						}
					}
				}

				i--;

				if (i == 2) {
					points[0] = tri4gdl->tri4.x2;
					points[1] = tri4gdl->tri4.y2;
					points[2] = tri4gdl->tri4.z2;
					triref = 2;
				} else if (i == 1) {
					points[0] = tri4gdl->tri4.x3;
					points[1] = tri4gdl->tri4.y3;
					points[2] = tri4gdl->tri4.z3;
					triref = 3;
				} else if (i == 0) {
					points[0] = tri4gdl->tri4.x4;
					points[1] = tri4gdl->tri4.y4;
					points[2] = tri4gdl->tri4.z4;
					triref = 1;
				}
			} while (i >= 0);
		}

		gdl++;
	}

	return hit;
}

bool bgTestHitInVtxBatch(struct coord *arg0, struct coord *arg1, struct coord *arg2, struct vtxbatch *batch, int roomnum, struct hitthing *hitthing, bool artifactTest)
{
	int16_t triref = 0;
	int trisremaining = 0;
	Gfx *gdl = batch->gdl;
	bool hit;
	int points[3];
	int numvertices = 0;
	float sqdist = 0.0f;
	float lowestsqdist = 0.0f;
	int texturenum = 0;
	int index = 0;
	struct coord *point1;
	struct coord *point2;
	struct coord *point3;
	struct coord spb0;
	struct coord spa4;
	struct coord min;
	struct coord max;
	Vtx *vtx;
	float *ptr;
	Gfx *iter;
	Gfx *tmpgdl;
	Gfx *tri4gdl;

	if(batch->type == VTXBATCHTYPE_XLU && artifactTest)
	{
		return false;
	}

	vtx = bgFindVerticesForGdl(roomnum, gdl);
	iter = &gdl[batch->gbicmdindex];
	vtx = (Vtx *)((UNSEGADDR(iter->words.w1) & 0xffffff) + (uintptr_t)vtx);
	numvertices = (((uint32_t) iter->bytes[GFX_W0_BYTE(1)] >> 4) & 0xf) + 1;
	ptr = g_TransformedVertices;

	while (numvertices > 0) {
		ptr[0] = g_BgRooms[roomnum].pos.x + vtx->x;
		ptr[1] = g_BgRooms[roomnum].pos.y + vtx->y;
		ptr[2] = g_BgRooms[roomnum].pos.z + vtx->z;

		ptr += 3;
		vtx++;
		numvertices--;
	}

	lowestsqdist = MAXFLOAT;
	hit = false;

	iter++;

	while (iter->dma.cmd != G_VTX && iter->dma.cmd != G_ENDDL) {
		if (iter->dma.cmd != G_TRI1 && iter->dma.cmd != G_TRI4) {
			iter++;
			continue;
		}

		if (iter->dma.cmd == G_TRI1) {
			trisremaining = 0;
			triref = 0;
			points[0] = iter->tri.tri.v[GFX_TRI_VTX(0)] / 10;
			points[1] = iter->tri.tri.v[GFX_TRI_VTX(1)] / 10;
			points[2] = iter->tri.tri.v[GFX_TRI_VTX(2)] / 10;
		} else if (iter->dma.cmd == G_TRI4) {
			tri4gdl = iter;
			trisremaining = 3;
			triref = 1;
			points[0] = tri4gdl->tri4.x1;
			points[1] = tri4gdl->tri4.y1;
			points[2] = tri4gdl->tri4.z1;
		}

		do {
			if (points[0] == 0 && points[1] == 0 && points[2] == 0) {
				break;
			}

			point1 = (struct coord *) (g_TransformedVertices + points[0] * 3);
			point2 = (struct coord *) (g_TransformedVertices + points[1] * 3);
			point3 = (struct coord *) (g_TransformedVertices + points[2] * 3);

			min.x = point1->x;

			if (point2->x < min.x) {
				min.x = point2->x;
			}

			if (point3->x < min.x) {
				min.x = point3->x;
			}

			if (!(arg0->x < min.x && arg1->x < min.x)) {
				max.x = point1->x;

				if (point2->x > max.x) {
					max.x = point2->x;
				}

				if (point3->x > max.x) {
					max.x = point3->x;
				}

				if (!(arg0->x > max.x && arg1->x > max.x)) {
					min.z = point1->z;

					if (point2->z < min.z) {
						min.z = point2->z;
					}

					if (point3->z < min.z) {
						min.z = point3->z;
					}

					if (!(arg0->z < min.z && arg1->z < min.z)) {
						max.z = point1->z;

						if (point2->z > max.z) {
							max.z = point2->z;
						}

						if (point3->z > max.z) {
							max.z = point3->z;
						}

						if (!(arg0->z > max.z && arg1->z > max.z)) {
							min.y = point1->y;

							if (point2->y < min.y) {
								min.y = point2->y;
							}

							if (point3->y < min.y) {
								min.y = point3->y;
							}

							if (!(arg0->y < min.y && arg1->y < min.y)) {
								max.y = point1->y;

								if (point2->y > max.y) {
									max.y = point2->y;
								}

								if (point3->y > max.y) {
									max.y = point3->y;
								}

								if (!(arg0->y > max.y && arg1->y > max.y)) {
									if (bgTestLineIntersectsBbox(arg0, arg2, &min, &max)
											&& utilsIntersectTest2(point1, point2, point3, NULL, arg0, arg1, arg2, &spb0, &spa4)) {
										float tmp;

										tmp = spb0.x - arg0->x;
										sqdist = tmp * tmp;
										tmp = spb0.y - arg0->y;
										sqdist += tmp * tmp;
										tmp = spb0.z - arg0->z;
										sqdist += tmp * tmp;

										if (sqdist < lowestsqdist) {
											hit = true;

											tmpgdl = iter;

											while (tmpgdl->bytes[GFX_W0_BYTE(0)] != G_SETTIMG && tmpgdl > gdl) {
												tmpgdl--;
											}

											if (tmpgdl == gdl
													|| (tmpgdl->words.w1 & 1)) {
												texturenum = -1;
											} else {
												uintptr_t tmp = UNSEGADDR(tmpgdl->words.w1) - 8;
												texturenum = *(int16_t *) (k_ptr_t)(tmp);
											}

											if (batch->type == VTXBATCHTYPE_XLU && texturenum >= 0 && g_Textures[texturenum].surfacetype == SURFACETYPE_DEFAULT) {
												hit = false;
											}

											if (hit) {
												lowestsqdist = sqdist;

												hitthing->pos.x = spb0.x;
												hitthing->pos.y = spb0.y;
												hitthing->pos.z = spb0.z;
												hitthing->unk0c.x = spa4.x;
												hitthing->unk0c.y = spa4.y;
												hitthing->unk0c.z = spa4.z;
												hitthing->point1 = &vtx[points[0]];
												hitthing->point2 = &vtx[points[1]];
												hitthing->point3 = &vtx[points[2]];
												hitthing->texturenum = texturenum;
												hitthing->tricmd = iter;
												hitthing->unk28 = triref;
												hitthing->unk2c = batch->type;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			trisremaining--;

			if (trisremaining == 2) {
				points[0] = tri4gdl->tri4.x2;
				points[1] = tri4gdl->tri4.y2;
				points[2] = tri4gdl->tri4.z2;
				triref = 2;
			} else if (trisremaining == 1) {
				points[0] = tri4gdl->tri4.x3;
				points[1] = tri4gdl->tri4.y3;
				points[2] = tri4gdl->tri4.z3;
				triref = 3;
			} else if (trisremaining == 0) {
				points[0] = tri4gdl->tri4.x4;
				points[1] = tri4gdl->tri4.y4;
				points[2] = tri4gdl->tri4.z4;
				triref = 1;
			}
		} while (trisremaining >= 0);

		iter++;
	}

	return hit;
}

int bgRayIntersectAABBEntryPoint(struct coord *bbmin, struct coord *bbmax, struct coord *frompos, struct coord *dist, struct coord *invdir, struct coord *out_pos)
{
	int i;
	uint8_t bail = true;
	int8_t sp48[3];
	int bestindex;
	float sp38[3];
	float sp2c[3];

	for (i = 0; i < 3; i++) {
		if (frompos->f[i] < bbmin->f[i]) {
			sp48[i] = 1;
			sp38[i] = bbmin->f[i];
			bail = false;
		} else if (frompos->f[i] > bbmax->f[i]) {
			sp48[i] = 0;
			sp38[i] = bbmax->f[i];
			bail = false;
		} else {
			sp48[i] = 2;
			sp38[i] = 0.0f;
		}
	}

	if (bail) {
		return -1;
	}

	for (i = 0; i < 3; i++) {
		if (sp48[i] != 2 && dist->f[i] != 0.0f) {
			sp2c[i] = (sp38[i] - frompos->f[i]) * invdir->f[i];
		} else {
			sp2c[i] = -1.0f;
		}
	}

	bestindex = 0;

	for (i = 1; i < 3; i++) {
		if (sp2c[i] > sp2c[bestindex]) {
			bestindex = i;
		}
	}

	if (sp2c[bestindex] < 0.0f) {
		return 0;
	}

	for (i = 0; i < 3; i++) {
		if (bestindex != i) {
			out_pos->f[i] = frompos->f[i] + sp2c[bestindex] * dist->f[i];

			if (out_pos->f[i] < bbmin->f[i] || out_pos->f[i] > bbmax->f[i]) {
				return 0;
			}
		} else {
			out_pos->f[i] = sp38[i];
		}
	}

	return 1;
}

/**
 * Figure out which piece of BG geometry is hit in the given room based on a
 * line intersection from frompos to topos. Populate the hitthing struct with
 * the details. Props are not considered. Return true if a hit occurred.
 *
 * This is used not only for shots, but blood splatters and explosion scorch
 * marks too.
 *
 * Room vertices are already grouped into batches, where each batch has a
 * precomputed bounding box.
 */
bool bgTestHitInRoom(struct coord *frompos, struct coord *topos, int roomnum, struct hitthing *hitthing, bool artifactTest)
{
	int i;
	int count;
	float f20;
	int a0;
	int numbatches;
	int j;
	float f0;
	float spc8;
	float f2;
	struct coord from;
	struct coord to;
	struct coord dist;
	struct coord sp94;
	struct vtxbatch *batch;
	struct hitthing sp60;
	int tmpindex;

	count = 0;

	from.x = frompos->x;
	from.y = frompos->y;
	from.z = frompos->z;

	to.x = topos->x;
	to.y = topos->y;
	to.z = topos->z;

	dist.x = to.x - from.x;
	dist.y = to.y - from.y;
	dist.z = to.z - from.z;

	sp94.x = 1.0f / dist.x;
	sp94.y = 1.0f / dist.y;
	sp94.z = 1.0f / dist.z;

	if (roomnum < 0 || roomnum >= g_Vars.roomcount) {
		return false;
	}

	batch = g_Rooms[roomnum].vtxbatches;

	if (batch == NULL) {
		return false;
	}

	numbatches = g_Rooms[roomnum].numvtxbatches;

	for (i = 0; i < numbatches; batch++, i++) {
		j = bgRayIntersectAABBEntryPoint(&batch->bbmin, &batch->bbmax, &from, &dist, &sp94, &hitthing->pos);

		if (j == 0) {
			continue;
		}

		if (j == 1) {
			f0 = from.x - hitthing->pos.x;
			f20 = f0 * f0;

			f0 = from.y - hitthing->pos.y;
			f20 += f0 * f0;

			f0 = from.z - hitthing->pos.z;
			f20 += f0 * f0;
		} else {
			f20 = -1.0f;
		}

		if (count < ARRAYCOUNT(var800a6538)) {
			a0 = i;

			for (j = 0; j < count; j++) {
				f2 = var800a6538[j].unk04;

				if (f2 > f20) {
					tmpindex = var800a6538[j].vtxbatchindex;
					var800a6538[j].vtxbatchindex = a0;
					a0 = tmpindex;

					var800a6538[j].unk04 = f20;
					f20 = f2;
				}
			}

			var800a6538[j].vtxbatchindex = a0;
			var800a6538[j].unk04 = f20;
			count++;
		} else {
			count = 0;

			for (j = 0; j < ARRAYCOUNT(var800a6538); j++) {
				if (bgTestHitInVtxBatch(&from, &to, &dist, &g_Rooms[roomnum].vtxbatches[var800a6538[j].vtxbatchindex], roomnum, hitthing, artifactTest)) {
					f0 = from.x - hitthing->pos.x;
					f2 = f0 * f0;

					f0 = from.y - hitthing->pos.y;
					f2 += f0 * f0;

					f0 = from.z - hitthing->pos.z;
					f2 += f0 * f0;

					if (count == 0) {
						var800a6538[0].vtxbatchindex = var800a6538[j].vtxbatchindex;
						var800a6538[0].unk04 = f2;
						count = 1;
					} else if (f2 < var800a6538[0].unk04) {
						var800a6538[0].vtxbatchindex = var800a6538[j].vtxbatchindex;
						var800a6538[0].unk04 = f2;
						count = 1;
					}
				}
			}

			if (count != 0) {
				if (f20 < var800a6538[0].unk04) {
					var800a6538[1].unk04 = var800a6538[0].unk04;
					var800a6538[0].unk04 = f20;
					var800a6538[1].vtxbatchindex = var800a6538[0].vtxbatchindex;
					var800a6538[0].vtxbatchindex = i;
				} else {
					var800a6538[1].vtxbatchindex = i;
					var800a6538[1].unk04 = f20;
				}

				count = 2;
			} else {
				var800a6538[0].vtxbatchindex = i;
				var800a6538[0].unk04 = f20;
				count = 1;
			}
		}
	}

	if (count == 0) {
		return false;
	}

	batch = g_Rooms[roomnum].vtxbatches;

	for (i = 0; i < count; i++) {
		if (bgTestHitInVtxBatch(&from, &to, &dist, &batch[var800a6538[i].vtxbatchindex], roomnum, hitthing, artifactTest)) {
			i++;

			if (i < count) {
				f0 = from.x - hitthing->pos.x;
				spc8 = f0 * f0;

				f0 = from.y - hitthing->pos.y;
				spc8 += f0 * f0;

				f0 = from.z - hitthing->pos.z;
				spc8 += f0 * f0;

				for (; i < count; i++) {
					if (var800a6538[i].unk04 <= spc8) {
						if (bgTestHitInVtxBatch(&from, &to, &dist, &batch[var800a6538[i].vtxbatchindex], roomnum, &sp60, artifactTest)) {
							f0 = from.f[0] - sp60.pos.f[0];
							f20 = f0 * f0;

							f0 = from.f[1] - sp60.pos.f[1];
							f20 += f0 * f0;

							f0 = from.f[2] - sp60.pos.f[2];
							f20 += f0 * f0;

							if (f20 < spc8) {
								hitthing->pos.x = sp60.pos.x;
								hitthing->pos.y = sp60.pos.y;
								hitthing->pos.z = sp60.pos.z;
								hitthing->unk0c.x = sp60.unk0c.x;
								hitthing->unk0c.y = sp60.unk0c.y;
								hitthing->unk0c.z = sp60.unk0c.z;
								hitthing->point1 = sp60.point1;
								hitthing->point2 = sp60.point2;
								hitthing->point3 = sp60.point3;
								hitthing->texturenum = sp60.texturenum;
								hitthing->tricmd = sp60.tricmd;
								hitthing->unk28 = sp60.unk28;
								hitthing->unk2c = sp60.unk2c;

								spc8 = f20;
							}
						}
					}
				}
			}

			return true;
		}
	}

	return false;
}

bool bgRoomIsLoaded(int room)
{
	return g_Rooms[room].loaded240;
}

bool bgRoomContainsCoord(struct coord *pos, RoomNum roomnum)
{
	struct coord copy;
	copy.x = pos->x;
	copy.y = pos->y;
	copy.z = pos->z;

	return copy.f[0] >= g_Rooms[roomnum].bbmin[0]
		&& copy.f[0] <= g_Rooms[roomnum].bbmax[0]
		&& copy.f[2] >= g_Rooms[roomnum].bbmin[2]
		&& copy.f[2] <= g_Rooms[roomnum].bbmax[2]
		&& copy.f[1] >= g_Rooms[roomnum].bbmin[1]
		&& copy.f[1] <= g_Rooms[roomnum].bbmax[1];
}

/**
 * Test if a world position is inside a room based on portal checks only.
 *
 * It works by iterating the portals for the given room and checking which side
 * of the portal the position is on. If the position is on the "inside" of all
 * portals then the position is determined to be in the room.
 *
 * Note that:
 * - Clockwise portal vertices makes the normal face towards the viewer.
 * - The normal points towards the front of the portal.
 * - The room on the front side is roomnum2.
 */
bool bgTestPosInRoomCheap(struct coord *pos, RoomNum roomnum)
{
	int i;

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		int portalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		struct portalmetric *metric = &g_PortalMetrics[portalnum];

		float value = metric->normal.f[0] * pos->f[0]
			+ metric->normal.f[1] * pos->f[1]
			+ metric->normal.f[2] * pos->f[2];

		if (value < metric->min) {
			if (roomnum != g_BgPortals[portalnum].roomnum1) {
				return false;
			}
		} else if (value > metric->max) {
			if (roomnum != g_BgPortals[portalnum].roomnum2) {
				return false;
			}
		}
	}

	return true;
}

bool bgTestPosInRoomExpensive(struct coord *pos, RoomNum roomnum)
{
	int t5;
	struct coord *next;
	int t4;
	int portalnum;
	struct portalmetric *metric;
	int j;
	float f0;
	struct portalvertices *pvertices;
	struct coord sp74;
	struct coord sp68;
	struct coord sp5c;
	float sp58[1];
	struct coord sp4c;
	struct coord *cur;
	float f18;
	int i;
	float sum;

	sp74.f[0] = g_Rooms[roomnum].centre.f[0];
	sp74.f[1] = g_Rooms[roomnum].centre.f[1];
	sp74.f[2] = g_Rooms[roomnum].centre.f[2];

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		portalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		pvertices = (struct portalvertices *)((uint8_t *) g_BgPortals + g_BgPortals[portalnum].verticesoffset);
		metric = &g_PortalMetrics[portalnum];

		f0 = pos->f[0] * metric->normal.f[0] + pos->f[1] * metric->normal.f[1] + pos->f[2] * metric->normal.f[2];
		f18 = sp74.f[0] * metric->normal.f[0] + sp74.f[1] * metric->normal.f[1] + sp74.f[2] * metric->normal.f[2];

		if (f0 < metric->min) {
			if (f18 < metric->min) {
				continue;
			}
		} else {
			if (f0 > metric->max && f18 > metric->max) {
				continue;
			}
		}

		sp68.f[0] = sp74.f[0] - pos->f[0];
		sp68.f[1] = sp74.f[1] - pos->f[1];
		sp68.f[2] = sp74.f[2] - pos->f[2];

		t4 = 0;
		t5 = true;
		cur = &pvertices->vertices[0];
		next = &pvertices->vertices[1];

		for (j = 0; j < pvertices->count; j++) {
			if (j + 1 == pvertices->count) {
				next = &pvertices->vertices[0];
			}

			sp5c.f[0] = next->f[0] - cur->f[0];
			sp5c.f[1] = next->f[1] - cur->f[1];
			sp5c.f[2] = next->f[2] - cur->f[2];

			sp4c.f[0] = sp5c.f[1] * sp68.f[2] - sp5c.f[2] * sp68.f[1];
			sp4c.f[1] = sp5c.f[2] * sp68.f[0] - sp5c.f[0] * sp68.f[2];
			sp4c.f[2] = sp5c.f[0] * sp68.f[1] - sp5c.f[1] * sp68.f[0];

			sum = sp4c.f[0] * sp4c.f[0] + sp4c.f[1] * sp4c.f[1] + sp4c.f[2] * sp4c.f[2];

			if (sum == 0.0f) {
				t5 = false;
				break;
			}

			sp58[0] = sp4c.f[0] * cur->f[0] + sp4c.f[1] * cur->f[1] + sp4c.f[2] * cur->f[2];
			sum = sp4c.f[0] * pos->f[0] + sp4c.f[1] * pos->f[1] + sp4c.f[2] * pos->f[2];

			if (sum < sp58[0]) {
				if (t4 == 2) {
					t5 = false;
					break;
				}

				t4 = 1;
			} else if (t4 == 1) {
				t5 = false;
				break;
			} else {
				t4 = 2;
			}

			cur++;
			next++;
		}

		if (t5) {
			if (f0 < metric->min) {
				if (roomnum == g_BgPortals[portalnum].roomnum2) {
					return false;
				}
			} else if (f0 > metric->max) {
				if (roomnum == g_BgPortals[portalnum].roomnum1) {
					return false;
				}
			}
		}
	}

	return true;
}

bool bgTestPosInRoom(struct coord *pos, RoomNum roomnum)
{
	if (g_Rooms[roomnum].flags & ROOMFLAG_COMPLICATEDPORTALS) {
		return bgTestPosInRoomExpensive(pos, roomnum);
	} else {
		return bgTestPosInRoomCheap(pos, roomnum);
	}
}

/**
 * Find rooms near the given pos so the caller can decide which room pos is
 * actually in.
 *
 * inrooms should point to an empty array of room numbers whose length is max + 1.
 * The function will populate this array with room numbers where the pos is
 * inside the room's bounding box.
 *
 * aboverooms works the same way, but it'll be populated with room numbers where
 * the pos is above the room's bounding box.
 *
 * For both arrays, the function will only consider rooms that have portals.
 * If this doesn't produce anything for both arrays, the function will try again
 * but with rooms that have no portals.
 *
 * If all of the above produces no results (ie. the pos is out of bounds) and
 * the bestroom pointer is not NULL, the function then finds the closest room
 * to pos and writes the room number to the bestroom pointer. The bestroom
 * pointer is a pointer to a single int16_t rather than an array.
 */
void bgFindRoomsByPos(struct coord *posarg, RoomNum *inrooms, RoomNum *aboverooms, int max, RoomNum *bestroom)
{
	int inlen = 0;
	int abovelen = 0;
	int closestroomnum = -1;
	struct coord pos;
	float closestdist = 0.0f;
	int i;
	int j;

	pos.x = posarg->x;
	pos.y = posarg->y;
	pos.z = posarg->z;

	// Try rooms which have portals
	for (i = 1; i < g_Vars.roomcount; i++) {
		if (g_Rooms[i].numportals > 0
				&& pos.x >= g_Rooms[i].bbmin[0] && pos.x <= g_Rooms[i].bbmax[0]
				&& pos.z >= g_Rooms[i].bbmin[2] && pos.z <= g_Rooms[i].bbmax[2]
				&& pos.y >= g_Rooms[i].bbmin[1]) {
			if (pos.y <= g_Rooms[i].bbmax[1]) {
				// Pos is inside the bbox
				if (inlen < max) {
					inrooms[inlen] = i;
					inlen++;
				}
			} else {
				// Pos is above the bbox
				if (abovelen < max) {
					aboverooms[abovelen] = i;
					abovelen++;
				}
			}
		}
	}

	// Try again but with rooms that have no portals
	if (inlen == 0 && abovelen == 0) {
		for (i = 1; i < g_Vars.roomcount; i++) {
			if (g_Rooms[i].numportals == 0
					&& pos.x >= g_Rooms[i].bbmin[0] && pos.x <= g_Rooms[i].bbmax[0]
					&& pos.z >= g_Rooms[i].bbmin[2] && pos.z <= g_Rooms[i].bbmax[2]
					&& pos.y >= g_Rooms[i].bbmin[1]) {
				if (pos.y <= g_Rooms[i].bbmax[1]) {
					// Pos is inside the bbox
					if (inlen < max) {
						inrooms[inlen] = i;
						inlen++;
					}
				} else {
					// Pos is above the bbox
					if (abovelen < max) {
						aboverooms[abovelen] = i;
						abovelen++;
					}
				}
			}
		}
	}

	inrooms[inlen] = -1;
	aboverooms[abovelen] = -1;

	if (bestroom != NULL) {
		if (inlen == 0 && abovelen == 0) {
			for (i = 1; i < g_Vars.roomcount; i++) {
				float dist = 0.0f;

				for (j = 0; j < 3; j++) {
					if (pos.f[j] < g_Rooms[i].bbmin[j] || pos.f[j] > g_Rooms[i].bbmax[j]) {
						float dist1 = pos.f[j] - g_Rooms[i].bbmin[j];
						float dist2 = pos.f[j] - g_Rooms[i].bbmax[j];

						if (dist1 < 0.0f) {
							dist1 = -dist1;
						}

						if (dist2 < 0.0f) {
							dist2 = -dist2;
						}

						if (dist2 < dist1) {
							dist1 = dist2;
						}

						dist += dist1;
					}
				}

				if (dist > 0.0f && (closestroomnum < 0 || dist < closestdist)) {
					closestroomnum = i;
					closestdist = dist;
				}
			}
		}

		*bestroom = closestroomnum >= 0 ? closestroomnum : -1;
	}
}

bool bgCmdPushValue(bool value)
{
	g_BgCmdStack[g_BgCmdStackIndex] = value;
	g_BgCmdStackIndex = (g_BgCmdStackIndex + 1) % 20;

	return value;
}

bool bgCmdPopValue(void)
{
	bool val = g_BgCmdStack[g_BgCmdStackIndex = (g_BgCmdStackIndex + 19) % 20];
	return val;
}

bool bgCmdGetNthValueFromEnd(int n)
{
	return g_BgCmdStack[((g_BgCmdStackIndex - n) + 19) % 20];
}

/**
 * BG files contain bytecode that is used to override the default portal
 * behaviour. They can be used to check if the camera is in particular rooms
 * and then force other rooms to show or hide.
 *
 * Only six BG files use of this feature. They are Villa, Chicago, Area 51,
 * Pelagic II, Deep Sea and Skedar Ruins. All other BG files contain a single
 * "end" instruction in their bytecode.
 *
 * The scripting language supports if-statements with an infinite nesting level.
 * The interpreter maintains a stack of boolean values which can be pushed to
 * or popped from the end. This can be used to build complex conditions that
 * combine "AND" and "OR" operations.
 *
 * All commands are interpreted in order. There is no support for loops.
 *
 * When processing conditional code the function calls itself recursively for
 * that branch. The execute argument denotes whether the condition passed and
 * these statements should be executed, or whether the condition failed and
 * it's just passing over them to get to the endif command.
 */
struct bgcmd *bgCmdExecuteBranch(struct bgcmd *cmd, bool execute)
{
	int i;

	g_BgCmdThrowing = false;

	if (!cmd) {
		return cmd;
	}

	while (true) {
		switch (cmd->type) {
		case BGCMD_END:
			return cmd;
		case BGCMD_PUSH:
			if (execute) {
				bgCmdPushValue(cmd->param);
			}
			cmd += cmd->len;
			break;
		case BGCMD_POP:
			if (execute) {
				bgCmdPopValue();
			}
			cmd += cmd->len;
			break;
		case BGCMD_AND:
			if (execute) {
				bgCmdPushValue(bgCmdPopValue() & bgCmdPopValue());
			}
			cmd += cmd->len;
			break;
		case BGCMD_OR:
			if (execute) {
				bgCmdPushValue(bgCmdPopValue() | bgCmdPopValue());
			}
			cmd += cmd->len;
			break;
		case BGCMD_NOT:
			if (execute) {
				bgCmdPushValue(bgCmdPopValue() == 0);
			}
			cmd += cmd->len;
			break;
		case BGCMD_XOR:
			if (execute) {
				bgCmdPushValue(bgCmdPopValue() ^ bgCmdPopValue());
			}
			cmd += cmd->len;
			break;
		case BGCMD_PUSH_CAMINROOMRANGE:
			if (execute) {
				bgCmdPushValue(g_CamRoom >= cmd[1].param && g_CamRoom <= cmd[2].param);
			}
			cmd += cmd->len;
			break;
		case BGCMD_SETRESULT_TRUE:
			if (execute) {
				g_BgCmdScreenBox.xmin = g_Vars.currentplayer->screenxminf;
				g_BgCmdScreenBox.ymin = g_Vars.currentplayer->screenyminf;
				g_BgCmdScreenBox.xmax = g_Vars.currentplayer->screenxmaxf;
				g_BgCmdScreenBox.ymax = g_Vars.currentplayer->screenymaxf;
				g_BgCmdResult = BGRESULT_TRUE;
			}
			cmd += cmd->len;
			break;
		case BGCMD_SETRESULT_IFPORTALINFOV:
			if (execute) {
				if (!PORTAL_IS_CLOSED(cmd[1].param)) {
					if (!bgGetPortalScreenBbox(cmd[1].param, &g_PortalScreenBbox)) {
						g_BgCmdResult = BGRESULT_FALSE;
					} else if (bgGetBoxIntersection(&g_BgCmdScreenBox, &g_PortalScreenBbox) == 0) {
						g_BgCmdResult = BGRESULT_FALSE;
					} else {
						g_BgCmdResult = BGRESULT_TRUE;
					}
				}
			}
			cmd += cmd->len;
			break;
		case BGCMD_SETRESULT_TRUEIFTHROUGHPORTAL:
			if (execute) {
				struct screenbox portalbox;

				if (!PORTAL_IS_CLOSED(cmd[1].param)) {
					if (bgGetPortalScreenBbox(cmd[1].param, &portalbox) && bgGetBoxIntersection(&g_BgCmdScreenBox, &portalbox)) {
						if (g_BgCmdResult != BGRESULT_TRUE) {
							bgCopyBox(&g_BgCmdScreenBox, &portalbox);
							g_BgCmdResult = BGRESULT_TRUE;
						} else {
							bgExpandBox(&g_BgCmdScreenBox, &portalbox);
						}
					}
				}
			}
			cmd += cmd->len;
			break;
		case BGCMD_SETRESULT_FALSEIFNOTTHROUGHPORTAL:
			if (execute) {
				if (g_BgCmdResult == BGRESULT_TRUE) {
					struct screenbox portalbox;

					if (PORTAL_IS_CLOSED(cmd[1].param)) {
						g_BgCmdResult = BGRESULT_FALSE;
					} else if (!bgGetPortalScreenBbox(cmd[1].param, &portalbox)) {
						g_BgCmdResult = BGRESULT_FALSE;
					} else if (bgGetBoxIntersection(&portalbox, (struct screenbox *)&g_Vars.currentplayer->screenxminf) == 0) {
						g_BgCmdResult = BGRESULT_FALSE;
					} else if (bgGetBoxIntersection(&g_PortalScreenBbox, &portalbox) == 0) {
						g_BgCmdResult = BGRESULT_FALSE;
					}
				}
			}
			cmd += cmd->len;
			break;
		case BGCMD_IFRESULT_SHOWROOM:
			if (execute) {
				if (g_BgCmdResult == BGRESULT_TRUE && bgRoomIntersectsScreenBox(cmd[1].param, &g_BgCmdScreenBox)) {
					bgSetRoomOnscreen(cmd[1].param, 0, &g_BgCmdScreenBox);
					g_BgForceOnscreenRooms[g_BgNumForceOnscreenRooms++] = cmd[1].param;
				}
			}
			cmd += cmd->len;
			break;
		case BGCMD_DISABLEROOM:
			if (execute) {
				g_Rooms[cmd[1].param].flags |= ROOMFLAG_DISABLEDBYSCRIPT;
			}
			cmd += cmd->len;
			break;
		case BGCMD_DISABLEROOMRANGE:
			if (execute) {
				for (i = cmd[1].param; i <= cmd[2].param; i++) {
					g_Rooms[i].flags |= ROOMFLAG_DISABLEDBYSCRIPT;
				}
			}
			cmd += cmd->len;
			break;
		case BGCMD_LOADROOM:
			cmd += cmd->len;
			break;
		case BGCMD_LOADROOMRANGE:
			cmd += cmd->len;
			break;
		case BGCMD_SETROOMTESTSDISABLED:
			if (execute) {
				g_BgRoomTestsDisabled = true;
			}
			cmd += cmd->len;
			break;
		case BGCMD_PUSH_PORTALISOPEN:
			if (execute) {
				bgCmdPushValue(!PORTAL_IS_CLOSED(cmd[1].param));
			}
			cmd += cmd->len;
			break;
		case BGCMD_2A:
			if (execute) {
				g_Rooms[cmd[1].param].unk07 = 0;
			}
			cmd += cmd->len;
			break;
		case BGCMD_SETRESULT_FALSE:
			if (execute) {
				g_BgCmdResult = BGRESULT_FALSE;
			}
			cmd += cmd->len;
			break;
		case BGCMD_BRANCH:
			cmd = bgCmdExecuteBranch(cmd + cmd->len, execute);
			cmd += cmd->len;
			break;
		case BGCMD_CATCH:
			cmd += cmd->len;
			g_BgCmdThrowing = false;
			return cmd;
		case BGCMD_THROW:
			cmd += cmd->len;
			if (execute) {
				g_BgCmdThrowing = true;
			}
			execute = false;
			break;
		case BGCMD_IF:
			cmd = bgCmdExecuteBranch(cmd + cmd->len, bgCmdPopValue() & execute);
			if (g_BgCmdThrowing) {
				execute = false;
			}
			break;
		case BGCMD_ELSE:
			/**
			 * Assuming this is indeed an else command, it's not safe to assume
			 * that the execution state can be unconditionally toggled.
			 * For example, given the following portal code:
			 *
			 * if (a false condition)
			 *     if (any condition)
			 *         branch 1
			 *     else
			 *         branch 2
			 *     endif
			 * endif
			 *
			 * ...when reaching the else, execution would be turned on.
			 *
			 * However, this command isn't even used.
			 */
			execute ^= 1;
			cmd += cmd->len;
			break;
		case BGCMD_ENDIF:
			/**
			 * Note the return here rather than break.
			 */
			cmd += cmd->len;
			return cmd;
		default:
			return cmd;
		}
	}

	g_BgCmdThrowing = false;

	return cmd;
}

struct bgcmd *bgCmdExecute(struct bgcmd *cmd)
{
	struct player *player = g_Vars.currentplayer;
	g_BgCmdResult = BGRESULT_TRUE;

	if (!cmd) {
		return cmd;
	}

	// This may have been used in an osSyncPrintf call
	bgCmdGetNthValueFromEnd(0);

	g_BgCmdScreenBox.xmin = player->screenxminf;
	g_BgCmdScreenBox.ymin = player->screenyminf;
	g_BgCmdScreenBox.xmax = player->screenxmaxf;
	g_BgCmdScreenBox.ymax = player->screenymaxf;

	return bgCmdExecuteBranch(cmd, true);
}

void bgTickPortalsXray(void)
{
	struct coord vismax;
	struct coord vismin;
	struct coord eraserpos;
	struct coord vismid;
	struct player *player = g_Vars.currentplayer;
	int16_t ymax;
	int16_t xmax;
	int16_t ymin;
	int16_t xmin;
	struct stagetableentry *stage;
	int i;
	struct drawslot *thing;

	static uint32_t edist = 400;

	bgCalculateScreenProperties();

	if (g_BgDrawSlots);

	if (g_BgNumAttemptedDrawSlots > g_BgMostAttemptedDrawSlots) {
		g_BgMostAttemptedDrawSlots = g_BgNumAttemptedDrawSlots;
	}

	xmin = player->screenxminf;
	ymin = player->screenyminf;
	xmax = player->screenxmaxf;
	ymax = player->screenymaxf;

	if (bgunGetWeaponNum(HAND_RIGHT) == WEAPON_FARSIGHT && player->gunsightoff == 0) {
		player->eraserdepth = -500.0f / camGetLodScaleZ();
	} else {
		player->eraserdepth = -500.0f;
	}

	eraserpos.f[0] = 0.0f;
	eraserpos.f[1] = 0.0f;
	eraserpos.f[2] = player->eraserdepth;

	mtx4TransformVecInPlace(camGetProjectionMtx(), &eraserpos);

	player->eraserpos.f[0] = eraserpos.f[0];
	player->eraserpos.f[1] = eraserpos.f[1];
	player->eraserpos.f[2] = eraserpos.f[2];

	stage = stageGetCurrent();

	player->eraserpropdist = stage->eraserpropdist;
	player->eraserbgdist = (float) stage->eraserpropdist + stage->unk30;

	vismax.f[0] = eraserpos.f[0] + player->eraserbgdist;
	vismax.f[1] = eraserpos.f[1] + player->eraserbgdist;
	vismax.f[2] = eraserpos.f[2] + player->eraserbgdist;

	vismin.f[0] = eraserpos.f[0] - player->eraserbgdist;
	vismin.f[1] = eraserpos.f[1] - player->eraserbgdist;
	vismin.f[2] = eraserpos.f[2] - player->eraserbgdist;

	vismid.f[0] = eraserpos.f[0];
	vismid.f[1] = eraserpos.f[1];
	vismid.f[2] = eraserpos.f[2];

	g_BgNumDrawSlots = 0;
	g_BgNumAttemptedDrawSlots = 0;

	g_BgDrawSlots[60].roomnum = -1;
	g_BgDrawSlots[60].draworder = 255;
	g_BgDrawSlots[60].box.xmin = xmin;
	g_BgDrawSlots[60].box.ymin = ymin;
	g_BgDrawSlots[60].box.xmax = xmax;
	g_BgDrawSlots[60].box.ymax = ymax;

	g_BgMaxDrawOrder = 0;
	g_BgMinDrawOrder = 0x7fff;

	for (i = 1; i < g_Vars.roomcount; i++) {
		if (!(vismax.f[0] < g_Rooms[i].bbmin[0]) && !(vismin.f[0] > g_Rooms[i].bbmax[0])
				&& !(vismax.f[2] < g_Rooms[i].bbmin[2]) && !(vismin.f[2] > g_Rooms[i].bbmax[2])
				&& !(vismax.f[1] < g_Rooms[i].bbmin[1]) && !(vismin.f[1] > g_Rooms[i].bbmax[1])) {
			int index = g_BgNumDrawSlots;

			if (xmin);
			if (g_Rooms[i].bbmax);

			if (index < 60) {
				float x;
				float y;
				float z;

				g_Rooms[i].flags |= ROOMFLAG_ONSCREEN;

				g_BgDrawSlots[index].roomnum = i;

				bgUnpausePropsInRoom(i, false);

				x = (g_Rooms[i].bbmin[0] + g_Rooms[i].bbmax[0]) / 2.0f - vismid.f[0];
				y = (g_Rooms[i].bbmin[1] + g_Rooms[i].bbmax[1]) / 2.0f - vismid.f[1];
				z = (g_Rooms[i].bbmin[2] + g_Rooms[i].bbmax[2]) / 2.0f - vismid.f[2];

				g_BgDrawSlots[index].draworder = sqrtf(x * x + y * y + z * z) / 100.0f;

				if (g_BgDrawSlots[index].draworder > g_BgMaxDrawOrder) {
					g_BgMaxDrawOrder = g_BgDrawSlots[index].draworder;
				}

				if (g_BgDrawSlots[index].draworder < g_BgMinDrawOrder) {
					g_BgMinDrawOrder = g_BgDrawSlots[index].draworder;
				}

				g_BgDrawSlots[index].box.xmin = xmin;
				g_BgDrawSlots[index].box.ymin = ymin;
				g_BgDrawSlots[index].box.xmax = xmax;
				g_BgDrawSlots[index].box.ymax = ymax;

				g_BgNumDrawSlots++;
				g_BgNumAttemptedDrawSlots++;

				g_Rooms[player->cam_room].flags |= ROOMFLAG_ONSCREEN;
			} else {
				// empty
			}
		}
	}

	bgChooseRoomsToLoad();
}

void bgAddToSnake(RoomNum fromroomnum, RoomNum roomnum, int16_t depth, struct screenbox *box)
{
	struct bgsnakeitem *item;
	int i;
	int j;

	if (g_Rooms[roomnum].flags & ROOMFLAG_DISABLEDBYSCRIPT) {
		return;
	}

	if (depth >= 2) {
		if (g_Rooms[roomnum].portalrecursioncount < 255) {
			g_Rooms[roomnum].portalrecursioncount++;
		}

		if (g_Rooms[roomnum].portalrecursioncount > g_Vars.roomportalrecursionlimit) {
			return;
		}
	}

	if (g_Rooms[roomnum].snakecount != 0 && g_Rooms[roomnum].unk07) {
		i = g_BgSnake.tailindex;
		item = &g_BgSnake.items[i];

		while (i != g_BgSnake.headindex) {
			if (item->roomnum == roomnum) {
				for (j = 0; j < ARRAYCOUNT(item->fromroomnums); j++) {
					if (item->fromroomnums[j] == -1) {
						bgExpandBox(&item->screenbox, box);
						item->fromroomnums[j] = fromroomnum;
						return;
					}
				}
			}

			i++;
			item++;

			if (i == ARRAYCOUNT(g_BgSnake.items)) {
				i = 0;
				item = &g_BgSnake.items[0];
			}
		}
	}

	item = &g_BgSnake.items[g_BgSnake.headindex];
	item->fromroomnums[0] = fromroomnum;
	item->roomnum = roomnum;
	item->depth = depth;
	item->roomportallistoffset = g_Rooms[roomnum].roomportallistoffset;
	item->numportals = g_Rooms[roomnum].numportals;

	item->screenbox.xmin = box->xmin;
	item->screenbox.ymin = box->ymin;
	item->screenbox.xmax = box->xmax;
	item->screenbox.ymax = box->ymax;

	g_Rooms[roomnum].snakecount++;

	for (i = 1; i < ARRAYCOUNT(item->fromroomnums); i++) {
		item->fromroomnums[i] = -1;
	}

	g_BgSnake.headindex++;

	if (g_BgSnake.headindex == 250) {
		g_BgSnake.headindex = 0;
	}

	if (g_BgSnake.headindex == g_BgSnake.tailindex) {
		g_BgSnake.headindex--;
	}
}

/**
 * Iterate the given item/room's portals and check if each of the neighbouring
 * rooms should be onscreen or not.
 *
 * Those that should be onscreen are added to the draw list and appended to the
 * snake so its neighbours will be processed recursively.
 */
void bgConsumeSnakeItem(struct bgsnakeitem *item)
{
	struct coord *campos;
	int i;
	int16_t portalnum;
	int16_t prevvalidcount;
	RoomNum prevfoundroom;
	RoomNum newfoundroom;
	int16_t side;
	RoomNum tmp;
	bool pass;
	struct portalmetric *metric;
	struct screenbox prevbox;
	struct screenbox newbox;
	float sum;

	g_Rooms[item->roomnum].snakecount--;
	g_BgSnake.count++;
	campos = &g_Vars.currentplayer->cam_pos;
	prevvalidcount = 0;
	prevfoundroom = -1;

	for (i = 0; i < item->numportals; i++) {
		portalnum = g_RoomPortals[item->roomportallistoffset + i];

		// Calculate which side of the portal the camera is on
		// if we haven't done it on this frame yet.
		if (g_PortalCameraCache[portalnum].updatedframe1 != g_BgFrameCount) {
			metric = &g_PortalMetrics[portalnum];

			sum = metric->normal.x * campos->f[0]
				+ metric->normal.y * campos->f[1]
				+ metric->normal.z * campos->f[2];

			if (sum < metric->min) {
				g_PortalCameraCache[portalnum].side = 1;
			} else if (sum > metric->max) {
				g_PortalCameraCache[portalnum].side = 0;
			} else {
				g_PortalCameraCache[portalnum].side = 2;
			}

			g_PortalCameraCache[portalnum].updatedframe1 = g_BgFrameCount;
		}

		// Swap the rooms if needed, or skip past this portal entirely if the
		// other room and camera room are on the same side.
		tmp = g_BgPortals[portalnum].roomnum1;
		side = g_PortalCameraCache[portalnum].side;

		if ((uint32_t)tmp == item->roomnum) {
			if (side == 0) {
				continue;
			}

			newfoundroom = g_BgPortals[portalnum].roomnum2;
		} else {
			if (side == 1) {
				continue;
			}

			newfoundroom = tmp;
		}

		// Avoid adding a room twice in a row, which would happen if there are
		// multiple portals between the same two rooms.
		if (prevfoundroom != newfoundroom) {
			if (prevvalidcount) {
				bgSetRoomOnscreen(prevfoundroom, item->depth, &prevbox);
				bgAddToSnake(item->roomnum, prevfoundroom, item->depth + 1, &prevbox);
			}

			prevvalidcount = 0;
			prevfoundroom = newfoundroom;
		}

		// Not sure why this check isn't done first?
		if (PORTAL_IS_CLOSED(portalnum)) {
			continue;
		}

		// If this room has already been processed in the reverse direction,
		// there's no need to add it again.
		if (newfoundroom == item->fromroomnums[0]
				|| newfoundroom == item->fromroomnums[1]
				|| newfoundroom == item->fromroomnums[2]
				|| newfoundroom == item->fromroomnums[3]
				|| newfoundroom == item->fromroomnums[4]) {
			continue;
		}

		// Reusing the side variable as a bboxisvalid variable
		if (g_BgPortals[portalnum].flags & PORTALFLAG_02) {
			newbox.xmin = item->screenbox.xmin;
			newbox.ymin = item->screenbox.ymin;
			newbox.xmax = item->screenbox.xmax;
			newbox.ymax = item->screenbox.ymax;
			side = true;
		} else {
			side = bgGetPortalScreenBbox(portalnum, &newbox);
		}

		if (side) {
			bgGetBoxIntersection(&newbox, &item->screenbox);

			if (newbox.xmin < newbox.xmax && newbox.ymin < newbox.ymax) {
				if (prevvalidcount == 0) {
					prevbox.xmin = newbox.xmin;
					prevbox.ymin = newbox.ymin;
					prevbox.xmax = newbox.xmax;
					prevbox.ymax = newbox.ymax;
				} else {
					bgExpandBox(&prevbox, &newbox);
				}

				prevvalidcount++;
			}
		}
	}

	if (prevvalidcount != 0) {
		bgSetRoomOnscreen(prevfoundroom, item->depth, &prevbox);
		bgAddToSnake(item->roomnum, prevfoundroom, item->depth + 1, &prevbox);
	}
}

/**
 * The "snake" is a circular array with a head index and tail index.
 * Items (rooms) are added to the head of the snake and consumed from the tail.
 * Consuming an item may cause more items to be added to the head.
 * Eventually the tail catches up to the head and the snake is finished.
 *
 * The structure is used for discovering onscreen rooms.
 */
bool bgTryConsumeSnake(void)
{
	if (g_BgSnake.tailindex == g_BgSnake.headindex) {
		return false;
	}

	bgConsumeSnakeItem(&g_BgSnake.items[g_BgSnake.tailindex]);

	g_BgSnake.tailindex++;

	if (g_BgSnake.tailindex == ARRAYCOUNT(g_BgSnake.items)) {
		g_BgSnake.tailindex = 0;
	}

	return true;
}

/**
 * Choose which rooms should be placed on standby and which are candidates for
 * loading.
 *
 * All offscreen neighbours of onscreen rooms are placed on standby and are
 * nominated for loading. Any tinted glass in these rooms will be unpaused.
 *
 * If the portal between the onscreen room and the offscreen room is blocked
 * (ie. a closed door or opaque glass) then nominate the neighbours of the
 * offscreen room for loading too. This is necessary because opening the door or
 * destroying the glass may make many rooms visible at once, and only one room
 * is loaded per tick.
 */
void bgChooseRoomsToLoad(void)
{
	int i;
	int j;

	g_BgNumRoomLoadCandidates = 0;

	for (i = 0; g_BgPortals[i].verticesoffset != 0; i++) {
		if ((g_BgPortals[i].flags & PORTALFLAG_SKIP) == 0) {
			int roomnum1 = g_BgPortals[i].roomnum1;
			int roomnum2 = g_BgPortals[i].roomnum2;
			int portalnum;

			if ((g_Rooms[roomnum1].flags & ROOMFLAG_ONSCREEN) && (g_Rooms[roomnum2].flags & ROOMFLAG_ONSCREEN) == 0) {
				// From room1 to room2
				g_Rooms[roomnum2].flags |= ROOMFLAG_STANDBY;

				if (g_Rooms[roomnum2].loaded240 == 0) {
					g_Rooms[roomnum2].flags |= ROOMFLAG_LOADCANDIDATE;
					g_BgNumRoomLoadCandidates++;
				}

				bgUnpausePropsInRoom(roomnum2, true);

				if (PORTAL_IS_CLOSED(i)) {
					for (j = 0; j < g_Rooms[roomnum2].numportals; j++) {
						portalnum = g_RoomPortals[g_Rooms[roomnum2].roomportallistoffset + j];

						if (roomnum2 == g_BgPortals[portalnum].roomnum1) {
							if (g_Rooms[g_BgPortals[portalnum].roomnum2].loaded240 == 0) {
								g_Rooms[g_BgPortals[portalnum].roomnum2].flags |= ROOMFLAG_LOADCANDIDATE;
								g_BgNumRoomLoadCandidates++;
							}
						} else {
							if (g_Rooms[g_BgPortals[portalnum].roomnum1].loaded240 == 0) {
								g_Rooms[g_BgPortals[portalnum].roomnum1].flags |= ROOMFLAG_LOADCANDIDATE;
								g_BgNumRoomLoadCandidates++;
							}
						}
					}
				}
			} else if ((g_Rooms[roomnum2].flags & ROOMFLAG_ONSCREEN)
					&& (g_Rooms[roomnum1].flags & ROOMFLAG_ONSCREEN) == 0) {
				// From room2 to room1
				g_Rooms[roomnum1].flags |= ROOMFLAG_STANDBY;

				if (g_Rooms[roomnum1].loaded240 == 0) {
					g_Rooms[roomnum1].flags |= ROOMFLAG_LOADCANDIDATE;
					g_BgNumRoomLoadCandidates++;
				}

				bgUnpausePropsInRoom(roomnum1, true);

				if (PORTAL_IS_CLOSED(i)) {
					for (j = 0; j < g_Rooms[roomnum1].numportals; j++) {
						portalnum = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + j];

						if (roomnum1 == g_BgPortals[portalnum].roomnum1) {
							if (g_Rooms[g_BgPortals[portalnum].roomnum1].loaded240 == 0) {
								g_Rooms[g_BgPortals[portalnum].roomnum1].flags |= ROOMFLAG_LOADCANDIDATE;
								g_BgNumRoomLoadCandidates++;
							}
						} else {
							if (g_Rooms[g_BgPortals[portalnum].roomnum1].loaded240 == 0) {
								g_Rooms[g_BgPortals[portalnum].roomnum2].flags |= ROOMFLAG_LOADCANDIDATE;
								g_BgNumRoomLoadCandidates++;
							}
						}
					}
				}
			}
		}
	}

	// Update visibility per player
	if (g_Vars.mplayerisrunning) {
		uint8_t flag1 = 0x01 << g_Vars.currentplayernum;
		uint8_t flag2 = 0x10 << g_Vars.currentplayernum;

		for (i = 0; i < g_Vars.roomcount; i++) {
			if (g_Rooms[i].flags & ROOMFLAG_ONSCREEN) {
				g_MpRoomVisibility[i] |= flag1;
			} else {
				g_MpRoomVisibility[i] &= ~flag1;
			}

			if (g_Rooms[i].flags & ROOMFLAG_STANDBY) {
				g_MpRoomVisibility[i] |= flag2;
			} else {
				g_MpRoomVisibility[i] &= ~flag2;
			}
		}
	}
}

void bgTickPortals(void)
{
	int i;
	int room;
	struct screenbox box;
	struct player *player = g_Vars.currentplayer;

	bgCalculateScreenProperties();

	box.xmin = player->screenxminf;
	box.ymin = player->screenyminf;
	box.xmax = player->screenxmaxf;
	box.ymax = player->screenymaxf;

	viGetZRange(&g_BgSnake.zrange);
	g_BgSnake.zrange.far = g_BgSnake.zrange.far / g_Vars.currentplayerstats->scale_bg2gfx;

	for (i = 0; i < g_Vars.roomcount; i++) {
		g_Rooms[i].flags &= ~(ROOMFLAG_DISABLEDBYSCRIPT | ROOMFLAG_ONSCREEN | ROOMFLAG_STANDBY | ROOMFLAG_LOADCANDIDATE);
		g_Rooms[i].portalrecursioncount = 0;
		g_Rooms[i].snakecount = 0;
		g_Rooms[i].unk07 = 1;
	}

	if (player->visionmode == VISIONMODE_XRAY) {
		bgTickPortalsXray();
	} else {
		if (g_BgNumAttemptedDrawSlots > g_BgMostAttemptedDrawSlots) {
			g_BgMostAttemptedDrawSlots = g_BgNumAttemptedDrawSlots;
		}

		g_BgNumDrawSlots = 0;
		g_BgNumAttemptedDrawSlots = 0;
		g_BgMaxDrawOrder = 0;
		g_BgMinDrawOrder = 32767;
		g_BgDrawSlots[60].roomnum = -1;
		g_BgDrawSlots[60].draworder = 255;
		g_BgSnake.count = 0;
		g_BgSnake.headindex = 0;
		g_BgSnake.tailindex = 0;
		g_BgRoomTestsDisabled = false;
		g_BgDrawSlots[60].box.xmin = box.xmin;
		g_BgDrawSlots[60].box.ymin = box.ymin;
		g_BgDrawSlots[60].box.xmax = box.xmax;
		g_BgDrawSlots[60].box.ymax = box.ymax;

		bgCmdExecute(g_BgCommands);

		if (!g_BgRoomTestsDisabled) {
			if (g_BgPortals[0].verticesoffset == 0) {
				for (room = 1; room < g_Vars.roomcount; room++) {
					if (bgRoomIntersectsScreenBox(room, &box)
							&& ((g_StageIndex != STAGEINDEX_INFILTRATION && g_StageIndex != STAGEINDEX_RESCUE && g_StageIndex != STAGEINDEX_ESCAPE) || room != 0xf)
							&& (g_StageIndex != STAGEINDEX_SKEDARRUINS || room != 0x02)
							&& ((g_StageIndex != STAGEINDEX_DEFECTION && g_StageIndex != STAGEINDEX_EXTRACTION) || room != 0x01)
							&& (g_StageIndex != STAGEINDEX_ATTACKSHIP || room != 0x71)) {
						bgSetRoomOnscreen(room, 0, &box);
					}
				}
			} else {
				bgSetRoomOnscreen(g_CamRoom, 0, &box);

				g_BgSnake.count = 0;
				g_BgSnake.headindex = 0;
				g_BgSnake.tailindex = 0;

				bgAddToSnake(g_CamRoom, g_CamRoom, 1, &box);

				while (bgTryConsumeSnake());
			}
		}

		bgChooseRoomsToLoad();
	}
}

Gfx *bgRenderSceneAndLoadCandidate(Gfx *gdl)
{
	gdl = bgRenderScene(gdl);
	gdl = bgScissorToViewport(gdl);

	if (g_Vars.currentplayerindex == 0) {
		g_BgLoadCandidateTimer240 -= g_Vars.lvupdate240;
	}

	if (g_BgLoadCandidateTimer240 < 0) {
		g_BgLoadCandidateTimer240 = 0;
	}

	// Consider loading one room by finding the load candidate that is closest to the player
	if (g_BgLoadCandidateTimer240 == 0 && g_NumRoomLoadsLeftThisFrame == 4 && g_Vars.tickmode == TICKMODE_NORMAL) {
		struct player *player = g_Vars.currentplayer;
		float value;
		struct coord dist;
		float bestvalue = MAXFLOAT;
		int bestroomnum = 0;
		float radius;

		if (g_BgNumRoomLoadCandidates) {
			for (int i = 1; i < g_Vars.roomcount; i++) {
				if (!g_Rooms[i].loaded240 && (g_Rooms[i].flags & ROOMFLAG_LOADCANDIDATE)) {
					dist.x = g_Vars.currentplayer->prop->pos.x - g_Rooms[i].centre.x;
					dist.y = g_Vars.currentplayer->prop->pos.y - g_Rooms[i].centre.y;
					dist.z = g_Vars.currentplayer->prop->pos.z - g_Rooms[i].centre.z;

					value = dist.f[0] * dist.f[0] + dist.f[1] * dist.f[1] + dist.f[2] * dist.f[2];

					radius = g_Rooms[i].radius;

					Mtx tmp;
					memcpy(&tmp, player->projectionmtx, sizeof(Mtx));

					if (g_CamFrustumViewOffset + radius < tmp[2][0] * g_Rooms[i].centre.f[0]
							+ tmp[2][1] * g_Rooms[i].centre.f[1]
							+ tmp[2][2] * g_Rooms[i].centre.f[2]) {
						value *= 3.0f;
					}

					if (g_CamFrustumLeftOffset + radius < g_CamFrustumLeftNormal.f[0] * g_Rooms[i].centre.f[0]
							+ g_CamFrustumLeftNormal.f[1] * g_Rooms[i].centre.f[1]
							+ g_CamFrustumLeftNormal.f[2] * g_Rooms[i].centre.f[2]) {
						value *= 1.5f;
					}

					if (g_CamFrustumRightOffset + radius < g_CamFrustumRightNormal.f[0] * g_Rooms[i].centre.f[0]
							+ g_CamFrustumRightNormal.f[1] * g_Rooms[i].centre.f[1]
							+ g_CamFrustumRightNormal.f[2] * g_Rooms[i].centre.f[2]) {
						value *= 1.5f;
					}

					if (g_CamFrustumTopOffset + radius < g_CamFrustumTopNormal.f[0] * g_Rooms[i].centre.f[0]
							+ g_CamFrustumTopNormal.f[1] * g_Rooms[i].centre.f[1]
							+ g_CamFrustumTopNormal.f[2] * g_Rooms[i].centre.f[2]) {
						value *= 2.0f;
					}

					if (g_CamFrustumBottomOffset + radius < g_CamFrustumBottomNormal.f[0] * g_Rooms[i].centre.f[0]
							+ g_CamFrustumBottomNormal.f[1] * g_Rooms[i].centre.f[1]
							+ g_CamFrustumBottomNormal.f[2] * g_Rooms[i].centre.f[2]) {
						value *= 2.0f;
					}

					if (value < bestvalue) {
						bestvalue = value;
						bestroomnum = i;
					}
				}
			}
		}

		if (bestroomnum != 0) {
			bgLoadRoom(bestroomnum);
			g_BgLoadCandidateTimer240 = 64;
		}
	}

	return gdl;
}

int bgGetForceOnscreenRooms(RoomNum *rooms, int len)
{
	int i;

	for (i = 0; i < g_BgNumForceOnscreenRooms && i < len; i++) {
		rooms[i] = g_BgForceOnscreenRooms[i];
	}

	rooms[i] = -1;

	return i;
}

int bgRoomGetNeighbours(int roomnum, RoomNum *dstrooms, int len)
{
	int count = 0;
	int i;
	int j;

	for (i = 0; i < g_Rooms[roomnum].numportals; i++) {
		int portalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		int neighbournum = g_BgPortals[portalnum].roomnum1;

		if (neighbournum == roomnum) {
			neighbournum = g_BgPortals[portalnum].roomnum2;
		}

		for (j = 0; j < count; j++) {
			if (dstrooms[j] == neighbournum) {
				goto end;
			}
		}

		dstrooms[count] = neighbournum;
		count++;

		if (count >= len) {
			break;
		}

end:
		;
	}

	dstrooms[count] = -1;

	return count;
}

bool bgRoomsAreNeighbours(int roomnum1, int roomnum2)
{
	int i;

	for (i = 0; i < g_Rooms[roomnum1].numportals; i++) {
		int portalnum = g_RoomPortals[g_Rooms[roomnum1].roomportallistoffset + i];

		if (g_BgPortals[portalnum].roomnum1 == roomnum2 || g_BgPortals[portalnum].roomnum2 == roomnum2) {
			return true;
		}
	}

	return false;
}

void bgCalculateScreenProperties(void)
{
	struct player *player = g_Vars.currentplayer;
	float width = viGetWidth();
	float height = viGetHeight();
	
	player->screenxminf = viGetViewLeft();

	if (player->screenxminf < 0) {
		player->screenxminf = 0;
	}

	if (player->screenxminf > width) {
		player->screenxminf = width;
	}

	player->screenyminf = viGetViewTop();

	if (player->screenyminf < 0) {
		player->screenyminf = 0;
	}

	if (player->screenyminf > height) {
		player->screenyminf = height;
	}

	player->screenxmaxf = viGetViewLeft() + viGetViewWidth();

	if (player->screenxmaxf < 0) {
		player->screenxmaxf = 0;
	}

	if (player->screenxmaxf > width) {
		player->screenxmaxf = width;
	}

	player->screenymaxf = viGetViewTop() + viGetViewHeight();

	if (player->screenymaxf < 0) {
		player->screenymaxf = 0;
	}

	if (player->screenymaxf > height) {
		player->screenymaxf = height;
	}
}

void bgExpandRoomToPortals(int roomnum)
{
	int count = 0;

	for (int i = 0; i < g_Rooms[roomnum].numportals; i++) {
		int portalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];
		struct portalvertices *pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum].verticesoffset);

		for (int j = 0; j < pvertices->count; j++) {
			for (int k = 0; k < 3; k++) {
				float value = pvertices->vertices[j].f[k];

				if (value < g_Rooms[roomnum].bbmin[k]) {
					g_Rooms[roomnum].bbmin[k] = value;
					count++;
				}

				if (value > g_Rooms[roomnum].bbmax[k]) {
					g_Rooms[roomnum].bbmax[k] = value;
					count++;
				}
			}
		}
	}
}

bool bgPortalExists(int portalnum)
{
	for (int i = 0; g_BgPortals[i].verticesoffset != 0; i++) {
		if (i == portalnum) {
			return true;
		}
	}

	return false;
}

void bgPortalSwapRooms(int portal)
{
	RoomNum tmp = g_BgPortals[portal].roomnum1;
	g_BgPortals[portal].roomnum1 = g_BgPortals[portal].roomnum2;
	g_BgPortals[portal].roomnum2 = tmp;
}

void bgInitPortal(int portalnum)
{
	struct coord room1centre;
	struct coord room2centre;
	float tmp;
	struct portalmetric sp28;
	struct portalmetric *ptr;
	float tmp1;
	float tmp2;
	bool sp18;
	int roomnum1;
	int roomnum2;

	roomnum1 = g_BgPortals[portalnum].roomnum1;
	roomnum2 = g_BgPortals[portalnum].roomnum2;

	room1centre.x = g_Rooms[roomnum1].centre.x;
	room1centre.y = g_Rooms[roomnum1].centre.y;
	room1centre.z = g_Rooms[roomnum1].centre.z;

	room2centre.x = g_Rooms[roomnum2].centre.x;
	room2centre.y = g_Rooms[roomnum2].centre.y;
	room2centre.z = g_Rooms[roomnum2].centre.z;

	ptr = &g_PortalMetrics[portalnum];
	sp28.normal.x = ptr->normal.x;
	sp28.normal.y = ptr->normal.y;
	sp28.normal.z = ptr->normal.z;
	sp28.min = ptr->min;
	sp28.max = ptr->max;

	tmp1 = sp28.normal.f[0] * room1centre.f[0] + sp28.normal.f[1] * room1centre.f[1] + sp28.normal.f[2] * room1centre.f[2];

	sp18 = 0;

	if (tmp1 > sp28.max) {
		sp18 = 1;

		bgPortalSwapRooms(portalnum);

		sp28.normal.x = -sp28.normal.x;
		sp28.normal.y = -sp28.normal.y;
		sp28.normal.z = -sp28.normal.z;

		tmp = sp28.min;
		sp28.min = -sp28.max;
		sp28.max = -tmp;
	}

	tmp2 = sp28.normal.f[0] * room2centre.f[0] + sp28.normal.f[1] * room2centre.f[1] + sp28.normal.f[2] * room2centre.f[2];

	if (tmp2 <= sp28.min && sp18) {
		bgPortalSwapRooms(portalnum);
	}
}

/**
 * Figure out if the room has complicated portals and set a flag on the room if so.
 *
 * Most rooms use a simple portal setup where the portals (doors) exist along the
 * bounding box of the room. The game engine can easily test if a position is
 * inside the room by checking which side of the portal's plane it's on.
 *
 * A complicated portal setup happens when you have an L-shaped room and there's
 * a portal (door) on the inside wall of the L. It means you can be inside the
 * room but your position is on the opposite side of the door's plane.
 */
void bgInitRoom(int roomnum)
{
	struct portalvertices *pvertices;
	struct portalmetric metric;
	int16_t portalnum;
	int16_t portalnum2;
	float tmp;

	for (int i = 0; i < g_Rooms[roomnum].numportals; i++) {
		portalnum = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + i];

		metric.normal.f[0] = (g_PortalMetrics + portalnum)->normal.f[0];
		metric.normal.f[1] = (g_PortalMetrics + portalnum)->normal.f[1];
		metric.normal.f[2] = (g_PortalMetrics + portalnum)->normal.f[2];
		metric.min = (g_PortalMetrics + portalnum)->min;
		metric.max = (g_PortalMetrics + portalnum)->max;

		if (roomnum == g_BgPortals[portalnum].roomnum1) {
			metric.normal.f[0] = -metric.normal.f[0];
			metric.normal.f[1] = -metric.normal.f[1];
			metric.normal.f[2] = -metric.normal.f[2];

			tmp = metric.min;
			metric.min = -metric.max;
			metric.max = -tmp;
		}

		for (int j = 0; j < g_Rooms[roomnum].numportals; j++) {
			portalnum2 = g_RoomPortals[g_Rooms[roomnum].roomportallistoffset + j];

			if (portalnum2 == portalnum) {
				continue;
			}

			pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum2].verticesoffset);

			for (int k = 0; k < pvertices->count; k++) {
				tmp = metric.normal.f[0] * pvertices->vertices[k].f[0]
					+ metric.normal.f[1] * pvertices->vertices[k].f[1]
					+ metric.normal.f[2] * pvertices->vertices[k].f[2];

				if (tmp < metric.min) {
					g_Rooms[roomnum].flags |= ROOMFLAG_COMPLICATEDPORTALS;
					return;
				}
			}
		}
	}
}

void bgSetPortalOpenState(int portal, bool open)
{
	g_BgPortals[portal].flags = (g_BgPortals[portal].flags | PORTALFLAG_CLOSED) ^ (open != false);
}

float g_PortalMidplaneOffset = 0.0f;

int bgFindPortalBetweenPositions(struct coord *pos1, struct coord *pos2)
{
	int bestportalnum = -1;
	int count = 0;
	float bestthing = MAXFLOAT;
	float thisthing;

	for (int i = 0; g_BgPortals[i].verticesoffset; i++) {
		if (portalCalculateIntersection(i, pos1, pos2) != PORTALINTERSECTION_NONE) {
			thisthing = g_PortalMidplaneOffset;

			if (thisthing < 0) {
				thisthing = -thisthing;
			}

			if (thisthing < bestthing) {
				bestportalnum = i;
				bestthing = thisthing;
				count++;
			}
		}
	}

	return bestportalnum;
}

bool bgIsBboxOverlapping(struct coord *portalbbmin, struct coord *portalbbmax, struct coord *propbbmin, struct coord *propbbmax)
{
	for (int i = 0; i < 3; i++) {
		if (propbbmin->f[i] > portalbbmax->f[i] || propbbmax->f[i] < portalbbmin->f[i]) {
			return false;
		}
	}

	return true;
}

void bgCalculatePortalBbox(int portalnum, struct coord *bbmin, struct coord *bbmax)
{
	struct portalvertices *pvertices;

	bbmin->x = MAXFLOAT;
	bbmin->y = MAXFLOAT;
	bbmin->z = MAXFLOAT;

	bbmax->x = MINFLOAT;
	bbmax->y = MINFLOAT;
	bbmax->z = MINFLOAT;

	pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum].verticesoffset);

	for (int i = 0; i < pvertices->count; i++) {
		for (int j = 0; j < 3; j++) {
			float value = pvertices->vertices[i].f[j];

			if (value < bbmin->f[j]) {
				bbmin->f[j] = value;
			}

			if (value > bbmax->f[j]) {
				bbmax->f[j] = value;
			}
		}
	}
}

void bgFindEnteredRooms(struct coord *bbmin, struct coord *bbmax, RoomNum *rooms, int maxlen, bool arg4)
{
	RoomNum room;
	RoomNum otherroom;
	int portalnum;
	struct coord propbbmin;
	struct coord propbbmax;
	int len;
	int i;
	int j;
	int k;
	int origlen;
	struct coord portalbbmin;
	struct coord portalbbmax;

	propbbmin.x = bbmin->x;
	propbbmin.y = bbmin->y;
	propbbmin.z = bbmin->z;

	propbbmax.x = bbmax->x;
	propbbmax.y = bbmax->y;
	propbbmax.z = bbmax->z;

	i = 0;

	for (len = 0; rooms[len] != -1; len++);

	while (true) {
		origlen = len;

		for (; i < origlen; i++) {
			room = rooms[i];

			for (j = 0; j < g_Rooms[room].numportals; j++) {
				portalnum = g_RoomPortals[g_Rooms[room].roomportallistoffset + j];

				if (arg4 && PORTAL_IS_CLOSED(portalnum)) {
					continue;
				}

				bgCalculatePortalBbox(portalnum, &portalbbmin, &portalbbmax);

				if (bgIsBboxOverlapping(&portalbbmin, &portalbbmax, &propbbmin, &propbbmax)) {
					if (room == g_BgPortals[portalnum].roomnum1) {
						otherroom = g_BgPortals[portalnum].roomnum2;
					} else {
						otherroom = g_BgPortals[portalnum].roomnum1;
					}

					for (k = 0; k < len; k++) {
						if (rooms[k] == otherroom) {
							break;
						}
					}

					if (k == len) {
						if (len < maxlen) {
							rooms[len] = otherroom;
							len++;
						}

						if (len >= maxlen) {
							goto end;
						}
					}
				}
			}
		}

		if (len == origlen) {
			break;
		}
	}

end:
	rooms[len] = -1;
}

void bgCalculateGlaresForVisibleRooms(void)
{
	g_NumRoomsWithGlares = 0;

	if (!g_Vars.mplayerisrunning) {
		for (int i = 1; i < g_Vars.roomcount; i++) {
			if (g_Rooms[i].flags & ROOMFLAG_ONSCREEN) {
				artifactsCalculateGlaresForRoom(i);
				if (g_NumRoomsWithGlares < 100) {
					g_GlareRooms[g_NumRoomsWithGlares++] = i;
				}
			}
		}
	}
}
