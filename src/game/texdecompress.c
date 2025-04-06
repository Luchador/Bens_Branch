#include <ultra64.h>
#include "constants.h"
#include "game/debug.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "game/file.h"
#include "bss.h"
#include "fs.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "lib/rzip.h"
#include "data.h"
#include "types.h"
#include "mod.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct texture *g_Textures;
struct texpool g_TexSharedPool;
struct texcacheitem g_TexCacheItems[150];
int g_TexCacheCount;
int g_TexNumToLoad;
uint8_t *g_TexBitstring;
uint32_t g_TexAccumValue;
int g_TexAccumNumBits;
uint32_t g_TexBase;
uint8_t *g_TextureConfigSegment;
int g_TexNumConfigs;
struct tex **g_TexWords;
struct textureconfig *g_TexWallhitConfigs;
Gfx *g_TexGdl1;
Gfx *g_TexGdl2;
Gfx *g_TexGdl3;
struct texturepair *g_ExplosionTexturePairs;
struct textureconfig *g_TexBeamConfigs;
struct textureconfig *g_TexLaserConfigs;
struct textureconfig *g_TexGroup03Configs;
struct textureconfig *g_TexGeCrosshairConfigs;
struct textureconfig *g_TexRedLinesConfigs;
struct textureconfig *g_TexShadowConfigs;
struct textureconfig *g_TexShieldConfigs;
struct textureconfig *g_TexShardConfigs;
struct textureconfig *g_TexScreenConfigs;
struct textureconfig *g_TexSkyWaterConfigs;
struct textureconfig *g_TexGroup11Configs;
struct textureconfig *g_TexLightGlareConfigs;
struct textureconfig *g_TexSparkConfigs;
struct textureconfig *g_TexGeneralConfigs;
struct textureconfig *g_TexRadarConfigs;
struct textureconfig *g_TexStarsConfigs;

