#include <ultra64.h>
#include "constants.h"
#include "game/player.h"
#include "game/healthbar.h"
#include "game/textutils.h"
#include "game/gfxmemory.h"
#include "game/file.h"
#include "bss.h"
#include "lib/main.h"
#include "data.h"
#include "gfx.h"
#include "types.h"

struct marker {
	float x1;
	float y1;
	float x2;
	float y2;
	float frac;
};

int healthbarMaybeInsertMarker(struct marker *markers, int *indexes, int maxlen, float fillfrac)
{
	int len = 0;
	int i;
	int j;

	if (fillfrac < 0.0f) {
		fillfrac = 0.0f;
	}

	if (fillfrac > 1.0f) {
		fillfrac = 1.0f;
	}

	for (i = 0; i < maxlen; i++) {
		if (indexes[i] > len) {
			len = indexes[i];
		}
	}

	len++;

	for (i = 0; i < len; i++) {
		int index1 = indexes[i];
		int index2 = indexes[i + 1];
		float x1;
		float x2;
		float y1;
		float y2;
		float tmp1;
		float tmp2;

		if (markers[index1].frac < fillfrac && markers[index2].frac > fillfrac) {
			x1 = markers[index2].x1 - markers[index1].x1;
			y1 = markers[index2].y1 - markers[index1].y1;
			x2 = markers[index2].x2 - markers[index1].x2;
			y2 = markers[index2].y2 - markers[index1].y2;
			tmp1 = markers[index2].frac - markers[index1].frac;
			tmp2 = fillfrac - markers[index1].frac;

			markers[len].x1 = markers[index1].x1 + tmp2 / tmp1 * x1;
			markers[len].y1 = markers[index1].y1 + tmp2 / tmp1 * y1;
			markers[len].x2 = markers[index1].x2 + tmp2 / tmp1 * x2;
			markers[len].y2 = markers[index1].y2 + tmp2 / tmp1 * y2;
			markers[len].frac = fillfrac;

			for (j = len - 1; j > i; j--) {
				indexes[j + 1] = indexes[j];
			}

			indexes[i + 1] = len;
			return 1;
		}
	}

	return 0;
}

uint32_t healthbarChooseColour(uint32_t fillcol, uint32_t bgcol, float fillexcfade, float fillincfade, float frac)
{
	float mult;
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t a;

	if (frac >= fillincfade) {
		return bgcol;
	}

	if (frac <= fillexcfade) {
		return fillcol;
	}

	mult = (frac - fillexcfade) / (fillincfade - fillexcfade);

	r = (fillcol >> 24) & 0xff;
	g = (fillcol >> 16) & 0xff;
	b = (fillcol >> 8) & 0xff;
	a = fillcol & 0xff;

	r += (int) ((int) (((bgcol >> 24) & 0xff) - r) * mult);
	g += (int) ((int) (((bgcol >> 16) & 0xff) - g) * mult);
	b += (int) ((int) (((bgcol >> 8) & 0xff) - b) * mult);
	a += (int) ((int) ((bgcol & 0xff) - a) * mult);

	return r << 24 | g << 16 | b << 8 | a;
}

/**
 * Terminology used in this function:
 *
 * armour - is the green part of the main health bar
 * trauma - is the red part of the main health bar
 * shield - is the green shield bar underneath the health bar
 * health - refers to armour + trauma collectively
 *
 * The health bar determines what shapes to draw by utilising markers. A marker
 * is a pair of vertices at a certain fraction (0.0 to 1.0) across a healthbar
 * element. Each marker's vertices are used to allocate proper graphics vertices
 * and are connected in order.
 *
 * For example, the green health part of the bar has a single marker on the left
 * at frac 0, then the next marker on the right side where the curve starts at
 * 0.9, then several markers in close proximity between 0.9 and 1.0 to implement
 * the rounded end.
 *
 * If the player's health falls between two markers, up to two two additional
 * markers will be created dynamically for colouring purposes. The coordinates
 * of these dynamic markers are tweened from the two neighbours. There are two
 * of these because the colour change uses a fade rather than a hard edge.
 */
