#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "preprocess/common.h"
#include "preprocess/gbi.h"

#if UINTPTR_MAX > 0xFFFFFFFFu
#define HOST_DWORDS_PER_CMD 2
#else
#define HOST_DWORDS_PER_CMD 1
#endif

struct vtx {
	int16_t x;
	int16_t y;
	int16_t z;
	uint8_t flags;
	uint8_t colour;
	int16_t s;
	int16_t t;
};

struct texaddr {
	uint32_t src;
	uint32_t dst;
};

static uint32_t gbiSegments[16];
static uint32_t srcVtxOffset;
static uint32_t dstVtxOffset;

static struct texaddr texAddrs[64];
static int numTexAddrs;

void gbiReset(void)
{
	for (int i = 0; i < ARRAYCOUNT(gbiSegments); i++) {
		gbiSegments[i] = 0;
	}
	
	srcVtxOffset = 0;
	dstVtxOffset = 0;
	numTexAddrs = 0;
}

void gbiSetSegment(int segment, uint32_t offset)
{
	gbiSegments[segment] = offset & 0x00ffffff;
}

void gbiSetVtx(uint32_t src_offset, uint32_t dst_offset)
{
	srcVtxOffset = src_offset & 0x00ffffff;
	dstVtxOffset = dst_offset & 0x00ffffff;
}

void gbiAddTexAddr(uint32_t src_offset, uint32_t dst_offset)
{
	texAddrs[numTexAddrs].src   = src_offset & 0x00ffffff;
	texAddrs[numTexAddrs++].dst = dst_offset & 0x00ffffff;
}

uint32_t gbiFindTexAddr(uint32_t src_offset)
{
	for (int i = 0; i < numTexAddrs; i++) {
		if (texAddrs[i].src == src_offset)
			return texAddrs[i].dst;
	}

	return 0;
}

void gbiConvertVtx(uint8_t *dst, uint32_t offset, int count)
{
	if (!offset) return;
	struct vtx* vtxs = (struct vtx*)(dst + offset);

	for (int i = 0; i < count; i++) {
		struct vtx* vtx = &vtxs[i];

		vtx->x = PD_BE16(vtx->x);
		vtx->y = PD_BE16(vtx->y);
		vtx->z = PD_BE16(vtx->z);
		vtx->s = PD_BE16(vtx->s);
		vtx->t = PD_BE16(vtx->t);
	}
}

#define CMD_IS_VTX(cmd)      ((cmd & 0xff00000000000000) == 0x0400000000000000)
#define CMD_IS_DL(cmd)       ((cmd & 0xff00000000000000) == 0x0600000000000000)
#define CMD_IS_COL(cmd)      ((cmd & 0xff00000000000000) == 0x0700000000000000)
#define CMD_IS_ENDDL(cmd)    ((cmd & 0xff00000000000000) == 0xb800000000000000)
#define CMD_IS_MOVEWORD(cmd) ((cmd & 0xff00000000000000) == 0xbc00000000000000)
#define CMD_IS_MOVEMEM(cmd)  ((cmd & 0xff00000000000000) == 0x0300000000000000)
#define CMD_IS_MTX(cmd)      ((cmd & 0xff00000000000000) == 0x0100000000000000)
#define CMD_IS_SETTIMG(cmd)  ((cmd & 0xff00000000000000) == 0xfd00000000000000)
#define CMD_IS_SETZIMG(cmd)  ((cmd & 0xff00000000000000) == 0xfe00000000000000)
#define CMD_IS_SETCIMG(cmd)  ((cmd & 0xff00000000000000) == 0xff00000000000000)

#define CMD_HAS_ADDR(cmd) CMD_IS_VTX(cmd) \
	|| CMD_IS_COL(cmd) \
	|| CMD_IS_MOVEWORD(cmd) \
	|| CMD_IS_SETTIMG(cmd)


#define CMD_IS_SEGMENTED(cmd) ( CMD_IS_MOVEMEM(cmd) \
	|| CMD_IS_MTX(cmd) \
	|| CMD_IS_VTX(cmd) \
	|| CMD_IS_COL(cmd) \
	|| CMD_IS_DL(cmd) \
	|| CMD_IS_SETTIMG(cmd) \
	|| CMD_IS_SETCIMG(cmd) \
	|| CMD_IS_SETZIMG(cmd))

/**
 * Segment 5 is the start of the model file. The data in these files shifts
 * depending on the pointer size, so this function adjusts the offset accordingly.
 */
static uint64_t gbiRewriteAddr(uint64_t cmd, uint8_t opcode)
{
	uint8_t segment = (cmd & 0x0f000000) >> 24;
	uint32_t offset = cmd & 0x00ffffff;

	if (segment == 5) {
		if (opcode == 0xfd) {
			offset = gbiFindTexAddr(offset);
		}
		else {
			if (offset < srcVtxOffset) {
				sysFatalError("Tried to load data from offset 0x%x but segment starts at 0x%x\n", offset, srcVtxOffset);
			}

			offset = offset - srcVtxOffset + dstVtxOffset;
		}
	}

	return (cmd & 0xffffffffff000000) | offset;
}

void gbiGdlRewriteAddrs(uint8_t *dst, uint32_t offset)
{
	if (!offset) return;

	uint64_t *cmds = (uint64_t *)(dst + (offset & 0x00ffffff));
	if (!cmds) return;

	uint64_t cmd;

	do {
		cmd = *cmds;
		Gfx* gfxcmd = (Gfx *)cmds;
		int idx = 0;
		uint8_t opcode = (cmd >> 24) & 0xff;

#if HOST_DWORDS_PER_CMD == 2
		idx = 1;
		opcode = (cmd >> 24) & 0xff;
#endif
		cmd = (uint64_t)opcode << 56;

		if (CMD_HAS_ADDR(cmd)) {
			cmds[idx] = gbiRewriteAddr(cmds[idx], opcode);
		}

		if (CMD_IS_SEGMENTED(cmd)) {
			gfxcmd->words.w1 |= 1;
		}

		cmds += HOST_DWORDS_PER_CMD;
	} while (!CMD_IS_ENDDL(cmd));
}

uint32_t gbiConvertGdl(uint8_t *dst, uint32_t dstpos, uint8_t *src, uint32_t srcpos, int segment_cmds)
{
	dstpos = ALIGN8(dstpos);

	uint64_t *n64_cmd = (uint64_t*)&src[srcpos];
	uint64_t *host_cmd = (uint64_t*)&dst[dstpos];
	uint64_t cmd;

	do {
		cmd = PD_BE64(*n64_cmd);

#if HOST_DWORDS_PER_CMD == 2
		host_cmd[0] = ((cmd & 0xffffffff00000000) >> 32);
		host_cmd[1] = (cmd & 0xffffffff);

		if (segment_cmds && CMD_IS_SEGMENTED(cmd)) {
			host_cmd[1] |= 1;
		}
#else
		host_cmd[0] = (cmd >> 32) | ((cmd & 0xffffffff) << 32);

		if (segment_cmds && CMD_IS_SEGMENTED(cmd)) {
			Gfx* gfxcmd = (Gfx*)host_cmd;
			gfxcmd->words.w1 |= 1;
		}
#endif

		dstpos += sizeof(*host_cmd) * HOST_DWORDS_PER_CMD;
		host_cmd += HOST_DWORDS_PER_CMD;
		n64_cmd++;
	} while (!CMD_IS_ENDDL(cmd));

	return dstpos;
}
