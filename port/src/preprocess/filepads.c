#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocess/common.h"

#include "constants.h"

struct n64_header {
	int num_pads;
	int num_covers;
	uint32_t ptr_waypoints;
	uint32_t ptr_waygroups;
	uint32_t ptr_cover;
};

struct host_header {
	int num_pads;
	int num_covers;
	uintptr_t ptr_waypoints;
	uintptr_t ptr_waygroups;
	uintptr_t ptr_cover;
};

struct padheader {
	uint32_t flags : 18;
	uint32_t room : 10;
	uint32_t liftnum : 4;
};

struct n64_waypoint {
	int padnum;
	uint32_t ptr_neighbours;
	int groupnum;
	int step;
};

struct n64_waygroup {
	uint32_t ptr_neighbours;
	uint32_t ptr_waypoints;
	int step;
};

static uint32_t convertPads(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int num_pads)
{
	uint16_t *src_offsets = (uint16_t *) &src[srcpos];
	uint16_t *dst_offsets = (uint16_t *) &dst[dstpos];

	dstpos += num_pads * sizeof(uint16_t);

	for (int i = 0; i < num_pads; i++) {
		srcpos = PD_BE16(src_offsets[i]);

		dst_offsets[i] = (dstpos);

		// Header
		uint32_t n64_padheader = PD_BE32(*(uint32_t *) &src[srcpos]);
		struct padheader *host_padheader = (struct padheader *) &dst[dstpos];
		uint32_t flags = (n64_padheader >> 14) & 0x3ffff;

		*(uint32_t*)host_padheader = (n64_padheader);

		srcpos += sizeof(struct padheader);
		dstpos += sizeof(struct padheader);

		// Position
		if (flags & PADFLAG_INTPOS) {
			s16 *srcptr = (s16 *) &src[srcpos];
			s16 *dstptr = (s16 *) &dst[dstpos];

			dstptr[0] = PD_BE16(srcptr[0]);
			dstptr[1] = PD_BE16(srcptr[1]);
			dstptr[2] = PD_BE16(srcptr[2]);

			srcpos += 8;
			dstpos += 8;
		} else {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Up
		if ((flags & (PADFLAG_UPALIGNTOX | PADFLAG_UPALIGNTOY | PADFLAG_UPALIGNTOZ)) == 0) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Look
		if ((flags & (PADFLAG_LOOKALIGNTOX | PADFLAG_LOOKALIGNTOY | PADFLAG_LOOKALIGNTOZ)) == 0) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);

			srcpos += 12;
			dstpos += 12;
		}

		// Bbox
		if (flags & PADFLAG_HASBBOXDATA) {
			uint32_t *srcptr = (uint32_t *) &src[srcpos];
			uint32_t *dstptr = (uint32_t *) &dst[dstpos];

			dstptr[0] = PD_BE32(srcptr[0]);
			dstptr[1] = PD_BE32(srcptr[1]);
			dstptr[2] = PD_BE32(srcptr[2]);
			dstptr[3] = PD_BE32(srcptr[3]);
			dstptr[4] = PD_BE32(srcptr[4]);
			dstptr[5] = PD_BE32(srcptr[5]);

			srcpos += 4 * 6;
			dstpos += 4 * 6;
		}
	}

	return dstpos;
}