// The number of channels, excluding 1-bit alpha channels.
int g_TexFormatNumChannels[] = { 
	4, 	 // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	3, 	 // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	3, 	 // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	3, 	 // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	2,	 // TEXFORMAT_IA16 16-bit grayscale+alpha
	2,	 // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	1, 	 // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	1, 	 // TEXFORMAT_I8 8-bit grayscale
	1, 	 // TEXFORMAT_I4 4-bit grayscale
	1,	 // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	1,	 // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	1, 	 // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	1 }; // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes

// Whether each format supports a 1-bit alpha channel.
int g_TexFormatHas1BitAlpha[] = { 
	0,    // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	1,    // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	0,    // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	0,    // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	0,    // TEXFORMAT_IA16 16-bit grayscale+alpha
	0,    // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	1,    // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	0,    // TEXFORMAT_I8 8-bit grayscale
	0,    // TEXFORMAT_I4 4-bit grayscale
	0,    // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	0,    // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	0,    // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	0 };  // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes

// For non-paletted images, size in decimal of each colour channel.
// Eg. 32 means each channel can store up to 32 values (5-bits per channel).
// For paletted images, same thing but for the palette indices instead.
int g_TexFormatChannelSizes[] = { 
	256,   // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	32,    // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	256,   // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	32,    // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	256,   // TEXFORMAT_IA16 16-bit grayscale+alpha
	16,    // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	8,     // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	256,   // TEXFORMAT_I8 8-bit grayscale
	16,    // TEXFORMAT_I4 4-bit grayscale
	256,   // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	16,    // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	256,   // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	16 };  // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes

int g_TexFormatBitsPerPixel[] = { 
	32,   // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	16,   // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	24,   // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	15,   // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	16,   // TEXFORMAT_IA16 16-bit grayscale+alpha
	8,    // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	4,    // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	8,    // TEXFORMAT_I8 8-bit grayscale
	4,    // TEXFORMAT_I4 4-bit grayscale
	16,   // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	16,   // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	16,   // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	16 }; // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes

// Mapping to GBI format
int g_TexFormatGbiMappings[] = {
	G_IM_FMT_RGBA,   // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	G_IM_FMT_RGBA,   // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	G_IM_FMT_RGBA,   // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	G_IM_FMT_RGBA,   // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	G_IM_FMT_IA,     // TEXFORMAT_IA16 16-bit grayscale+alpha
	G_IM_FMT_IA,     // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	G_IM_FMT_IA,     // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	G_IM_FMT_I,      // TEXFORMAT_I8 8-bit grayscale
	G_IM_FMT_I,      // TEXFORMAT_I4 4-bit grayscale
	G_IM_FMT_CI,     // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	G_IM_FMT_CI,     // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	G_IM_FMT_CI,     // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	G_IM_FMT_CI,     // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes
};

int g_TexFormatDepths[] = {
	G_IM_SIZ_32b,  // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	G_IM_SIZ_16b,  // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	G_IM_SIZ_32b,  // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	G_IM_SIZ_16b,  // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	G_IM_SIZ_16b,  // TEXFORMAT_IA16 16-bit grayscale+alpha
	G_IM_SIZ_8b,   // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	G_IM_SIZ_4b,   // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	G_IM_SIZ_8b,   // TEXFORMAT_I8 8-bit grayscale
	G_IM_SIZ_4b,   // TEXFORMAT_I4 4-bit grayscale
	G_IM_SIZ_8b,   // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	G_IM_SIZ_4b,   // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	G_IM_SIZ_8b,   // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	G_IM_SIZ_4b,   // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes
};

int g_TexFormatLutModes[] = {
	G_TT_NONE,    // TEXFORMAT_RGBA32 32-bit RGBA (8/8/8/8)
	G_TT_NONE,    // TEXFORMAT_RGBA16 16-bit RGBA (5/5/5/1)
	G_TT_NONE,    // TEXFORMAT_RGB24 24-bit RGB (8/8/8)
	G_TT_NONE,    // TEXFORMAT_RGB15 15-bit RGB (5/5/5)
	G_TT_NONE,    // TEXFORMAT_IA16 16-bit grayscale+alpha
	G_TT_NONE,    // TEXFORMAT_IA8-bit grayscale+alpha (4/4)
	G_TT_NONE,    // TEXFORMAT_IA4 4-bit grayscale+alpha (3/1)
	G_TT_NONE,    // TEXFORMAT_I8 8-bit grayscale
	G_TT_NONE,    // TEXFORMAT_I4 4-bit grayscale
	G_TT_RGBA16,  // TEXFORMAT_RGBA16_CI8 16-bit 5551 paletted colour with 8-bit palette indexes
	G_TT_RGBA16,  // TEXFORMAT_RGBA16_CI4 16-bit 5551 paletted colour with 4-bit palette indexes
	G_TT_IA16,    // TEXFORMAT_IA16_CI8 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
	G_TT_IA16,    // TEXFORMAT_IA16_CI4 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes
};

/**
 * Inflate images (levels of detail) from a zlib-compressed texture.
 *
 * Zlib-compressed textures are always paletted and always use 16-bit colours.
 * The texture header contains palette information, then each image follows with
 * its own header and zlib compresed data.
 *
 * The texture header is:
 *
 * ffffffff nnnnnnnn [palette]
 *
 * f = pixel format (see TEXFORMAT constants)
 * n = number of colours in the palette minus 1
 * [palette] = 16 bits * number of colours
 *
 * Each images's header is:
 *
 * wwwwwwww hhhhhhhh [data]
 *
 * w = width in pixels
 * h = height in pixels
 * [data] = zlib compressed list of indices into the palette
 *
 * The zlib data is prefixed with the standard 5-byte rarezip header.
 */
int texInflateZlib(uint8_t *src, uint8_t *dst, bool hasloddata, int numlods, struct texpool *pool)
{
	int i;
	int imagebytesout;
	int numimages;
	bool writetocache;
	int format;
	bool foundthething;
	int totalbytesout;
	int width;
	int height;
	int numcolours;
	uint8_t *loddst;
	uint8_t *lodsrc;
	int lod;
	uint8_t scratch2[0x800];
	uint16_t palette[256];
	uint8_t scratch[5120];

	writetocache = false;
	totalbytesout = 0;

	texSetBitstring(src);

	if (hasloddata && numlods) {
		numimages = numlods;
	} else {
		numimages = 1;
	}

	pool->rightpos->numlods = numlods;
	pool->rightpos->hasloddata = hasloddata;

	if (hasloddata) {
		writetocache = true;

		for (i = 0; i < g_TexCacheCount; i++) {
			if (g_TexCacheItems[i].texturenum == pool->rightpos->texturenum) {
				writetocache = false;
			}
		}
	}

	format = texReadBits(8);
	numcolours = texReadBits(8) + 1;

	for (i = 0; i < numcolours; i++) {
		palette[i] = texReadBits(16);
	}

	foundthething = false;

	for (lod = 0; lod < numimages; lod++) {
		width = texReadBits(8);
		height = texReadBits(8);

		if (lod == 0) {
			pool->rightpos->width = width;
			pool->rightpos->height = height;
			pool->rightpos->unk0a = numcolours - 1;
			pool->rightpos->gbiformat = g_TexFormatGbiMappings[format];
			pool->rightpos->depth = g_TexFormatDepths[format];
			pool->rightpos->lutmodeindex = g_TexFormatLutModes[format] >> G_MDSFT_TEXTLUT;
		} else if (writetocache) {
			g_TexCacheItems[g_TexCacheCount].widths[lod - 1] = width;
			g_TexCacheItems[g_TexCacheCount].heights[lod - 1] = height;
		}

		if (rzipInflate(g_TexBitstring, scratch2, scratch) == 0) {

		}

		imagebytesout = texAlignIndices(scratch2, width, height, format, &dst[totalbytesout]);
		texSetBitstring(rzipGetSomething());

		if (hasloddata == true) {
			if (totalbytesout + imagebytesout > 0x800 || foundthething) {
				if (!foundthething) {
					pool->rightpos->numlods = lod;
					foundthething = true;
				}
			} else {
				texSwizzle(&dst[totalbytesout], width, height, format);
				totalbytesout += imagebytesout;
			}
		} else {
			totalbytesout += imagebytesout;
		}
	}

	if (writetocache) {
		g_TexCacheItems[g_TexCacheCount].texturenum = pool->rightpos->texturenum;

		g_TexCacheCount++;

		if (g_TexCacheCount >= ARRAYCOUNT(g_TexCacheItems)) {
			g_TexCacheCount = 0;
		}
	}

	// If the texture data doesn't contain multiple LODs but the header has a numlods value,
	// generate the other LODs by shrinking the image.
	if (!hasloddata) {
		if (numlods >= 2) {
			int tmpwidth = width;
			int tmpheight = height;

			lodsrc = dst;
			loddst = &dst[totalbytesout];

			for (lod = 1; lod < numlods; lod++) {
				imagebytesout = texShrinkPaletted(lodsrc, loddst, tmpwidth, tmpheight, format, palette, numcolours);

				if (totalbytesout + imagebytesout > 0x800) {
					pool->rightpos->numlods = lod;
					break;
				}

				texSwizzle(lodsrc, tmpwidth, tmpheight, format);

				totalbytesout += imagebytesout;

				tmpwidth = (tmpwidth + 1) >> 1;
				tmpheight = (tmpheight + 1) >> 1;

				lodsrc = loddst;
				loddst += imagebytesout;
			}

			texSwizzle(lodsrc, tmpwidth, tmpheight, format);
		} else {
			texSwizzle(dst, width, height, format);
		}
	}

	for (i = 0; i < numcolours; i++) {
		dst[totalbytesout + 0] = palette[i] >> 8;
		dst[totalbytesout + 1] = palette[i] & 0xff;
		totalbytesout += 2;
	}

	if (numcolours & 1) {
		dst[totalbytesout + 0] = dst[totalbytesout - 2];
		dst[totalbytesout + 1] = dst[totalbytesout - 1];
		totalbytesout += 2;
		pool->rightpos->unk0a++;
	}

	totalbytesout = (totalbytesout + 7) & ~7;

	return totalbytesout;
}

/**
 * Copy a list of palette indices to the dst buffer, but ensure each row is
 * aligned to an 8 byte boundary.
 *
 * Return the number of output bytes.
 */
int texAlignIndices(uint8_t *src, int width, int height, int format, uint8_t *dst)
{
	uint8_t *outptr = dst;
	int x;
	int y;
	int indicesperbyte;

	if (format == TEXFORMAT_RGBA16_CI8 || format == TEXFORMAT_IA16_CI8) {
		indicesperbyte = 1;
	} else if (format == TEXFORMAT_RGBA16_CI4 || format == TEXFORMAT_IA16_CI4) {
		indicesperbyte = 2;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x += indicesperbyte) {
			*outptr = *src;
			outptr++;
			src++;
		}

		outptr = (uint8_t *)(((uintptr_t)outptr + 7) & ~7);
	}

	return outptr - dst;
}

int texGetAverageRed(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4)
{
	int value = 0;

	value += (((colour1 >> 11) & 0x1f) << 3) | ((colour1 >> 13) & 7);
	value += (((colour2 >> 11) & 0x1f) << 3) | ((colour2 >> 13) & 7);
	value += (((colour3 >> 11) & 0x1f) << 3) | ((colour3 >> 13) & 7);
	value += (((colour4 >> 11) & 0x1f) << 3) | ((colour4 >> 13) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

int texGetAverageGreen(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4)
{
	int value = 0;

	value += (((colour1 >> 6) & 0x1f) << 3) | ((colour1 >> 8) & 7);
	value += (((colour2 >> 6) & 0x1f) << 3) | ((colour2 >> 8) & 7);
	value += (((colour3 >> 6) & 0x1f) << 3) | ((colour3 >> 8) & 7);
	value += (((colour4 >> 6) & 0x1f) << 3) | ((colour4 >> 8) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

int texGetAverageBlue(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4)
{
	int value = 0;

	value += (((colour1 >> 1) & 0x1f) << 3) | ((colour1 >> 3) & 7);
	value += (((colour2 >> 1) & 0x1f) << 3) | ((colour2 >> 3) & 7);
	value += (((colour3 >> 1) & 0x1f) << 3) | ((colour3 >> 3) & 7);
	value += (((colour4 >> 1) & 0x1f) << 3) | ((colour4 >> 3) & 7);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

int texGetAverageAlpha(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4)
{
	int value = 0
		+ (colour1 & 1 ? 0xff : 0)
		+ (colour2 & 1 ? 0xff : 0)
		+ (colour3 & 1 ? 0xff : 0)
		+ (colour4 & 1 ? 0xff : 0);

	value = (value + 2) >> 2;

	if (value < 0) {
		value = 0;
	}

	if (value > 0xff) {
		value = 0xff;
	}

	return value;
}

/**
 * Shrink a paletted texture to half its size by averaging each each 2x2 group
 * of pixels.
 *
 * Return the number of bytes written.
 */
int texShrinkPaletted(uint8_t *src, uint8_t *dst, int srcwidth, int srcheight, int format, uint16_t *palette, int numcolours)
{
	int j;
	int i;
	int alignedsrcwidth;
	int aligneddstwidth;
	int dstheight = (srcheight + 1) >> 1;
	uint16_t colour1;
	uint16_t colour2;
	uint16_t colour3;
	uint16_t colour4;
	int r;
	int g;
	int b;
	int a;
	int nextrow;
	int nextcol;
	int c;
	uint8_t *dst8;
	uint8_t *src8;
	uint8_t palette32[1024];

	switch (format) {
	case TEXFORMAT_RGBA16_CI8:
	case TEXFORMAT_IA16_CI8:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 7) & 0xff8;
		alignedsrcwidth = (srcwidth + 7) & 0xff8;
		break;
	case TEXFORMAT_RGBA16_CI4:
	case TEXFORMAT_IA16_CI4:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 15) & 0xff0;
		alignedsrcwidth = (srcwidth + 15) & 0xff0;
		break;
	}

	if (format == TEXFORMAT_RGBA16_CI8 || format == TEXFORMAT_RGBA16_CI4) {
		for (i = 0; i < numcolours; i++) {
			colour1 = palette[i];

			palette32[i * 4 + 0] = ((((colour1 >> 11) & 0x1f) * 8) | ((colour1 >> 13) & 7));
			palette32[i * 4 + 1] = ((((colour1 >> 6) & 0x1f) * 8) | ((colour1 >> 8) & 7));
			palette32[i * 4 + 2] = ((((colour1 >> 1) & 0x1f) * 8) | ((colour1 >> 3) & 7));
			palette32[i * 4 + 3] = ((colour1 & 1) ? 0xff : 0);
		}
	}

	dst8 = dst;
	src8 = src;

	switch (format) {
	case TEXFORMAT_RGBA16_CI8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				colour1 = palette[src8[j]];
				colour2 = palette[src8[nextcol]];
				colour3 = palette[src8[nextrow + j]];
				colour4 = palette[src8[nextrow + nextcol]];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 1] = texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a);
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_IA16_CI8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				colour1 = palette[src8[j]];
				colour2 = palette[src8[nextcol]];
				colour3 = palette[src8[nextrow + j]];
				colour4 = palette[src8[nextrow + nextcol]];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 1] = texFindClosestColourIndexIA(palette, numcolours, c, a);
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_RGBA16_CI4:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth >> 1 : 0;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				colour1 = palette[(src8[j >> 1] >> 4) & 0xf];
				colour2 = palette[src8[j >> 1] >> ((j + 1 < srcwidth ? 0 : 4)) & 0xf];
				colour3 = palette[(src8[nextrow + (j >> 1)] >> 4) & 0xf];
				colour4 = palette[src8[nextrow + (j >> 1)] >> ((j + 1 < srcwidth ? 0 : 4)) & 0xf];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 2] = texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a) << 4;

				colour1 = palette[(src8[(j + 2) >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[(j + 2) >> 1] >> (j + 3 < srcwidth ? 0 : 4)) & 0xf];
				colour3 = palette[(src8[nextrow + ((j + 2) >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + ((j + 2) >> 1)] >> (j + 3 < srcwidth ? 0 : 4)) & 0xf];

				r = texGetAverageRed(colour1, colour2, colour3, colour4);
				g = texGetAverageGreen(colour1, colour2, colour3, colour4);
				b = texGetAverageBlue(colour1, colour2, colour3, colour4);
				a = texGetAverageAlpha(colour1, colour2, colour3, colour4);

				dst8[j >> 2] |= texFindClosestColourIndexRGBA(palette32, numcolours, r, g, b, a) & 0xff;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	case TEXFORMAT_IA16_CI4:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth >> 1 : 0;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				// @bug: The brackets are wrong in colour2 and colour4 which
				// causes the index shift to be part of the ternary condition.
				// It's done correctly in TEXFORMAT_RGBA16_CI4 (above).
				// This buggy calculation is repeated further below.
				colour1 = palette[(src8[j >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[j >> 1] >> (j + 1 < srcwidth) ? 0 : 4) & 0xf];
				colour3 = palette[(src8[nextrow + (j >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + (j >> 1)] >> (j + 1 < srcwidth) ? 0 : 4) & 0xf];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 2] = texFindClosestColourIndexIA(palette, numcolours, c, a) << 4;

				colour1 = palette[(src8[(j + 2) >> 1] >> 4) & 0xf];
				colour2 = palette[(src8[(j + 2) >> 1] >> (j + 3 < srcwidth) ? 0 : 4) & 0xf];
				colour3 = palette[(src8[nextrow + ((j + 2) >> 1)] >> 4) & 0xf];
				colour4 = palette[(src8[nextrow + ((j + 2) >> 1)] >> (j + 3 < srcwidth) ? 0 : 4) & 0xf];

				c = ((((colour1 >> 8) & 0xff) + ((colour2 >> 8) & 0xff) + ((colour3 >> 8) & 0xff) + ((colour4 >> 8) & 0xff)) >> 2) & 0xff;
				a = ((((colour1 >> 0) & 0xff) + ((colour2 >> 0) & 0xff) + ((colour3 >> 0) & 0xff) + ((colour4 >> 0) & 0xff) + 1) >> 2) & 0xff;

				dst8[j >> 2] |= texFindClosestColourIndexIA(palette, numcolours, c, a) & 0xff;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	}

	return 0;
}

int texFindClosestColourIndexRGBA(uint8_t *palette, int numcolours, int r, int g, int b, int a)
{
	int minindex = 0;
	int minvalue = 99999999;
	int curvalue;
	int tmp;
	int i;

	for (i = 0; i < numcolours; i++) {
		tmp = palette[i * 4 + 0] - r;
		curvalue = tmp * tmp;

		tmp = palette[i * 4 + 1] - g;
		curvalue += tmp * tmp;

		tmp = palette[i * 4 + 2] - b;
		curvalue += tmp * tmp;

		tmp = palette[i * 4 + 3] - a;
		curvalue += tmp * tmp;

		if (curvalue < minvalue) {
			minindex = i;
			minvalue = curvalue;
		}
	}

	return minindex;
}

int texFindClosestColourIndexIA(uint16_t *palette, int numcolours, int intensity, int alpha)
{
	int bestindex = 0;
	int bestvalue = 99999999;
	int i;

	for (i = 0; i < numcolours; i++) {
		int value = palette[i];
		int a = ((value >> 8) & 0xff) - intensity;
		int b = (value & 0xff) - alpha;
		int sum = a * a + b * b;

		if (sum < bestvalue) {
			bestindex = i;
			bestvalue = sum;
		}
	}

	return bestindex;
}

/**
 * Inflate images (levels of detail) from a non-zlib texture.
 *
 * Each image can have a different compression method and pixel format,
 * which is described in a three byte header per image:
 *
 * ffffwwww wwwwhhhh hhhhcccc
 *
 * f = pixel format (see TEXFORMAT constants)
 * w = width in pixels
 * h = height in pixels
 * c = compression method (see TEXCOMPMETHOD constants)
 */
int texInflateNonZlib(uint8_t *src, uint8_t *dst, bool hasloddata, int numlods, struct texpool *pool)
{
	uint8_t scratch[0x2000];
	uint8_t lookup[0x1000];
	int i;
	int numimages;
	int width;
	int height;
	int compmethod;
	int j;
	int totalbytesout = 0;
	int imagebytesout;
	int format;
	int value;
	uint8_t *lodsrc;
	uint8_t *loddst;
	bool writetocache = false;

	texSetBitstring(src);

	numimages = hasloddata && numlods ? numlods : 1;

	pool->rightpos->numlods = numlods;
	pool->rightpos->hasloddata = hasloddata;

	if (hasloddata) {
		writetocache = true;

		for (i = 0; i < g_TexCacheCount; i++) {
			if (g_TexCacheItems[i].texturenum == pool->rightpos->texturenum) {
				writetocache = false;
			}
		}
	}

	for (i = 0; i < numimages; i++) {
		format = texReadBits(4);
		width = texReadBits(8);
		height = texReadBits(8);
		compmethod = texReadBits(4);

		if (i == 0) {
			pool->rightpos->width = width;
			pool->rightpos->height = height;
			pool->rightpos->gbiformat = g_TexFormatGbiMappings[format];
			pool->rightpos->depth = g_TexFormatDepths[format];
			pool->rightpos->lutmodeindex = g_TexFormatLutModes[format] >> G_MDSFT_TEXTLUT;
		} else if (writetocache) {
			g_TexCacheItems[g_TexCacheCount].widths[i - 1] = width;
			g_TexCacheItems[g_TexCacheCount].heights[i - 1] = height;
		}

		if (width * height > 0x2000) {
			return 0;
		}

		switch (compmethod) {
		case TEXCOMPMETHOD_UNCOMPRESSED0:
		case TEXCOMPMETHOD_UNCOMPRESSED1:
			imagebytesout = texReadUncompressed(&dst[totalbytesout], width, height, format);
			break;
		case TEXCOMPMETHOD_HUFFMAN:
			texInflateHuffman(scratch, g_TexFormatNumChannels[format] * width * height, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_HUFFMANPERHCHANNEL:
			for (j = 0; j < g_TexFormatNumChannels[format]; j++) {
				texInflateHuffman(&scratch[width * height * j], width * height, g_TexFormatChannelSizes[format]);
			}

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_RLE:
			texInflateRle(scratch, g_TexFormatNumChannels[format] * width * height);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_LOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			imagebytesout = texInflateLookup(width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_HUFFMANLOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			texInflateHuffman(scratch, width * height, value);
			imagebytesout = texInflateLookupFromBuffer(scratch, width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_RLELOOKUP:
			value = texBuildLookup(lookup, g_TexFormatBitsPerPixel[format]);
			texInflateRle(scratch, width * height);
			imagebytesout = texInflateLookupFromBuffer(scratch, width, height, &dst[totalbytesout], lookup, value, format);
			break;
		case TEXCOMPMETHOD_HUFFMANBLUR:
			value = texReadBits(3);
			texInflateHuffman(scratch, g_TexFormatNumChannels[format] * width * height, g_TexFormatChannelSizes[format]);
			texBlur(scratch, width, g_TexFormatNumChannels[format] * height, value, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		case TEXCOMPMETHOD_RLEBLUR:
			value = texReadBits(3);
			texInflateRle(scratch, g_TexFormatNumChannels[format] * width * height);
			texBlur(scratch, width, g_TexFormatNumChannels[format] * height, value, g_TexFormatChannelSizes[format]);

			if (g_TexFormatHas1BitAlpha[format]) {
				texReadAlphaBits(&scratch[width * height * 3], width * height);
			}

			imagebytesout = texChannelsToPixels(scratch, width, height, &dst[totalbytesout], format);
			break;
		}

		if (hasloddata == true) {
			texSwizzle(&dst[totalbytesout], width, height, format);
		}

		imagebytesout = (imagebytesout + 7) & ~7;
		totalbytesout += imagebytesout;

		if (g_TexAccumNumBits == 0) {
			g_TexBitstring++;
		} else {
			g_TexAccumNumBits = 0;
		}
	}

	if (writetocache) {
		g_TexCacheItems[g_TexCacheCount].texturenum = pool->rightpos->texturenum;

		g_TexCacheCount++;

		// Resetting this variable to 0 here suggests that the g_TexCacheItems
		// array is used in a circular manner, and that g_TexCacheCount is just
		// the index of the oldest/next element. But earlier in this function
		// there's a loop that iterates up to g_TexCacheCount, which doesn't
		// make any sense if this value is used as a pointer in a circular list.
		// Could be a @bug, or maybe they intended to reset the cache every time
		// it fills up.
		if (g_TexCacheCount >= ARRAYCOUNT(g_TexCacheItems)) {
			g_TexCacheCount = 0;
		}
	}

	// If the texture data doesn't contain multiple LODs but the header has a numlods value,
	// generate the other LODs by shrinking the image.
	if (!hasloddata) {
		if (numlods >= 2) {
			int tmpwidth = width;
			int tmpheight = height;

			lodsrc = dst;
			if (1);
			loddst = &dst[totalbytesout];

			for (i = 1; i < numlods; i++) {
				imagebytesout = texShrinkNonPaletted(lodsrc, loddst, tmpwidth, tmpheight, format);

				texSwizzle(lodsrc, tmpwidth, tmpheight, format);

				totalbytesout += imagebytesout;

				tmpwidth = (tmpwidth + 1) >> 1;
				tmpheight = (tmpheight + 1) >> 1;

				lodsrc = loddst;
				loddst += imagebytesout;
			}

			texSwizzle(lodsrc, tmpwidth, tmpheight, format);
		} else {
			texSwizzle(dst, width, height, format);
		}
	}

	return totalbytesout;
}

/**
 * Shrink a non-paletted texture to half its size by averaging each each 2x2
 * group of pixels.
 *
 * Return the number of bytes written.
 *
 * If the source width is an odd number, the destination's final column is
 * calculated by sampling the final source column twice. Likewise for the height.
 */
int texShrinkNonPaletted(uint8_t *src, uint8_t *dst, int srcwidth, int srcheight, int format)
{
	int i;
	int j;
	int alignedsrcwidth;
	int aligneddstwidth;
	uint32_t *dst32 = (uint32_t *) dst;
	uint16_t *dst16 = (uint16_t *) dst;
	uint8_t *dst8 = dst;
	uint32_t *src32 = (uint32_t *) src;
	uint16_t *src16 = (uint16_t *) src;
	uint8_t *src8 = src;
	int dstheight = (srcheight + 1) >> 1;
	int r;
	int g;
	int b;
	int a;
	int c;
	uint32_t tl32;
	uint32_t tr32;
	uint32_t bl32;
	uint32_t br32;
	uint16_t tl16;
	uint16_t tr16;
	uint16_t bl16;
	uint16_t br16;
	uint8_t tl8;
	uint8_t tr8;
	uint8_t bl8;
	uint8_t br8;
	int nextrow;
	int nextcol;

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 3) & 0xffc;
		alignedsrcwidth = (srcwidth + 3) & 0xffc;
		break;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
	case TEXFORMAT_IA16:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 3) & 0xffc;
		alignedsrcwidth = (srcwidth + 3) & 0xffc;
		break;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 7) & 0xff8;
		alignedsrcwidth = (srcwidth + 7) & 0xff8;
		break;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		aligneddstwidth = (((srcwidth + 1) >> 1) + 15) & 0xff0;
		alignedsrcwidth = (srcwidth + 15) & 0xff0;
		break;
	}

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl32 = src32[j];
				tr32 = src32[nextcol];
				bl32 = src32[nextrow + j];
				br32 = src32[nextrow + nextcol];

				r = ((((tl32 >> 24) & 0xff) + ((tr32 >> 24) & 0xff) + ((bl32 >> 24) & 0xff) + ((br32 >> 24) & 0xff)) >> 2) & 0xff;
				g = ((((tl32 >> 16) & 0xff) + ((tr32 >> 16) & 0xff) + ((bl32 >> 16) & 0xff) + ((br32 >> 16) & 0xff)) >> 2) & 0xff;
				b = ((((tl32 >>  8) & 0xff) + ((tr32 >>  8) & 0xff) + ((bl32 >>  8) & 0xff) + ((br32 >>  8) & 0xff)) >> 2) & 0xff;
				a = ((((tl32 >>  0) & 0xff) + ((tr32 >>  0) & 0xff) + ((bl32 >>  0) & 0xff) + ((br32 >>  0) & 0xff) + 1) >> 2) & 0xff;

				dst32[j >> 1] = r << 24 | g << 16 | b << 8 | a;
			}

			dst32 += aligneddstwidth;
			src32 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl16 = src16[j];
				tr16 = src16[nextcol];
				bl16 = src16[nextrow + j];
				br16 = src16[nextrow + nextcol];

				r = ((((tl16 >> 11) & 0x1f) + ((tr16 >> 11) & 0x1f) + ((bl16 >> 11) & 0x1f) + ((br16 >> 11) & 0x1f)) >> 2) & 0x1f;
				g = ((((tl16 >>  6) & 0x1f) + ((tr16 >>  6) & 0x1f) + ((bl16 >>  6) & 0x1f) + ((br16 >>  6) & 0x1f)) >> 2) & 0x1f;
				b = ((((tl16 >>  1) & 0x1f) + ((tr16 >>  1) & 0x1f) + ((bl16 >>  1) & 0x1f) + ((br16 >>  1) & 0x1f)) >> 2) & 0x1f;
				a = ((((tl16 >>  0) & 0x01) + ((tr16 >>  0) & 0x01) + ((bl16 >>  0) & 0x01) + ((br16 >>  0) & 0x01) + 2) >> 2) & 0x01;

				dst16[j >> 1] = r << 11 | g << 6 | b << 1 | a;
			}

			dst16 += aligneddstwidth;
			src16 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 2;
	case TEXFORMAT_IA16:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl16 = src16[j];
				tr16 = src16[nextcol];
				bl16 = src16[nextrow + j];
				br16 = src16[nextrow + nextcol];

				c = ((tl16 >> 8) & 0xff) + ((tr16 >> 8) & 0xff) + ((bl16 >> 8) & 0xff) + ((br16 >> 8) & 0xff) + 2;
				c = c >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xff) {
					c = 0xff;
				}

				a = (tl16 & 0xff) + (tr16 & 0xff) + (bl16 & 0xff) + (br16 & 0xff) + 2; // optimised out

				// @bug: Should be a >> 2
				a = c >> 2;

				if (a < 0) {
					a = 0;
				}

				if (a > 0xff) {
					a = 0xff;
				}

				dst16[j >> 1] = c << 8 | a;
			}

			dst16 += aligneddstwidth;
			src16 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth * 2;
	case TEXFORMAT_IA8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				tl8 = src8[j];
				tr8 = src8[nextcol];
				bl8 = src8[nextrow + j];
				br8 = src8[nextrow + nextcol];

				c = (((tl8 >> 4) & 0xf) + ((tr8 >> 4) & 0xf) + ((bl8 >> 4) & 0xf) + ((br8 >> 4) & 0xf) + 2) >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xf) {
					c = 0xf;
				}

				c <<= 4;

				a = ((tl8 & 0xf) + (tr8 & 0xf) + (bl8 & 0xf) + (br8 & 0xf) + 2) >> 2;

				if (a < 0) {
					a = 0;
				}

				if (a > 0xf) {
					a = 0xf;
				}

				dst8[j >> 1] = c | a;
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_I8:
		for (i = 0; i < srcheight; i += 2) {
			nextrow = i + 1 < srcheight ? alignedsrcwidth : 0;

			for (j = 0; j < alignedsrcwidth; j += 2) {
				nextcol = j + 1 < srcwidth ? j + 1 : j;

				// @bug: The code reads from uninitialised variable br8.
				// This is assumed to be due to a double write to bl8 as shown:
				tl8 = src8[j];
				tr8 = src8[nextcol];
				bl8 = src8[nextrow + j]; // optimised out
				bl8 = src8[nextrow + nextcol];

				c = (tl8 + tr8 + bl8 + br8 + 2) >> 2;

				if (c < 0) {
					c = 0;
				}

				if (c > 0xff) {
					c = 0xff;
				}

				dst8[j >> 1] = c;
			}

			dst8 += aligneddstwidth;
			src8 += alignedsrcwidth * 2;
		}

		return dstheight * aligneddstwidth;
	case TEXFORMAT_IA4:
		for (i = 0; i < srcheight; i += 2) {
			nextcol = i + 1;

			for (j = 0; j < alignedsrcwidth; j += 4) {
				tl8 = src8[j >> 1];
				tr8 = src8[(nextcol < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1)];
				bl8 = src8[(j >> 1) + 1];
				br8 = src8[(nextcol < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1) + 1];

				c = (((((tl8 >> 5) & 7) + ((tl8 >> 1) & 7) + ((tr8 >> 5) & 7) + ((tr8 >> 1) & 7)) << 3) & 0xe0)
					| (((((bl8 >> 5) & 7) + ((bl8 >> 1) & 7) + ((br8 >> 5) & 7) + ((br8 >> 1) & 7)) >> 1) & 0xe);

				a = (((((tl8 >> 4) & 1) + (tl8 & 1) + ((tr8 >> 4) & 1) + (tr8 & 1) + 1) << 2) & 0x10)
					| (((((bl8 >> 4) & 1) + (bl8 & 1) + ((br8 >> 4) & 1) + (br8 & 1) + 1) >> 2) & 1);

				dst8[j >> 2] = c | a;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	case TEXFORMAT_I4:
		for (i = 0; i < srcheight; i += 2) {
			for (j = 0; j < alignedsrcwidth; j += 4) {
				tl8 = src8[j >> 1];
				tr8 = src8[(i + 1 < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1)];
				bl8 = src8[(j >> 1) + 1];
				br8 = src8[(i + 1 < srcheight ? (alignedsrcwidth >> 1) : 0) + (j >> 1) + 1];

				c = ((((tl8 >> 4) & 0xf) + (tl8 & 0xf) + ((tr8 >> 4) & 0xf) + (tr8 & 0xf)) << 2) & 0xf0;
				a = ((((bl8 >> 4) & 0xf) + (bl8 & 0xf) + ((br8 >> 4) & 0xf) + (br8 & 0xf)) >> 2) & 0xf;

				dst8[j >> 2] = c | a;
			}

			dst8 += aligneddstwidth >> 1;
			src8 += alignedsrcwidth;
		}

		return (aligneddstwidth >> 1) * dstheight;
	}

	return 0;
}

