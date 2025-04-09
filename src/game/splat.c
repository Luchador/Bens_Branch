#include <ultra64.h>
#include "constants.h"
#include "game/bg.h"
#include "game/mtxutils.h"
#include "game/propobj.h"
#include "game/smoke.h"
#include "game/splat.h"
#include "game/utils.h"
#include "game/wallhit.h"
#include "bss.h"
#include "lib/lib_17ce0.h"
#include "lib/model.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "data.h"
#include "types.h"

#define SPLATTYPE_PUDDLE 1
#define SPLATTYPE_DROP   2

struct splatdata {
	struct coord relpos;
	struct coord unk0c;
	struct coord unk18;
	struct coord gunpos;
	int splattype;
	struct prop *objprop;
	struct prop *chrprop;
	struct chrdata *chr;
	int mtxindex;
	int room;
	int isskedar;
	int translucent;
	float sizescale;
	int timermax;
	int timerspeed;
};

float g_SplatDistanceScaleFactor = 0.15;
float g_SplatRandomOffsetMax = 12; // When a splat is made there's some randomness from where the hit is calculated to where it's actually placed, with 12 being the max possible distance
float g_SplatMaxDistance = 180;
float g_SplatMinSize = 5;
float g_SplatMaxSize = 50;

bool splat0f149274(float arg0, struct prop *prop, struct shotdata *shotdata, float arg3, bool isskedar, int arg5, int arg6, struct chrdata *chr, int arg8);
void splat0f14986c(struct splatdata *splatdata);

void splatTickChr(struct prop *prop)
{
	struct chrdata *chr = prop->chr;
	struct chrdata *attacker = chr->lastattacker;
	int race;

	if (chr->noblood || (chr->chrflags & CHRCFLAG_HIDDEN) || chr->bulletstaken == 0) {
		return;
	}

	race = CHRRACE(chr);

	if (race != RACE_DRCAROLL && race != RACE_ROBOT) {
		uint8_t isskedar = false;

		if (race == RACE_SKEDAR || chr->bodynum == BODY_MRBLONDE) {
			isskedar = true;
		}

		if (chr->actiontype == ACT_DEAD || chr->actiontype == ACT_DIE) {
			float thudframe = -1.0f;

			if (chr->actiontype == ACT_DIE) {
				if (chr->act_die.thudframe2 != -1) {
					thudframe = chr->act_die.thudframe2;
				} else if (chr->act_die.thudframe1 != -1) {
					thudframe = chr->act_die.thudframe1;
				}
			}

			if (thudframe != -1.0f && modelGetCurAnimFrame(chr->model) < thudframe) {
			} else if (chr->tickssincesplat > TICKS(30) && chr->deaddropsplatsadded < 6) {
				chr->deaddropsplatsadded += splatsCreate(1, 1.1f, prop, NULL, 0, 0, isskedar, SPLATTYPE_PUDDLE, TICKS(150), attacker, rngRandom() & 8);
			}
		} else {
			// Consider creating a wounded drop
			uint32_t value = chr->bulletstaken * chr->tickssincesplat;

			if (value > TICKS(240)) {
				float dist = coordsGetDistance(&chr->lastdroppos, &prop->pos);
				int addmore = false;

				if (dist > 40) {
					addmore = true;
					chr->splatsdroppedhere = 0;
				} else if (chr->splatsdroppedhere < 8) {
					addmore = true;
					chr->splatsdroppedhere++;
				}

				if (addmore) {
					chr->woundedsplatsadded += splatsCreate(1, 0.3f, prop, NULL, 0, 0, isskedar, SPLATTYPE_DROP, TICKS(80), attacker, 0);
				}
			}

			if (chr->woundedsplatsadded >= 40) {
				wallhitRemoveOldestWoundedSplatByChr(prop);
				chr->woundedsplatsadded--;
			}

			chr->deaddropsplatsadded = 0;
		}
	}

	chr->tickssincesplat += g_Vars.lvupdate60;
}

void splatsCreateForChrHit(struct prop *prop, struct shotdata *shotdata, struct coord *arg2, struct coord *arg3, bool isskedar, int splattype, struct chrdata *chr2)
{
	struct chrdata *chr = prop->chr;

	if (chr->bulletstaken < 7) {
		chr->bulletstaken++;
	}

	if (splattype == 0) {
		uint32_t qty = rngRandom() % 3;

		if (qty) {
			chr->stdsplatsadded += splatsCreate(qty, 0.8f, prop, shotdata, arg2, arg3, isskedar, splattype, TICKS(50), chr2, 0);
		}
	}
}

