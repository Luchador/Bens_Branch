#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/dyntex.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

/**
 * dyntex - dynamic textures
 *
 * This file handles textures which animate automatically, such as water.
 *
 * The dyntex system maintains three conceptually nested arrays: rooms, types
 * and vertices.
 *
 * Rooms are the first tier. Rooms will only exist in the array if they contain
 * animated textures. Rooms contain types.
 *
 * Types are a type of animation. Linear is the most common, but there's also
 * ocean waves, and some specific types such as Attack Ship triangular arrows.
 * Types contain vertices.
 *
 * Vertices contain an offset to the graphics vertex, as well as a copy of its
 * S and T values.
 *
 * The caller should call dyntexSetCurrentRoom and dyntexSetCurrentType, then
 * add vertices with dyntexAddVertex. Lastly, dyntexTickRoom should be called
 * on each tick for each nearby room. dyntexTickRoom can be called multiple
 * times on the same frame (such as if there's two players), as dyntex will
 * ensure it's only updated once per frame.
 *
 * Data is added to dyntex during gameplay as rooms are loaded. When a room is
 * unloaded the data remains in the dyntex arrays. When a room is loaded again
 * dyntex will not add it a second time.
 */

struct dyntexroom {
	uint16_t roomnum;
	uint16_t typelistoffset;
	uint16_t numtypes;
	int updatedframe;
};

struct dyntextype {
	uint16_t type : 7;
	uint16_t initialised : 1;
	int8_t numvertices;
	uint16_t vertexlistoffset;
};

struct dyntexvtx {
	uint16_t offset;
	int16_t s;
	int16_t t;
};

int g_DyntexVerticesMax;
int g_DyntexTypesMax;
int g_DyntexRoomsMax;
struct dyntexvtx *g_DyntexVertices;
struct dyntextype *g_DyntexTypes;
struct dyntexroom *g_DyntexRooms;

int g_DyntexCurRoom = -1;
int g_DyntexCurType = -1;
bool g_DyntexRoomPopulated = false;
bool g_DyntexTypePopulated = false;
int g_DyntexRoomsCount = 0;
int g_DyntexTypesCount = 0;
int g_DyntexVerticesCount = 0;

void dyntexUpdateLinear(Vtx *vertices, struct dyntextype *type)
{
	// Old: int16_t tmp = (int) (g_Lv80SecIntervalFrac * 10.0f * 4096.0f) % 4096;
    int16_t tmp = (int) (g_Lv80SecIntervalFrac * (g_StageIndex == STAGEINDEX_AIRBASE ? 1.0f : 10.0f ) * 4096.0f) % 4096; //Make water in Air Base scroll more slowly
	int i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t + tmp;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s;
	}
}

void dyntexUpdateReset(Vtx *vertices, struct dyntextype *type)
{
	int i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->s = 0;
		vertex->t = 0;
	}
}

void dyntexUpdateMonitor(Vtx *vertices, struct dyntextype *type)
{
	int16_t tmp = (int) (g_Lv80SecIntervalFrac * 4.0f * 4096.0f) % 4096;
	int i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t - tmp;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s;
	}
}

void dyntexUpdateOcean(Vtx *vertices, struct dyntextype *type)
{
	float f24 = g_Lv80SecIntervalFrac * 5.0f;
	float angle;
	int i;

	static uint32_t ripsize = 65;
	static uint32_t modula = 22;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		angle = ((g_DyntexVertices[type->vertexlistoffset + i].t % modula) / (float) modula + f24) * M_TAU;
		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t + (int16_t) (sinf(angle) * ripsize);

		angle = (((g_DyntexVertices[type->vertexlistoffset + i].s + 22) % modula) / (float) modula + f24) * M_TAU;
		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s + (int16_t) (cosf(angle) * ripsize);
	}
}

void dyntexUpdateArrows(Vtx *vertices, struct dyntextype *type)
{
	int tmp = ((int) ((1.0f - g_Lv80SecIntervalFrac) * 60.0f * 8.0f) % 8) * 256;
	int i;

	for (i = 0; i < type->numvertices; i++) {
		Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + i].offset);

		vertex->s = g_DyntexVertices[type->vertexlistoffset + i].s + tmp;
		vertex->t = g_DyntexVertices[type->vertexlistoffset + i].t;
	}
}

