#include <ultra64.h>
#include "constants.h"
#include "game/room.h"
#include "game/mtxutils.h"
#include "bss.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

/**
 * Room matrices
 *
 * Each room that is rendered needs to have a modelview matrix. The matrix can
 * be pushed onto the displaylist multiple times per frame, even in single player.
 *
 * A simple approach would be to allocate these matrices out of graphics memory
 * as needed, populating and converting them to the RSP's matrix format each time.
 *
 * The solution used here keeps a cache of these matrices. This allows the same
 * matrix to be reused instead of having to allocate, calculate and convert them
 * each time which saves CPU time and memory. However, the game has to spend
 * CPU time keeping track of cache usage so it can free slots, and the cache
 * allocation uses more total memory than on-demand allocations would require,
 * so this system is perhaps not as beneficial as it seems.
 *
 * The cache is implemented using 5 parallel arrays:
 *
 * g_RoomMtxAges
 *  - The number of frames since each cache slot was last used.
 *  - When it reaches 2 the slot is freed.
 * g_RoomMtxLinkedRooms
 *  - The linked room number per cache slot, or -1 if free.
 * g_RoomMtxBaseRooms
 *  - The room that each matrix's position is relative to.
 *  - This is the player's room, as the world is drawn relative to this.
 * g_RoomMtxScales
 *  - The scale that is applied to the matrix. Usually 1.
 * g_RoomMtxMatrices
 *  - The matrices themselves, in the RSP's matrix format.
 *
 * Each room can have at most one matrix slot. When multiplayer is in use, the
 * same room matrix can be used for both players, but only if they have the same
 * global draw offset (ie. are in the same room).
 *
 * When players are not in the same room but can see the same rooms, each
 * player's matrix cache invalidates the other's until they move into the same
 * room or stop looking at each other's rooms.
 */

uint8_t *g_RoomMtxAges;
RoomNum *g_RoomMtxLinkedRooms;
RoomNum *g_RoomMtxBaseRooms;
float *g_RoomMtxScales;
Mtxf *g_RoomMtxMatrices;

int g_RoomMtxNumSlots = 0;

void roomSetLastForOffset(int room)
{
	g_Vars.currentplayer->lastroomforoffset = room;
}

void roomLinkMtx(int index, int roomnum)
{
	g_Rooms[roomnum].roommtxindex = index;
	g_RoomMtxLinkedRooms[index] = roomnum;
}

void roomUnlinkMtx(int index, int roomnum)
{
	g_Rooms[roomnum].roommtxindex = -1;
	g_RoomMtxLinkedRooms[index] = -1;
}

void roomFreeMtx(int index)
{
	if (g_RoomMtxLinkedRooms[index] != -1) {
		roomUnlinkMtx(index, g_RoomMtxLinkedRooms[index]);
	}

	g_RoomMtxAges[index] = NUM_GFXTASKS;
	g_RoomMtxBaseRooms[index] = -1;
	g_RoomMtxScales[index] = 1;
}

int roomAllocateMtx(void)
{
	int i;

	for (i = 0; i < g_RoomMtxNumSlots; i++) {
		if (g_RoomMtxAges[i] >= NUM_GFXTASKS && g_RoomMtxBaseRooms[i] == -1) {
			return i;
		}
	}

	return 0;
}

void roomPopulateMtx(Mtxf *mtx, int roomnum)
{
	int stagenum = g_Vars.stagenum;

	mtx4LoadIdentity(mtx);

	mtx->m[0][0] = 1;
	mtx->m[1][1] = 1;
	mtx->m[2][2] = 1;

	// These are rooms that are always active, such as the moon in Defection.
	// This is probably making those rooms always drawn a certain distance away
	// relative to the camera, so the moon never gets bigger as you go closer.
	if (((stagenum == g_Stages[STAGEINDEX_INFILTRATION].id
					|| stagenum == g_Stages[STAGEINDEX_RESCUE].id
					|| stagenum == g_Stages[STAGEINDEX_ESCAPE].id
					|| stagenum == g_Stages[STAGEINDEX_MAIANSOS].id) && roomnum == 0x0f)
			|| ((stagenum == g_Stages[STAGEINDEX_SKEDARRUINS].id
					|| stagenum == g_Stages[STAGEINDEX_WAR].id) && roomnum == 0x02)
			|| ((stagenum == g_Stages[STAGEINDEX_DEFECTION].id
					|| stagenum == g_Stages[STAGEINDEX_EXTRACTION].id
					|| stagenum == g_Stages[STAGEINDEX_MBR].id) && roomnum == 0x01)
			|| (stagenum == g_Stages[STAGEINDEX_ATTACKSHIP].id && roomnum == 0x71)) {
		mtx->m[3][0] = g_BgRooms[roomnum].pos.x;
		mtx->m[3][1] = g_BgRooms[roomnum].pos.y;
		mtx->m[3][2] = g_BgRooms[roomnum].pos.z;
	} else {
		mtx->m[3][0] = g_BgRooms[roomnum].pos.x - g_Vars.currentplayer->globaldrawworldoffset.x;
		mtx->m[3][1] = g_BgRooms[roomnum].pos.y - g_Vars.currentplayer->globaldrawworldoffset.y;
		mtx->m[3][2] = g_BgRooms[roomnum].pos.z - g_Vars.currentplayer->globaldrawworldoffset.z;
	}
}

/**
 * "Touch" a room's modelview matrix in the cache.
 *
 * The function is named after Unix's touch command, which creates a file if
 * missing and updates the modification time. The function creates the cache
 * entry if missing and resets the cache entry's age to 0.
 */
int roomTouchMtx(int roomnum)
{
	int index = g_Rooms[roomnum].roommtxindex;
	Mtxf mtx;

	if (index == -1
			|| g_Vars.currentplayer->lastroomforoffset != g_RoomMtxBaseRooms[index]) {
		// There's no cache for this room or it's invalid.
		// Unlink the old cache item if any and create a new one.
		if (index != -1) {
			roomUnlinkMtx(index, roomnum);
		}

		index = roomAllocateMtx();

		roomLinkMtx(index, roomnum);
		g_RoomMtxAges[index] = 0;
	} else {
		// The room has an existing, valid cache entry.
		g_RoomMtxAges[index] = 0;
		return index;
	}

	g_RoomMtxBaseRooms[index] = g_Vars.currentplayer->lastroomforoffset;

	roomPopulateMtx(&mtx, roomnum);
	mtxF2L(&mtx, &g_RoomMtxMatrices[index]);

	return index;
}

/**
 * Retrieve a room's modelview matrix from cache, or create a new one and cache
 * it, and apply it to the displaylist.
 */
Gfx *roomApplyMtx(Gfx *gdl, int roomnum)
{
	int index = roomTouchMtx(roomnum);

	gSPMatrix(gdl++, &g_RoomMtxMatrices[index], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

	return gdl;
}

struct coord *roomGetPosPtr(int room)
{
	return &g_BgRooms[room].pos;
}

void roomGetPos(int room, struct coord *pos)
{
	pos->x = g_BgRooms[room].pos.x;
	pos->y = g_BgRooms[room].pos.y;
	pos->z = g_BgRooms[room].pos.z;
}