int splatsCreate(int qty, float arg1, struct prop *prop, struct shotdata *shotdataarg,
		struct coord *arg4, struct coord *arg5, bool isskedar, int splattype,
		int timermax, struct chrdata *chr, int timerspeed)
{
	struct shotdata stackshotdata;
	struct shotdata *shotdata = splattype == 0 ? shotdataarg : &stackshotdata;
	struct coord spfc;
	struct coord spf0;
	struct coord spe4;
	Mtxf spa4;
	int numdropped = 0;
	float dist;
	int i;
	int j;

	if (splattype == 0) {
		dist = coordsGetDistance(&shotdata->gunpos3d, arg5);

		for (i = 0; i < 3; i++) {
			spfc.f[i] = shotdata->gundir3d.f[i];
			spf0.f[i] = shotdata->gundir2d.f[i];
			shotdata->gunpos3d.f[i] = arg5->f[i];
			shotdata->gunpos2d.f[i] = arg4->f[i];
		}
	} else {
		float extraheight;

		if (prop->type == PROPTYPE_CHR) {
			extraheight = 50;
		} else {
			extraheight = 0;
		}

		dist = 0.7f;

		spfc.x = shotdata->gundir3d.x = 0;
		spfc.y = shotdata->gundir3d.y = -1;
		spfc.z = shotdata->gundir3d.z = 0;

		spf0.x = shotdata->gundir2d.x = 0;
		spf0.y = shotdata->gundir2d.y = -1;
		spf0.z = shotdata->gundir2d.z = 0;

		shotdata->gunpos3d.x = prop->pos.x;
		shotdata->gunpos3d.y = prop->pos.y + extraheight;
		shotdata->gunpos3d.z = prop->pos.z;

		shotdata->gunpos2d.x = prop->pos.x;
		shotdata->gunpos2d.y = prop->pos.y + extraheight;
		shotdata->gunpos2d.z = prop->pos.z;
	}

	for (i = 0; i < qty; i++) {
		for (j = 0; j < 3; j++) {
			spe4.f[j] = (RANDOMFRAC() * g_SplatRandomOffsetMax * 2.0f - g_SplatRandomOffsetMax) * 0.017453292384744f;
		}

		mtx4LoadRotationF(&spe4, &spa4);
		mtx4RotateVec(&spa4, &spfc, &shotdata->gundir3d);
		mtx4RotateVec(&spa4, &spf0, &shotdata->gundir2d);

		utilsNormalizeVec(&shotdata->gundir3d, &shotdata->gundir3d);
		utilsNormalizeVec(&shotdata->gundir2d, &shotdata->gundir2d);

		if (splat0f149274(arg1, prop, shotdata, /*reused var*/ dist, isskedar, splattype, timermax, chr, timerspeed)) {
			numdropped++;
		}
	}

	if (numdropped) {
		struct chrdata *chr = prop->chr;

		chr->tickssincesplat = 0;
		chr->lastdroppos.x = prop->pos.x;
		chr->lastdroppos.y = prop->pos.y;
		chr->lastdroppos.z = prop->pos.z;
	}

	return numdropped;
}

