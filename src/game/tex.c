#include <ultra64.h>
#include <stdio.h>
#include "constants.h"
#include "game/dyntex.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "data.h"
#include "gbiex.h"
#include "textures.h"
#include "types.h"
#include "platform.h"
#include "game/debug.h"

#define TXMODE_WRAP   0
#define TXMODE_CLAMP  1
#define TXMODE_MIRROR 2

struct tilestate {
	bool inuse;
	int format;
	int depth;
	int line;
	int tmem;
	int smode;
	int tmode;
	int masks;
	int maskt;
	int shifts;
	int shiftt;
};

struct tilesize {
	bool inuse;
	int uls;
	int ult;
	int lrs;
	int lrt;
};

int g_TexLutMode;
struct tilestate g_TexTileStates[8];
struct tilesize g_TexTileSizes[8];
uint32_t g_TexFilter2D = G_TF_BILERP;

// Default
uint16_t g_SurfaceTypeDefaultSounds[] = { SFX_HIT_STONE_8087, SFX_HIT_STONE_8088 };
uint8_t g_SurfaceTypeDefaultTexes[] = { WALLHITTEX_BULLET2 };

struct surfacetype g_SurfaceTypeDefault = {
	g_SurfaceTypeDefaultSounds, g_SurfaceTypeDefaultTexes, 2, 1,
};

// Stone
uint16_t g_SurfaceTypeStoneSounds[] = { SFX_HIT_STONE_8087, SFX_HIT_STONE_8088 };
uint8_t g_SurfaceTypeStoneTexes[] = { WALLHITTEX_BULLET1 };

struct surfacetype g_SurfaceTypeStone = {
	g_SurfaceTypeStoneSounds, g_SurfaceTypeStoneTexes, 2, 1,
};

// Wood
uint16_t g_SurfaceTypeWoodSounds[] = { SFX_HIT_WOOD_807E, SFX_HIT_WOOD_807F };
uint8_t g_SurfaceTypeWoodTexes[] = { WALLHITTEX_WOOD };

struct surfacetype g_SurfaceTypeWood = {
	g_SurfaceTypeWoodSounds, g_SurfaceTypeWoodTexes, 2, 1,
};

// Metal
uint16_t g_SurfaceTypeMetalSounds[] = { SFX_HIT_METAL_8079, SFX_HIT_METAL_807B };
uint8_t g_SurfaceTypeMetalTexes[] = { WALLHITTEX_METAL };

struct surfacetype g_SurfaceTypeMetal = {
	g_SurfaceTypeMetalSounds, g_SurfaceTypeMetalTexes, 2, 1,
};

// Glass
uint16_t g_SurfaceTypeGlassSounds[] = { SFX_HIT_GLASS };
uint8_t g_SurfaceTypeGlassTexes[] = { WALLHITTEX_GLASS1, WALLHITTEX_GLASS2, WALLHITTEX_GLASS3 };

struct surfacetype g_SurfaceTypeGlass = {
	g_SurfaceTypeGlassSounds, g_SurfaceTypeGlassTexes, 1, 3,
};

// Snow
uint16_t g_SurfaceTypeSnowSounds[] = { SFX_HIT_SNOW };
uint8_t g_SurfaceTypeSnowTexes[] = { WALLHITTEX_BULLET1 };

struct surfacetype g_SurfaceTypeSnow = {
	g_SurfaceTypeSnowSounds, g_SurfaceTypeSnowTexes, 1, 1,
};

// Dirt
uint16_t g_SurfaceTypeDirtSounds[] = { SFX_HIT_DIRT_8084, SFX_HIT_DIRT_8085 };
uint8_t g_SurfaceTypeDirtTexes[] = { WALLHITTEX_SOFT };

struct surfacetype g_SurfaceTypeDirt = {
	g_SurfaceTypeDirtSounds, g_SurfaceTypeDirtTexes, 2, 1,
};

// Mud
uint16_t g_SurfaceTypeMudSounds[] = { SFX_HIT_MUD_8081, SFX_HIT_MUD_8082, SFX_HIT_MUD_8083 };
uint8_t g_SurfaceTypeMudTexes[] = { WALLHITTEX_SOFT };

struct surfacetype g_SurfaceTypeMud = {
	g_SurfaceTypeMudSounds, g_SurfaceTypeMudTexes, 3, 1,
};

// Tile
uint16_t g_SurfaceTypeTileSounds[] = { SFX_HIT_TILE };
uint8_t g_SurfaceTypeTileTexes[] = { WALLHITTEX_BULLET1 };

struct surfacetype g_SurfaceTypeTile = {
	g_SurfaceTypeTileSounds, g_SurfaceTypeTileTexes, 1, 1,
};

