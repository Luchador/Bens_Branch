#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "game/prop.h"
#include "game/textutils.h"
#include "game/utils.h"
#include "game/bg.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "data.h"
#include "types.h"
#ifndef PLATFORM_N64
#include "mod.h"
#endif

#define ANIM_HEADER_CACHE_SIZE 40
#define ANIM_FRAME_CACHE_SIZE  32

uint8_t *g_AnimFrameByteSlots;
uint8_t **g_AnimFrameBytes;
int16_t *g_AnimFrameAnimNums;
int16_t *g_AnimFrameFrameNums;
uint8_t *g_AnimFrameBirths;
uint8_t *g_AnimHeaderByteSlots;
uint8_t **g_AnimHeaderBytes;
int16_t *g_AnimHeaderAnimNums;
int *g_AnimHeaderBirths;
int16_t g_NumRomAnimations;
struct animtableentry *g_RomAnims;

unsigned int g_NextAnimFrameIndex = 0;
int g_NextAnimHeaderIndex = 0;
int16_t g_NumAnimations = 0;
struct animtableentry *g_Anims = NULL;
uint8_t *g_AnimToHeaderSlot = NULL;
int16_t *var8005f014 = NULL;
int g_AnimMaxBytesPerFrame = 176;
int g_AnimMaxHeaderLength = 608;
uint8_t *g_AnimHostSegment = NULL;
uint8_t **g_AnimReplacements;

extern uint8_t EXT_SEG _animationsTableRomStart;
extern uint8_t EXT_SEG _animationsTableRomEnd;

void animsInit(void)
{
	int i;
	unsigned int *ptr;
	unsigned int tablelen = ALIGN64(REF_SEG _animationsTableRomEnd - REF_SEG _animationsTableRomStart);

	ptr = mempAlloc(tablelen, MEMPOOL_PERMANENT);
	dmaExec(ptr, (romptr_t) REF_SEG _animationsTableRomStart, tablelen);

	g_NumAnimations = g_NumRomAnimations = ptr[0];
	g_Anims = g_RomAnims = (struct animtableentry *)&ptr[1];

	g_AnimMaxHeaderLength = 1;
	g_AnimMaxBytesPerFrame = 1;

	for (i = 0; i < g_NumAnimations; i++) {
		if (g_Anims[i].headerlen > g_AnimMaxHeaderLength) {
			g_AnimMaxHeaderLength = g_Anims[i].headerlen;
		}

		if (g_Anims[i].bytesperframe > g_AnimMaxBytesPerFrame) {
			g_AnimMaxBytesPerFrame = g_Anims[i].bytesperframe;
		}
	}

	g_AnimMaxHeaderLength = ALIGN16(g_AnimMaxHeaderLength + 34);
	g_AnimMaxBytesPerFrame = ALIGN16(g_AnimMaxBytesPerFrame + 34);

	g_AnimToHeaderSlot    = mempAlloc(ALIGN64(g_NumAnimations), MEMPOOL_PERMANENT);
	var8005f014           = mempAlloc(ALIGN64(g_NumAnimations * sizeof(*var8005f014)), MEMPOOL_PERMANENT);
	g_AnimFrameByteSlots  = mempAlloc(ALIGN64(ANIM_FRAME_CACHE_SIZE * g_AnimMaxBytesPerFrame), MEMPOOL_PERMANENT);
	g_AnimFrameBytes      = mempAlloc(ALIGN64(ANIM_FRAME_CACHE_SIZE * sizeof(*g_AnimFrameBytes)), MEMPOOL_PERMANENT);
	g_AnimFrameAnimNums   = mempAlloc(ALIGN64(ANIM_FRAME_CACHE_SIZE * sizeof(*g_AnimFrameAnimNums)), MEMPOOL_PERMANENT);
	g_AnimFrameFrameNums  = mempAlloc(ALIGN64(ANIM_FRAME_CACHE_SIZE * sizeof(*g_AnimFrameFrameNums)), MEMPOOL_PERMANENT);
	g_AnimFrameBirths     = mempAlloc(ALIGN64(ANIM_FRAME_CACHE_SIZE * sizeof(*g_AnimFrameBirths)), MEMPOOL_PERMANENT);
	g_AnimHeaderByteSlots = mempAlloc(ALIGN64(ANIM_HEADER_CACHE_SIZE * g_AnimMaxHeaderLength), MEMPOOL_PERMANENT);
	g_AnimHeaderBytes     = mempAlloc(ALIGN64(ANIM_HEADER_CACHE_SIZE * sizeof(*g_AnimHeaderBytes)), MEMPOOL_PERMANENT);
	g_AnimHeaderAnimNums  = mempAlloc(ALIGN64(ANIM_HEADER_CACHE_SIZE * sizeof(*g_AnimHeaderAnimNums)), MEMPOOL_PERMANENT);
	g_AnimHeaderBirths    = mempAlloc(ALIGN64(ANIM_HEADER_CACHE_SIZE * sizeof(*g_AnimHeaderBirths)), MEMPOOL_PERMANENT);
	g_AnimReplacements    = mempAlloc(ALIGN64(g_NumAnimations * sizeof(uint8_t *)), MEMPOOL_PERMANENT);
	bzero(g_AnimReplacements, g_NumAnimations * sizeof(uint8_t *));

	animsInitTables();

	g_AnimHostSegment = NULL;
}