void dyntexTickRoom(int roomnum, Vtx *vertices)
{
	int index = -1;
	int i;
	int j;

	for (i = 0; i < g_DyntexRoomsCount; i++) {
		if (g_DyntexRooms[i].roomnum == roomnum) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		return;
	}

	if (g_Vars.lvframenum == g_DyntexRooms[index].updatedframe) {
		return;
	}

	for (i = 0; i < g_DyntexRooms[index].numtypes; i++) {
		struct dyntextype *type = &g_DyntexTypes[g_DyntexRooms[index].typelistoffset + i];
		int mins = 32767;
		int maxs = -32766;
		int mint = 32767;
		int maxt = -32766;

		if (!type->initialised) {
			int adds = 0;
			int addt = 0;

			// @bug: Using i for both outer and inner loops
			// Ben's change: fixing this bug, although I think in practice it happened to be harmless in the original PD.
			// What it would have done is, if there were multiple types of dynamic textures in one room, only one would animate.
			for (j = 0; j < type->numvertices; j++) {
				Vtx *vertex = (Vtx *)((uintptr_t)vertices + g_DyntexVertices[type->vertexlistoffset + j].offset);

				g_DyntexVertices[type->vertexlistoffset + j].s = vertex->s;
				g_DyntexVertices[type->vertexlistoffset + j].t = vertex->t;

				if (vertex->s < mins) {
					mins = vertex->s;
				}

				if (vertex->t < mint) {
					mint = vertex->t;
				}

				if (vertex->s > maxs) {
					maxs = vertex->s;
				}

				if (vertex->t > maxt) {
					maxt = vertex->t;
				}
			}

			type->initialised = true;

			if (mins < -0x5d00) {
				adds = 0x2000;
			}

			if (mint < -0x5d00) {
				addt = 0x2000;
			}

			if (maxs > 0x5d00) {
				adds = -0x2000;
			}

			if (maxt > 0x5d00) {
				addt = -0x2000;
			}

			if (adds || addt) {
				for (i = 0; i < type->numvertices; i++) {
					g_DyntexVertices[type->vertexlistoffset + i].s += adds;
					g_DyntexVertices[type->vertexlistoffset + i].t += addt;
				}
			}
		}

		switch (g_DyntexTypes[g_DyntexRooms[index].typelistoffset + i].type) {
		case DYNTEXTYPE_RIVER:
			dyntexUpdateLinear(vertices, type);
			break;
		case DYNTEXTYPE_MONITOR:
			dyntexUpdateMonitor(vertices, type);
			break;
		case DYNTEXTYPE_OCEAN:
			dyntexUpdateOcean(vertices, type);
			break;
		case DYNTEXTYPE_ARROWS:
			dyntexUpdateArrows(vertices, type);
			break;
		case DYNTEXTYPE_TELEPORTAL:
			// Deep Sea - teleports enabled and not SA disabled
			if (chrHasStageFlag(0, 0x00000100) && !chrHasStageFlag(0, 0x00010000)) {
				dyntexUpdateLinear(vertices, type);
			}
			break;
		case DYNTEXTYPE_POWERRING:
			if (chrHasStageFlag(0, 0x00010000)) {
				// Attack Ship engines are destroyed
				dyntexUpdateReset(vertices, type);
			} else {
				// Attack Ship engines are healthy
				dyntexUpdateLinear(vertices, type);
			}
			break;
		case DYNTEXTYPE_POWERJUICE:
			if (!chrHasStageFlag(0, 0x00010000)) {
				// Attack Ship engines are healthy
				dyntexUpdateLinear(vertices, type);
			}
			break;
		}
	}

	g_DyntexRooms[index].updatedframe = g_Vars.lvframenum;
}