/**
 * Inflate Huffman data.
 *
 * This function operates on single channels rather than whole colours.
 * For example, for an RGBA32 image this function may be called once for each
 * channel with chansize = 256. This means the resulting data is in the format
 * RRR...GGG...BBB...AAA..., and the caller must convert it into a proper pixel
 * format.
 *
 * A typical Huffman implementation stores a tree, where each node contains
 * the lookup value and its frequency (number of uses). However, Rare's
 * implementation only stores a list of frequencies. It uses the chansize
 * to know how many values there are.
 */
void texInflateHuffman(uint8_t *dst, int numiterations, int chansize)
{
	uint16_t frequencies[2048];
	int16_t nodes[2048][2];
	int i;
	int rootindex;
	int sum;
	uint16_t minfreq1;
	uint16_t minfreq2;
	int minindex1;
	int minindex2;
	bool done = false;

	// Read the frequencies list
	for (i = 0; i < chansize; i++) {
		frequencies[i] = texReadBits(8);
	}

	// Initialise the tree
	for (i = 0; i < 2048; i++) {
		nodes[i][0] = -1;
		nodes[i][1] = -1;
	}

	// Find the two smallest frequencies
	minfreq1 = 9999;
	minfreq2 = 9999;

	for (i = 0; i < chansize; i++) {
		if (frequencies[i] < minfreq1) {
			if (minfreq2 < minfreq1) {
				minfreq1 = frequencies[i];
				minindex1 = i;
			} else {
				minfreq2 = frequencies[i];
				minindex2 = i;
			}
		} else if (frequencies[i] < minfreq2) {
			minfreq2 = frequencies[i];
			minindex2 = i;
		}
	}

	// Build the tree.
	// For each node in tree, a branch value < 10000 means this branch
	// leads to another node, and the value is the target node's index.
	// A branch value >= 10000 means the branch is a leaf node,
	// and the value is the channel value + 10000.
	while (!done) {
		sum = frequencies[minindex1] + frequencies[minindex2];

		if (sum == 0) {
			sum = 1;
		}

		frequencies[minindex1] = 9999;
		frequencies[minindex2] = 9999;

		if (nodes[minindex1][0] < 0 && nodes[minindex1][1] < 0) {
			nodes[minindex1][0] = minindex1 + 10000;
			rootindex = minindex1;
			frequencies[minindex1] = sum;

			if (nodes[minindex2][0] < 0 && nodes[minindex2][1] < 0) {
				nodes[minindex1][1] = minindex2 + 10000;
			} else {
				nodes[minindex1][1] = minindex2;
			}
		} else if (nodes[minindex2][0] < 0 && nodes[minindex2][1] < 0) {
			nodes[minindex2][0] = minindex2 + 10000;
			rootindex = minindex2;
			frequencies[minindex2] = sum;

			if (nodes[minindex1][0] < 0 && nodes[minindex1][1] < 0) {
				nodes[minindex2][1] = minindex1 + 10000;
			} else {
				nodes[minindex2][1] = minindex1;
			}
		} else {
			for (rootindex = 0; nodes[rootindex][0] >= 0 || nodes[rootindex][1] >= 0 || frequencies[rootindex] < 9999; rootindex++);

			frequencies[rootindex] = sum;
			nodes[rootindex][0] = minindex1;
			nodes[rootindex][1] = minindex2;
		}

		// Find the two smallest frequencies again for the next iteration
		minfreq1 = 9999;
		minfreq2 = 9999;

		for (i = 0; i < chansize; i++) {
			if (frequencies[i] < minfreq1) {
				if (minfreq1 > minfreq2) {
					minfreq1 = frequencies[i];
					minindex1 = i;
				} else {
					minfreq2 = frequencies[i];
					minindex2 = i;
				}
			} else if (frequencies[i] < minfreq2) {
				minfreq2 = frequencies[i];
				minindex2 = i;
			}
		}

		if (minfreq1 == 9999 || minfreq2 == 9999) {
			done = true;
		}
	}

	// Read bits off the bitstring, traverse the tree
	// and write the channel values to dst
	for (i = 0; i < numiterations; i++) {
		int indexorvalue = rootindex;

		while (indexorvalue < 10000) {
			indexorvalue = nodes[indexorvalue][texReadBits(1)];
		}

		if (chansize <= 256) {
			dst[i] = indexorvalue - 10000;
		} else {
			uint16_t *tmp = (uint16_t *)dst;
			tmp[i] = indexorvalue - 10000;
		}
	}
}