void animsInitTables(void)
{
	int i;

	for (i = 0; i < g_NumAnimations; i++) {
		g_AnimToHeaderSlot[i] = 0xff;
		var8005f014[i] = 0;
	}

	for (i = 0; i < ANIM_FRAME_CACHE_SIZE; i++) {
		g_AnimFrameAnimNums[i] = 0;
		g_AnimFrameFrameNums[i] = 0;
		g_AnimFrameBirths[i] = 0;
	}

	for (i = 0; i < ANIM_HEADER_CACHE_SIZE; i++) {
		g_AnimHeaderAnimNums[i] = 0;
		g_AnimHeaderBirths[i] = -2;
	}
}

void animsReset(void)
{
	g_NumAnimations = g_NumRomAnimations;
	g_Anims = g_RomAnims;
}

int animGetNumFrames(int16_t animnum)
{
	return g_Anims[animnum].numframes;
}

bool animHasFrames(int16_t animnum)
{
	return animnum < g_NumAnimations && g_Anims[animnum].numframes > 0;
}

int animGetNumAnimations(void)
{
	return g_NumAnimations;
}

extern uint8_t EXT_SEG _animationsSegmentRomStart;

uint8_t *animDma(uint8_t *dst, unsigned int segoffset, unsigned int len)
{
	/*if (g_AnimHostEnabled) {
		bcopy(&g_AnimHostSegment[segoffset], dst, len);
		return dst;
	}*/

	return dmaExecWithAutoAlign(dst, (romptr_t) REF_SEG _animationsSegmentRomStart + segoffset, len);
}

/**
 * Return -1 if the given apparent frame is a repeat frame, or if not a repeat
 * frame then remap the apparent frame to a real one and return it.
 *
 * The end of the header can contain a sequence of shorts such as:
 * -1, 55, 30
 *
 * The values are iterated backwards in pairs of 2 and are terminated by -1.
 *
 * In each pair, the right value is the repeatfromframe and the left value is
 * the repeattoframe. In the above example, apparent frames 30 to 55 are
 * repeated, so the remapping looks like:
 * 29 -> 29
 * 30 -> -1
 * ...
 * 55 -> -1
 * 56 -> 30
 * 57 -> 31
 */
int animGetRemappedFrame(int16_t animnum, int apparentframe)
{
	uint8_t *ptr = (uint8_t *)(g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]] + g_Anims[animnum].headerlen - 2);
	int realframe = apparentframe;

	while (true) {
		int16_t repeatfromframe = ptr[0] << 8 | ptr[1];
		int16_t repeattoframe;

		if (repeatfromframe < 0) {
			break;
		}

		repeattoframe = ptr[-2] << 8 | ptr[-1];
		ptr -= 4;

		if (repeatfromframe <= apparentframe) {
			if (repeattoframe < apparentframe) {
				realframe = realframe - repeattoframe + repeatfromframe - 1;
			} else {
				realframe = -1;
				break;
			}
		}
	}

	return realframe;
}

