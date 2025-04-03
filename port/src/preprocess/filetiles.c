#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "preprocess/common.h"

struct tile {
	uint8_t type;
	uint8_t numvertices;
	u16 flags;
	u16 floortype;
	uint8_t xmin;
	uint8_t ymin;
	uint8_t zmin;
	uint8_t xmax;
	uint8_t ymax;
	uint8_t zmax;
	u16 floorcol;
};

static u32 convertTiles(uint8_t *dst, uint8_t *src, size_t srclen)
{
	int num_rooms = PD_BE32(*(u32 *) src);

	size_t src_ptr_table_len = (num_rooms + 1) * 4;
	size_t dst_ptr_table_len = (num_rooms + 1) * sizeof(u32);

	u32 *src_offsets = (u32 *) &src[4];

	size_t data_len = srclen - src_ptr_table_len - 4;

	*(u32 *) dst = (num_rooms);

	u32 *dst_offsets = (u32 *) (dst + sizeof(u32));
	u32 cur_dst_offset = dst_ptr_table_len + sizeof(u32);

	u32 cur_src_offset = PD_BE32(src_offsets[0]);
	u32 end_src_offset = PD_BE32(src_offsets[num_rooms]);
	int room = 0;

	while (1) {
		// Check if this is a new room and write the pointer offset if so
		while (room < (num_rooms + 1) && PD_BE32(src_offsets[room]) == cur_src_offset) {
			dst_offsets[room] = (cur_dst_offset);
			room++;
		}

		if (cur_src_offset >= end_src_offset) {
			break;
		}

		// Write tile data
		struct tile *n64tile = (struct tile *) &src[cur_src_offset];
		struct tile hosttile;

		hosttile.type = n64tile->type;
		hosttile.numvertices = n64tile->numvertices;
		hosttile.flags = PD_BE16(n64tile->flags);
		hosttile.floortype = PD_BE16(n64tile->floortype);
		hosttile.xmin = n64tile->xmin;
		hosttile.ymin = n64tile->ymin;
		hosttile.zmin = n64tile->zmin;
		hosttile.xmax = n64tile->xmax;
		hosttile.ymax = n64tile->ymax;
		hosttile.zmax = n64tile->zmax;
		hosttile.floorcol = PD_BE16(n64tile->floorcol);

		memcpy(&dst[cur_dst_offset], &hosttile, sizeof(struct tile));

		cur_dst_offset += sizeof(struct tile);
		cur_src_offset += sizeof(struct tile);

		// Write tile vertices
		for (int i = 0; i < hosttile.numvertices; i++) {
			*(s16 *) &dst[cur_dst_offset + 0] = PD_BE16(*(s16 *) &src[cur_src_offset + 0]);
			*(s16 *) &dst[cur_dst_offset + 2] = PD_BE16(*(s16 *) &src[cur_src_offset + 2]);
			*(s16 *) &dst[cur_dst_offset + 4] = PD_BE16(*(s16 *) &src[cur_src_offset + 4]);
			cur_dst_offset += 6;
			cur_src_offset += 6;
		}
	}

	return ALIGN16(cur_dst_offset);
}

uint8_t *preprocessTilesFile(uint8_t *data, u32 size, u32 *outSize)
{
	u32 newSizeEstimated = romdataFileGetEstimatedSize(size, LOADTYPE_TILES);
	uint8_t *dst = sysMemZeroAlloc(newSizeEstimated);

	u32 newSize = convertTiles(dst, data, size);

	if (newSize > newSizeEstimated) {
		sysFatalError("overflow when trying to preprocess a tiles file, size %d newsize %d", size, newSize);
	}

	memcpy(data, dst, newSize);
	sysMemFree(dst);

	*outSize = newSize;

	return 0;
}