// Metal obj
uint16_t g_SurfaceTypeMetalObjSounds[] = { SFX_HIT_METALOBJ_8089, SFX_HIT_METALOBJ_808A };
uint8_t g_SurfaceTypeMetalObjTexes[] = { WALLHITTEX_BULLET1, WALLHITTEX_BULLET2 };

struct surfacetype g_SurfaceTypeMetalObj = {
	g_SurfaceTypeMetalObjSounds, g_SurfaceTypeMetalObjTexes, 2, 2,
};

// Chr
uint16_t g_SurfaceTypeChrSounds[] = { SFX_HIT_CHR };
uint8_t g_SurfaceTypeChrTexes[] = { WALLHITTEX_SOFT };

struct surfacetype g_SurfaceTypeChr = {
	g_SurfaceTypeChrSounds, g_SurfaceTypeChrTexes, 1, 1,
};

// Glass XLU
uint16_t g_SurfaceTypeGlassXluSounds[] = { SFX_HIT_GLASS };
uint8_t g_SurfaceTypeGlassXluTexes[] = { WALLHITTEX_GLASS1, WALLHITTEX_GLASS2, WALLHITTEX_GLASS3 };

struct surfacetype g_SurfaceTypeGlassXlu = {
	g_SurfaceTypeGlassXluSounds, g_SurfaceTypeGlassXluTexes, 1, 3,
};

// None
struct surfacetype g_SurfaceTypeNone = {
	NULL, NULL, 0, 0,
};

// Shallow water
uint16_t g_SurfaceTypeShallowWaterSounds[] = { SFX_HIT_WATER };
uint8_t g_SurfaceTypeShallowWaterTexes[] = { WALLHITTEX_WATER };

struct surfacetype g_SurfaceTypeShallowWater = {
	g_SurfaceTypeShallowWaterSounds, g_SurfaceTypeShallowWaterTexes, 1, 1,
};

// Deep water
uint16_t g_SurfaceTypeDeepWaterSounds[] = { SFX_HIT_WATER };
uint8_t g_SurfaceTypeDeepWaterTexes[] = { WALLHITTEX_WATER };

struct surfacetype g_SurfaceTypeDeepWater = {
	g_SurfaceTypeDeepWaterSounds, g_SurfaceTypeDeepWaterTexes, 1, 1,
};

struct surfacetype *g_SurfaceTypes[] = {
	/* 0*/ &g_SurfaceTypeDefault,
	/* 1*/ &g_SurfaceTypeStone,
	/* 2*/ &g_SurfaceTypeWood,
	/* 3*/ &g_SurfaceTypeMetal,
	/* 4*/ &g_SurfaceTypeGlass,
	/* 5*/ &g_SurfaceTypeShallowWater,
	/* 6*/ &g_SurfaceTypeSnow,
	/* 7*/ &g_SurfaceTypeDirt,
	/* 8*/ &g_SurfaceTypeMud,
	/* 9*/ &g_SurfaceTypeTile,
	/*10*/ &g_SurfaceTypeMetalObj,
	/*11*/ &g_SurfaceTypeChr,
	/*12*/ &g_SurfaceTypeGlassXlu,
	/*13*/ &g_SurfaceTypeNone,
	/*14*/ &g_SurfaceTypeDeepWater,
};

void texResetTiles(void)
{
	int i;

	for (i = 0; i < ARRAYCOUNT(g_TexTileStates); i++) {
		g_TexTileStates[i].inuse = false;
		g_TexTileSizes[i].inuse = false;
	}

	g_TexLutMode = -1;
}

bool texTrySetLutMode(uint32_t lutmode)
{
	if (g_TexLutMode == lutmode) {
		return false;
	}

	g_TexLutMode = lutmode;
	return true;
}

bool texTrySetTileState(int tile, int format, int depth, int line, int tmem, int smode, int tmode, int masks, int maskt, int shifts, int shiftt)
{
	bool changed = false;

	if (g_TexTileStates[tile].inuse == false
			|| format != g_TexTileStates[tile].format
			|| depth != g_TexTileStates[tile].depth
			|| line != g_TexTileStates[tile].line
			|| tmem != g_TexTileStates[tile].tmem
			|| smode != g_TexTileStates[tile].smode
			|| tmode != g_TexTileStates[tile].tmode
			|| masks != g_TexTileStates[tile].masks
			|| maskt != g_TexTileStates[tile].maskt
			|| shifts != g_TexTileStates[tile].shifts
			|| shiftt != g_TexTileStates[tile].shiftt) {
		changed = true;
		g_TexTileStates[tile].inuse = true;
		g_TexTileStates[tile].format = format;
		g_TexTileStates[tile].depth = depth;
		g_TexTileStates[tile].line = line;
		g_TexTileStates[tile].tmem = tmem;
		g_TexTileStates[tile].smode = smode;
		g_TexTileStates[tile].tmode = tmode;
		g_TexTileStates[tile].masks = masks;
		g_TexTileStates[tile].maskt = maskt;
		g_TexTileStates[tile].shifts = shifts;
		g_TexTileStates[tile].shiftt = shiftt;
	}

	return changed;
}