/**
 * Similar to the above, but with the following differences:
 * - Write the remapped frame to the frameptr pointer instead of returning it.
 * - If the apparent frame is a repeat, write the original frame rather than -1.
 * - Return true if the frame is original or false if it's a repeat.
 */
bool animRemapFrameForLoad(int16_t animnum, int apparentframe, int *frameptr)
{
	uint8_t *ptr = (uint8_t *)(g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]] + g_Anims[animnum].headerlen - 2);
	int result = apparentframe;
	bool ret = true;

	while (true) {
		int16_t repeatfromframe = ptr[0] << 8 | ptr[1];
		int16_t repeattoframe;

		if (repeatfromframe < 0) {
			break;
		}

		repeattoframe = ptr[-2] << 8 | ptr[-1];
		ptr -= 4;

		if (repeatfromframe <= apparentframe) {
			if (repeattoframe < apparentframe) {
				result = result - repeattoframe + repeatfromframe - 1;
			} else {
				result = result - apparentframe + repeatfromframe;
				ret = false;
				break;
			}
		}
	}

	*frameptr = result;

	return ret;
}

/**
 * Return true if the given animation and frame should be skipped.
 *
 * Used by cutscenes.
 *
 * The skip frame numbers are stored at the tail end of the header, prior to the
 * frame repeat data. The frame numbers are stored as a list of shorts.
 * The list is terminated on the left side with a negative value.
 */
bool animIsFrameCutSkipped(int16_t animnum, int frame)
{
	uint8_t *ptr = (uint8_t *)(g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]] + g_Anims[animnum].headerlen - 2);

	// Iterate past the repeat list
	if (g_Anims[animnum].flags & ANIMFLAG_HASREPEATFRAMES) {
		while (true) {
			int16_t repeatfromframe = ptr[0] << 8 | ptr[1];

			if (repeatfromframe < 0) {
				break;
			}

			ptr -= 4;
		}

		ptr -= 2;
	}

	while (true) {
		int16_t skipframe = ptr[0] << 8 | ptr[1];

		if (skipframe < 0) {
			break;
		}

		if (skipframe == frame) {
			return true;
		}

		ptr -= 2;
	}

	return false;
}

uint8_t animLoadFrame(int16_t animnum, int framenum)
{
	int slot = -1;
	int i;
	int offset;
	int loadframenum = framenum;

	for (i = 0; i < ANIM_FRAME_CACHE_SIZE; i++) {
		if (g_AnimFrameAnimNums[i] == animnum && g_AnimFrameFrameNums[i] == loadframenum) {
			slot = i;
			break;
		}
	}

	if (slot >= 0) {
		g_AnimFrameBirths[slot] = 1;
	} else {
		slot = g_NextAnimFrameIndex;

		while (g_AnimFrameBirths[slot]) {
			slot = (slot + 1) % ANIM_FRAME_CACHE_SIZE;
		}

		if (g_Anims[animnum].flags & ANIMFLAG_HASREPEATFRAMES) {
			animRemapFrameForLoad(animnum, framenum, &loadframenum);
		}

		if (g_Anims[animnum].bytesperframe) {
			offset = g_Anims[animnum].bytesperframe * loadframenum + (g_Anims[animnum].data + g_Anims[animnum].headerlen);
			if (g_Anims[animnum].data == 0xffffffff) {
				// load external replacement (this will fatal error if there's no data)
				if (!g_AnimReplacements[animnum]) {
					g_AnimReplacements[animnum] = modAnimationLoadData(animnum);
				}
				offset = g_Anims[animnum].bytesperframe * loadframenum + g_Anims[animnum].headerlen;
				g_AnimFrameBytes[slot] = g_AnimReplacements[animnum] + offset;
			} else
			g_AnimFrameBytes[slot] = animDma(&g_AnimFrameByteSlots[slot * g_AnimMaxBytesPerFrame], offset, g_Anims[animnum].bytesperframe);
		} else {
			g_AnimFrameBytes[slot] = &g_AnimFrameByteSlots[slot * g_AnimMaxBytesPerFrame];
		}

		g_AnimFrameAnimNums[slot] = animnum;
		g_AnimFrameFrameNums[slot] = framenum;
		g_AnimFrameBirths[slot] = 1;
		g_NextAnimFrameIndex = (slot + 1) % ANIM_FRAME_CACHE_SIZE;
	}

	return slot;
}