Gfx *healthbarDraw(Gfx *gdl, struct chrdata *chr, int offyarg, float heightfracarg)
{
	struct marker shieldmarkers[12];
	struct marker armourmarkers[8];
	struct marker traumamarkers[8];
	int shieldmarkerindexes[12];
	int armourmarkerindexes[8];
	int traumamarkerindexes[8];
	Vtx *vertices;
	Col *colours;
	Col *shieldcolours;
	Col *armourcolours;
	Col *traumacolours;
	Vtx *shieldvertices;
	Vtx *armourvertices;
	Vtx *traumavertices;
	float shieldfillincfade;
	float shieldfillexcfade;
	float armourfillincfade;
	float armourfillexcfade;
	float traumafillincfade;
	float traumafillexcfade;
	struct marker *marker;
	int numshieldmarkers;
	int numtraumamarkers;
	int numarmourmarkers;
	float shieldfrac;
	float armourfrac;
	float traumafrac;
	float healthfrac;
	float heightfrac;
	uint32_t colour;
	int index;
	int i;
	int numverts;
	int numvertsremaining;
	float len0 = 0.0f;
	int coloursize;

	static int radmax = 30; // outer radius of the shield
	static int radmed = 18; // inner radius of the shield
	static int radmin = 12; // radius of the health bar
	static int len1 = 170; // x pos of right side radius centre
	static int len2 = 47;  // x pos of the left side of armour
	static int len3 = 40;  // x pos of the right side of trauma (left side is 0)
	static uint32_t shieldcol = 0x10500090;
	static uint32_t armourcol = 0x00c00060;
	static uint32_t traumacol = 0xff000060;
	static uint32_t bgcol = 0x00000080;
	static int offx = -85;
	static int offy = -185;
	static int shieldfade = 100;
	static int armourfade = 100;
	static int traumafade = 200;
	static int shielddir = 1;
	static int underleft = 95;
	static int undertop = 18;
	static int underright = 225;
	static int underbottom = 46;
	static uint32_t undercol = 0x00000000;

	vertices = gfxAllocateVertices(56);
	colours = gfxAllocateColours(56);

	// 12 markers (24 vertices) for shield
	// 8 markers (16 vertices) for armour
	// 8 markers (16 vertices) for trauma
	shieldvertices = vertices;
	armourvertices = vertices + 24;
	traumavertices = vertices + 40;

	shieldcolours = colours;
	armourcolours = colours + 24;
	traumacolours = colours + 40;

	if (chr == NULL || offyarg <= 0 || heightfracarg <= 0.0f) {
		// Use the player's health
		shieldfrac = g_Vars.currentplayer->apparentarmour;
		armourfrac = (g_Vars.currentplayer->apparenthealth - 0.25f) / 0.75f;
		traumafrac = (0.25f - g_Vars.currentplayer->apparenthealth) * 4.0f;
		heightfrac = playerGetHealthBarHeightFrac();
	} else {
		// Use the given chr's health
		healthfrac = (chr->maxdamage - chr->damage) / chr->maxdamage;
		shieldfrac = chr->cshield * 0.125f;
		armourfrac = (healthfrac - 0.25f) / 0.75f;
		traumafrac = (0.25f - healthfrac) * 4.0f;
		heightfrac = heightfracarg;
		offy += offyarg;
	}

	armourfrac = armourfrac > 0 ? armourfrac : 0;
	traumafrac = traumafrac > 0 ? traumafrac : 0;

	for (i = 0; i < 12; i++) {
		shieldmarkerindexes[i] = i < 10 ? i : -1;

		if (i < 8) {
			armourmarkerindexes[i] = i < 6 ? i : -1;
			traumamarkerindexes[i] = i < 6 ? i : -1;
		}
	}

	/**
	 * Shield markers
	 *
	 * This is actually defined and drawn right to left rather than left to
	 * right, but is coloured the other way around using the shielddir variable.
	 *
	 * len0 - is the X position of the left side, and is defined as 0.
	 *        A variable must be used in some places for this to match, and a
	 *        literal 0 in others. It's even required in a Y value for a match.
	 * len1 - is the X position of the right side.
	 */
	shieldmarkers[0].x1 = len1 + radmax * 1.08f;
	shieldmarkers[0].y1 = 0;
	shieldmarkers[0].x2 = len1 + (float)radmed;
	shieldmarkers[0].y2 = 0;
	shieldmarkers[0].frac = 0;

	shieldmarkers[1].x1 = len1 + radmax * 0.924f * 1.04f;
	shieldmarkers[1].y1 = heightfrac * radmax * 0.383f;
	shieldmarkers[1].x2 = len1 + radmed * 0.924f;
	shieldmarkers[1].y2 = heightfrac * radmed * 0.383f;
	shieldmarkers[1].frac = 0.05f;

	shieldmarkers[2].x1 = len1 + radmax * 0.707f * 1.02f;
	shieldmarkers[2].y1 = heightfrac * radmax * 0.707f;
	shieldmarkers[2].x2 = len1 + radmed * 0.707f;
	shieldmarkers[2].y2 = heightfrac * radmed * 0.707f;
	shieldmarkers[2].frac = 0.1f;

	shieldmarkers[3].x1 = len1 + radmax * 0.383f;
	shieldmarkers[3].y1 = heightfrac * radmax * 0.924f;
	shieldmarkers[3].x2 = len1 + radmed * 0.383f;
	shieldmarkers[3].y2 = heightfrac * radmed * 0.924f;
	shieldmarkers[3].frac = 0.15f;

	shieldmarkers[4].x1 = len1;
	shieldmarkers[4].y1 = heightfrac * radmax;
	shieldmarkers[4].x2 = len1;
	shieldmarkers[4].y2 = heightfrac * radmed;
	shieldmarkers[4].frac = 0.2f;

	shieldmarkers[5].x1 = len0;
	shieldmarkers[5].y1 = heightfrac * radmax;
	shieldmarkers[5].x2 = len0;
	shieldmarkers[5].y2 = heightfrac * radmed;
	shieldmarkers[5].frac = 0.8f;

	shieldmarkers[6].x1 = len0 - radmax * 0.383f;
	shieldmarkers[6].y1 = heightfrac * radmax * 0.924f;
	shieldmarkers[6].x2 = len0 - radmed * 0.383f;
	shieldmarkers[6].y2 = heightfrac * radmed * 0.924f;
	shieldmarkers[6].frac = 0.85f;

	shieldmarkers[7].x1 = 0.0f - radmax * 0.707f * 1.02f;
	shieldmarkers[7].y1 = heightfrac * radmax * 0.707f;
	shieldmarkers[7].x2 = 0.0f - radmed * 0.707f;
	shieldmarkers[7].y2 = heightfrac * radmed * 0.707f;
	shieldmarkers[7].frac = 0.9f;

	shieldmarkers[8].x1 = len0 - radmax * 0.924f * 1.04f;
	shieldmarkers[8].y1 = heightfrac * radmax * 0.383f;
	shieldmarkers[8].x2 = len0 - radmed * 0.924f;
	shieldmarkers[8].y2 = heightfrac * radmed * 0.383f;
	shieldmarkers[8].frac = 0.95f;

	shieldmarkers[9].x1 = len0 - radmax * 1.08f;
	shieldmarkers[9].y1 = len0;
	shieldmarkers[9].x2 = len0 - radmed;
	shieldmarkers[9].y2 = len0;
	shieldmarkers[9].frac = 1;

	/**
	 * Armour markers
	 *
	 * Drawn left to right as expected.
	 *
	 * len2 - is the X position of the left side.
	 * len1 - is the X position of the right side.
	 */
	armourmarkers[0].x1 = len2;
	armourmarkers[0].y1 = heightfrac * radmin;
	armourmarkers[0].x2 = len2;
	armourmarkers[0].y2 = -heightfrac * radmin;
	armourmarkers[0].frac = 0;

	armourmarkers[1].x1 = len1;
	armourmarkers[1].y1 = heightfrac * radmin;
	armourmarkers[1].x2 = len1;
	armourmarkers[1].y2 = -heightfrac * radmin;
	armourmarkers[1].frac = 0.9f;

	armourmarkers[2].x1 = len1 + radmin * 0.342f;
	armourmarkers[2].y1 = heightfrac * radmin * 0.94f;
	armourmarkers[2].x2 = len1 + radmin * 0.342f;
	armourmarkers[2].y2 = -heightfrac * radmin * 0.94f;
	armourmarkers[2].frac = 0.94f;

	armourmarkers[3].x1 = len1 + radmin * 0.643f;
	armourmarkers[3].y1 = heightfrac * radmin * 0.766f;
	armourmarkers[3].x2 = len1 + radmin * 0.643f;
	armourmarkers[3].y2 = -heightfrac * radmin * 0.766f;
	armourmarkers[3].frac = 0.97f;

	armourmarkers[4].x1 = len1 + radmin * 0.866f;
	armourmarkers[4].y1 = heightfrac * radmin * 0.5f;
	armourmarkers[4].x2 = len1 + radmin * 0.866f;
	armourmarkers[4].y2 = -heightfrac * radmin * 0.5f;
	armourmarkers[4].frac = 0.99f;

	armourmarkers[5].x1 = len1 + radmin * 0.985f;
	armourmarkers[5].y1 = heightfrac * radmin * 0.174f;
	armourmarkers[5].x2 = len1 + radmin * 0.985f;
	armourmarkers[5].y2 = -heightfrac * radmin * 0.174f;
	armourmarkers[5].frac = 1;

	/**
	 * Trauma markers
	 *
	 * Drawn right to left because it fills right to left.
	 *
	 * len0 - is the X position of the left side. Like the shield markers, this
	 *        is defined as 0 and must only be used sometimes.
	 * len3 - is the X position of the right side.
	 *
	 * Note that the trailing 0 on 0.7070f is absolutely required for a match.
	 *
	 * Fun fact: There are also trauma markers on my brain after matching this
	 * code.
	 */
	traumamarkers[0].x1 = len3;
	traumamarkers[0].y1 = heightfrac * radmin;
	traumamarkers[0].x2 = len3;
	traumamarkers[0].y2 = -heightfrac * radmin;
	traumamarkers[0].frac = 0;

	traumamarkers[1].x1 = 0.0f;
	traumamarkers[1].y1 = heightfrac * radmin;
	traumamarkers[1].x2 = 0.0f;
	traumamarkers[1].y2 = -heightfrac * radmin;
	traumamarkers[1].frac = 0.8f;

	traumamarkers[2].x1 = 0.0f - radmin * 0.383f;
	traumamarkers[2].y1 = heightfrac * radmin * 0.924f;
	traumamarkers[2].x2 = 0.0f - radmin * 0.383f;
	traumamarkers[2].y2 = -heightfrac * radmin * 0.924f;
	traumamarkers[2].frac = 0.85f;

	traumamarkers[3].x1 = 0.0f - radmin * 0.707f;
	traumamarkers[3].y1 = heightfrac * radmin * 0.7070f;
	traumamarkers[3].x2 = 0.0f - radmin * 0.707f;
	traumamarkers[3].y2 = -heightfrac * radmin * 0.7070f;
	traumamarkers[3].frac = 0.9f;

	traumamarkers[4].x1 = len0 - radmin * 0.924f;
	traumamarkers[4].y1 = heightfrac * radmin * 0.383f;
	traumamarkers[4].x2 = len0 - radmin * 0.924f;
	traumamarkers[4].y2 = -heightfrac * radmin * 0.383f;
	traumamarkers[4].frac = 0.95f;

	traumamarkers[5].x1 = len0 - radmin;
	traumamarkers[5].y1 = 0.0f;
	traumamarkers[5].x2 = len0 - radmin;
	traumamarkers[5].y2 = 0.0f;
	traumamarkers[5].frac = 1;

	// Build shield graphics data
	if (shielddir != 0) {
		shieldfrac = 1.0f - shieldfrac;
	}

	shieldfillincfade = shieldfade * 0.001f;
	shieldfillincfade = (1.0f + shieldfillincfade) * shieldfrac;
	shieldfillexcfade = shieldfillincfade - shieldfade * 0.001f;

	numshieldmarkers = 10;
	numshieldmarkers += healthbarMaybeInsertMarker(shieldmarkers, shieldmarkerindexes, ARRAYCOUNT(shieldmarkers), shieldfillexcfade);
	numshieldmarkers += healthbarMaybeInsertMarker(shieldmarkers, shieldmarkerindexes, ARRAYCOUNT(shieldmarkers), shieldfillincfade);

	for (i = 0; i < numshieldmarkers; i++) {
		index = shieldmarkerindexes[i];
		marker = &shieldmarkers[index];

		if (shielddir != 0) {
			colour = healthbarChooseColour(bgcol, shieldcol, shieldfillexcfade, shieldfillincfade, marker->frac);
		} else {
			colour = healthbarChooseColour(shieldcol, bgcol, shieldfillexcfade, shieldfillincfade, marker->frac);
		}

		shieldvertices->x = (int)marker->x1 + offx;
		shieldvertices->y = 0;
		shieldvertices->z = (int)marker->y1 + offy;
		shieldvertices->colour = (i + i) << 2;
		shieldvertices++;

		shieldcolours->word = PD_BE32(colour);
		shieldcolours++;

		shieldvertices->x = (int)marker->x2 + offx;
		shieldvertices->y = 0;
		shieldvertices->z = (int)marker->y2 + offy;
		shieldvertices->colour = (i + i + 1) << 2;
		shieldvertices++;

		shieldcolours->word = PD_BE32(colour);
		shieldcolours++;
	}

	// Build armour graphics data
	armourfillincfade = armourfade * 0.001f;
	armourfillincfade = (1.0f + armourfillincfade) * armourfrac;
	armourfillexcfade = armourfillincfade - armourfade * 0.001f;

	numarmourmarkers = 6;
	numarmourmarkers += healthbarMaybeInsertMarker(armourmarkers, armourmarkerindexes, ARRAYCOUNT(armourmarkers), armourfillexcfade);
	numarmourmarkers += healthbarMaybeInsertMarker(armourmarkers, armourmarkerindexes, ARRAYCOUNT(armourmarkers), armourfillincfade);

	for (i = 0; i < numarmourmarkers; i++) {
		index = armourmarkerindexes[i];
		marker = &armourmarkers[index];

		colour = healthbarChooseColour(armourcol, bgcol, armourfillexcfade, armourfillincfade, marker->frac);

		armourvertices->x = (int)marker->x1 + offx;
		armourvertices->y = 0;
		armourvertices->z = (int)marker->y1 + offy;
		armourvertices->colour = (i + i) << 2;
		armourvertices++;

		armourcolours->word = PD_BE32(colour);
		armourcolours++;

		armourvertices->x = (int)marker->x2 + offx;
		armourvertices->y = 0;
		armourvertices->z = (int)marker->y2 + offy;
		armourvertices->colour = (i + i + 1) << 2;
		armourvertices++;

		armourcolours->word = PD_BE32(colour);
		armourcolours++;
	}

	// Build trauma graphics data
	traumafillincfade = traumafade * 0.001f;
	traumafillincfade = (1.0f + traumafillincfade) * traumafrac;
	traumafillexcfade = traumafillincfade - traumafade * 0.001f;

	numtraumamarkers = 6;
	numtraumamarkers += healthbarMaybeInsertMarker(traumamarkers, traumamarkerindexes, ARRAYCOUNT(traumamarkers), traumafillexcfade);
	numtraumamarkers += healthbarMaybeInsertMarker(traumamarkers, traumamarkerindexes, ARRAYCOUNT(traumamarkers), traumafillincfade);

	for (i = 0; i < numtraumamarkers; i++) {
		index = traumamarkerindexes[i];
		marker = &traumamarkers[index];

		colour = healthbarChooseColour(traumacol, bgcol, traumafillexcfade, traumafillincfade, marker->frac);

		traumavertices->x = (int)marker->x1 + offx;
		traumavertices->y = 0;
		traumavertices->z = (int)marker->y1 + offy;
		traumavertices->colour = (i + i) << 2;
		traumavertices++;

		traumacolours->word = PD_BE32(colour);
		traumacolours++;

		traumavertices->x = (int)marker->x2 + offx;
		traumavertices->y = 0;
		traumavertices->z = (int)marker->y2 + offy;
		traumavertices->colour = (i + i + 1) << 2;
		traumavertices++;

		traumacolours->word = PD_BE32(colour);
		traumacolours++;
	}

	gdl = textConfigureGfxPipeline(gdl);
	gdl = text0f153a34(gdl, underleft, undertop, underright, underbottom, undercol);
	gdl = textSetPerspAndLOD(gdl);

	gfx_Set_Render_Mode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
	gfx_Set_Combine_LERP(gdl++,
		G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 0
		G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE,              // Alpha 0
		G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_TEXEL0, G_CCMUX_SHADE,              // Color 1
		G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_TEXEL0, G_ACMUX_SHADE);             // Alpha 1

	// Same as gSPColor but casts the sizeof to an int, which is required for a match.
#define gSPColor2(pkt, v, n) \
	gfx_Dma1p(pkt, G_COL, v, (int)sizeof(uint32_t)*(n),((n)-1)<<2)

	// Draw shield
	// There's between 20 and 24 colours and vertices to load here.
	// The colours are loaded all at once, but the vertices are split into two.
	numverts = numshieldmarkers * 2;

	gSPColor2(gdl++, (uintptr_t)(colours), numverts);
	gfx_Vertex(gdl++, vertices, 14, 0);

	gfx_Tri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gfx_Tri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);
	gfx_Tri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

	// numvertsremaining will be 8-12
	numvertsremaining = numverts - 12U;

	// This is a macro expansion of gSPVertex but cases the sizeof to an int,
	// and separates numvertsremaining and numverts - 12.
	// Both are required for a match.
	gfx_Dma1p(gdl++, G_VTX,
			(uintptr_t)(vertices + 12),
			numvertsremaining * (int) sizeof(Vtx),
			(numverts - 12 - 1) << 4);

	gfx_Tri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);

	if (numvertsremaining < 10) {
		gfx_Tri2(gdl++, 4, 5, 6, 5, 6, 7);
	} else {
		gfx_Tri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

		if (numvertsremaining >= 12) {
			gfx_Tri2(gdl++, 8, 9, 10, 9, 10, 11);
		}
	}

	// Draw armour
	numverts = numarmourmarkers * 2;
	coloursize = numverts * 4;

	gSPColor2(gdl++, (uintptr_t)(colours + 24), numverts);
	gfx_Vertex(gdl++, vertices + 24, numverts, 0);

	gfx_Tri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gfx_Tri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

	if (coloursize < 56) {
		gfx_Tri2(gdl++, 8, 9, 10, 9, 10, 11);
	} else {
		gfx_Tri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

		if (coloursize >= 64) {
			gfx_Tri2(gdl++, 12, 13, 14, 13, 14, 15);
		}
	}

	// Draw trauma
	numverts = numtraumamarkers * 2;
	coloursize = numverts * 4;

	gSPColor2(gdl++, (uintptr_t)(colours + 40), numverts);
	gfx_Vertex(gdl++, vertices + 40, numverts, 0);

	gfx_Tri4(gdl++, 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5);
	gfx_Tri4(gdl++, 4, 5, 6, 5, 6, 7, 6, 7, 8, 7, 8, 9);

	if (coloursize < 56) {
		gfx_Tri2(gdl++, 8, 9, 10, 9, 10, 11);
	} else {
		gfx_Tri4(gdl++, 8, 9, 10, 9, 10, 11, 10, 11, 12, 11, 12, 13);

		if (coloursize >= 64) {
			gfx_Tri2(gdl++, 12, 13, 14, 13, 14, 15);
		}
	}

	return gdl;
}