static uint32_t convertWayPoints(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_waypoint *n64_waypoints = (struct n64_waypoint *) &src[srcpos];
	struct waypoint *host_waypoints = (struct waypoint *) &dst[dstpos];
	int num_waypoints;

	for (num_waypoints = 0; n64_waypoints[num_waypoints].padnum != -1; num_waypoints++);

	dstpos += (num_waypoints + 1) * sizeof(struct waypoint);

	uint32_t *host_neighbours = (uint32_t *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waypoints; i++) {
		host_waypoints[i].padnum = PD_BE32(n64_waypoints[i].padnum);
		host_waypoints[i].neighbours = (void *)(uintptr_t)dstpos;
		host_waypoints[i].groupnum = PD_BE32(n64_waypoints[i].groupnum);
		host_waypoints[i].step = 0;

		uint32_t *n64_neighbours = (uint32_t *) &src[PD_BE32(n64_waypoints[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = PD_BE32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waypoints[num_waypoints].padnum = 0xffffffff;
	host_waypoints[num_waypoints].neighbours = NULL;
	host_waypoints[num_waypoints].groupnum = 0;
	host_waypoints[num_waypoints].step = 0;

	return dstpos;
}

static uint32_t convertWayGroups(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos)
{
	struct n64_waygroup *n64_waygroups = (struct n64_waygroup *) &src[srcpos];
	struct waygroup *host_waygroups = (struct waygroup *) &dst[dstpos];
	int num_waygroups;

	for (num_waygroups = 0; n64_waygroups[num_waygroups].ptr_neighbours != 0; num_waygroups++);

	dstpos += (num_waygroups + 1) * sizeof(struct waygroup);

	// Waygroups and child waypoints
	uint32_t *host_waypoints = (uint32_t *) &dst[dstpos];
	int n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].waypoints = (void *)(uintptr_t)dstpos;
		host_waygroups[i].step = 0;

		uint32_t *n64_waypoints = (uint32_t *) &src[PD_BE32(n64_waygroups[i].ptr_waypoints)];

		for (int j = 0; n64_waypoints[j] != 0xffffffff; j++) {
			host_waypoints[n++] = PD_BE32(n64_waypoints[j]);
			dstpos += 4;
		}

		host_waypoints[n++] = 0xffffffff;
		dstpos += 4;
	}

	// Terminator
	host_waygroups[num_waygroups].neighbours = NULL;
	host_waygroups[num_waygroups].waypoints = NULL;
	host_waygroups[num_waygroups].step = 0;

	// Waygroup neighbours
	uint32_t *host_neighbours = (uint32_t *) &dst[dstpos];
	n = 0;

	for (int i = 0; i < num_waygroups; i++) {
		host_waygroups[i].neighbours = (void *)(uintptr_t)dstpos;

		uint32_t *n64_neighbours = (uint32_t *) &src[PD_BE32(n64_waygroups[i].ptr_neighbours)];

		for (int j = 0; n64_neighbours[j] != 0xffffffff; j++) {
			host_neighbours[n++] = PD_BE32(n64_neighbours[j]);
			dstpos += 4;
		}

		host_neighbours[n++] = 0xffffffff;
		dstpos += 4;
	}

	return dstpos;
}

static uint32_t convertCover(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int num_covers)
{
	struct coverdefinition *n64_covers = (struct coverdefinition *) &src[srcpos];
	struct coverdefinition *host_covers = (struct coverdefinition *) &dst[dstpos];

	for (int i = 0; i < num_covers; i++) {
		host_covers[i].pos = PD_SWAPPED_VAL(n64_covers[i].pos);
		host_covers[i].look = PD_SWAPPED_VAL(n64_covers[i].look);
		host_covers[i].flags = PD_BE16(n64_covers[i].flags);
	}

	dstpos += num_covers * sizeof(struct coverdefinition);

	return dstpos;
}

static uint32_t convertPadsFile(uint8_t *dst, uint8_t *src)
{
	uint32_t dstpos = 0;
	struct n64_header *n64_header = (struct n64_header *) src;
	struct host_header *host_header = (struct host_header *) dst;

	int num_pads = PD_BE32(n64_header->num_pads);
	int num_covers = PD_BE32(n64_header->num_covers);
	host_header->num_pads = (num_pads);
	host_header->num_covers = (num_covers);

	dstpos += sizeof(struct host_header);

	// Pads
	dstpos = convertPads(dst, dstpos, src, sizeof(struct n64_header), num_pads);

	// Waypoints
	host_header->ptr_waypoints = (dstpos);
	dstpos = convertWayPoints(dst, dstpos, src, PD_BE32(n64_header->ptr_waypoints));

	// Waygroups
	host_header->ptr_waygroups = (dstpos);
	dstpos = convertWayGroups(dst, dstpos, src, PD_BE32(n64_header->ptr_waygroups));

	// Cover
	host_header->ptr_cover = (dstpos);
	dstpos = convertCover(dst, dstpos, src, PD_BE32(n64_header->ptr_cover), num_covers);

	return dstpos;
}

uint8_t* preprocessPadsFile(uint8_t *data, uint32_t size, uint32_t *outSize) {
	uint32_t newSizeEstimated = romdataFileGetEstimatedSize(size, LOADTYPE_PADS);
	uint8_t* dst = sysMemZeroAlloc(newSizeEstimated);

	uint32_t newSize = convertPadsFile(dst, data);

	if (newSize > newSizeEstimated) {
		sysFatalError("overflow when trying to preprocess a pads file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