void animForgetFrameBirths(void)
{
	int i;

	for (i = 0; i < ANIM_FRAME_CACHE_SIZE; i++) {
		g_AnimFrameBirths[i] = 0;
	}
}

void animLoadHeader(int16_t animnum)
{
	int i;

	if (g_AnimToHeaderSlot[animnum] != 0xff) {
		g_AnimHeaderBirths[g_AnimToHeaderSlot[animnum]] = g_Vars.thisframestart240;
		g_NextAnimHeaderIndex = (g_AnimToHeaderSlot[animnum] + 1) % ANIM_HEADER_CACHE_SIZE;
	} else {
		int tmp;
		int slot = g_NextAnimHeaderIndex;

		for (i = 0; i < ANIM_HEADER_CACHE_SIZE; i++) {
			if (g_AnimHeaderBirths[i] < g_AnimHeaderBirths[slot]) {
				slot = i;
			}
		}

		if (g_AnimHeaderBirths[slot]);
		if (&g_Vars && &g_Vars);

		if (g_AnimHeaderAnimNums[slot]) {
			g_AnimToHeaderSlot[g_AnimHeaderAnimNums[slot]] = 0xff;
		}

		tmp = g_Anims[animnum].headerlen;

#ifndef PLATFORM_N64
		if (g_Anims[animnum].data == 0xffffffff) {
			// load external replacement (this will fatal error if there's no data)
			if (!g_AnimReplacements[animnum]) {
				g_AnimReplacements[animnum] = modAnimationLoadData(animnum);
			}
			g_AnimHeaderBytes[slot] = g_AnimReplacements[animnum];
		} else
#endif
		g_AnimHeaderBytes[slot] = animDma(&g_AnimHeaderByteSlots[slot * g_AnimMaxHeaderLength], g_Anims[animnum].data, tmp);
		g_AnimToHeaderSlot[animnum] = slot;
		g_AnimHeaderAnimNums[slot] = animnum;
		g_AnimHeaderBirths[slot] = g_Vars.thisframestart240;
		g_NextAnimHeaderIndex = (slot + 1) % ANIM_HEADER_CACHE_SIZE;
	}
}

/**
 * Read a number of bits from the given ptr and return it as an integer.
 *
 * remainingbits in the number of bits to read.
 * bitoffset is the starting bit offset relative to ptr.
 */
int animReadBits(uint8_t *ptr, uint8_t remainingbits, unsigned int bitoffset)
{
	unsigned int result = 0;
	unsigned int mask;
	uint8_t numbitsthisbyte;

	result *= bitoffset / 8;

	// Move ptr forward past all the bytes that should be fully skipped
	ptr += bitoffset / 8;

	// Calculate the number of bits to read in the first byte
	bitoffset %= 8;
	numbitsthisbyte = 8 - bitoffset;

	// Iterate bytes, except for the last if it's a partial read
	while (remainingbits >= numbitsthisbyte) {
		remainingbits -= numbitsthisbyte;
		mask = (1 << numbitsthisbyte) - 1;
		result |= (*ptr & mask) << remainingbits;
		ptr++;
		numbitsthisbyte = 8;
	}

	// Read bits from the final byte if it's partial read
	if (remainingbits > 0) {
		mask = (1 << remainingbits) - 1;
		result |= (*ptr >> (numbitsthisbyte - remainingbits)) & mask;
	}

	return result;
}

int animReadSignedShort(uint8_t *ptr, uint8_t readbitlen, int bitoffset)
{
	uint16_t result = animReadBits(ptr, readbitlen, bitoffset);

	if (readbitlen < 16 && (result & (1 << (readbitlen - 1)))) {
		result |= ((1 << (16 - readbitlen)) - 1) << readbitlen;
	}

	return result;
}

/**
 * Read the rotation, position and scale values for the given part for the frame
 * at the given frameslot.
 *
 * Both the anim header and frame data must be loaded already.
 */