/**
 * Inflate runlength-encoded data.
 *
 * This data consists of a 10 bit header followed by a list of directives,
 * where each directive can either be a literal block or a repeat (run) of
 * blocks within a sliding window.
 *
 * The header format is:
 *
 * 3 bits btfieldsize: The size in bits of the backtrack distance fields
 * 3 bits rlfieldsize: The size in bits of the runlen fields
 * 4 bits blocksize: The size in bits of each block of data
 *
 * In the data, the first bit is 0 if it's a literal block or 1 if it's a run.
 *
 * For literal blocks, the next <blocksize> bits should be read and appended to
 * the output stream.
 *
 * For runs, the next <btfieldsize> bits are the backtrack length (in blocks)
 * plus one, and the next <rlfieldsize> bits are the run length (in blocks)
 * minus a calculated fudge value.
 *
 * The fudge value is calculated based on the field sizes. For small runs it is
 * more space efficient to use multiple literal directives rather than a run
 * directive. Because of this, smaller runs are not used and the run lengths
 * in the data can be offset accordingly - this offset is the fudge value.
 *
 * Every run must be followed by a literal block without the 1-bit marker.
 * The algorithm does not support back to back runs.
 */
void texInflateRle(uint8_t *dst, int blockstotal)
{
	int btfieldsize = texReadBits(3);
	int rlfieldsize = texReadBits(3);
	int blocksize = texReadBits(4);
	int cost;
	int fudge;
	int blocksdone;
	int i;

	// Calculate the fudge value
	cost = btfieldsize + rlfieldsize + blocksize + 1;
	fudge = 0;

	while (cost > 0) {
		cost = cost - blocksize - 1;
		fudge++;
	}

	blocksdone = 0;

	while (blocksdone < blockstotal) {
		if (texReadBits(1) == 0) {
			// Found a literal directive
			if (blocksize <= 8) {
				dst[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			} else {
				uint16_t *tmp = (uint16_t *)dst;
				tmp[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			}
		} else {
			// Found a run directive
			int startblockindex = blocksdone - texReadBits(btfieldsize) - 1;
			int runnumblocks = texReadBits(rlfieldsize) + fudge;

			if (blocksize <= 8) {
				for (i = startblockindex; i < startblockindex + runnumblocks; i++) {
					dst[blocksdone] = dst[i];
					blocksdone++;
				}

				// The next instruction must be a literal
				dst[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			} else {
				uint16_t *tmp = (uint16_t *)dst;

				for (i = startblockindex; i < startblockindex + runnumblocks; i++) {
					tmp[blocksdone] = tmp[i];
					blocksdone++;
				}

				// The next instruction must be a literal
				tmp[blocksdone] = texReadBits(blocksize);
				blocksdone++;
			}
		}
	}
}

/**
 * Populate a lookup table by reading it out of the bit string.
 *
 * The first 11 bits denote the number of colours in the lookup table.
 * The data following this is a list of colours, where each colour is sized
 * according to the texture's format.
 *
 * This function does NOT work with pixel formats of 8 bits or less.
 */
int texBuildLookup(uint8_t *lookup, int bitsperpixel)
{
	int numcolours = texReadBits(11);
	int i;

	if (bitsperpixel <= 16) {
		uint16_t *dst = (uint16_t *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(bitsperpixel);
		}
	} else if (bitsperpixel <= 24) {
		uint32_t *dst = (uint32_t *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(bitsperpixel);
		}
	} else {
		uint32_t *dst = (uint32_t *)lookup;

		for (i = 0; i < numcolours; i++) {
			dst[i] = texReadBits(24) << 8 | texReadBits(bitsperpixel - 24);
		}
	}

	return numcolours;
}

int texGetBitSize(int decimal)
{
	int count = 0;

	decimal--;

	while (decimal > 0) {
		decimal >>= 1;
		count++;
	}

	return count;
}

void texReadAlphaBits(uint8_t *dst, int count)
{
	int i;

	for (i = 0; i < count; i++) {
		dst[i] = texReadBits(1);
	}
}

/**
 * Read pixel data from the bitstream and write to dst,
 * ensuring each row is aligned according to the pixel format.
 *
 * Return the number of output bytes.
 */
int texReadUncompressed(uint8_t *dst, int width, int height, int format)
{
	uint32_t *dst32 = (uint32_t *)(((uintptr_t)dst + 0xf) & ~0xf);
	uint16_t *dst16 = (uint16_t *)(((uintptr_t)dst + 7) & ~7);
	uint8_t *dst8 = (uint8_t *)(((uintptr_t)dst + 7) & ~7);
	int x;
	int y;

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = texReadBits(16) << 16;
				dst32[x] |= texReadBits(16);
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = texReadBits(24) << 8 | 0xff;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = texReadBits(16);
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = texReadBits(15) << 1 | 1;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = texReadBits(8);
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = texReadBits(8);
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Read grouped channel values and convert it to a proper pixel format.
 *
 * For example, for RGBA32 images the input is in the format
 * RRR...GGG...BBB...AAA... and is converted to RGBARGBARGBA...
 *
 * The existence and size of the channels depends on the pixel format.
 */
int texChannelsToPixels(uint8_t *src, int width, int height, uint8_t *dst, int format)
{
	uint32_t *dst32 = (uint32_t *)dst;
	uint16_t *dst16 = (uint16_t *)dst;
	uint8_t *dst8 = (uint8_t *)dst;
	int x;
	int y;
	int pos = 0;
	int mult = width * height;

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = src[pos] << 24 | src[pos + mult] << 16 | src[pos + mult * 2] << 8 | src[pos + mult * 3];
				pos++;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = src[pos] << 24 | src[pos + mult] << 16 | src[pos + mult * 2] << 8 | 0xff;
				pos++;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = PD_BE16(src[pos] << 11 | src[pos + mult] << 6 | src[pos + mult * 2] << 1 | src[pos + mult * 3]);
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = src[pos] << 8 | src[pos + mult];
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = PD_BE16(src[pos] << 11 | src[pos + mult] << 6 | src[pos + mult * 2] << 1 | 1);
				pos++;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
		for (y = 0; y < height; y++) {
			if ((width + 7) & 0xff8);

			for (x = 0; x < width; x++) {
				dst8[x] = src[pos] << 4 | src[pos + mult];
				pos++;
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = src[pos];
				pos++;
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
		for (y = 0; y < height; y++) {
			if ((width + 15) & 0xff0);

			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = src[pos] << 5 | src[pos + mult * 3] << 4 | src[pos + 1] << 1 | src[pos + mult * 3 + 1];
				pos += 2;
			}

			if (width & 1) {
				pos--;
			}

			dst8 += (width + 15) & 0xff0;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = src[pos] << 4 | src[pos + 1];
				pos += 2;
			}

			if (width & 1) {
				pos--;
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Inflate a texture using the provided lookup table.
 *
 * The lookup table is a bitstring of colours in the pixel format described by
 * the format argument. The number of colours in the lookup table is given by
 * the numcolours argument.
 *
 * The data in the global source bitstring is expected to be a tightly packed
 * list of indices into the lookup table. The number of bits for each index
 * is calculated based on the number of colours in the lookup table. For
 * example, if the lookup table contains 8 colours then the indices will be 0-7,
 * which requires 3 bits per index.
 *
 * Return the number of bytes written to dst.
 */
int texInflateLookup(int width, int height, uint8_t *dst, uint8_t *lookup, int numcolours, int format)
{
	uint32_t *lookup32 = (uint32_t *)lookup;
	uint16_t *lookup16 = (uint16_t *)lookup;
	uint32_t *dst32 = (uint32_t *)dst;
	uint16_t *dst16 = (uint16_t *)dst;
	uint8_t *dst8 = (uint8_t *)dst;
	int x;
	int y;
	int bitspercolour = texGetBitSize(numcolours);

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = lookup32[texReadBits(bitspercolour)];
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst32[x] = lookup32[texReadBits(bitspercolour)] << 8;
			}

			dst32 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = lookup16[texReadBits(bitspercolour)];
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst16[x] = lookup16[texReadBits(bitspercolour)] << 1 | 1;
			}

			dst16 += (width + 3) & 0xffc;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				dst8[x] = lookup16[texReadBits(bitspercolour)];
			}

			dst8 += (width + 7) & 0xff8;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				dst8[x >> 1] = lookup16[texReadBits(bitspercolour)] << 4;

				if (x + 1 < width) {
					dst8[x >> 1] |= lookup[(texReadBits(bitspercolour) * 2) + 1];
				}
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * Like texInflateLookup, but the indices are provided in the src argument
 * as uint8_ts or uint16_ts rather than read from the global bitstring as tightly packed
 * bits.
 *
 * Whether uint8_ts or uint16_ts are expected depends on whether the number of colours
 * in the lookup table. If there are more than 256 colours then it must use
 * uint16_ts, otherwise it expects uint8_ts.
 */
int texInflateLookupFromBuffer(uint8_t *src, int width, int height, uint8_t *dst, uint8_t *lookup, int numcolours, int format)
{
	int x;
	int y;
	uint32_t *lookup32 = (uint32_t *)lookup;
	uint16_t *lookup16 = (uint16_t *)lookup;
	uint8_t *src8;
	uint16_t *src16;
	uint32_t *dst32 = (uint32_t *)dst;
	uint16_t *dst16 = (uint16_t *)dst;
	uint8_t *dst8 = (uint8_t *)dst;

	if (numcolours <= 256) {
		src8 = (uint8_t *)src;
	} else {
		src16 = (uint16_t *)src;
	}

	switch (format) {
	case TEXFORMAT_RGBA32:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst32[x] = lookup32[src8[x]];
				} else {
					dst32[x] = lookup32[src16[x]];
				}
			}

			dst32 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGB24:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst32[x] = lookup32[src8[x]] << 8 | 0xff;
				} else {
					dst32[x] = lookup32[src16[x]] << 8 | 0xff;
				}
			}

			dst32 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 4;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_IA16:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst16[x] = PD_BE16(lookup16[src8[x]]);
				} else {
					dst16[x] = PD_BE16(lookup16[src16[x]]);
				}
			}

			dst16 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_RGB15:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst16[x] = PD_BE16(lookup16[src8[x]] << 1 | 1);
				} else {
					dst16[x] = PD_BE16(lookup16[src16[x]] << 1 | 1);
				}
			}

			dst16 += (width + 3) & 0xffc;
			src8 += width;
			src16 += width;
		}

		return ((width + 3) & 0xffc) * height * 2;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
		for (y = 0; y < height; y++) {
			if ((width + 7) & 0xff8);

			for (x = 0; x < width; x++) {
				if (numcolours <= 256) {
					dst8[x] = lookup16[src8[x]];
				} else {
					dst8[x] = lookup16[src16[x]];
				}
			}

			dst8 += (width + 7) & 0xff8;
			src8 += width;
			src16 += width;
		}

		return ((width + 7) & 0xff8) * height;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x += 2) {
				if (numcolours <= 256) {
					dst8[x >> 1] = lookup16[src8[x]] << 4 | lookup16[src8[x + 1]];
				} else {
					dst8[x >> 1] = lookup16[src16[x]] << 4 | lookup16[src16[x + 1]];
				}
			}

			dst8 += ((width + 15) & 0xff0) >> 1;
			src8 += width;
			src16 += width;
		}

		return (((width + 15) & 0xff0) >> 1) * height;
	}

	return 0;
}

