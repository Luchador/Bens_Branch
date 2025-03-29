#ifndef IN_GAME_TEX_H
#define IN_GAME_TEX_H
#include "data.h"
#include "types.h"

void texInit(void);

void surfaceReset(void);

void texSetBitstring(uint8_t *arg0);
int texReadBits(int arg0);
void texReset(void);

int texGetMask(int value);
int tex0f0b33f8(int width, int height, int lod);
int tex0f0b3468(int width, int height, int lod);
int tex0f0b34d8(int width, int height, int lod);
int tex0f0b3548(int width, int height, int lod);
void texSetRenderMode(Gfx **gdlptr, int rendermode, int numcycles, int arg3);
void texLoadFromConfig(struct textureconfig *config);
void texSelect(Gfx **gdl, struct textureconfig *tconfig, uint32_t arg2, int arg3, uint32_t ulst, bool arg5, struct texpool *pool);

int texGetWidthAtLod(struct tex *tex, int lod);
int texGetHeightAtLod(struct tex *tex, int lod);
int texGetLineSizeInBytes(struct tex *tex, int lod);
int texGetSizeInBytes(struct tex *tex, int lod);
void texGetDepthAndSize(struct tex *tex, int *arg1, int *arg2);
int texLoadFromGdl(Gfx *instart, int gdlsizeinbytes, Gfx *outstart, struct texpool *pool, uint8_t *vtxstart);
void texCopyGdls(Gfx *src, Gfx *dst, int numbytes);

#endif