bool texTrySetTileSize(int tile, int uls, int ult, int lrs, int lrt)
{
	bool changed = false;

	if (g_TexTileSizes[tile].inuse == false
			|| uls != g_TexTileSizes[tile].uls
			|| ult != g_TexTileSizes[tile].ult
			|| lrs != g_TexTileSizes[tile].lrs
			|| lrt != g_TexTileSizes[tile].lrt) {
		changed = true;
		g_TexTileSizes[tile].inuse = true;
		g_TexTileSizes[tile].uls = uls;
		g_TexTileSizes[tile].ult = ult;
		g_TexTileSizes[tile].lrs = lrs;
		g_TexTileSizes[tile].lrt = lrt;
	}

	return changed;
}

int texGetWidthAtLod(struct tex *tex, int lod)
{
	int i;
	int width = tex->width;

	if (lod == 0) {
		return width;
	}

	if (tex->hasloddata) {
		for (i = 0; i < g_TexCacheCount; i++) {
			if (tex->texturenum == g_TexCacheItems[i].texturenum) {
				return g_TexCacheItems[i].widths[lod - 1];
			}
		}

		return 1;
	}

	for (i = 0; i < lod; i++) {
		width = (width + 1) >> 1;
	}

	return width;
}

int texGetHeightAtLod(struct tex *tex, int lod)
{
	int i;
	int height = tex->height;

	if (lod == 0) {
		return height;
	}

	if (tex->hasloddata) {
		for (i = 0; i < g_TexCacheCount; i++) {
			if (tex->texturenum == g_TexCacheItems[i].texturenum) {
				return g_TexCacheItems[i].heights[lod - 1];
			}
		}

		return 1;
	}

	for (i = 0; i < lod; i++) {
		height = (height + 1) >> 1;
	}

	return height;
}

int texGetLineSizeInBytes(struct tex *tex, int lod)
{
	int depth = tex->depth;
	int width = texGetWidthAtLod(tex, lod);

	if (depth == G_IM_SIZ_32b) {
		return (width + 3) / 4;
	}

	if (depth == G_IM_SIZ_16b) {
		return (width + 3) / 4;
	}

	if (depth == G_IM_SIZ_8b) {
		return (width + 7) / 8;
	}

	return (width + 15) / 16;
}

int texGetSizeInBytes(struct tex *tex, int lod)
{
	return texGetHeightAtLod(tex, lod) * texGetLineSizeInBytes(tex, lod);
}

void texGetDepthAndSize(struct tex *tex, int *deptharg, int *lenarg)
{
	int depth = tex->depth;
	int numlods = tex->numlods ? tex->numlods : 1;
	int lod;

	*lenarg = 0;

	if (depth == G_IM_SIZ_32b) {
		*deptharg = G_IM_SIZ_32b;
	} else if (depth == G_IM_SIZ_16b) {
		*deptharg = G_IM_SIZ_16b;
	} else if (depth == G_IM_SIZ_8b) {
		*deptharg = G_IM_SIZ_16b;
	} else {
		*deptharg = G_IM_SIZ_16b;
	}

	for (lod = 0; lod < numlods; lod++) {
		*lenarg += texGetSizeInBytes(tex, lod) * 4;
	}
}

int texDimensionToMask(int dimension)
{
	int i = 0;

	dimension--;

	while (dimension > 0 && i < 8) {
		dimension >>= 1;
		i++;
	}

	return i;
}

int texModeToGbiMode(int txmode)
{
	if (txmode == TXMODE_CLAMP) {
		return G_TX_CLAMP;
	}

	if (txmode == TXMODE_MIRROR) {
		return G_TX_MIRROR;
	}

	return G_TX_WRAP;
}