/**
 * For every second row, swap every pair of words within that row.
 */
int texConfigToFormat(const struct textureconfig *tex)
{
	switch (tex->format) {
		case G_IM_FMT_I:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_I4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_I8;
				default:
					break;
			}
			break;
		case G_IM_FMT_IA:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_IA4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_IA8;
				case G_IM_SIZ_16b:
					return TEXFORMAT_IA16;
				default:
					break;
			}
			break;
		case G_IM_FMT_CI:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_IA16_CI4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_IA16_CI8;
				default:
					break;
			}
			break;
		case G_IM_FMT_RGBA:
			switch (tex->depth) {
				case G_IM_SIZ_4b:
					return TEXFORMAT_RGBA16_CI4;
				case G_IM_SIZ_8b:
					return TEXFORMAT_RGBA16_CI8;
				case G_IM_SIZ_16b:
					return TEXFORMAT_RGBA16;
				case G_IM_SIZ_32b:
					return TEXFORMAT_RGBA32;
				default:
					break;
			}
			break;
		default:
			break;
	}

	return TEXFORMAT_I8;
}

void texSwizzle(uint8_t *dst, int width, int height, int format)
{
	/**
	 * The N64 GPU wants swizzled textures, we don't.
	 * Thus this function is stubbed out, but its functionality is still made
	 * available for unswizzling the embedded textures in preprocess.c.
	 */
}

