#include <ultra64.h>
#include <math.h>
#include <stdint.h>
#include "constants.h"
#include "game/padhalllv.h"
#include "bss.h"
#include "lib/lib_17ce0.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

uint8_t g_PortalTraceCounter = 254;

uint8_t g_PortalIntersectionCache[456][2]; // Cache to avoid recalculating expensive intersection logic for each portal multiple times within the same trace pass

void portalGetAvgVertexPos(int portalnum, struct coord *avg)
{
	struct portalvertices *pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum].verticesoffset);
	float f0;
	int i;

	avg->x = pvertices->vertices[0].x;
	avg->y = pvertices->vertices[0].y;
	avg->z = pvertices->vertices[0].z;

	f0 = 1.0f / pvertices->count;

	for (i = 1; i < pvertices->count; i++) {
		avg->x += pvertices->vertices[i].x;
		avg->y += pvertices->vertices[i].y;
		avg->z += pvertices->vertices[i].z;
	}

	avg->x *= f0;
	avg->y *= f0;
	avg->z *= f0;
}

/**
 * Add roomnum to the rooms list, provided it's not already there and there's
 * space available at the end of the list.
 *
 * The list is assumed to have 16 slots, with the last being reserved for the
 * -1 terminator.
 */
void portalTryAppendRoom(RoomNum *rooms, RoomNum roomnum)
{
	int i;

	for (i = 0; i < 16 && rooms[i] != -1; i++) {
		if (rooms[i] == roomnum) {
			return;
		}
	}

	if (i < 15) {
		rooms[i] = roomnum;
		rooms[i + 1] = -1;
	}
}

/**
 * Determine whether a line from pos1 to pos2 intersects the portal,
 * and if so then in which direction.
 *
 * Return one of:
 * PORTALINTERSECTION_NONE
 * PORTALINTERSECTION_BEHINDTOFRONT
 * PORTALINTERSECTION_FRONTTOBEHIND
 *
 * The portal's normal vector is the front.
 */
int portalCalculateIntersection(int portalnum, struct coord *pos1, struct coord *pos2)
{
	int i;
	struct coord sp60;
	struct portalvertices *pvertices;
	struct coord *curr;
	struct coord *next;
	struct coord sp48;
	uint8_t lastside;
	float sp40[1];
	struct coord sp34;
	float value1;
	float value2;
	float tmp;

	lastside = 0;
	pvertices = (struct portalvertices *)((uintptr_t)g_BgPortals + g_BgPortals[portalnum].verticesoffset);
	value1 = pos1->f[0] * (g_PortalMetrics + portalnum)->normal.f[0] + pos1->f[1] * (g_PortalMetrics + portalnum)->normal.f[1] + pos1->f[2] * (g_PortalMetrics + portalnum)->normal.f[2];
	value2 = pos2->f[0] * (g_PortalMetrics + portalnum)->normal.f[0] + pos2->f[1] * (g_PortalMetrics + portalnum)->normal.f[1] + pos2->f[2] * (g_PortalMetrics + portalnum)->normal.f[2];

	if (value1 < (g_PortalMetrics + portalnum)->min) {
		if (value2 < (g_PortalMetrics + portalnum)->min) {
			return PORTALINTERSECTION_NONE;
		}
	} else if ((g_PortalMetrics + portalnum)->max < value1 && (g_PortalMetrics + portalnum)->max < value2) {
		return PORTALINTERSECTION_NONE;
	}

	sp60.f[0] = pos2->f[0] - pos1->f[0];
	sp60.f[1] = pos2->f[1] - pos1->f[1];
	sp60.f[2] = pos2->f[2] - pos1->f[2];

	// Tells us how far along the viewing ray the portal lies
	g_PortalMidplaneOffset = (value1 + value2) * 0.5f - (g_PortalMetrics + portalnum)->min;

	curr = &pvertices->vertices[0];
	next = &pvertices->vertices[1];

	for (i = 0; i < pvertices->count; i++) {
		if (i + 1 == pvertices->count) {
			next = &pvertices->vertices[0];
		}

		sp48.f[0] = next->f[0] - curr->f[0];
		sp48.f[1] = next->f[1] - curr->f[1];
		sp48.f[2] = next->f[2] - curr->f[2];

		sp34.f[0] = sp48.f[1] * sp60.f[2] - sp48.f[2] * sp60.f[1];
		sp34.f[1] = sp48.f[2] * sp60.f[0] - sp48.f[0] * sp60.f[2];
		sp34.f[2] = sp48.f[0] * sp60.f[1] - sp48.f[1] * sp60.f[0];

		tmp = sp34.f[0] * sp34.f[0] + sp34.f[1] * sp34.f[1] + sp34.f[2] * sp34.f[2];

		if (tmp == 0.0f) {
			return PORTALINTERSECTION_NONE;
		}

		sp40[0] = sp34.f[0] * curr->f[0] + sp34.f[1] * curr->f[1] + sp34.f[2] * curr->f[2];
		tmp = sp34.f[0] * pos1->f[0] + sp34.f[1] * pos1->f[1] + sp34.f[2] * pos1->f[2];

		if (tmp < sp40[0]) {
			if (lastside == 2) {
				return PORTALINTERSECTION_NONE;
			}

			lastside = 1;
		} else {
			if (lastside == 1) {
				return PORTALINTERSECTION_NONE;
			}

			lastside = 2;
		}

		curr++;
		next++;
	}

	return (value1 < (g_PortalMetrics + portalnum)->min)
		? PORTALINTERSECTION_BEHINDTOFRONT
		: PORTALINTERSECTION_FRONTTOBEHIND;
}