Gfx *texWriteTileFromDefinition(Gfx *gdl, struct tex *tex, int offset, int shifts, int shiftt, int min)
{
	struct texture *s0 = &g_Textures[tex->texturenum];
	int masks;
	int maskt;
	int line;
	int uls;
	int ult;
	int lrs;
	int lrt;

	masks = texDimensionToMask(tex->width);
	maskt = texDimensionToMask(tex->height);

	line = texGetLineSizeInBytes(tex, 0);

	gDPSetPrimColorViaWord(gdl++, min, 0, 0xffffffff);

	if (texTrySetLutMode(tex->lutmodeindex << G_MDSFT_TEXTLUT)) {
		gDPSetTextureLUT(gdl++, tex->lutmodeindex << G_MDSFT_TEXTLUT);
	}

	if (texTrySetTileState(0, tex->gbiformat, tex->depth, line, s0->unk04_00 + line * s0->unk04_04, 0, 0, masks - s0->unk04_08, maskt - s0->unk04_0c, shifts, shiftt)) {
		gDPSetTile(gdl++, tex->gbiformat, tex->depth, line, s0->unk04_00 + line * s0->unk04_04, 0, 0,
				texModeToGbiMode(TXMODE_WRAP), maskt - s0->unk04_0c, shiftt,
				texModeToGbiMode(TXMODE_WRAP), masks - s0->unk04_08, shifts);
	}

	uls = (offset == 2 && !tex->hasloddata ? 2 : 0) + 0;
	ult = (offset == 2 && !tex->hasloddata ? 2 : 0) + 0;
	lrs = (offset == 2 && !tex->hasloddata ? 2 : 0) + ((tex->width - 1) << 2);
	lrt = (offset == 2 && !tex->hasloddata ? 2 : 0) + ((tex->height - 1) << 2);

	if (texTrySetTileSize(0, uls, ult, lrs, lrt)) {
		gDPSetTileSize(gdl++, 0, uls, ult, lrs, lrt);
	}

	return gdl;
}

/**
 * Write a gSPTexture command with the lowest detail LOD number.
 *
 * If texcmd is provided then it will be used as a reference, otherwise a new
 * command is written with some assumed arguments.
 *
 * If append is true then the command will be appended to the given gdl,
 * otherwise the existing texcmd will be patched in place.
 */
Gfx *texWriteTextureCmd(Gfx *gdl, Gfx *texcmd, struct tex *tex, bool append)
{
	int lod = tex->numlods ? tex->numlods - 1 : 0;

	if (append) {
		if (texcmd != NULL) {
			// Copy the gSPTexture command, but with the new lod
			uint32_t w0 = (texcmd->words.w0 & ~0x3800) | (lod << 11);

			if (w0 != texcmd->words.w0) {
				gdl->words.w0 = w0;
				gdl->words.w1 = texcmd->words.w1;
				gdl++;
			}
		} else {
			gSPTexture(gdl++, 0xffff, 0xffff, lod, G_TX_RENDERTILE, G_ON);
		}
	} else {
		texcmd->words.w0 &= ~0x3800;
		texcmd->words.w0 |= lod << 11;
	}

	return gdl;
}

Gfx *texWriteLoadToTmemAddr(Gfx *gdl, struct tex *tex, int tmemoffset)
{
	int depth;
	int len;

	texGetDepthAndSize(tex, &depth, &len);

	if (tex->lutmodeindex == 0) {
		gDPSetTextureImage(gdl++, tex->gbiformat, depth, 1, tex->data);

		if (depth == G_IM_SIZ_16b && tmemoffset == 0) {
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, len - 1, 0);
		} else {
			if (texTrySetTileState(5, 0, depth, 0, tmemoffset, 0, 0, 0, 0, 0, 0)) {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth, 0, tmemoffset, 5, 0,
						G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
						G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
			}

			gDPLoadBlock(gdl++, 5, 0, 0, len - 1, 0);
		}
	} else {
		gDPSetTextureImage(gdl++, tex->gbiformat, depth, 1, tex->data);

		if (depth == G_IM_SIZ_16b && tmemoffset == 0) {
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, len - 1, 0);
		} else {
			if (texTrySetTileState(5, 0, depth, 0, tmemoffset, 0, 0, 0, 0, 0, 0)) {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth, 0, tmemoffset, 5, 0,
						G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD,
						G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
			}

			gDPLoadBlock(gdl++, 5, 0, 0, len - 1, 0);
		}

		{
			int tmp = len;
			int a2 = (uint32_t)(0x3ff - tex->texTlutTmemOffset) < len ? (uint32_t)(0x3ff - tex->texTlutTmemOffset) : 0;

			tmp -= a2;

			gDPLoadTLUT06(gdl++, tmp, a2, tex->texTlutTmemOffset + tmp, a2);
		}
	}

	return gdl;
}