void texSwizzleInternal(uint8_t *dst, int width, int height, int format, uint32_t dstlen)
{
	int x;
	int y;
	int wordsperrow;
	uint32_t *row = (uint32_t *)dst;
	uint32_t *end = (uint32_t *)(dst + dstlen);
	int tmp;

	switch (format) {
	case TEXFORMAT_RGBA32:
	case TEXFORMAT_RGB24:
		wordsperrow = (width + 3) & 0xffc;
		break;
	case TEXFORMAT_RGBA16:
	case TEXFORMAT_RGB15:
	case TEXFORMAT_IA16:
		wordsperrow = ((width + 3) & 0xffc) >> 1;
		break;
	case TEXFORMAT_IA8:
	case TEXFORMAT_I8:
	case TEXFORMAT_RGBA16_CI8:
	case TEXFORMAT_IA16_CI8:
		wordsperrow = ((width + 7) & 0xff8) >> 2;
		break;
	case TEXFORMAT_IA4:
	case TEXFORMAT_I4:
	case TEXFORMAT_RGBA16_CI4:
	case TEXFORMAT_IA16_CI4:
		wordsperrow = ((width + 0xf) & 0xff0) >> 3;
		break;
	}

	row += wordsperrow;

	if (format == TEXFORMAT_RGBA32 || format == TEXFORMAT_RGB24) {
		for (y = 1; y < height; y += 2) {
			for (x = 0; x < wordsperrow && row + x < end; x += 4) {
				tmp = row[x + 0];
				row[x + 0] = row[x + 2];
				row[x + 2] = tmp;

				tmp = row[x + 1];
				row[x + 1] = row[x + 3];
				row[x + 3] = tmp;
			}

			row += wordsperrow * 2;
		}
	} else {
		for (y = 1; y < height; y += 2) {
			for (x = 0; x < wordsperrow && row + x < end; x += 2) {
				tmp = row[x + 0];
				row[x + 0] = row[x + 1];
				row[x + 1] = tmp;
			}

			row += wordsperrow * 2;
		}
	}
}