void animGetRotTranslateScale(int part, bool flip, struct skeleton *skel, int16_t animnum, uint8_t frameslot, struct coord *rot, struct coord *translate, struct coord *scale)
{
	int i;
	uint16_t introt[3];
	uint8_t readbitlen;
	uint8_t *framebytes = g_AnimFrameBytes[frameslot];
	uint8_t framelen;
	uint8_t *ptr;
	uint8_t *end;
	int bitoffset;

	if (flip) {
		part = skel->things[part][1];
	}

	framelen = g_Anims[animnum].framelen;
	ptr = g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]];
	bitoffset = 0;
	end = ptr + g_Anims[animnum].headerlen;

	for (i = 0; i < part && ptr < end; i++) {
		uint8_t flags = *ptr;
		ptr++;

		if (flags & ANIMFIELD_08) {
			bitoffset += ptr[2] + ptr[5] + ptr[8] + ptr[11];
			ptr += 12;
		} else if (flags & ANIMFIELD_S16_TRANSLATE) {
			bitoffset += ptr[2] + ptr[5] + ptr[8];
			ptr += 9;
		} else if (flags & ANIMFIELD_S32_TRANSLATE) {
			bitoffset += ptr[0] + ptr[5] + ptr[10];
			ptr += 15;
		}

		if (flags & ANIMFIELD_S16_ROTATE) {
			bitoffset += ptr[2] + ptr[5] + ptr[8];
			ptr += 9;
		} else if (flags & ANIMFIELD_F32_ROTATE) {
			bitoffset += 96;
		}

		if (flags & ANIMFIELD_CAMERA) {
			bitoffset += ptr[0];
			ptr += 5;
		}

		if (flags & ANIMFIELD_F32_SCALE) {
			bitoffset += 0x60;
		}
	}

	if (ptr < end) {
		uint8_t flags = *ptr;
		ptr++;

		if (flags & ANIMFIELD_S16_TRANSLATE) {
			readbitlen = ptr[2];
			translate->x = (int16_t) (animReadSignedShort(framebytes, readbitlen, bitoffset) + (ptr[0] << 8) + ptr[1]);
			bitoffset += readbitlen;

			readbitlen = ptr[5];
			translate->y = (int16_t) (animReadSignedShort(framebytes, readbitlen, bitoffset) + (ptr[3] << 8) + ptr[4]);
			bitoffset += readbitlen;

			readbitlen = ptr[8];
			translate->z = (int16_t) (animReadSignedShort(framebytes, readbitlen, bitoffset) + (ptr[6] << 8) + ptr[7]);
			bitoffset += readbitlen;

			ptr += 9;
		} else if (flags & ANIMFIELD_S32_TRANSLATE) {
			readbitlen = ptr[0];
			translate->x = (animReadBits(framebytes, readbitlen, bitoffset) + ((ptr[1] << 24) + (ptr[2] << 16) + (ptr[3] << 8) + ptr[4])) * 0.001f;
			bitoffset += readbitlen;

			readbitlen = ptr[5];
			translate->y = (animReadBits(framebytes, readbitlen, bitoffset) + ((ptr[6] << 24) + (ptr[7] << 16) + (ptr[8] << 8) + ptr[9])) * 0.001f;
			bitoffset += readbitlen;

			readbitlen = ptr[10];
			translate->z = (animReadBits(framebytes, readbitlen, bitoffset) + ((ptr[11] << 24) + (ptr[12] << 16) + (ptr[13] << 8) + ptr[14])) * 0.001f;
			bitoffset += readbitlen;

			ptr += 15;
		} else {
			if (flags & ANIMFIELD_08) {
				bitoffset += ptr[2] + ptr[5] + ptr[8] + ptr[11];
				ptr += 12;
			}

			translate->x = translate->y = translate->z = 0.0f;
		}

		if (flags & ANIMFIELD_S16_ROTATE) {
			readbitlen = ptr[2];
			introt[0] = animReadBits(framebytes, readbitlen, bitoffset);
			introt[0] += (ptr[0] << 8) + ptr[1];
			introt[0] <<= 16 - framelen;
			bitoffset += readbitlen;

			readbitlen = ptr[5];
			introt[1] = animReadBits(framebytes, readbitlen, bitoffset);
			introt[1] += (ptr[3] << 8) + ptr[4];
			introt[1] <<= 16 - framelen;
			bitoffset += readbitlen;

			readbitlen = ptr[8];
			introt[2] = animReadBits(framebytes, readbitlen, bitoffset);
			introt[2] += (ptr[6] << 8) + ptr[7];
			introt[2] <<= 16 - framelen;
			bitoffset += readbitlen;

			rot->x = introt[0] * M_TAU / 65536.0f;

			if (flip) {
				if (introt[1] != 0) {
					rot->y = (0x10000 - introt[1]) * M_TAU / 65536.0f;
				} else {
					rot->y = 0.0f;
				}

				if (introt[2] != 0) {
					rot->z = (0x10000 - introt[2]) * M_TAU / 65536.0f;
				} else {
					rot->z = 0.0f;
				}
			} else {
				rot->y = introt[1] * M_TAU / 65536.0f;
				rot->z = introt[2] * M_TAU / 65536.0f;
			}
		} else if (flags & ANIMFIELD_F32_ROTATE) {
			int sp38;

			sp38 = animReadBits(framebytes, 32, bitoffset);
			rot->x = *(float *)&sp38;
			bitoffset += 32;

			sp38 = animReadBits(framebytes, 32, bitoffset);
			rot->y = *(float *)&sp38;
			bitoffset += 32;

			sp38 = animReadBits(framebytes, 32, bitoffset);
			rot->z = *(float *)&sp38;
			bitoffset += 32;

			if (flip) {
				if (rot->y != 0.0f) {
					rot->y = M_TAU - rot->y;
				}

				if (rot->z != 0.0f) {
					rot->z = M_TAU - rot->z;
				}
			}
		} else {
			rot->x = rot->y = rot->z = 0.0f;
		}

		if (flags & ANIMFIELD_F32_SCALE) {
			int word;

			word = animReadBits(framebytes, 32, bitoffset);
			scale->x = *(float *)&word;
			bitoffset += 32;

			word = animReadBits(framebytes, 32, bitoffset);
			scale->y = *(float *)&word;
			bitoffset += 32;

			word = animReadBits(framebytes, 32, bitoffset);
			scale->z = *(float *)&word;
		} else {
			scale->x = scale->y = scale->z = 1.0f;
		}

		return;
	}

	rot->x = rot->y = rot->z = 0.0f;
	translate->x = translate->y = translate->z = 0.0f;
	scale->x = scale->y = scale->z = 1.0f;
}