Gfx *texWriteTileLods(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset, int starttile, int numlodsarg, uint32_t tmemoffsetarg)
{
	uint32_t tmemoffset;
	int numlods;
	int tile;

	tmemoffset = tmemoffsetarg;
	numlods = tex->numlods;

	if (numlodsarg >= 0 && numlodsarg < numlods) {
		numlods = numlodsarg;
	}

	for (tile = starttile; tile < numlods + starttile && tile < 6; tile++) {
		int lod = tile - starttile;
		int masks = texDimensionToMask(texGetWidthAtLod(tex, lod));
		int maskt = texDimensionToMask(texGetHeightAtLod(tex, lod));
		int line = texGetLineSizeInBytes(tex, lod);
		int uls;
		int ult;
		int lrs;
		int lrt;
		int bytes = texGetSizeInBytes(tex, lod);
		bool hasloddata = tex->hasloddata;

		if (texTrySetLutMode(tex->lutmodeindex << G_MDSFT_TEXTLUT)) {
			gDPSetTextureLUT(gdl++, tex->lutmodeindex << G_MDSFT_TEXTLUT);
		}

		if (texTrySetTileState(tile, tex->gbiformat, tex->depth, line, tmemoffset, smode, tmode, masks, maskt, lod, lod)) {
			gDPSetTile(gdl++, tex->gbiformat, tex->depth, line, tmemoffset, tile, 0,
					texModeToGbiMode(tmode), maskt, lod,
					texModeToGbiMode(smode), masks, tile - starttile);
		}

		uls = (offset == 2 && hasloddata == false ? 2 : 0) + 0;
		ult = (offset == 2 && hasloddata == false ? 2 : 0) + 0;
		lrs = ((texGetWidthAtLod(tex, lod) - 1) << 2) + (offset == 2 && hasloddata == false ? 2 : 0);
		lrt = ((texGetHeightAtLod(tex, lod) - 1) << 2) + (offset == 2 && hasloddata == false ? 2 : 0);

		if (texTrySetTileSize(tile, uls, ult, lrs, lrt)) {
			gDPSetTileSize(gdl++, tile, uls, ult, lrs, lrt);
		}

		tmemoffset += bytes;
	}

	return gdl;
}

Gfx *texWriteLoadToTmemZero(Gfx *gdl, struct tex *tex)
{
	int depth;
	int len;

	texGetDepthAndSize(tex, &depth, &len);

	if (tex->lutmodeindex == 0) {
		gDPSetTextureImage(gdl++, tex->gbiformat, depth, 1, tex->data);

		if (depth == G_IM_SIZ_16b) {
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, len - 1, 0);
		} else {
			if (texTrySetTileState(5, 0, depth, 0, 0, 0, 0, 0, 0, 0, 0)) {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
			}

			gDPLoadBlock(gdl++, 5, 0, 0, len - 1, 0);
		}
	} else {
		gDPSetTextureImage(gdl++, tex->gbiformat, depth, 1, tex->data);

		if (depth == G_IM_SIZ_16b) {
			gDPLoadBlock(gdl++, G_TX_LOADTILE, 0, 0, len - 1, 0);
		} else {
			if (texTrySetTileState(5, 0, depth, 0, 0, 0, 0, 0, 0, 0, 0)) {
				gDPSetTile(gdl++, G_IM_FMT_RGBA, depth, 0, 0x0000, 5, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD);
			}

			gDPLoadBlock(gdl++, 5, 0, 0, len - 1, 0);
		}

		{
			int tmp = len;
			int a2 = (uint32_t)(0x3ff - tex->texTlutTmemOffset) < len ? (uint32_t)(0x3ff - tex->texTlutTmemOffset) : 0;

			tmp -= a2;

			gDPLoadTLUT06(gdl++, tmp, a2, tex->texTlutTmemOffset + tmp, a2);
		}
	}

	return gdl;
}

Gfx *texWriteTile(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset, int tile)
{
	int masks;
	int maskt;
	int line;
	int uls;
	int ult;
	int lrs;
	int lrt;
	bool hasloddata;

	masks = texDimensionToMask(tex->width);
	maskt = texDimensionToMask(tex->height);
	line = texGetLineSizeInBytes(tex, 0);
	hasloddata = tex->hasloddata;

	if (texTrySetLutMode(tex->lutmodeindex << G_MDSFT_TEXTLUT)) {
		gDPSetTextureLUT(gdl++, tex->lutmodeindex << G_MDSFT_TEXTLUT);
	}

	if (texTrySetTileState(tile, tex->gbiformat, tex->depth, line, 0, smode, tmode, masks, maskt, 0, 0)) {
		gDPSetTile(gdl++, tex->gbiformat, tex->depth, line, 0x0000, tile, 0,
				texModeToGbiMode(tmode), maskt, G_TX_NOLOD,
				texModeToGbiMode(smode), masks, G_TX_NOLOD);
	}

	uls = (offset == 2 && hasloddata == false ? 2 : 0) + 0;
	ult = (offset == 2 && hasloddata == false ? 2 : 0) + 0;
	lrs = (offset == 2 && hasloddata == false ? 2 : 0) + ((tex->width - 1) << 2);
	lrt = (offset == 2 && hasloddata == false ? 2 : 0) + ((tex->height - 1) << 2);

	if (texTrySetTileSize(tile, uls, ult, lrs, lrt)) {
		gDPSetTileSize(gdl++, tile, uls, ult, lrs, lrt);
	}

	return gdl;
}