/**
 * Blur the pixels in the image with the surrounding pixels.
 */
void texBlur(uint8_t *pixels, int width, int height, int method, int chansize)
{
	int x;
	int y;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			int cur = pixels[y * width + x] + chansize * 2;
			int left = x > 0 ? pixels[y * width + x - 1] : 0;
			int above = y > 0 ? pixels[(y - 1) * width + x] : 0;
			int aboveleft = x > 0 && y > 0 ? pixels[(y - 1) * width + x - 1] : 0;

			switch (method) {
			case 0:
				pixels[y * width + x] = (cur + left) % chansize;
				break;
			case 1:
				pixels[y * width + x] = (cur + above) % chansize;
				break;
			case 2:
				pixels[y * width + x] = (cur + aboveleft) % chansize;
				break;
			case 3:
				pixels[y * width + x] = (cur + (left + above - aboveleft)) % chansize;
				break;
			case 4:
				pixels[y * width + x] = (cur + ((above - aboveleft) / 2 + left)) % chansize;
				break;
			case 5:
				pixels[y * width + x] = (cur + ((left - aboveleft) / 2 + above)) % chansize;
				break;
			case 6:
				pixels[y * width + x] = (cur + ((left + above) / 2)) % chansize;
				break;
			}
		}
	}
}

void texInitPool(struct texpool *pool, uint8_t *start, int len)
{
	pool->start = start;
	pool->end = (struct tex *)(start + len);
	pool->leftpos = start;
	pool->rightpos = (struct tex *)((uintptr_t)start + len);
}

struct tex *texFindInPool(int texturenum, struct texpool *pool)
{
	struct tex *end;
	struct tex *cur;
	int i;

	if (pool == NULL) {
		pool = &g_TexSharedPool;
	}

	if (pool == &g_TexSharedPool) {
		cur = pool->head;

		while (cur) {
			if (cur->texturenum == texturenum) {
				return cur;
			}

			if (!cur->next) {
				return NULL;
			}

			cur = (struct tex *) (k_ptr_t)(cur->next);
		}

		return NULL;
	}

	end = pool->end;
	cur = pool->rightpos;

	while (cur < end) {
		if (cur->texturenum == texturenum) {
			return cur;
		}

		cur++;
	}

	return NULL;
}

int texGetPoolFreeBytes(struct texpool *pool)
{
	return (uintptr_t) pool->rightpos - (uintptr_t) pool->leftpos;
}

uint8_t *texGetPoolLeftPos(struct texpool *pool)
{
	return pool->leftpos;
}

void texLoadFromDisplayList(Gfx *gdl, struct texpool *pool, int arg2)
{
	uint8_t *bytes = (uint8_t *)gdl;
	uint8_t ofs = 4;
#ifdef PLATFORM_64BIT
	ofs = 8;
#endif


	while (bytes[GFX_W0_BYTE(0)] != (uint8_t)G_ENDDL) {
		// Look for GBI sequence: fd...... abcd....
		if (bytes[GFX_W0_BYTE(0)] == G_SETTIMG && bytes[GFX_W1_BYTE(0)] == 0xab && bytes[GFX_W1_BYTE(1)] == 0xcd) {
			texLoad((texnum_t *)((uintptr_t)bytes + ofs), pool);
		}

		bytes += sizeof(Gfx);
	}
}

extern uint8_t *_texturesdataSegmentRomStart;

/**
 * Load and decompress a texture from ROM.
 *
 * The given pointer points to a word which determines what to load.
 * The formats of the word are:
 *
 *     abcdxxxx -> load texture number xxxx
 *     0000xxxx -> load texture number xxxx
 *     (memory address) -> the texture is already loaded, so do nothing
 *
 * After loading and decompressing the texture, the value that's pointed to is
 * changed to be a pointer to... something.
 *
 * There are two types of textures:
 *
 * - Zlib-compressed textures, which are always paletted
 * - Non-zlib textures, which use a variety of (non-zlib) compression methods
 *   and are sometimes paletted
 *
 * Both types have support for multiple levels of detail (ie. multiple images
 * of varying size) within each texture. There are enough bits in the header
 * byte to support 64 levels of detail, but this function caps it to 5. Some
 * textures actually specify up to 7 levels of detail. However testing suggests
 * that the additional levels of detail are not even read.
 *
 * This function reads the above information from the first byte of texture data,
 * then calls the texInflateZlib or texInflateNonZlib to inflate the images.
 *
 * The format of the first byte is:
 * uzllllll
 *
 * u = unknown
 * z = texture is compressed with zlib
 * l = number of levels of detail within the texture
 */
