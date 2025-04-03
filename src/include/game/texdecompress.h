#ifndef IN_GAME_TEXDECOMPRESS_H
#define IN_GAME_TEXDECOMPRESS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

int texInflateZlib(u8 *src, u8 *dst, bool arg2, int forcenumimages, struct texpool *pool);
int texAlignIndices(u8 *arg0, int width, int height, int format, u8 *dst);
int texGetAverageRed(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
int texGetAverageGreen(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
int texGetAverageBlue(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
int texGetAverageAlpha(u16 colour1, u16 colour2, u16 colour3, u16 colour4);
int texShrinkPaletted(u8 *src, u8 *dst, int srcwidth, int srcheight, int format, u16 *palette, int numcolours);
int texFindClosestColourIndexRGBA(u8 *palette, int numcolours, int r, int g, int b, int a);
int texFindClosestColourIndexIA(u16 *palette, int numcolours, int intensity, int alpha);
int texInflateNonZlib(u8 *src, u8 *dst, int arg2, int forcenumimages, struct texpool *pool);
int texShrinkNonPaletted(u8 *src, u8 *dst, int srcwidth, int srcheight, int format);
void texInflateHuffman(u8 *dst, int numiterations, int chansize);
void texInflateRle(u8 *arg0, int arg1);
int texBuildLookup(u8 *arg0, int bpp);
int texGetBitSize(int arg0);
void texReadAlphaBits(u8 *dst, int count);
int texReadUncompressed(u8 *dst, int width, int height, int format);
int texChannelsToPixels(u8 *src, int width, int height, u8 *dst, int format);
int texInflateLookup(int width, int height, u8 *dst, u8 *lookup, int numcolours, int format);
int texInflateLookupFromBuffer(u8 *src, int width, int height, u8 *dst, u8 *lookup, int numcolours, int format);
void texSwizzle(u8 *arg0, int width, int height, int format);
void texBlur(u8 *pixels, int width, int height, int method, int chansize);
void texInitPool(struct texpool *pool, u8 *start, int len);
struct tex *texFindInPool(int texturenum, struct texpool *pool);
int texGetPoolFreeBytes(struct texpool *pool);
u8 *texGetPoolLeftPos(struct texpool *pool);
void texLoadFromDisplayList(Gfx *gdl, struct texpool *pool, int arg2);
void texLoad(texnum_t *updateword, struct texpool *pool);
void texLoadFromConfigs(struct textureconfig *configs, int numconfigs, struct texpool *pool, uintptr_t arg3);
void texLoadFromTextureNum(u32 arg0, struct texpool *pool);
void texSwizzleInternal(u8 *dst, int width, int height, int format, u32 dstlen);
int texConfigToFormat(const struct textureconfig *tex);
unsigned char *texLoadBMP(const char *filename, int width, int height);
int createBMP(u16 num, int width, int height, void *dst, u32 dstSize);

#endif