Gfx *texHandleType2(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset, bool flag)
{
	int tile = 0;

	gdl = texWriteLoadToTmemAddr(gdl, tex, 0);

	if (flag) {
		gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, 1, 0);
		tile++;
	}

	gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, -1, 0);
	tile += tex->numlods;

	if (!flag && tex->numlods == 1) {
		gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, -1, 0);
		tile += tex->numlods;
	}

	return gdl;
}

Gfx *texHandleType1(Gfx *gdl, struct tex *tex1, int smode, int tmode, int offset, struct tex *tex2, int shifts, int shiftt, int min, bool flag)
{
	int size = texGetSizeInBytes(tex2, 0);
	int tile = 0;

	gdl = texWriteLoadToTmemZero(gdl, tex2);
	gdl = texWriteLoadToTmemAddr(gdl, tex1, size);

	gdl = texWriteTileFromDefinition(gdl, tex2, offset, shifts, shiftt, min);
	tile++;

	if (flag) {
		gdl = texWriteTileLods(gdl, tex1, smode, tmode, offset, tile, 1, size);
		tile++;
	}

	gdl = texWriteTileLods(gdl, tex1, smode, tmode, offset, tile, -1, size);
	tile += tex1->numlods;

	if (!flag && tex1->numlods == 1) {
		gdl = texWriteTileLods(gdl, tex1, smode, tmode, offset, tile, -1, size);
		tile += tex1->numlods;
	}

	return gdl;
}

Gfx *texHandleType0(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset, int shifts, int shiftt, int min, bool flag)
{
	int tile = 0;

	gdl = texWriteLoadToTmemAddr(gdl, tex, 0);

	gdl = texWriteTileFromDefinition(gdl, tex, offset, shifts, shiftt, min);
	tile++;

	if (flag) {
		gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, 1, 0);
		tile++;
	}

	gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, -1, 0);
	tile += tex->numlods;

	if (!flag && tex->numlods == 1) {
		gdl = texWriteTileLods(gdl, tex, smode, tmode, offset, tile, -1, 0);
		tile += tex->numlods;
	}

	return gdl;
}

Gfx *texHandleType4(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset)
{
	gdl = texWriteLoadToTmemZero(gdl, tex);
	gdl = texWriteTile(gdl, tex, smode, tmode, offset, 0);

	return gdl;
}

Gfx *texHandleType3(Gfx *gdl, struct tex *tex, int smode, int tmode, int offset)
{
	gdl = texWriteLoadToTmemZero(gdl, tex);
	gdl = texWriteTile(gdl, tex, smode, tmode, offset, 0);
	gdl = texWriteTile(gdl, tex, smode, tmode, offset, 1);

	return gdl;
}

/**
 * Read the instart displaylist and write the same displaylist to outstart,
 * but substitute the special C0 commands with ones that the RSP will understand,
 * and load textures from the ROM as they are found in the displaylist.
 *
 * The C0 command only exists in asset files. It has no implementation in the RSP.
 * The command packs together several texture-related commands. It's likely done
 * for ease of development, as it allows asset designers to spam its use everywhere
 * in the displaylist rather than have to keep track of the RSP state.
 * The code that expands the C0 commands keeps track of RSP state and avoids
 * emitting duplicate or redundant commands.
 */