bool splat0f149274(float arg0, struct prop *chrprop, struct shotdata *shotdata, float arg3, bool isskedar, int splattype, int timermax, struct chrdata *chr, int timerspeed)
{
	struct prop **propptr;
	struct prop *objprop;
	struct hitthing hitthing;
	struct hitthing besthitthing;
	RoomNum rooms[32];
	RoomNum gunrooms[8];
	RoomNum endrooms[8];
	struct coord endpos;
	int i;
	struct coord *sp50c;
	struct coord *hitpos;
	struct coord *sp504;
	int bestroom = 0;
	int mtxindex;
	int room;
	float spraydistance;
	bool translucent;
	bool hasresult = false;
	struct shotdata stackshotdata;

	for (i = 0; i < 3; i++) {
		stackshotdata.gunpos2d.f[i] = shotdata->gunpos2d.f[i];
		stackshotdata.gundir2d.f[i] = shotdata->gundir2d.f[i];
		stackshotdata.gunpos3d.f[i] = shotdata->gunpos3d.f[i];
		stackshotdata.gundir3d.f[i] = shotdata->gundir3d.f[i];
	}

	stackshotdata.penetration = 1;

	for (i = 0; i < 3; i++) {
		endpos.f[i] = stackshotdata.gunpos3d.f[i] + stackshotdata.gundir3d.f[i] * g_SplatMaxDistance;
	}

	portalTraceLineThroughRooms(&chrprop->pos, &stackshotdata.gunpos3d, chrprop->rooms, gunrooms, NULL, 0);
	portalTraceLineThroughRooms(&stackshotdata.gunpos3d, &endpos, gunrooms, endrooms, rooms, ARRAYCOUNT(rooms) - 1);

	for (i = 0; rooms[i] != -1; i++) {
		if (bgTestHitInRoom(&stackshotdata.gunpos3d, &endpos, rooms[i], &hitthing)
				&& ((stackshotdata.gunpos3d.x <= endpos.x && hitthing.pos.x <= endpos.x && stackshotdata.gunpos3d.x <= hitthing.pos.x) || (endpos.x <= stackshotdata.gunpos3d.x && endpos.x <= hitthing.pos.x && hitthing.pos.x <= stackshotdata.gunpos3d.x))
					&& ((stackshotdata.gunpos3d.y <= endpos.y && hitthing.pos.y <= endpos.y && stackshotdata.gunpos3d.y <= hitthing.pos.y) || (endpos.y <= stackshotdata.gunpos3d.y && endpos.y <= hitthing.pos.y && hitthing.pos.y <= stackshotdata.gunpos3d.y))
					&& ((stackshotdata.gunpos3d.z <= endpos.z && hitthing.pos.z <= endpos.z && stackshotdata.gunpos3d.z <= hitthing.pos.z) || (endpos.z <= stackshotdata.gunpos3d.z && endpos.z <= hitthing.pos.z && hitthing.pos.z <= stackshotdata.gunpos3d.z))) {
			if (stackshotdata.gunpos3d.x != hitthing.pos.x || stackshotdata.gunpos3d.y != hitthing.pos.y || stackshotdata.gunpos3d.z != hitthing.pos.z) {
				bestroom = rooms[i];
				besthitthing = hitthing;

				endpos.x = hitthing.pos.x;
				endpos.y = hitthing.pos.y;
				endpos.z = hitthing.pos.z;

				hasresult = true;
				break;
			}
		}
	}

	if (hasresult) {
		spraydistance = coordsGetDistance(&stackshotdata.gunpos3d, &besthitthing.pos);

		if (spraydistance < g_SplatMaxDistance) {
			sp50c = &hitthing.pos;
			hitpos = &hitthing.pos;
			sp504 = &hitthing.unk0c;
			objprop = NULL;
			mtxindex = 0;
			room = bestroom;
			translucent = hitthing.unk2c == 2;
		} else {
			hasresult = false;
		}
	} else {
		spraydistance = 999999.0f;
	}

	if (splattype == 0) {
		stackshotdata.distance = (spraydistance < g_SplatMaxDistance ? spraydistance : g_SplatMaxDistance);
		stackshotdata.distance += arg3;

		for (i = 0; i < ARRAYCOUNT(stackshotdata.hits); i++) {
			stackshotdata.hits[i].prop = NULL;
			stackshotdata.hits[i].hitpart = 0;
			stackshotdata.hits[i].bboxnode = NULL;
		}

		propptr = g_Vars.endonscreenprops - 1;

		while (propptr >= g_Vars.onscreenprops) {
			struct prop *prop = *propptr;

			if (prop) {
				if (prop->type == PROPTYPE_OBJ || prop->type == PROPTYPE_DOOR || prop->type == PROPTYPE_WEAPON) {
					objTestHit(prop, &stackshotdata);
				}
			}

			propptr--;
		}

		for (i = 0; i < ARRAYCOUNT(stackshotdata.hits); i++) {
			struct hit *hit = &stackshotdata.hits[i];

			if (hit->prop && (hit->hitthing.texturenum < 0
						|| hit->hitthing.texturenum >= NUM_TEXTURES
						|| g_SurfaceTypes[g_Textures[hit->hitthing.texturenum].surfacetype]->numwallhittexes != 0)) {
				sp50c = &hit->hitthing.pos;
				hitpos = &hit->pos;
				sp504 = &hit->hitthing.unk0c;
				objprop = hit->prop;
				mtxindex = (int8_t)hit->mtxindex;
				room = 1;
				translucent = false;
				hasresult = true;
				break;
			}
		}
	}

	if (hasresult) {
		struct splatdata splatdata;

		for (i = 0; i < 3; i++) {
			splatdata.relpos.f[i] = sp50c->f[i];
			splatdata.unk0c.f[i] = hitpos->f[i];
			splatdata.gunpos.f[i] = stackshotdata.gunpos3d.f[i];
			splatdata.unk18.f[i] = sp504->f[i];
		}

		splatdata.chrprop = chrprop;
		splatdata.objprop = objprop;
		splatdata.chr = chr;
		splatdata.mtxindex = mtxindex;
		splatdata.room = room;
		splatdata.isskedar = isskedar;
		splatdata.timermax = timermax;
		splatdata.sizescale = arg0;
		splatdata.splattype = splattype;
		splatdata.timerspeed = timerspeed;
		splatdata.translucent = translucent;

		splat0f14986c(&splatdata);

		return true;
	}

	return false;
}