/**
 * Read the position and Y rotation (?) values for the given part at the given
 * frame number.
 *
 * No data needs to be loaded by the caller - the function will ensure the
 * header and frame are loaded.
 */
uint16_t animGetPosAngleAsInt(int part, bool flip, struct skeleton *skel, int16_t animnum, int framenum, int16_t inttranslate[3], bool arg6)
{
	uint16_t result = 0;
	int bitoffset;
	uint8_t readbitlen;
	uint8_t slot;
	uint8_t *framebytes;
	uint8_t *ptr;
	int i;

	if (arg6) {
		inttranslate[0] = 0;
		inttranslate[1] = 0;
		inttranslate[2] = var8005f014[animnum];
	} else {
		animLoadHeader(animnum);
		slot = animLoadFrame(animnum, framenum);
		animForgetFrameBirths();

		framebytes = g_AnimFrameBytes[slot];

		if (flip) {
			part = skel->things[part][1];
		}

		bitoffset = 0;
		ptr = g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]];

		for (i = 0; i < part; i++) {
			uint8_t flags = *ptr;
			ptr++;

			if (flags & ANIMFIELD_08) {
				bitoffset += ptr[2] + ptr[5] + ptr[8] + ptr[11];
				ptr += 12;
			} else if (flags & ANIMFIELD_S16_TRANSLATE) {
				bitoffset += ptr[2] + ptr[5] + ptr[8];
				ptr += 9;
			} else if (flags & ANIMFIELD_S32_TRANSLATE) {
				bitoffset += ptr[0] + ptr[5] + ptr[10];
				ptr += 15;
			}

			if (flags & ANIMFIELD_S16_ROTATE) {
				bitoffset += ptr[2] + ptr[5] + ptr[8];
				ptr += 9;
			} else if (flags & ANIMFIELD_F32_ROTATE) {
				bitoffset += 96;
			}

			if (flags & ANIMFIELD_CAMERA) {
				bitoffset += *ptr;
				ptr += 5;
			}

			if (flags & ANIMFIELD_F32_SCALE) {
				bitoffset += 96;
			}
		}

		readbitlen = ptr[3];
		inttranslate[0] = animReadSignedShort(framebytes, readbitlen, bitoffset) + ptr[1] * 256 + ptr[2];
		bitoffset += readbitlen;

		readbitlen = ptr[6];
		inttranslate[1] = animReadSignedShort(framebytes, readbitlen, bitoffset) + ptr[4] * 256 + ptr[5];
		bitoffset += readbitlen;

		readbitlen = ptr[9];
		inttranslate[2] = animReadSignedShort(framebytes, readbitlen, bitoffset) + ptr[7] * 256 + ptr[8];
		bitoffset += readbitlen;

		readbitlen = ptr[12];
		result = animReadSignedShort(framebytes, readbitlen, bitoffset) + ptr[10] * 256 + ptr[11];

		if (flip) {
			inttranslate[0] = -inttranslate[0];

			if (result != 0) {
				result = 0x10000 - result;
			}
		}
	}

	return result;
}