int texLoadFromGdl(Gfx *instart, int gdlsizeinbytes, Gfx *outstart, struct texpool *pool, uint8_t *vtxstart)
{
	struct tex *tex1;
	struct tex *tex2;
	Gfx *texcmd;
	int numcmdsremaining;
	int smode;
	int tmode;
	int offset;
	int shifts;
	int shiftt;
	int min;
	bool flag;
	int j;
	bool appendtex;
	uint8_t animated;
	Gfx *ingdl;
	Gfx *outgdl;
	uint32_t spf4;
	int texturenum;
	int texturenum2;
	bool spe8;
	int spe4;
	int spe0;
	Vtx *spa0[16];
	uint8_t sp90[16];
	int i;

	texcmd = NULL;
	appendtex = true;
	animated = false;
	spe8 = false;
	spe4 = false;
	spf4 = 0;
	ingdl = instart;
	outgdl = outstart;

#ifdef PLATFORM_64BIT
	numcmdsremaining = gdlsizeinbytes >> 4;
#else
	numcmdsremaining = gdlsizeinbytes >> 3;
#endif

	texResetTiles();

	spe0 = dyntexHasRoom();

	if (spe0) {
		for (j = 0; j < ARRAYCOUNT(sp90); j++) {
			sp90[j] = 0;
		}
	}

	if (pool == NULL) {
		pool = &g_TexSharedPool;
	}

	while (numcmdsremaining > 0) {
		switch (ingdl->texture.cmd) {
		case G_NOOP: // 0xc0, repurposed
			spe4 = true;

			if (animated) {
				spe8 = true;
			}

			texturenum = ingdl->words.w1 & 0xfff;
			flag = ingdl->words.w0 & 0x200;

			texLoadFromTextureNum(texturenum, pool);

			tex1 = texFindInPool(texturenum, pool);

			spf4 = 0;

			if (tex1 != NULL) {
				outgdl = texWriteTextureCmd(outgdl, texcmd, tex1, appendtex);
				appendtex = false;
				animated = false;

				switch (ingdl->unkc0.subcmd) {
				case 0:
					min = (ingdl->words.w1 >> 24) & 0xff;
					smode = (ingdl->words.w0 >> 22) & 3;
					tmode = (ingdl->words.w0 >> 20) & 3;
					offset = (ingdl->words.w0 >> 18) & 3;
					shifts = (ingdl->words.w0 >> 14) & 0xf;
					shiftt = (ingdl->words.w0 >> 10) & 0xf;

					outgdl = texHandleType0(outgdl, tex1, smode, tmode, offset, shifts, shiftt, min, flag);
					break;
				case 1:
					texturenum2 = (ingdl->words.w1 >> 12) & 0xfff;
					texLoadFromTextureNum(texturenum2, pool);
					tex2 = texFindInPool(texturenum2, pool);

					if (tex2 != NULL) {
						min = (ingdl->words.w1 >> 24) & 0xff;
						smode = (ingdl->words.w0 >> 22) & 3;
						tmode = (ingdl->words.w0 >> 20) & 3;
						offset = (ingdl->words.w0 >> 18) & 3;
						shifts = (ingdl->words.w0 >> 14) & 0xf;
						shiftt = (ingdl->words.w0 >> 10) & 0xf;

						outgdl = texHandleType1(outgdl, tex1, smode, tmode, offset, tex2, shifts, shiftt, min, flag);
					}
					break;
				case 2:
					smode = (ingdl->words.w0 >> 22) & 3;
					tmode = (ingdl->words.w0 >> 20) & 3;
					offset = (ingdl->words.w0 >> 18) & 3;

					outgdl = texHandleType2(outgdl, tex1, smode, tmode, offset, flag);
					break;
				case 3:
					smode = (ingdl->words.w0 >> 22) & 3;
					tmode = (ingdl->words.w0 >> 20) & 3;
					offset = (ingdl->words.w0 >> 18) & 3;

					outgdl = texHandleType3(outgdl, tex1, smode, tmode, offset);
					break;
				case 4:
					smode = (ingdl->words.w0 >> 22) & 3;
					tmode = (ingdl->words.w0 >> 20) & 3;
					offset = (ingdl->words.w0 >> 18) & 3;

					outgdl = texHandleType4(outgdl, tex1, smode, tmode, offset);
					break;
				}

				if (spe0 != 0) {
					// Deep Sea - green river under floor
					if (texturenum == TEXTURE_06CB) {
						dyntexSetCurrentType(DYNTEXTYPE_RIVER);
						animated = true;
					}

					// Deep Sea - juice that flows inside SA megaweapon
					// Attack Ship - juice that flows inside engine power node
					if (texturenum == TEXTURE_0A6A) {
						dyntexSetCurrentType(DYNTEXTYPE_POWERJUICE);
						animated = true;
					}

					// Deep Sea - white rings around SA megaweapon node
					// Attack Ship - white rings around engine power node
					if (texturenum == TEXTURE_0A69) {
						dyntexSetCurrentType(DYNTEXTYPE_POWERRING);
						animated = true;
					}

					// Deep Sea - teleport
					if (texturenum == TEXTURE_06E2) {
						dyntexSetCurrentType(DYNTEXTYPE_TELEPORTAL);
						animated = true;
					}

					// 01c7 - Air Base - distant water
					// 01c7 - Investigation - puddle behind glass near shield
					// 0dae - Chicago - canal
					// 0dae - Villa - shallow water
					// 0dae - Sewers (MP)
					if (texturenum == TEXTURE_01C7 || texturenum == TEXTURE_0DAE) {
						dyntexSetCurrentType(DYNTEXTYPE_RIVER);
						animated = true;
					}

					// Air Force One - Monitor
					if (texturenum == TEXTURE_029B) {
						dyntexSetCurrentType(DYNTEXTYPE_MONITOR);
						animated = true;
					}

					// Villa - deep water
					// Complex - water
					if (texturenum == TEXTURE_090F) {
						dyntexSetCurrentType(DYNTEXTYPE_OCEAN);
						animated = true;
					}

					// Attack Ship - triangular arrows
					if (texturenum == TEXTURE_0A42) {
						dyntexSetCurrentType(DYNTEXTYPE_ARROWS);
						animated = true;
					}
				}
			}

			ingdl++;
			break;
		case G_VTX:
			{
				int start;
				int count;
				uint32_t offset;
				Vtx *vtx;

				if (spe0) {
					start = ingdl->bytes[GFX_W0_BYTE(1)] & 0xf;
					count = ingdl->vtx.unk08 + 1;
					vtx = (Vtx *)(UNSEGADDR(ingdl->dma.addr) & 0x00ffffff);

					for (i = start; i < start + count; i++) {
						if (animated && sp90[i]) {
							dyntexAddVertex(spa0[i]);
							sp90[i] = 0;
						}

						spa0[i] = vtx;
						vtx++;
					}
				}

				if (spf4 && vtxstart) {
					int count = ingdl->vtx.unk08 + 1;
					Vtx *vtx;
					int i;

					offset = UNSEGADDR(ingdl->dma.addr) & 0x00ffffff;
					vtx = (Vtx *) (vtxstart + offset);

					for (i = 0; i < count; i++) {
						vtx[i].s >>= 1;
						vtx[i].t >>= 1;
					}
				}
			}

			*outgdl = *ingdl;
			outgdl++;
			ingdl++;
			break;
		case (uint8_t) G_TRI4:
		case (uint8_t) G_TRI1:
			if (animated) {
				if (ingdl->texture.cmd == (uint8_t) G_TRI1) {
					sp90[ingdl->tri.tri.v[GFX_TRI_VTX(0)] / 10] = 1;
					sp90[ingdl->tri.tri.v[GFX_TRI_VTX(1)] / 10] = 1;
					sp90[ingdl->tri.tri.v[GFX_TRI_VTX(2)] / 10] = 1;
				} else {
					Gfx *tmp = ingdl;

					if (tmp->tri4.x1 != tmp->tri4.y1 || tmp->tri4.z1 != tmp->tri4.y1) {
						sp90[tmp->tri4.x1] = 1;
						sp90[tmp->tri4.y1] = 1;
						sp90[tmp->tri4.z1] = 1;
					}

					if (tmp->tri4.x2 != tmp->tri4.y2 || tmp->tri4.z2 != tmp->tri4.y2) {
						sp90[tmp->tri4.x2] = 1;
						sp90[tmp->tri4.y2] = 1;
						sp90[tmp->tri4.z2] = 1;
					}

					if (tmp->tri4.x3 != tmp->tri4.y3 || tmp->tri4.z3 != tmp->tri4.y3) {
						sp90[tmp->tri4.x3] = 1;
						sp90[tmp->tri4.y3] = 1;
						sp90[tmp->tri4.z3] = 1;
					}

					if (tmp->tri4.x4 != tmp->tri4.y4 || tmp->tri4.z4 != tmp->tri4.y4) {
						sp90[tmp->tri4.x4] = 1;
						sp90[tmp->tri4.y4] = 1;
						sp90[tmp->tri4.z4] = 1;
					}
				}
			}

			appendtex = true;

			*outgdl = *ingdl;
			outgdl++;
			ingdl++;
			break;
		case (uint8_t) G_TEXTURE:
			spe4 = true;

			if (animated) {
				spe8 = true;
			}

			animated = false;
			appendtex = false;

			texcmd = outgdl;
			*outgdl = *ingdl;
			outgdl++;
			ingdl++;
			break;
		case (uint8_t) G_SETOTHERMODE_H:
			*outgdl = *ingdl;
			outgdl++;
			ingdl++;
			break;
		default:
			*outgdl = *ingdl;
			outgdl++;
			ingdl++;
			break;
		}

		numcmdsremaining--;

		if (spe4 || numcmdsremaining <= 0) {
			spe4 = false;

			if (spe8 || animated) {
				int i;

				spe8 = false;

				for (i = 0; i < ARRAYCOUNT(sp90); i++) {
					if (sp90[i]) {
						dyntexAddVertex(spa0[i]);
						sp90[i] = 0;
					}
				}
			}
		}
	}

	return (uintptr_t) outgdl - (uintptr_t) outstart;
}

void texCopyGdls(Gfx *src, Gfx *dst, int count)
{
#ifdef PLATFORM_64BIT
	count = (count >> 4);
#else
	count = (count >> 3);
#endif

	src = src + (count - 1);
	dst = dst + (count - 1);

	while (count--) {
		*dst = *src;
		dst--;
		src--;
	}
}