// Runs a trace from startPos to endPos and fills outputRooms with all the rooms the line crosses. Has an option to return another list of all rooms visited along the way.
void portalTraceLineThroughRooms(struct coord *startPos, struct coord *endPos, RoomNum *startRooms, RoomNum *outputRooms, RoomNum *allVisitedRooms, int maxVisitedRooms)
{
    int i, j;
    int numPortals;
    int portalIndex;
    RoomNum room;
    int16_t *portalList;

    RoomNum currentWave[16];
    RoomNum newWave[16];
    RoomNum visitedRooms[16];

    // Initialize the starting room wave and visited list
    for (i = 0; i < 8; i++) {
        currentWave[i] = startRooms[i];
        visitedRooms[i] = startRooms[i];
        if (startRooms[i] == -1) break;
    }

    // Advance the portal trace counter
    g_PortalTraceCounter++;
    if (g_PortalTraceCounter == 255) {
        for (i = 0; i < g_BgNumPortalCameraCacheItems; i++) {
            g_PortalIntersectionCache[i][0] = 0xff; // Reset cache
        }
        g_PortalTraceCounter = 0;
    }

    // Flood traversal through portals
    do {
        newWave[0] = -1; // Clear next wave

        for (j = 0; (room = currentWave[j]) != -1 && j < 16; j++) {
            numPortals = g_Rooms[room].numportals;
            portalList = &g_RoomPortals[g_Rooms[room].roomportallistoffset];

            for (i = 0; i < numPortals; i++) {
                portalIndex = portalList[i];
                uint8_t *cacheEntry = g_PortalIntersectionCache[portalIndex];

                // Check cache or calculate new portal intersection
                if (cacheEntry[0] != g_PortalTraceCounter) {
                    cacheEntry[0] = g_PortalTraceCounter;
                    cacheEntry[1] = portalCalculateIntersection(portalIndex, startPos, endPos);
                }

                // Check portal directionality and traverse
                if (cacheEntry[1] != PORTALINTERSECTION_NONE) {
                    if (cacheEntry[1] == PORTALINTERSECTION_BEHINDTOFRONT && room == g_BgPortals[portalIndex].roomnum1) {
                        portalTryAppendRoom(newWave, g_BgPortals[portalIndex].roomnum2);
                        portalTryAppendRoom(visitedRooms, g_BgPortals[portalIndex].roomnum2);
                        cacheEntry[1] = PORTALINTERSECTION_NONE;
                    } else if (cacheEntry[1] == PORTALINTERSECTION_FRONTTOBEHIND && room == g_BgPortals[portalIndex].roomnum2) {
                        portalTryAppendRoom(newWave, g_BgPortals[portalIndex].roomnum1);
                        portalTryAppendRoom(visitedRooms, g_BgPortals[portalIndex].roomnum1);
                        cacheEntry[1] = PORTALINTERSECTION_NONE;
                    }
                }
            }
        }

        // Copy newWave into currentWave for next loop iteration
        for (i = 0; i < 16; i++) {
            currentWave[i] = newWave[i];
            if (newWave[i] == -1) break;
        }
    } while (newWave[0] != -1);

    // Output the final traversed rooms
    for (i = 0; i < 7 && visitedRooms[i] != -1; i++) {
        outputRooms[i] = visitedRooms[i];
    }
    outputRooms[i] = -1;

    // Optional: output all rooms encountered
    if (allVisitedRooms != NULL) {
        for (i = 0; i < maxVisitedRooms; i++) {
            allVisitedRooms[i] = visitedRooms[i];
            if (visitedRooms[i] == -1) break;
        }
        allVisitedRooms[i] = -1;
    }
}