void texLoad(texnum_t *updateword, struct texpool *pool)
{
	//uint8_t compbuffer[4 * 1024 + 0x40];
	uint8_t compbuffer[4 * 1024 * 2 + 0x40];
	uint8_t *compptr;
	bool hasloddata;
	int iszlib;
	int numlods;
	struct tex *tex;
	uint8_t *alignedcompbuffer;
	struct tex *tail;
	uint32_t freebytes;
	uint8_t usingsharedpool = 0;
	//int8_t buffer5kb[5 * 1024 + 0x40];
	int8_t buffer5kb[5 * 1024 * 2 + 0x40];
	int thisoffset;
	int nextoffset;
	int16_t *texnumptr;
	int bytesout;

	usingsharedpool = 0;

	if (pool == NULL) {
		pool = &g_TexSharedPool;
	}

	if (pool == &g_TexSharedPool) {
		usingsharedpool = 1;
	}

	// If the value at updateword isn't already a pointer
	if ((*updateword & 0xffff0000) == 0 || (*updateword & 0xffff0000) == 0xabcd0000) {
		g_TexNumToLoad = *updateword & 0xffff;

		tex = texFindInPool(g_TexNumToLoad, pool);

		if (tex == NULL) {
			if (g_TexNumToLoad >= NUM_TEXTURES) {
				return;
			}

			alignedcompbuffer = (uint8_t *) (((uintptr_t)compbuffer + 0xf) >> 4 << 4);

			thisoffset = g_Textures[g_TexNumToLoad].dataoffset;
			nextoffset = g_Textures[g_TexNumToLoad + 1].dataoffset;

			if (thisoffset == nextoffset) {
				// The texture has no data
				return;
			}

			// try to load external replacement if present
			if (modTextureLoad(g_TexNumToLoad, alignedcompbuffer, 4096) > 0) {
				compptr = alignedcompbuffer;
			} 
			// try to load from the data/textures folder if present
			else if(createBMP(g_TexNumToLoad, 48, 31, alignedcompbuffer, 8192) > 0) {
				compptr = alignedcompbuffer;
			}
			else
			{
				// Copy the compressed texture to RAM
				memcpy(alignedcompbuffer,
				     	(const void *) ((romptr_t) _texturesdataSegmentRomStart + (thisoffset & 0xfffffff8)),
						((uintptr_t) (nextoffset - thisoffset) + 0x1f) >> 4 << 4);
				compptr = (uint8_t *) alignedcompbuffer + (thisoffset & 7);
			}
			thisoffset = 0;
			hasloddata = (*compptr & 0x80) >> 7;
			iszlib = (*compptr & 0x40) >> 6;
			numlods = *compptr & 0x3f;

			if (numlods > 5) {
				numlods = 5;
			}

			compptr++;

			// If there's not enough memory to load the texture, set the texture
			// pointer to the start of the pool. It'll be garbage data but the
			// only other option is a crash. GBI commands contain texture IDs
			// instead of pointers, and they must be replaced with pointers.
			if (usingsharedpool) {
				freebytes = mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_ONBOARD) + mempGetPoolFree(MEMPOOL_STAGE, MEMBANK_EXPANSION);
			} else {
				freebytes = texGetPoolFreeBytes(pool);
			}

			if ((!iszlib && freebytes < 4300) || (iszlib && freebytes < 2600)) {
				*updateword = (uintptr_t)(pool->start);
				return;
			}

			// If we're using the shared pool:
			// - rightpos is the head of a linked list, so grab it and find the tail
			// - set rightpos to a spot in the buffer that can fit a tex before it
			// - set leftpos to 0x10 after rightpos
			if (usingsharedpool) {
				tail = pool->rightpos;
				pool->rightpos = (struct tex *) ((((uintptr_t) buffer5kb + 0xf) >> 4 << 4) + sizeof(struct tex));
				pool->leftpos = ((uint8_t *) pool->rightpos + sizeof(struct tex));

				while (tail) {
					if (tail->next == 0) {
						break;
					}

					tail = (struct tex *) (k_ptr_t)(tail->next);
				}
			}

			// Write the texturenum into the allocation
			texnumptr = (int16_t *) pool->leftpos;
			*texnumptr = g_TexNumToLoad;
			pool->leftpos += 8;

			// Write a tex into the allocation
			pool->rightpos--;
			tex = pool->rightpos;
			tex->texturenum = g_TexNumToLoad;
			tex->data = pool->leftpos;

			// Extract the texture data to the allocation (pool->leftpos)
			if (iszlib) {
				bytesout = texInflateZlib(compptr, pool->leftpos, hasloddata, numlods, pool);
			} else {
				bytesout = texInflateNonZlib(compptr, pool->leftpos, hasloddata, numlods, pool);
			}

			// If we're using the shared pool, the data must be copied out of
			// the stack and into the heap.
			if (usingsharedpool) {
				uint8_t *ptr = mempAllocFromRight(ALIGN16(bytesout + 2 * sizeof(struct tex)), MEMPOOL_STAGE);
				pool->rightpos = (struct tex *) ptr;

				memcpy(ptr, tex, sizeof(struct tex));

				tex = (struct tex *) ptr;
				ptr += sizeof(struct tex);

				memcpy(ptr, pool->leftpos - 8, bytesout + 8);

				pool->rightpos->data = ptr + 8;
				pool->rightpos->next = 0;

				if (tail != NULL) {
					tail->next = (uintptr_t) pool->rightpos;
				} else {
					pool->head = pool->rightpos;
				}

				pool->start = (uint8_t *) pool->rightpos;
			}

			pool->leftpos += bytesout;

			if (!usingsharedpool) {
				texGetPoolFreeBytes(pool);
			}
		}

		*updateword = (uintptr_t)(tex->data);
	}
}

void texLoadFromConfigs(struct textureconfig *configs, int numconfigs, struct texpool *pool, uintptr_t arg3)
{
	int i;

	for (i = 0; i < numconfigs; i++) {
		if ((uintptr_t)configs[i].texturenum < NUM_TEXTURES) {
			texLoad(&configs[i].texturenum, pool);
			configs[i].unk0b = 1;
		} else {
			configs[i].texturenum += arg3;
		}
	}
}

void texLoadFromTextureNum(uint32_t texturenum, struct texpool *pool)
{
	texnum_t texturenumcopy = texturenum;

	texLoad(&texturenumcopy, pool);
}

unsigned char *texLoadBMP(const char *filename, int width, int height) {
    FILE *file = fopen(filename, "rb");  // Open in binary mode
    if (!file) {
        //debug_log("Error: Could not open BMP file.\n", 0);
        return NULL;
    }

    // Read BMP headers
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Check BMP signature ("BM")
    if (fileHeader.type != 0x4D42) {
        //debug_log("Error: Not a valid BMP file.\n", 0);
        fclose(file);
        return NULL;
    }

    // Store width & height
    width = infoHeader.width;
    height = infoHeader.height;

    // Ensure it's a 24-bit BMP (uncompressed)
    if (infoHeader.bitCount != 24 || infoHeader.compression != 0) {
        //debug_log("Error: Only uncompressed 24-bit BMP files are supported.\n", 0);
        fclose(file);
        return NULL;
    }

    // Allocate memory for pixel data (3 bytes per pixel: R, G, B)
    int row_padded = (width * 3 + 3) & (~3); // Align rows to 4 bytes
    unsigned char *data = (unsigned char *)malloc(row_padded * (height));
    if (!data) {
       // debug_log("Error: Memory allocation failed.\n", 0);
        fclose(file);
        return NULL;
    }

    // Move file pointer to the pixel data location
    fseek(file, fileHeader.offset, SEEK_SET);

    // Read pixel data (BMP is stored bottom-to-top)
    for (int i = 0; i < height; i++) {
        fread(data + (i * row_padded), 1, row_padded, file);
    }

    fclose(file);
    return data;
}

//	int width, height;
//	unsigned char *imageData = createBMP("0255.bmp", &width, &height);

int createBMP(uint16_t num, int width, int height, void *dst, uint32_t dstSize)
{
	char *fullpath = "./" DEFAULT_BASEDIR_NAME "/textures"; // ./data/textures
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%d", num);
	size_t len = strlen(buffer) + 6; // Integer length + "/" + ".bmp" (4 chars) + null terminator
    char *filename = malloc(len); // Allocate memory for final string

	if (!filename) {
        return -1; // Return NULL if allocation fails
    }

	snprintf(filename, len, "/%s.bmp", buffer);

	static int dirExists = -1;
	if (dirExists < 0) {
		dirExists = (fsFileSize(fullpath) >= 0);
	}

	if (!dirExists) {
		return -1;
	}

	unsigned char *imageData = texLoadBMP(buildDynamicPath(fullpath, filename), width, height);

	const int ret = fsFileLoadTo(buildDynamicPath(fullpath, filename), dst, dstSize);

	//TODO: Incomplete
	
	return -1;
}