void dyntexAddVertex(Vtx *vertex)
{
	if (g_DyntexCurRoom < 0) {
		return;
	}

	if (g_DyntexVerticesCount == g_DyntexVerticesMax) {
		return;
	}

	if (!g_DyntexRoomPopulated) {
		if (g_DyntexTypesCount >= g_DyntexTypesMax || g_DyntexRoomsCount >= g_DyntexRoomsMax) {
			return;
		}

		g_DyntexRooms[g_DyntexRoomsCount].roomnum = g_DyntexCurRoom;
		g_DyntexRooms[g_DyntexRoomsCount].typelistoffset = g_DyntexTypesCount;
		g_DyntexRooms[g_DyntexRoomsCount].numtypes = 0;
		g_DyntexRooms[g_DyntexRoomsCount].updatedframe = 0;

		g_Rooms[g_DyntexCurRoom].flags |= ROOMFLAG_HASDYNTEX;

		g_DyntexRoomsCount++;
		g_DyntexRoomPopulated = true;
	}

	if (!g_DyntexTypePopulated) {
		if (g_DyntexTypesCount >= g_DyntexTypesMax) {
			return;
		}

		g_DyntexTypes[g_DyntexTypesCount].type = g_DyntexCurType;
		g_DyntexTypes[g_DyntexTypesCount].initialised = false;
		g_DyntexTypes[g_DyntexTypesCount].numvertices = 0;
		g_DyntexTypes[g_DyntexTypesCount].vertexlistoffset = g_DyntexVerticesCount;
		g_DyntexTypesCount++;

		g_DyntexRooms[g_DyntexRoomsCount - 1].numtypes++;
		g_DyntexTypePopulated = true;
	}

	g_DyntexVertices[g_DyntexVerticesCount].offset = (uint16_t)vertex;
	g_DyntexVerticesCount++;

	g_DyntexTypes[g_DyntexTypesCount - 1].numvertices++;
}

void dyntexSetCurrentType(int16_t type)
{
	// Investigation - don't animate the puddle of water behind the glass
	if (g_StageIndex == STAGEINDEX_INVESTIGATION && type == DYNTEXTYPE_RIVER) {
		return;
	}

	// Villa - don't animate shallow water
	if (g_StageIndex == STAGEINDEX_VILLA && type == DYNTEXTYPE_RIVER) {
		return;
	}

	// Power juice and power rings exist on Deep Sea and Attack Ship.
	//
	// Deep Sea - in the SA megaweapon
	// Attack Ship - in the engine's power node
	//
	// These both use a linear animation, but Attack Ship's are conditional on
	// the ship's engines running. To avoid doing a stage check on every tick,
	// Deep Sea's are retyped to the river type which uses an unconditional
	// linear animation. Attack Ship's remains as is.
	if (g_StageIndex != STAGEINDEX_ATTACKSHIP && (type == DYNTEXTYPE_POWERJUICE || type == DYNTEXTYPE_POWERRING)) {
		type = DYNTEXTYPE_RIVER;
	}

	if (type != g_DyntexCurType) {
		g_DyntexTypePopulated = false;
	}

	g_DyntexCurType = type;
}

void dyntexSetCurrentRoom(RoomNum roomnum)
{
	int i;

	if (roomnum >= 0) {
		for (i = 0; i < g_DyntexRoomsCount; i++) {
			if (g_DyntexRooms[i].roomnum == roomnum) {
				return;
			}
		}
	}

	if (g_DyntexRooms != NULL) {
		g_DyntexRoomPopulated = false;
		g_DyntexTypePopulated = false;
		g_DyntexCurRoom = roomnum;
		g_DyntexCurType = -1;
	}
}

void dyntexReset(void)
{
	uint32_t size3;
	uint32_t size2;
	uint32_t size1;

	g_DyntexCurRoom = -1;
	g_DyntexCurType = -1;
	g_DyntexRoomPopulated = false;
	g_DyntexRoomsCount = 0;
	g_DyntexTypesCount = 0;
	g_DyntexVerticesCount = 0;

	g_DyntexVerticesMax = 1200;
	g_DyntexTypesMax = 50;
	g_DyntexRoomsMax = 50;

	size1 = ALIGN64(g_DyntexTypesMax * sizeof(struct dyntextype));
	g_DyntexTypes = mempAlloc(size1, MEMPOOL_STAGE);

	size2 = ALIGN64(g_DyntexVerticesMax * sizeof(struct dyntexvtx));
	g_DyntexVertices = mempAlloc(size2, MEMPOOL_STAGE);

	size3 = ALIGN64(g_DyntexRoomsMax * sizeof(struct dyntexroom));
	g_DyntexRooms = mempAlloc(size3, MEMPOOL_STAGE);
}

bool dyntexHasRoom(void)
{
	return g_DyntexCurRoom >= 0;
}
