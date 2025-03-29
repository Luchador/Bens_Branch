#ifndef IN_GAME_TEXDECOMPRESS_H
#define IN_GAME_TEXDECOMPRESS_H
#include "data.h"
#include "types.h"

int texInflateZlib(uint8_t *src, uint8_t *dst, bool arg2, int forcenumimages, struct texpool *pool);
int texAlignIndices(uint8_t *arg0, int width, int height, int format, uint8_t *dst);
int texGetAverageRed(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4);
int texGetAverageGreen(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4);
int texGetAverageBlue(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4);
int texGetAverageAlpha(uint16_t colour1, uint16_t colour2, uint16_t colour3, uint16_t colour4);
int texShrinkPaletted(uint8_t *src, uint8_t *dst, int srcwidth, int srcheight, int format, uint16_t *palette, int numcolours);
int texFindClosestColourIndexRGBA(uint8_t *palette, int numcolours, int r, int g, int b, int a);
int texFindClosestColourIndexIA(uint16_t *palette, int numcolours, int intensity, int alpha);
int texInflateNonZlib(uint8_t *src, uint8_t *dst, int arg2, int forcenumimages, struct texpool *pool);
int texShrinkNonPaletted(uint8_t *src, uint8_t *dst, int srcwidth, int srcheight, int format);
void texInflateHuffman(uint8_t *dst, int numiterations, int chansize);
void texInflateRle(uint8_t *arg0, int arg1);
int texBuildLookup(uint8_t *arg0, int bpp);
int texGetBitSize(int arg0);
void texReadAlphaBits(uint8_t *dst, int count);
int texReadUncompressed(uint8_t *dst, int width, int height, int format);
int texChannelsToPixels(uint8_t *src, int width, int height, uint8_t *dst, int format);
int texInflateLookup(int width, int height, uint8_t *dst, uint8_t *lookup, int numcolours, int format);
int texInflateLookupFromBuffer(uint8_t *src, int width, int height, uint8_t *dst, uint8_t *lookup, int numcolours, int format);
void texSwizzle(uint8_t *arg0, int width, int height, int format);
void texBlur(uint8_t *pixels, int width, int height, int method, int chansize);
void texInitPool(struct texpool *pool, uint8_t *start, int len);
struct tex *texFindInPool(int texturenum, struct texpool *pool);
int texGetPoolFreeBytes(struct texpool *pool);
uint8_t *texGetPoolLeftPos(struct texpool *pool);
void texLoadFromDisplayList(Gfx *gdl, struct texpool *pool, int arg2);
void texLoad(texnum_t *updateword, struct texpool *pool);
void texLoadFromConfigs(struct textureconfig *configs, int numconfigs, struct texpool *pool, uintptr_t arg3);
void texLoadFromTextureNum(uint32_t arg0, struct texpool *pool);
void texSwizzleInternal(uint8_t *dst, int width, int height, int format, uint32_t dstlen);
int texConfigToFormat(const struct textureconfig *tex);
unsigned char *texLoadBMP(const char *filename, int width, int height);
int createBMP(uint16_t num, int width, int height, void *dst, uint32_t dstSize);

#endif