float animGetTranslateAngle(int part, bool flip, struct skeleton *skel, int16_t animnum, int framenum, struct coord *translate, bool arg6)
{
	int16_t inttranslate[3];

	float angle = animGetPosAngleAsInt(part, flip, skel, animnum, framenum, inttranslate, arg6);

	translate->x = inttranslate[0];
	translate->y = inttranslate[1];
	translate->z = inttranslate[2];

	return angle * M_TAU / 65536.0f;
}

/**
 * Return a camera value (FOV Y or blur frac) for the current frame.
 *
 * The function assumes the current frame's data has been loaded.
 * Its slot is provided by the frameslot argument.
 *
 * When part = 1, the returned value is the FOV Y.
 * When part = 2, the returned value is the blur frac.
 */
float animGetCameraValue(int part, int16_t animnum, uint8_t frameslot)
{
	uint8_t *framebytes = g_AnimFrameBytes[frameslot];
	uint8_t *ptr = g_AnimHeaderBytes[g_AnimToHeaderSlot[animnum]];
	float result = 0;
	int bitoffset = 0;
	int i;
	uint8_t *end = ptr + g_Anims[animnum].headerlen;

	for (i = 0; i < part && ptr < end; i++) {
		uint8_t flags = ptr[0];
		ptr++;

		if (flags & ANIMFIELD_08) {
			bitoffset += ptr[2] + ptr[5] + ptr[8] + ptr[11];
			ptr += 12;
		} else if (flags & ANIMFIELD_S16_TRANSLATE) {
			bitoffset += ptr[2] + ptr[5] + ptr[8];
			ptr += 9;
		} else if (flags & ANIMFIELD_S32_TRANSLATE) {
			bitoffset += ptr[0] + ptr[5] + ptr[10];
			ptr += 15;
		}

		if (flags & ANIMFIELD_S16_ROTATE) {
			bitoffset += ptr[2] + ptr[5] + ptr[8];
			ptr += 9;
		} else if (flags & ANIMFIELD_F32_ROTATE) {
			bitoffset += 0x60;
		}

		if (flags & ANIMFIELD_CAMERA) {
			bitoffset += ptr[0];
			ptr += 5;
		}

		if (flags & ANIMFIELD_F32_SCALE) {
			bitoffset += 0x60;
		}
	}

	if (ptr < end) {
		uint8_t flags = ptr[0];
		ptr++;

		if (flags & ANIMFIELD_CAMERA) {
			/**
			 * In the header:
			 * ptr[0] = number of bits to read in the frame data
			 * ptr[1,2,3,4] = base value
			 *
			 * The value in the frame data is an adjustment value that is added
			 * to the base value.
			 */
			int framevalue = animReadBits(framebytes, ptr[0], bitoffset);
			result = (framevalue + ptr[1] * 0x1000000 + ptr[2] * 0x10000 + ptr[3] * 0x100 + ptr[4]) * 0.001f;
		}
	}

	return result;
}