void splat0f14986c(struct splatdata *splat)
{
	float splatscalex; // Splat width before randomness is applied
	float splatscaley; // Splat height before randomness is applied
	struct defaultobj *obj;
	float splatscaledbydistance; // Splats get bigger the farther behind a character it's made
	float splatsizetype = 0; // Splats can be little, medium, or big
	float height;
	float width;
	uint8_t maxalpha = 255;
	uint8_t minalpha = 192;
	int texnum;
	bool isskedarblood = splat->isskedar & 1;
	bool translucent = splat->translucent;
	float distance;
	RoomNum smokerooms[2];

	texnum = WALLHITTEX_BLOOD1 + (rngRandom() % 3);

	if (splat->objprop != NULL && splat->objprop->type == PROPTYPE_OBJ) {
		obj = splat->objprop->obj;

		if (obj && (obj->type == OBJTYPE_GLASS || obj->type == OBJTYPE_TINTEDGLASS)) {
			minalpha = 0x40;
			translucent = true;
		}
	}

	switch (splat->splattype) {
	case SPLATTYPE_PUDDLE:
		texnum = WALLHITTEX_BLOOD1 + (rngRandom() % 3);
		break;
	case SPLATTYPE_DROP:
		texnum = WALLHITTEX_BLOOD4 + (rngRandom() % 1);
		break;
	}

	switch (rngRandom() % 6) {
	case 0:
	case 1:
	case 2:
		splatsizetype = 1.5f; // 1/2 chance of a little splat
		break;
	case 3:
	case 4:
		splatsizetype = 5.0f; // 1/3 chance of a big splat
		break;
	case 5:
		splatsizetype = 3.0f; // 1/6 chance of a medium splat
		break;
	}

	distance = coordsGetDistance(&splat->gunpos, &splat->unk0c);
	splatscaledbydistance = g_SplatDistanceScaleFactor * distance * splatsizetype;

	if (g_SplatMaxSize < splatscaledbydistance) {
		splatscaledbydistance = g_SplatMaxSize;
	}

	if (g_SplatMinSize > splatscaledbydistance) {
		splatscaledbydistance = g_SplatMinSize;
	}

	splatscalex = 0.5f * splatscaledbydistance;
	splatscaley = 0.5f * splatscaledbydistance;

	if (splatscalex < 1.0f) {
		splatscalex = 1.0f;
	}

	if (splatscaley < 1.0f) {
		splatscaley = 1.0f;
	}

	width = RANDOMFRAC() * splatscalex * 2.0f - splatscalex + splatscaledbydistance;
	height = RANDOMFRAC() * splatscaley * 2.0f - splatscaley + splatscaledbydistance;

	if (width > g_SplatMaxSize) {
		width = g_SplatMaxSize;
	}

	if (height > g_SplatMaxSize) {
		height = g_SplatMaxSize;
	}

	width *= splat->sizescale;
	height *= splat->sizescale;

	wallhitChooseBloodColour(splat->chrprop);

	wallhitCreateWith20Args(&splat->relpos, &splat->unk18, splat->chr ? &splat->chr->prop->pos : NULL, NULL,
			NULL, texnum, splat->room, splat->objprop,
			splat->chrprop, splat->mtxindex, 0, splat->chr,
			width, height, minalpha, maxalpha,
			rngRandom() % 360, (uint16_t)splat->timermax, splat->timerspeed, translucent);

	if (isskedarblood) {
		smokerooms[0] = splat->room;
		smokerooms[1] = -1;

		//smokeCreateSimple(&splat->unk0c, smokerooms, isskedarblood ? SMOKETYPE_SKCORPSE : SMOKETYPE_14); // Ben's comment: this ternary doesn't make sense since the condition must be true?
		smokeCreateSimple(&splat->unk0c, smokerooms, SMOKETYPE_SKCORPSE);
	}
}

void splatResetChr(struct chrdata *chr)
{
	chr->bulletstaken = 0;
	chr->tickssincesplat = 0;
	chr->stdsplatsadded = 0;
	chr->woundedsplatsadded = 0;
	chr->deaddropsplatsadded = 0;
	chr->splatsdroppedhere = 0;
	chr->lastdroppos.x = 0;
	chr->lastdroppos.y = 0;
	chr->lastdroppos.z = 0;
}
