#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "n_libaudio.h"
#include "constants.h"
#include "tiles.h"
#include "gfx.h"
#include "gbi.h"
#include "platform.h"

#define romptr_t uintptr_t

typedef int PakErr1;
typedef int PakErr2;
typedef int MenuDialogHandlerResult;
typedef uintptr_t MenuItemHandlerResult;
typedef int16_t RoomNum;

#ifdef PLATFORM_64BIT
typedef uint64_t k_ptr_t;
#else
typedef uint32_t k_ptr_t;
#endif

#define texnum_t uintptr_t

// Ben's comment: used for loading plaintext files for game strings e.g. briefings, weapon names, etc...
typedef struct {
	char **lines;
	int count;
} TextData;

// This hacky structure allows coords to be accessed using
// coord->x, coord->y and coord->z, but also as
// coord->f[0], coord->f[1] and coord->f[2].
// In some places code only matches when using the float array.
struct coord {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float f[3];
	};
};

struct dcoord {
	double x;
	double y;
	double z;
};

struct bbox {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
};

struct propstate {
	/*0x00*/ uint16_t propcount;
	/*0x02*/ uint16_t chrpropcount;
	/*0x04*/ uint16_t foregroundpropcount;
	/*0x06*/ uint16_t foregroundchrpropcount;
	/*0x08*/ int updatetime;
	/*0x0c*/ int chrupdatetime;
	/*0x10*/ int slotupdate240;
	/*0x14*/ int slotupdate60error;
	/*0x18*/ uint16_t lastupdateframe;
};

struct playerstats {
	/*0x00*/ int shotcount[7];
	/*0x1c*/ int killcount;
	/*0x20*/ int ggkillcount;
	/*0x24*/ int kills[MAX_PLAYERS];
	/*0x34*/ int drawplayercount;
	/*0x38*/ float distance;
	/*0x3c*/ int backshotcount;
	/*0x40*/ float armourcount;
	/*0x44*/ int fastest2kills;
	/*0x48*/ int slowest2kills;
	/*0x4c*/ int longestlife;
	/*0x50*/ int shortestlife;
	/*0x54*/ int maxkills;
	/*0x58*/ int maxsimulkills;
	/*0x5c*/ float damagescale; // of received damage
	/*0x60*/ int tokenheldtime;
	/*0x64*/ int unk64;
	/*0x68*/ int cloaktime;
	/*0x6c*/ int speedpillcount;

	// Temporary hack
	union {
	/*0x70*/ int mpindex;
	/*0x70*/ uint32_t mpindexu32;
	};

	/*0x74*/ float scale_bg2gfx;
	/*0x78*/ float damreceived;
	/*0x7c*/ float damtransmitted;
};

struct fileguid {
	int fileid;
	uint16_t deviceserial;
};

struct g_vars {
	// lv values pause when the game is paused, and reset when a new level is loaded
	// diff values run all the time
	// names ending in f are the same as their non-f counterparts, but as a float
	// names ending in freal are the same as f but multiplied by 1.2 on PAL
	// names ending in 60 mean their units are 60ths of a second
	// names ending in 240 mean their units are 240ths of a second
	// names ending in t mean their units are CPU ticks/cycles

	/*0x000*/ int diffframe60;         // time between current frame start and previous frame start in 60ths of a second
	/*0x004*/ float diffframe60f;        // always .0 decimal
	/*0x008*/ int lvframe60;           // incrementing 60ths, but it doesn't apply remainder so it drops behind lvframe240 / 4
	/*0x00c*/ int lvframenum;          // increments by 1 each frame
	/*0x010*/ float diffframe60freal;
	/*0x014*/ int prevframestartt;     // cycle count at start of previous frame
	/*0x018*/ int thisframestartt;     // cycle count at start of current frame
	/*0x01c*/ int diffframet;          // previous frame's duration in ticks, including any wait time after it finished
	/*0x020*/ int lostframetime60t;
	/*0x024*/ int lostframetime240t;
	/*0x028*/ uint32_t mininc60;            // minimum amount of time between frames (always set to 1, ie. max 60fps)
	/*0x02c*/ int roomportalrecursionlimit;
	/*0x030*/ int lvframe240;          // incrementing 240ths
	/*0x034*/ int lvupdate240;         // update multiplier, capped at 4 if slow motion
	/*0x038*/ int lvupdate60;          // update multiplier, factoring in remainder from previous frame
	/*0x03c*/ int lvupdate240rem;      // update multiplier's remainder for next frame's lvupdate60 (0 - 3)
	/*0x040*/ int diffframe240;        // time between current frame start and previous frame start in 240ths of a second
	/*0x044*/ float lvupdate60f;         // with decimal
	/*0x048*/ float diffframe240f;       // always .0 decimal
	/*0x04c*/ float lvupdate60freal;
	/*0x050*/ float lvupdate60frealprev; // previous frame's lvupdate60freal
	/*0x054*/ int prevframestart240;   // previous frame's start time in 240ths (incrementing)
	/*0x058*/ int thisframestart240;   // current frame's start time in 240ths (incrementing)
	/*0x05c*/ float diffframe240freal;
	/*0x060*/ int16_t *waypointnums; // ordered by room asc, padnum asc
	/*0x064*/ struct player *players[MAX_PLAYERS];
	/*0x074*/ struct playerstats playerstats[MAX_PLAYERS];
	/*0x274*/ uint32_t playerorder[MAX_PLAYERS];
	/*0x284*/ struct player *currentplayer;
	/*0x288*/ struct playerstats *currentplayerstats;
	/*0x28c*/ int currentplayernum; // 0-3 - controller numbers I think
	/*0x290*/ int currentplayerindex; // 0-3 - but 2 or 3 probably only used in combat simulator
	/*0x294*/ int bondplayernum;
	/*0x298*/ int coopplayernum;
	/*0x29c*/ int antiplayernum;
	/*0x2a0*/ struct player *bond; // Joanna
	/*0x2a4*/ struct player *coop; // Co-op buddy when controlled by human
	/*0x2a8*/ struct player *anti; // Counter-op
	/*0x2ac*/ int tickmode;
	/*0x2b0*/ int killcount;
	/*0x2b4*/ uint32_t knockoutcount;
	/*0x2b8*/ struct textoverride *textoverrides;
	/*0x2bc*/ int roomcount;
	/*0x2c0*/ int hitboundscount;
	/*0x2c4*/ struct modelnode *hitnodes[20];
	/*0x314*/ uint32_t mplayerisrunning;     // combat sim with any number of players, coop with human buddy, and counter-op
	/*0x318*/ uint32_t normmplayerisrunning; // combat sim with any number of players
	/*0x31c*/ int lvmpbotlevel;
	/*0x320*/ int lockscreen;
	/*0x324*/ bool bondvisible;
	/*0x328*/ bool bondcollisions;
	/*0x32c*/ bool enableslopes;
	/*0x330*/ uint32_t padrandomroutes;
	/*0x334*/ int maxprops;
	/*0x338*/ struct prop *props; // pointer to array of structs
	/*0x33c*/ struct prop *activeprops; // head of a doubly linked list
	/*0x340*/ struct prop *activepropstail; // next pointer points to pausedprops
	/*0x344*/ struct prop *freeprops; // head of a singularly linked list
	/*0x348*/ struct prop **onscreenprops; // sorted by distance, furthest first
	/*0x34c*/ struct prop **endonscreenprops;
	/*0x350*/ int numonscreenprops;
	/*0x354*/ struct prop *pausedprops; // head of a doubly linked list, prev pointer points to activepropstail
	/*0x358*/ uint8_t numpropstates;
	/*0x359*/ uint8_t allocstateindex;
	/*0x35a*/ uint8_t runstateindex;
	/*0x35b*/ uint8_t alwaystick;
	/*0x35c*/ uint16_t updateframe;
	/*0x35e*/ uint16_t prevupdateframe;
#ifdef AVOID_UB // there will be an OOB access otherwise; this does fuck up the offsets here though
	/*0x360*/ struct propstate propstates[8];
#else
	/*0x360*/ struct propstate propstates[7];
#endif
	/*0x424*/ struct chrdata *chrdata;
	/*0x428*/ struct truckobj *truck;
	/*0x42c*/ struct heliobj *heli;
	/*0x430*/ struct chopperobj *hovercar;
	/*0x434*/ uint8_t *ailist;
	/*0x438*/ uint32_t aioffset;
	/*0x43c*/ int hardfreeabletally;
	/*0x440*/ int antiheadnum;
	/*0x444*/ int antibodynum;
	/*0x448*/ int coopradaron;
	/*0x44c*/ int antiradaron;
	/*0x450*/ int pendingantiplayernum;
	/*0x454*/ int coopfriendlyfire;
	/*0x458*/ uint32_t modifiedfiles;
	/*0x45c*/ int speedpilltime; // in time60
	/*0x460*/ int speedpillchange;
	/*0x464*/ uint32_t speedpillwant;
	/*0x468*/ bool speedpillon;
	/*0x46c*/ int restartlevel;
	/*0x470*/ int perfectbuddynum;
	/*0x474*/ int numaibuddies;
	/*0x478*/ bool aibuddiesspawned;
	/*0x47c*/ int bossfileid;
	/*0x480*/ uint16_t bossdeviceserial;
	/*0x482*/ uint16_t language;
	/*0x484*/ int mphilltime;
	/*0x488*/ int totalkills;
	/*0x48c*/ bool useperimshoot;
	/*0x490*/ int mpsetupmenu;
	/*0x494*/ int8_t waitingtojoin[MAX_PLAYERS];
	/*0x498*/ int unk000498;
	/*0x49c*/ bool usingadvsetup;
	/*0x4a0*/ int unk0004a0;
	/*0x4a4*/ int mpquickteamnumsims;
	/*0x4a8*/ int mpsimdifficulty;
	/*0x4ac*/ int8_t mpplayerteams[MAX_PLAYERS];
	/*0x4b0*/ uint32_t mpquickteam;
	/*0x4b4*/ int stagenum;
	/*0x4b8*/ struct prop *aibuddies[4];
	/*0x4c8*/ uint32_t dontplaynrg; // allow X music to be played (NRG = energy track)
	/*0x4cc*/ int in_cutscene;
	/*0x4d0*/ uint8_t paksneededformenu;
	/*0x4d1*/ uint8_t paksneededforgame;
	/*0x4d2*/ int8_t autocutnum; // cutscene scene number (0, 1 or 2), set to -1 while loading cutscene
	/*0x4d3*/ int8_t autocutplaying; // true if playing a cutscene via the menu, false when a button is pressed
	/*0x4d4*/ int8_t autocutgroupcur; // index into g_Cutscenes
	/*0x4d5*/ int8_t autocutgroupleft; // number of cutscenes left to play
	/*0x4d6*/ int8_t autocutfinished; // true if cutscene reached natural end
	/*0x4d7*/ int8_t autocutgroupskip; // true if pressed start during auto cutscene
	/*0x4d8*/ int joydisableframestogo;
	/*0x4dc*/ uint8_t playertojoymap[MAX_PLAYERS];
	/*0x4e1*/ uint8_t remakewallhitvtx;
	/*0x4e2*/ uint8_t cutsceneskip60ths;
	/*0x4e3*/ uint8_t langfilteron;

	// 000f = Counter: if 10-15 then ticks down 1 per 7 frames, if under 10 then 1 per frame.
	// 00f0 = One bit per pak. These paks are checked and ticked.
	// 0f00 = One bit per pak. These paks are checked but not ticked.
	/*0x4e4*/ uint16_t pakstocheck;

	/*0x510*/ float unk000510;
};

struct weaponobj;
struct prop;
struct explosion;

/**
 * Most, if not all, entity types (chrs, objs, weapons etc) have a pointer to a
 * prop struct. The struct contains properties that most entities have in
 * common such as coords and a room number. You can think of it as generic
 * entity.
 *
 * The type field indicates what type it is (chr, obj etc) and the entity
 * pointer points back to the proper entity struct.
 */
struct prop {
	/*0x00*/ uint8_t type;
	/*0x01*/ uint8_t flags;
	/*0x02*/ int16_t timetoregen; // 0 when available, ticks down when unavailable

	/*0x04*/
	union {
		struct chrdata *chr;
		struct defaultobj *obj;
		struct doorobj *door;
		struct weaponobj *weapon;
		struct explosion *explosion;
		struct smoke *smoke;
	};

	/*0x08*/ struct coord pos;
	/*0x14*/ float z;
	union {
		/*0x18*/ struct prop *parent;
		/*0x18*/ struct model *parentmodel;
	};
	/*0x1c*/ struct prop *child;
	/*0x20*/ struct prop *next;
	/*0x24*/ struct prop *prev;
	/*0x28*/ RoomNum rooms[8];
	/*0x38*/ uint16_t lastupdateframe;
	/*0x3a*/ uint16_t propupdate240;
	/*0x3c*/ uint8_t propupdate60err;
	/*0x3d*/ uint8_t propstateindex;
	/*0x3e*/ uint8_t backgroundedframes;
	/*0x3f*/ uint8_t forceonetick : 1;
	/*0x3f*/ uint8_t backgrounded : 1;
	/*0x3f*/ uint8_t forcetick : 1;
	/*0x3f*/ uint8_t active : 1;
	/*0x40*/ struct wallhit *opawallhits; // opaque
	/*0x44*/ struct wallhit *xluwallhits; // translucent
};

struct packedpad {
#ifdef PLATFORM_BIG_ENDIAN
	int liftnum : 4;
	int flags : 18;
	int room : 10;
#else
	int liftnum : 4;
	int room : 10;
	int flags : 18;
#endif
};

struct pad {
	/*0x00*/ struct coord pos;
	/*0x0c*/ struct coord look;
	/*0x18*/ struct coord up;
	/*0x24*/ struct coord normal;
	/*0x30*/ struct bbox bbox;
	/*0x48*/ int room;
	/*0x4c*/ uint32_t flags;
	/*0x50*/ uint8_t liftnum; // 1-10, 0 indicates no lift
	/*0x52*/ int16_t unk52;
};

union filedataptr {
	uint8_t *uint8_t;
	uint32_t *uint32_t;
};

struct attackanimconfig {
	/*0x00*/ int16_t animnum;
	/*0x04*/ float unk04; // frame number
	/*0x08*/ float unk08;
	/*0x0c*/ float unk0c;
	/*0x10*/ float unk10; // frame number
	/*0x14*/ float unk14; // frame number
	/*0x18*/ float unk18; // frame number
	/*0x1c*/ float unk1c; // frame number
	/*0x20*/ float unk20; // frame number
	/*0x24*/ float unk24; // frame number
	/*0x28*/ float unk28; // frame number
	/*0x2c*/ float unk2c; // frame number
	/*0x30*/ float unk30;
	/*0x34*/ float unk34;
	/*0x38*/ float unk38;
	/*0x3c*/ float unk3c;
	/*0x40*/ float unk40;
	/*0x44*/ float unk44;
};

struct model;

struct anim {
	/*0x00*/ int16_t animnum;
	/*0x02*/ int16_t animnum2;
	/*0x04*/ uint8_t frameslot1;
	/*0x05*/ uint8_t frameslot2;
	/*0x06*/ uint8_t frameslot3;
	/*0x07*/ uint8_t frameslot4;
	/*0x08*/ int8_t flip;
	/*0x09*/ int8_t flip2;
	/*0x0a*/ int8_t looping;
	/*0x0b*/ int8_t average;
	/*0x0c*/ float frame;
	/*0x10*/ float frac;
	/*0x14*/ int16_t framea;
	/*0x16*/ int16_t frameb;
	/*0x18*/ float endframe;
	/*0x1c*/ float speed;
	/*0x20*/ float newspeed;
	/*0x24*/ float oldspeed;
	/*0x28*/ float timespeed;
	/*0x2c*/ float elapsespeed;
	/*0x30*/ float frame2;
	/*0x34*/ float frac2;
	/*0x38*/ int16_t frame2a;
	/*0x3a*/ int16_t frame2b;
	/*0x3c*/ float endframe2;
	/*0x40*/ float speed2;
	/*0x44*/ float newspeed2;
	/*0x48*/ float oldspeed2;
	/*0x4c*/ float timespeed2;
	/*0x50*/ float elapsespeed2;
	/*0x54*/ float fracmerge;
	/*0x58*/ float timemerge;
	/*0x5c*/ float elapsemerge;
	/*0x60*/ float loopframe;
	/*0x64*/ float loopmerge;
	/*0x68*/ void (*flipfunc)(void);
	/*0x6c*/ uint32_t unk6c;
	/*0x70*/ bool (*unk70)(struct model *model, struct coord *arg1, struct coord *arg2, float *ground);
	/*0x74*/ float playspeed;
	/*0x78*/ float newplay;
	/*0x7c*/ float oldplay;
	/*0x80*/ float timeplay;
	/*0x84*/ float elapseplay;
	/*0x88*/ float animscale;
};

struct skeleton {
	int16_t skel;
	uint16_t numthings;
	uint8_t (*things)[2];
};

struct modelrodata_chrinfo { // type 0x01
	uint16_t animpart;
	int16_t mtxindex;
	float unk04;
	uint16_t rwdataindex;
};

struct modelrodata_position { // type 0x02
	struct coord pos;
	uint16_t part;
	union {
		int16_t mtxindexes[3];
		struct {
			int16_t mtxindex0;
			int16_t mtxindex1;
			int16_t mtxindex2;
		};
	};
	float drawdist;
};

struct modelrodata_gundl { // type 0x04
	Gfx *opagdl;
	Gfx *xlugdl;
	void *baseaddr;
	Vtx *vertices;
	int16_t numvertices;
	int16_t unk12;
};

struct modelrodata_distance { // type 0x08
	float near;
	float far;
	struct modelnode *target;
	uint16_t rwdataindex;
};

struct modelrodata_reorder { // type 0x09
	float unk00;
	float unk04;
	float unk08;
	float unk0c[3];
	struct modelnode *unk18;
	struct modelnode *unk1c;
	int16_t side;
	uint16_t rwdataindex;
};

struct modelrodata_bbox { // type 0x0a
	int hitpart;
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
};

struct modelrodata_type0b { // type 0x0b
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	uint32_t unk10;
	uint32_t unk14;
	uint32_t unk18;
	uint32_t unk1c;
	uint32_t unk20;
	uint32_t unk24;
	uint32_t unk28;
	uint32_t unk2c;
	uint32_t unk30;
	uint32_t unk34;
	uint32_t unk38;
	void *unk3c;
	uint32_t unk40;
	uint16_t rwdataindex;
	void *baseaddr;
};

struct modelrodata_chrgunfire { // type 0x0c
	struct coord pos;
	struct coord dim;
	struct textureconfig *texture;
	float unk1c;
	uint16_t rwdataindex;
	void *baseaddr;
};

struct modelrodata_type0d { // type 0x0d
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	void *unk10;
	void *unk14;
	uint32_t unk18;
	void *baseaddr;
};

struct modelrodata_type11 { // type 0x11
	uint32_t unk00;
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	uint32_t unk10;
	void *unk14;
};

struct modelrodata_toggle { // type 0x12
	struct modelnode *target;
	uint16_t rwdataindex;
};

struct modelrodata_positionheld { // type 0x15
	struct coord pos;
	int16_t mtxindex;
};

struct modelrodata_stargunfire { // type 0x16
	int unk00;
	Vtx *vertices;
	Gfx *gdl;
	void *baseaddr;
};

struct modelrodata_headspot { // type 0x17
	uint16_t rwdataindex;
};

struct modelrodata_dl { // type 0x18
	/*0x00*/ Gfx *opagdl;
	/*0x04*/ Gfx *xlugdl;
	/*0x08*/ Col *colours;
	/*0x0c*/ Vtx *vertices; // colours follow this array
	/*0x10*/ int16_t numvertices;
	/*0x12*/ int16_t mcount;
	/*0x14*/ uint16_t rwdataindex;
	/*0x16*/ uint16_t numcolours;
};

struct modelrodata_type19 { // type 0x19
	/*0x00*/ int numvertices;
	/*0x04*/ struct coord vertices[4];
};

union modelrodata {
	struct modelrodata_chrinfo chrinfo;
	struct modelrodata_position position;
	struct modelrodata_gundl gundl;
	struct modelrodata_distance distance;
	struct modelrodata_reorder reorder;
	struct modelrodata_bbox bbox;
	struct modelrodata_type0b type0b;
	struct modelrodata_chrgunfire chrgunfire;
	struct modelrodata_type0d type0d;
	struct modelrodata_type11 type11;
	struct modelrodata_toggle toggle;
	struct modelrodata_positionheld positionheld;
	struct modelrodata_stargunfire stargunfire;
	struct modelrodata_headspot headspot;
	struct modelrodata_dl dl;
	struct modelrodata_type19 type19;
};

struct modelnode {
	/*0x00*/ uint16_t type;
	/*0x04*/ union modelrodata *rodata;
	/*0x08*/ struct modelnode *parent;
	/*0x0c*/ struct modelnode *next;
	/*0x10*/ struct modelnode *prev;
	/*0x14*/ struct modelnode *child;
};

struct modeldef {
	struct modelnode *rootnode;
	struct skeleton *skel;

	// This is a pointer to a variable length array of pointers to modelnodes,
	// but the array is followed by an int16_t array of part numbers.
	struct modelnode **parts;

	int16_t numparts;
	int16_t nummatrices;
	float scale;
	int16_t rwdatalen; // in words
	int16_t numtexconfigs;
	struct textureconfig *texconfigs;
};

struct model {
	/*0x00*/ uint8_t unk00;
	/*0x01*/ uint8_t unk01;
	/*0x02*/ int16_t rwdatalen; // in words
	/*0x04*/
	union {
		struct chrdata *chr;
		struct defaultobj *obj;
	};
	/*0x08*/ struct modeldef *definition;
	/*0x0c*/ Mtx *matrices;
	/*0x10*/ uint32_t *rwdatas;
	/*0x14*/ float scale;
	/*0x18*/ struct model *attachedtomodel;
	/*0x1c*/ struct modelnode *attachedtonode;
	/*0x20*/ struct anim *anim;
};

struct modelrwdata_chrinfo { // type 0x01
	int8_t unk00;
	int8_t unk01;
	int8_t unk02;
	float ground;
	struct coord pos;
	float yrot; // angle
	float unk18;
	float unk1c;
	float unk20; // angle
	struct coord unk24;
	float unk30; // angle
	struct coord unk34;
	struct coord unk40; // "2" version of unk24
	struct coord unk4c; // "2" version of unk34
	float unk58;
	float unk5c;
};

struct modelrwdata_05 { // type 0x05
	int unk00;
};

struct modelrwdata_distance { // type 0x08
	int visible;
};

struct modelrwdata_reorder { // type 0x09
	int reverse;
};

struct modelrwdata_0b { // type 0x0b
	uint16_t unk00;
	uint16_t unk04;
};

struct modelrwdata_chrgunfire { // type 0x0c
	int16_t visible;
	uint16_t unk02;
};

struct modelrwdata_toggle { // type 0x12
	int visible;
};

struct modelrwdata_headspot { // type 0x17
	struct modeldef *headmodeldef;
	void *rwdatas;
};

struct modelrwdata_dl { // type 0x18
	Vtx *vertices;
	Gfx *gdl;
	Col *colours;
};

union modelrwdata {
	struct modelrwdata_chrinfo chrinfo;
	struct modelrwdata_05 type05;
	struct modelrwdata_distance distance;
	struct modelrwdata_reorder reorder;
	struct modelrwdata_0b type0b;
	struct modelrwdata_chrgunfire chrgunfire;
	struct modelrwdata_toggle toggle;
	struct modelrwdata_headspot headspot;
	struct modelrwdata_dl dl;
};

struct waygroup {
	int *neighbours;
	int *waypoints;
	int step;
};

struct waypoint {
	int padnum;
	int *neighbours; // most significant two bits are booleans, remaining bits are waypoint index
	int groupnum;
	int step;
};

struct aibot {
	/*0x000*/ uint8_t followchance;
	/*0x002*/ int16_t aibotnum;
	/*0x004*/ struct mpbotconfig *config;
	/*0x008*/ int16_t attackingplayernum;
	/*0x00a*/ int16_t followingplayernum;
	/*0x00c*/ int16_t dangerouspropnum; // index into g_DangerousProps
	/*0x010*/ struct prop *gotoprop;
	/*0x014*/ struct invitem *items;
	/*0x018*/ int8_t maxitems;
	/*0x01c*/ int *ammoheld;
	/*0x020*/ int weaponnum;
	/*0x024*/ int loadedammo[2]; // amount of ammo in current clip
	/*0x02c*/ int16_t timeuntilreload60[2];
	/*0x030*/ uint32_t unk030; // unused
	/*0x034*/ int throwtimer60;
	/*0x038*/ uint32_t unk038; // unused
	/*0x03c*/ int16_t punchtimer60[2];
	/*0x040*/ float unk040; // unused
	/*0x044*/ struct prop *skrocket;
	/*0x048*/ int16_t hillpadnum;
	/*0x04a*/ int16_t hillcovernum;
	/*0x04c*/ uint8_t inhill : 1;
	/*0x04c*/ uint8_t ismeleeweapon : 1;
	/*0x04c*/ uint8_t gunfunc : 1;
	/*0x04c*/ uint8_t unk04c_03 : 1; // unused
	/*0x04c*/ uint8_t unk04c_04 : 1; // unused
	/*0x04c*/ uint8_t hasuplink : 1;
	/*0x04c*/ uint8_t cloakdeviceenabled : 1;
	/*0x04d*/ uint8_t burstsdone[2];
	/*0x04f*/ uint8_t teamisonlyai : 1;
	/*0x050*/ struct prop *distoverrideprop;
	/*0x054*/ uint32_t unk054; // unused
	/*0x058*/ uint8_t fadeintimer60;
	/*0x059*/ uint8_t respawning;
	/*0x05c*/ int nextbullettimer60[2];
	/*0x064*/ uint16_t flags;
	/*0x068*/ struct attackanimconfig *attackanimconfig;
	/*0x06c*/ float speedmultforwards;
	/*0x070*/ float speedmultsideways;
	/*0x074*/ int8_t distmode;
	/*0x076*/ int16_t lastkilledbyplayernum;
	/*0x078*/ uint8_t forceslowupdates;
	/*0x079*/ uint8_t command;
	/*0x07a*/ RoomNum defendholdrooms[1];
	/*0x07c*/ uint32_t unk07c; // unused
	/*0x080*/ uint32_t unk080; // unused
	/*0x084*/ uint32_t unk084; // unused
	/*0x088*/ uint32_t unk088; // unused
	/*0x08c*/ struct coord defendholdpos;
	/*0x098*/ float defendholdrot;
	/*0x09c*/ uint8_t hasbriefcase : 1; // htb
	/*0x09c*/ uint8_t hascase : 1;      // ctc
	/*0x09c*/ uint8_t rcp120cloakenabled : 1;
	/*0x09c*/ uint8_t canseecloaked : 1;
	/*0x09c*/ uint8_t unk09c_04 : 3; // unused
	/*0x09c*/ uint8_t cheap : 1; // True if aibot is not in any visible room
	/*0x09d*/ uint8_t distoverridetimer60;
	/*0x0a0*/ int htbheldtimer60;
	/*0x0a4*/ float roty;
	/*0x0a8*/ float angleoffset;
	/*0x0ac*/ float speedtheta;
	/*0x0b0*/ float lookangle; // likely to be the turn angle to 360 degrees, in radians
	/*0x0b4*/ float moveratex;
	/*0x0b8*/ float moveratey;
	/*0x0bc*/ int lastknownhill;
	/*0x0c0*/ int attackpropnum;
	/*0x0c4*/ bool cyclonedischarging[2];
	/*0x0cc*/ int changeguntimer60;
	/*0x0d0*/ int distmodettl60;
	/*0x0d4*/ int followprotectpropnum;
	/*0x0d8*/ bool forcemainloop;
	/*0x0dc*/ int returntodefendtimer60;
	/*0x0e0*/ int16_t reaperspeed[2];
	/*0x0e4*/ float maulercharge[2];
	/*0x0ec*/ struct coord gotopos;
	/*0x0f8*/ RoomNum gotorooms[8];
	/*0x108*/ struct coord shotspeed; // "boost" when aibot is shot
	/*0x114*/ int feudplayernum;
	/*0x118*/ int commandtimer60;
	/*0x11c*/ int shootdelaytimer60; // ticks up when target onscreen, down when offscreen, always >= 0
	/*0x120*/ int targetlastseen60;
	/*0x124*/ int lastseenanytarget60;
	/*0x128*/ bool targetinsight;
	/*0x12c*/ int queryplayernum;
	/*0x130*/ int8_t chrnumsbydistanceasc[MAX_MPCHRS];
	/*0x13c*/ float chrdistances[MAX_MPCHRS];
	/*0x16c*/ uint8_t chrsinsight[MAX_MPCHRS];
	/*0x178*/ int chrslastseen60[MAX_MPCHRS];
	/*0x1a8*/ RoomNum chrrooms[MAX_MPCHRS];
	/*0x1c0*/ float extraangle;
	/*0x1c4*/ float extraanglerate;
	/*0x1c8*/ float extraanglebase;
	/*0x1cc*/ int random3ttl60;
	/*0x1d0*/ uint32_t random3;
	/*0x1d4*/ float targetinsighttemperature;
	/*0x1d8*/ int abortattacktimer60;
	/*0x1dc*/ bool canbreakdefend;
	/*0x1e0*/ bool canbreakfollow;
	/*0x1e4*/ int realignangleframe;
	/*0x1e8*/ struct waypoint *waypoints[8];
	/*0x208*/ int numwaystepstotarget;
	/*0x20c*/ int random1ttl60;
	/*0x210*/ uint32_t random1;
	/*0x214*/ float killsbygunfunc[NUM_MPWEAPONSLOTS][2];
	/*0x244*/ float suicidesbygunfunc[NUM_MPWEAPONSLOTS][2];
	/*0x274*/ int equipdurations60[NUM_MPWEAPONSLOTS][2];
	/*0x2a4*/ int equipextrascorestimer60;
	/*0x2a8*/ int equipextrascores[NUM_MPWEAPONSLOTS];
	/*0x2c0*/ int dampensuicidesttl60;
	/*0x2c4*/ float rcpcloaktimer60;

	/**
	 * 120 when target is in sight
	 * 0 when target is not in sight
	 * Ticks down when target is in sight but cloaked
	 *
	 * Bot maintains target while cloak timer is active
	 */
	/*0x2c8*/ int targetcloaktimer60;

	/*0x2cc*/ int random2ttl60;
	/*0x2d0*/ uint32_t random2;
	/*0x2d4*/ float randomfrac;
	/*0x2d8*/ uint32_t unk2d8; // unused
	/*0x2dc*/ uint32_t unk2dc; // unused
};

struct geo {
	/*0x00*/ uint8_t type;
	/*0x01*/ uint8_t numvertices;
	/*0x02*/ uint16_t flags;
};

struct geotilei {
	struct geo header;
	/*0x04*/ uint16_t floortype;
	/*0x06*/ uint8_t xmin; // These are byte offsets relative to the start of tile
	/*0x07*/ uint8_t ymin;
	/*0x08*/ uint8_t zmin;
	/*0x09*/ uint8_t xmax;
	/*0x0a*/ uint8_t ymax;
	/*0x0b*/ uint8_t zmax;
	/*0x0c*/ uint16_t floorcol;
	/*0x0e*/ int16_t vertices[64][3];
};

struct geotilef {
	struct geo header;
	/*0x04*/ uint16_t floortype;
	union {
		// The arrays are surely the correct type here, but they create
		// mismatches in code that has already been matched using individual
		// properties (eg. cdCollectGeoForCylFromList). @TODO: Rematch them using the arrays.
		struct {
			/*0x06*/ uint8_t min[3]; // These are indexes into vertices
			/*0x09*/ uint8_t max[3];
		};
		struct {
			/*0x06*/ uint8_t xmin;
			/*0x07*/ uint8_t ymin;
			/*0x08*/ uint8_t zmin;
			/*0x09*/ uint8_t xmax;
			/*0x0a*/ uint8_t ymax;
			/*0x0b*/ uint8_t zmax;
		};
	};
	/*0x0c*/ uint16_t floorcol;
	/*0x10*/ struct coord vertices[64];
};

struct geoblock {
	struct geo header;
	/*0x04*/ float ymax;
	/*0x08*/ float ymin;
	/*0x0c*/ float vertices[8][2];
};

struct geocyl {
	struct geo header;
	/*0x04*/ float ymax;
	/*0x08*/ float ymin;
	/*0x0c*/ float x;
	/*0x10*/ float z;
	/*0x14*/ float radius;
};

union geounion {
	struct geotilei tilei;
	struct geotilef tilef;
	struct geoblock block;
	struct geocyl cyl;
};

struct act_stand {
	/*0x2c*/ int prestand;
	/*0x30*/ int flags;
	/*0x34*/ int entityid;
	/*0x38*/ bool reaim;
	/*0x3c*/ int turning;
	/*0x40*/ bool checkfacingwall;
	/*0x44*/ int wallcount;
	/*0x48*/ float mergetime;
	/*0x4c*/ uint8_t playwalkanim;
};

struct act_anim {
	/*0x2c*/ bool movewheninvis;
	/*0x30*/ bool pauseatend;
	/*0x34*/ bool completed;
	/*0x38*/ bool slowupdate;
	/*0x3c*/ bool lockpos;
	/*0x40*/ uint8_t ishitanim;
	/*0x41*/ uint8_t reverse;
	/*0x42*/ uint16_t hitframe;
	/*0x44*/ uint16_t hitdamage;
	/*0x46*/ uint16_t hitradius;
	/*0x48*/ int16_t animnum;
	/*0x4a*/ uint8_t flip;
	/*0x4c*/ float startframe;
	/*0x50*/ float endframe;
	/*0x54*/ float speed;
	/*0x58*/ float blend;
};

struct act_die {
	/*0x2c*/ int notifychrindex;
	/*0x30*/ float thudframe1;
	/*0x34*/ float thudframe2;
	/*0x38*/ float timeextra;
	/*0x3c*/ float elapseextra;
	/*0x40*/ struct coord extraspeed; // for changing position while airborne (eg. when shot by magnum)
	/*0x4c*/ int16_t drcarollimagedelay;
};

struct act_dead {
	/*0x2c*/ bool fadenow;
	/*0x30*/ bool fadewheninvis;
	/*0x34*/ int invistimer60;
	/*0x38*/ int fadetimer60;
	/*0x3c*/ int notifychrindex;
};

struct act_argh {
	/*0x2c*/ int notifychrindex;
	/*0x30*/ int lvframe60;
};

// Gun settings
struct gset {
	uint8_t weaponnum;
	uint8_t unk0639;
	uint8_t unk063a;
	uint8_t weaponfunc; // 0 or 1
};

struct act_preargh {
	/*0x2c*/ struct coord dir;
	/*0x38*/ float relshotdir;
	/*0x3c*/ int hitpart;
	/*0x40*/ struct gset gset;
	/*0x44*/ int aplayernum;
};

struct act_attack {
	/*0x2c*/ struct attackanimconfig *animcfg;
	/*0x30*/ int8_t turning;
	/*0x31*/ int8_t fired;
	/*0x32*/ int8_t nextgun;
	/*0x33*/ int8_t numshots; // number of shots fired so far in this anim
	/*0x34*/ int8_t maxshots; // number of shots to attempt in this anim
	/*0x35*/ int8_t onehanded;
	/*0x36*/ int8_t dorecoil;
	/*0x37*/ int8_t dooneburst;
	/*0x38*/ int8_t firegun[2]; // whether this gun is going to attempt to fire in this anim
	/*0x3a*/ int8_t everytick[2]; // whether gun's fire rate is high enough to fire on every tick
	/*0x3c*/ uint8_t singleshot[2];
	/*0x3e*/ int8_t flip;
	/*0x40*/ int pausecount;
	/*0x44*/ int lastfire60;
	/*0x48*/ int lastontarget60;
	/*0x4c*/ uint32_t flags;
	/*0x50*/ int entityid;
	/*0x54*/ int standing;
	/*0x58*/ int reaim;
};

struct act_attackwalk {
	/*0x2c*/ int unk02c;
	/*0x30*/ int frame60count;
	/*0x34*/ int frame60max;
	/*0x38*/ bool facedtarget;
	/*0x3c*/ struct attackanimconfig *animcfg;
	/*0x40*/ int nextshot60;
	/*0x44*/ int nextgun;
	/*0x48*/ int8_t firegun[2];
	/*0x4a*/ int8_t everytick[2];
	/*0x4c*/ int8_t singleshot[2];
	/*0x4e*/ uint8_t flip;
	/*0x50*/ int dorecoil;
	/*0x54*/ float turnspeed;
};

struct act_sidestep {
	/*0x2c*/ bool side;
};

struct act_jumpout {
	/*0x2c*/ bool side;
};

struct act_runpos {
	/*0x2c*/ struct coord pos;
	/*0x38*/ float neardist;
	/*0x3c*/ int eta60;
	/*0x40*/ float turnspeed;
};

struct waydata {
	/*0x00*/ int8_t mode;
	/*0x01*/ int8_t iter;
	/*0x02*/ int8_t gotaimpos;
	/*0x03*/ int8_t gotaimposobj;
	/*0x04*/ struct coord aimpos; // world coords of next pad
	/*0x10*/ struct coord obstacleleft;  // world coord of left (from chr's perspective) edge of obj
	/*0x1c*/ struct coord obstacleright; // world coord of right (from chr's perspective) edge of obj
	/*0x28*/ int age;
	/*0x2c*/ struct coord aimposobj; // left or right edge + chr's width, or aimpos if no obj

	// These are the distances between the current waypoint and the previous
	// when using magic mode.
	/*0x38*/ float magicdone;
	/*0x3c*/ float magictotal;
	/*0x40*/ int lastvisible60;
};

struct act_patrol {
	/*0x02c*/ struct path *path;
	/*0x030*/ int nextstep;
	/*0x034*/ int forward;
	/*0x038*/ struct waydata waydata;
	/*0x07c*/ float turnspeed;
};

struct act_gopos {
	/*0x02c*/ struct coord endpos;
	/*0x038*/ RoomNum endrooms[8];
	/*0x048*/ struct waypoint *target; // Target/final waypoint

	// Array of pointers to the next couple of waypoints. Recalculated each time
	// a waypoint is reached, and probably even more frequently than that.
	/*0x04c*/ struct waypoint *waypoints[MAX_CHRWAYPOINTS];

	// Index of the waypoint in the above array that the chr is running to. If
	// the chr has line of sight (through doors) to the next or next + 1 then
	// the index can be changed to that one and the chr will run straight to it.
	// This index will always be 0, 1 or 2. When it reaches 3 the pathfinding is
	// recalculated, the array is replaced with a new one and index is set to 0.
	/*0x064*/ uint8_t curindex;

	/*0x065*/ uint8_t flags;
	/*0x066*/ uint16_t restartttl;
	/*0x068*/ struct waydata waydata;
	/*0x0ac*/ float turnspeed;
};

struct act_surprised {
	/*0x2c*/ uint32_t type;
};

struct act_throwgrenade {
	/*0x2c*/ uint32_t flags;
	/*0x30*/ uint32_t entityid;
	/*0x34*/ uint32_t hand;
	/*0x38*/ bool needsequip;
};

struct act_bondmulti {
	/*0x2c*/ struct attackanimconfig *animcfg;
};

struct act_druggedcomingup {
	/*0x2c*/ int16_t timer60;
};

struct act_robotattack {
	/*0x2c*/ struct coord pos[2];
	/*0x44*/ struct coord dir[2];
	/*0x5c*/ uint32_t guntype[2];
	/*0x64*/ int numshots[2];
	/*0x6c*/ uint8_t firing[2];
	/*0x6e*/ uint8_t finished;
};

struct act_skjump {
	/*0x2c*/ uint8_t state;
	/*0x2d*/ uint8_t needsnewanim;
	/*0x2e*/ uint8_t hit;
	/*0x30*/ float vel[2];
	/*0x38*/ float roty;
	/*0x3c*/ int timer60; // counts down
	/*0x40*/ struct coord pos;
	/*0x4c*/ int total60;
	/*0x50*/ float ground;
};

struct chrdata {
	/*0x000*/ int16_t chrnum;
	/*0x002*/ int8_t accuracyrating;
	/*0x003*/ int8_t speedrating; // 0-100
	/*0x004*/ uint8_t firecount[2];
	/*0x006*/ int8_t headnum;
	/*0x007*/ int8_t actiontype;
	/*0x008*/ int8_t sleep;
	/*0x009*/ int8_t invalidmove;
	/*0x00a*/ int8_t numclosearghs;
	/*0x00b*/ int8_t numarghs;
	/*0x00c*/ uint8_t fadealpha;
	/*0x00d*/ int8_t arghrating;
	/*0x00e*/ int8_t aimendcount;
	/*0x00f*/ uint8_t grenadeprob;
	/*0x010*/ int16_t bodynum;
	/*0x012*/ int8_t flinchcnt;
	/*0x013*/ int8_t path;
	/*0x014*/ uint32_t hidden;
	/*0x018*/ uint32_t chrflags;
	/*0x01c*/ struct prop *prop;
	/*0x020*/ struct model *model;
	/*0x024*/ float radius;
	/*0x028*/ float height;

	union {
		struct act_stand act_stand;
		struct act_anim act_anim;
		struct act_die act_die;
		struct act_dead act_dead;
		struct act_argh act_argh;
		struct act_preargh act_preargh;
		struct act_attack act_attack;
		struct act_attackwalk act_attackwalk;
		// act_attackroll uses act_attack
		struct act_sidestep act_sidestep;
		struct act_jumpout act_jumpout;
		struct act_runpos act_runpos;
		struct act_patrol act_patrol;
		struct act_gopos act_gopos;
		struct act_surprised act_surprised;
		struct act_throwgrenade act_throwgrenade;
		struct act_bondmulti act_bondmulti;
		// act_bot_attackstand uses act_attack
		// act_bot_attackkneel uses act_attack
		// act_bot_attackstrafe uses act_attack
		// act_druggeddrop uses act_die
		// act_druggedko uses act_dead
		struct act_druggedcomingup act_druggedcomingup;
		// act_attackamount uses act_attack
		struct act_robotattack act_robotattack;
		struct act_skjump act_skjump;
	};

	/*0x0b0*/ float sumground;
	/*0x0b4*/ float manground;
	/*0x0b8*/ float ground;
	/*0x0bc*/ struct coord fallspeed;
	/*0x0c8*/ struct coord prevpos;
	/*0x0d4*/ int lastwalk60;
	/*0x0d8*/ int lastmoveok60;
	/*0x0dc*/ float visionrange;
	/*0x0e0*/ int lastseetarget60;
	/*0x0e4*/ int lastvisibletarget60;
	/*0x0e8*/ struct prop *poisonprop;
	/*0x0ec*/ int16_t lastshooter;
	/*0x0ee*/ int16_t timeshooter;
	/*0x0f0*/ float hearingscale;
	/*0x0f4*/ int lastheartarget60;
	/*0x0f8*/ uint8_t shadecol[4];
	/*0x0fc*/ uint8_t nextcol[4];
	/*0x100*/ float damage;
	/*0x104*/ float maxdamage;
	/*0x108*/ uint8_t *ailist;
	/*0x10c*/ uint16_t aioffset;
	/*0x10e*/ int16_t aireturnlist;
	/*0x110*/ int16_t aishotlist;
	/*0x112*/ uint8_t morale;
	/*0x113*/ uint8_t alertness;
	/*0x114*/ uint32_t flags;
	/*0x118*/ uint32_t flags2;
	/*0x11c*/ int timer60;
	/*0x120*/ int soundtimer;
	/*0x124*/ uint8_t random;
	/*0x125*/ uint8_t team;
	/*0x126*/ uint8_t soundgap;
	/*0x128*/ int16_t padpreset1;
	/*0x12a*/ int16_t chrpreset1;
	/*0x12c*/ int16_t proppreset1;
	/*0x12e*/ int16_t chrseeshot;
	/*0x130*/ int16_t chrseedie;
	/*0x132*/ int16_t chrdup;
	struct geocyl geo;
	/*0x14c*/ float shotbondsum;
	/*0x150*/ float aimuplshoulder;
	/*0x154*/ float aimuprshoulder;
	/*0x158*/ float aimupback;
	/*0x15c*/ float aimsideback;
	/*0x160*/ float aimendlshoulder;
	/*0x164*/ float aimendrshoulder;
	/*0x168*/ float aimendback;
	/*0x16c*/ float aimendsideback;
	/*0x170*/ struct prop *weapons_held[3]; // gun 1, gun 2, hat
	/*0x17c*/ int8_t fireslots[2];
	/*0x17e*/ int16_t target; // index into g_Vars.props
	/*0x180*/ float cshield;

	// The cm fields are related to the chr's shield visual effect
	/*0x184*/ int8_t cmnum;
	/*0x185*/ int8_t cmnum2;
	/*0x186*/ int8_t cmnum3;
	/*0x187*/ int8_t cmnum4;
	/*0x188*/ uint16_t cmcount;

	/*0x18a*/ uint16_t floorcol;
	/*0x18c*/ float oldframe;
	/*0x190*/ int8_t footstep;
	/*0x191*/ uint8_t floortype;
	/*0x192*/ uint16_t hidden2; // First 3 bits are a single number - flinch type
	/*0x194*/ float magicframe;
	/*0x198*/ float magicspeed;
	/*0x19c*/ int16_t magicanim;
	/*0x19e*/ int16_t goposforce;
	/*0x1a0*/ int bdlist[60];
	/*0x290*/ uint8_t bdstart;
	/*0x291*/ uint8_t goposhitcount;
	/*0x292*/ int16_t cover;
	/*0x294*/ struct coord targetlastseenp;
	/*0x2a0*/ uint8_t myaction;
	/*0x2a1*/ uint8_t orders;
	/*0x2a2*/ uint8_t squadron;
	/*0x2a3*/ uint8_t listening;
	/*0x2a4*/ uint32_t convtalk;
	/*0x2a8*/ int talktimer;
	/*0x2ac*/ uint8_t question;
	/*0x2ad*/ uint8_t talkgap;
	/*0x2ae*/ uint16_t unk2ae;
	/*0x2b0*/ uint8_t tude;
	/*0x2b1*/ uint8_t voicebox;
	/*0x2b2*/ RoomNum floorroom;
	/*0x2b4*/ uint32_t unk2b4;
	/*0x2b8*/ RoomNum oldrooms[8];
	/*0x2c8*/ struct coord runfrompos;
	/*0x2d4*/ struct aibot *aibot;
	/*0x2d8*/ int16_t blurdrugamount;

	// Cloakpause is set to a positive value when shooting, then decreases to
	// zero over a couple of seconds. When zero is reached, the cloak is
	// applied again.
	/*0x2da*/ int16_t cloakpause;

	/*0x2dc*/ float drugheadsway;
	/*0x2e0*/ uint8_t drugheadcount;
	/*0x2e1*/ uint8_t cloakfadefrac : 7;
	/*0x2e1*/ uint8_t cloakfadefinished : 1;
	/*0x2e2*/ uint8_t teamscandist;
	/*0x2e3*/ uint8_t naturalanim;
	/*0x2e4*/ int myspecial; // This is an object tag ID
	/*0x2e8*/ float timeextra;
	/*0x2ec*/ float elapseextra;
	/*0x2f0*/ struct coord extraspeed;
	/*0x2fc*/ uint8_t yvisang;
	/*0x2fd*/ uint8_t hitpart;
	/*0x2fe*/ uint8_t race;
	/*0x2ff*/ uint8_t blurnumtimesdied;
	/*0x300*/ struct prop *gunprop;
	/*0x304*/ float pushspeed[2];
	/*0x30c*/ float gunroty[2];
	/*0x314*/ float gunrotx[2];
	/*0x31c*/ bool  onladder;
	/*0x320*/ struct coord laddernormal;

	/*0x32c*/
	uint8_t liftaction : 8;

	bool    inlift : 1;
	uint8_t pouncebits : 3;
	uint8_t unk32c_12 : 2;
	uint8_t darkroomthing : 1;
	uint8_t playerdeadthing : 1;

	uint8_t p1p2 : 2;
	bool    unk32c_18 : 1;
	bool    noblood : 1;
	bool    rtracked : 1;
	bool    unk32c_21 : 1;
	uint8_t unk32c_22 : 2;

	uint8_t specialdie : 8;

	/*0x330*/ uint16_t roomtosearch;
	/*0x332*/ uint8_t propsoundcount;
	/*0x333*/ int8_t patrolnextstep;
	/*0x334*/ uint8_t bulletstaken;
	/*0x335*/ uint8_t woundedsplatsadded;
	/*0x336*/ uint16_t tickssincesplat;
	/*0x338*/ uint8_t splatsdroppedhere;
	/*0x339*/ uint8_t stdsplatsadded;
	/*0x33a*/ uint8_t deaddropsplatsadded;
	/*0x33b*/ int8_t aimtesttimer60;
	/*0x33c*/ struct coord lastdroppos;
	/*0x348*/ struct fireslotthing *roboguns[2];
	/*0x350*/ struct chrdata *lastattacker;
	/*0x354*/ int16_t aipunchdodgelist;
	/*0x356*/ int16_t aishootingatmelist;
	/*0x358*/ int16_t poisoncounter;
	/*0x35a*/ int16_t aidarkroomlist;
	/*0x35c*/ int16_t aiplayerdeadlist;
	/*0x35e*/ uint8_t dodgerating;
	/*0x35f*/ uint8_t maxdodgerating;
	/*0x360*/ uint8_t unarmeddodgerating;
	/*0x361*/ uint8_t lastfootsample;
	/*0x362*/ uint8_t drcarollimage_left : 4;
	/*0x362*/ uint8_t drcarollimage_right : 4;
	/*0x364*/ struct prop *lift;
};

// This appears to be misnamed. Not only is it projectiles such as grenades and
// crossbow bolts, but objects being pushed like couches also have this struct.
// So I guess it denotes a generic moving object?
struct projectile {
	/*0x000*/ uint32_t flags;
	/*0x004*/ struct coord speed; // distance moved in last tick
	/*0x010*/ float unk010;
	/*0x014*/ float unk014;
	/*0x018*/ float unk018;
	/*0x01c*/ float unk01c;
	/*0x020*/ Mtx mtx;
	/*0x060*/ float unk060;
	/*0x064*/ float unk064;
	/*0x068*/ float unk068[4];
	/*0x078*/ float unk078[4];
	/*0x088*/ struct prop *ownerprop;
	/*0x08c*/ float bounciness;
	/*0x090*/ int bouncecount;
	/*0x094*/ int bounceframe;
	/*0x098*/ float unk098;
	/*0x09c*/ int lastwooshframe;
	/*0x0a0*/ int flighttime240;
	/*0x0a4*/ int unk0a4;
	/*0x0a8*/ float unk0a8;
	/*0x0ac*/ float unk0ac;
	/*0x0b0*/ int16_t droptype;
	/*0x0b2*/ int16_t powerlimit240;
	/*0x0b4*/ int pickuptimer240;
	/*0x0b8*/ float unk0b8[3];
	/*0x0c4*/ struct coord nextsteppos;
	/*0x0d0*/ int losttimer240;
	/*0x0d4*/ struct defaultobj *obj;
	/*0x0d8*/ int unk0d8;
	/*0x0dc*/ float unk0dc;
	/*0x0e0*/ float unk0e0;
	/*0x0e4*/ float unk0e4;
	/*0x0e8*/ struct prop *targetprop; // for homing rockets
	/*0x0ec*/ float unk0ec;
	/*0x0f0*/ float unk0f0;
	/*0x0f4*/ int smoketimer240;
	/*0x0f8*/ int16_t waypads[MAX_CHRWAYPOINTS];
	/*0x104*/ uint8_t numwaypads;
	/*0x105*/ uint8_t step;
	/*0x108*/ struct prop *pickupby;
};

struct embedment {
	/*0x000*/ uint32_t flags;
	/*0x004*/ Mtx matrix;
	/*0x044*/ struct projectile *projectile;
};

struct tvscreen {
	/*0x00*/ uint32_t *cmdlist;
	/*0x04*/ uint16_t offset;
	/*0x06*/ int16_t pause60;
	/*0x08*/ struct textureconfig *tconfig;
	/*0x0c*/ float rot;
	/*0x10*/ float xscale;
	/*0x14*/ float xscalefrac;
	/*0x18*/ float xscaleinc;
	/*0x1c*/ float xscaleold;
	/*0x20*/ float xscalenew;
	/*0x24*/ float yscale;
	/*0x28*/ float yscalefrac;
	/*0x2c*/ float yscaleinc;
	/*0x30*/ float yscaleold;
	/*0x34*/ float yscalenew;
	/*0x38*/ float xmid;
	/*0x3c*/ float xmidfrac;
	/*0x40*/ float xmidinc;
	/*0x44*/ float xmidold;
	/*0x48*/ float xmidnew;
	/*0x4c*/ float ymid;
	/*0x50*/ float ymidfrac;
	/*0x54*/ float ymidinc;
	/*0x58*/ float ymidold;
	/*0x5c*/ float ymidnew;
	/*0x60*/ uint8_t red;
	/*0x61*/ uint8_t redold;
	/*0x62*/ uint8_t rednew;
	/*0x63*/ uint8_t green;
	/*0x64*/ uint8_t greenold;
	/*0x65*/ uint8_t greennew;
	/*0x66*/ uint8_t blue;
	/*0x67*/ uint8_t blueold;
	/*0x68*/ uint8_t bluenew;
	/*0x69*/ uint8_t alpha;
	/*0x6a*/ uint8_t alphaold;
	/*0x6b*/ uint8_t alphanew;
	/*0x6c*/ float colfrac;
	/*0x70*/ float colinc;
};

struct hov {
	/*0x00*/ uint8_t type;
	/*0x01*/ uint8_t flags;
	/*0x04*/ float bobycur;
	/*0x08*/ float bobytarget;
	/*0x0c*/ float bobyspeed;
	/*0x10*/ float yrot;
	/*0x14*/ float bobpitchcur;
	/*0x18*/ float bobpitchtarget;
	/*0x1c*/ float bobpitchspeed;
	/*0x20*/ float bobrollcur;
	/*0x24*/ float bobrolltarget;
	/*0x28*/ float bobrollspeed;
	/*0x2c*/ float groundpitch;
	/*0x30*/ float y;
	/*0x34*/ float ground;
	/*0x38*/ int prevframe60;
	/*0x3c*/ int prevgroundframe60;
};

struct defaultobj {
	/*0x00*/ uint16_t extrascale;
	/*0x02*/ uint8_t hidden2;
	/*0x03*/ uint8_t type;
	/*0x04*/ int16_t modelnum;
	/*0x06*/ int16_t pad;
	/*0x08*/ uint32_t flags;
	/*0x0c*/ uint32_t flags2;
	/*0x10*/ uint32_t flags3;
	/*0x14*/ struct prop *prop;
	/*0x18*/ struct model *model;
	/*0x1c*/ float realrot[3][3];
	/*0x40*/ uint32_t hidden;
	union {
		/*0x44*/ struct geotilef *geotilef;
		/*0x44*/ struct geoblock *geoblock;
		/*0x44*/ struct geocyl *geocyl;
		/*0x44*/ struct geocyl *unkgeo; // temporary, to indicate that I don't know which geo pointer is being used
	};
	union {
		/*0x48*/ struct projectile *projectile;
		/*0x48*/ struct embedment *embedment;
	};
	/*0x4c*/ int16_t damage;
	/*0x4e*/ int16_t maxdamage;
	/*0x50*/ uint8_t shadecol[4];
	/*0x54*/ uint8_t nextcol[4];
	/*0x58*/ uint16_t floorcol;
	/*0x5a*/ int8_t geocount;
};

struct doorobj { // objtype 0x01
	struct defaultobj base;
	/*0x5c*/ float maxfrac;
	/*0x60*/ float perimfrac;
	/*0x64*/ float accel;
	/*0x68*/ float decel;
	/*0x6c*/ float maxspeed;
	/*0x70*/ uint16_t doorflags;
	/*0x72*/ uint16_t doortype;
	/*0x74*/ uint32_t keyflags;
	/*0x78*/ int autoclosetime;
	/*0x7c*/ float frac; // 0 = closed, maxfrac = fully open
	/*0x80*/ float fracspeed;
	/*0x84*/ int8_t mode;
	/*0x85*/ int8_t glasshits;
	/*0x86*/ int16_t fadealpha;
	/*0x88*/ int16_t xludist;
	/*0x8a*/ int16_t opadist;
	/*0x8c*/ struct coord startpos;
	union {
		struct {
			/*0x98*/ struct coord unk98;
			/*0xa4*/ Vtx *unka4;
		};
		float mtx98[3][3];
	};
	/*0xbc*/ struct doorobj *sibling;
	/*0xc0*/ int lastopen60;
	/*0xc4*/ int16_t portalnum;
	/*0xc6*/ int8_t soundtype;
	/*0xc7*/ int8_t fadetime60; // counts down
	/*0xc8*/ int lastcalc60; // port: actually stores 240hz frame number
	/*0xcc*/ uint8_t laserfade;
	/*0xcd*/ uint8_t unusedmaybe[3];
	/*0xd0*/ uint8_t shadeinfo1[4]; // player 1
	/*0xd4*/ uint8_t shadeinfo2[4]; // player 2
	/*0xd8*/ uint8_t actual1;
	/*0xd9*/ uint8_t actual2;
	/*0xda*/ uint8_t extra1;
	/*0xdb*/ uint8_t extra2;
};

struct doorscaleobj { // objtype 0x02
	uint32_t unk00;
	int scale;
};

struct keyobj { // objtype 0x04
	struct defaultobj base;
	uint32_t keyflags;
};

struct alarmobj { // objtype 0x05
	struct defaultobj base;
};

struct cctvobj { // objtype 0x06
	struct defaultobj base;

	// Note y is being used as an abbreviation for yaw
	/*0x5c*/ int16_t lookatpadnum;
	/*0x5e*/ int16_t toleft;
	/*0x60*/ Mtx camrotm;
	/*0xa0*/ float yzero;
	/*0xa4*/ float yrot;
	/*0xa8*/ float yleft;
	/*0xac*/ float yright;
	/*0xb0*/ float yspeed;
	/*0xb4*/ float ymaxspeed;
	/*0xb8*/ int seebondtime60;
	/*0xbc*/ float maxdist;
	/*0xc0*/ float xzero;
};

struct ammocrateobj { // objtype 0x07
	struct defaultobj base;
	/*0x5c*/ int ammotype;
};

struct weaponobj { // objtype 0x08
	struct defaultobj base;

	union {
		struct gset gset;
		struct {
			/*0x5c*/ uint8_t weaponnum;
			/*0x5d*/ int8_t unk5d;
			/*0x5e*/ int8_t unk5e;
			/*0x5f*/ uint8_t gunfunc;
		};
	};

	/*0x60*/ int8_t fadeouttimer60;
	/*0x61*/ int8_t dualweaponnum;

	union {
		/**
		 * timer240 is used for activation of proxy Dragons, grenades and proxy
		 * mines. It ticks down to 1 where it becomes active, and set to 0 when the
		 * item should explode.
		 */
		/*0x62*/ int16_t timer240;

		/**
		 * team is used for MP briefcases (Capture The Case bases).
		 */
		/*0x62*/ int16_t team;
	};

	/*0x64*/ struct weaponobj *dualweapon; // other weapon when dual wielding
};

struct packedchr { // objtype 0x09
	int16_t chrindex;
	int8_t unk02;
	int8_t typenum;
	uint32_t spawnflags;
	int16_t chrnum;
	uint16_t padnum;
	uint8_t bodynum;
	int8_t headnum;
	uint16_t ailistnum;
	uint16_t padpreset;
	uint16_t chrpreset;
	uint16_t hearscale;
	uint16_t viewdist;
	uint32_t flags;
	uint32_t flags2;
	uint8_t team;
	uint8_t squadron;
	int16_t chair;
	uint32_t convtalk;
	uint8_t tude;
	uint8_t naturalanim;
	uint8_t yvisang;
	uint8_t teamscandist;
};

struct singlemonitorobj { // objtype 0x0a
	struct defaultobj base;
	/*0x5c*/ struct tvscreen screen;
	/*0xd0*/ int16_t owneroffset;
	/*0xd2*/ int8_t ownerpart;
	/*0xd3*/ uint8_t imagenum;
};

struct multimonitorobj { // objtype 0x0b
	struct defaultobj base;
	struct tvscreen screens[4];
	uint8_t imagenums[4];
};

struct hangingmonitorsobj { // objtype 0x0c
	struct defaultobj base;
};

struct autogunobj { // objtype 0x0d
	struct defaultobj base;
	/*0x5c*/ int16_t targetpad;
	/*0x5e*/ int8_t firing;
	/*0x5f*/ uint8_t firecount;
	/*0x60*/ float yzero;
	/*0x64*/ float ymaxleft;
	/*0x68*/ float ymaxright;
	/*0x6c*/ float yrot;
	/*0x70*/ float yspeed;
	/*0x74*/ float xzero;
	/*0x78*/ float xrot;
	/*0x7c*/ float xspeed;
	/*0x80*/ float maxspeed;
	/*0x84*/ float aimdist;
	/*0x88*/ float barrelspeed;
	/*0x8c*/ float barrelrot;
	/*0x90*/ int lastseebond60;
	/*0x94*/ int lastaimbond60;
	/*0x98*/ int allowsoundframe;
	/*0x9c*/ struct beam *beam;
	/*0xa0*/ float shotbondsum;
	/*0xa4*/ struct prop *target;
	/*0xa8*/ uint8_t targetteam;
	/*0xa9*/ uint8_t ammoquantity;
	/*0xaa*/ int16_t nextchrtest;
};

struct linkgunsobj { // objtype 0x0e
	uint32_t unk00;
	int16_t offset1;
	int16_t offset2;
};

struct debrisobj { // objtype 0x0f
	struct defaultobj base;
};

struct hatobj { // objtype 0x11
	struct defaultobj base;
};

struct grenadeprobobj { // objtype 0x12
	uint32_t unk00;
	int16_t chrnum;
	uint16_t probability;
};

struct linkliftdoorobj {
	uint32_t unk00;
	struct prop *door;
	struct prop *lift;
	struct linkliftdoorobj *next;
	int stopnum;
};

struct multiammocrateslot {
	uint16_t modelnum;
	uint16_t quantity;
};

struct multiammocrateobj { // objtype 0x14
	struct defaultobj base;
	/*0x5c*/ struct multiammocrateslot slots[19]; // indexed by ammotype minus 1
};

struct shieldobj { // objtype 0x15
	struct defaultobj base;
	/*0x5c*/ float initialamount;
	/*0x60*/ float amount;
	/*0x64*/ uint32_t unk64;
};

struct tag { // objtype 0x16
	/*0x00*/ uint32_t identifier; // always 0x00000016
	/*0x04*/ uint16_t tagnum;
	/*0x06*/ int16_t cmdoffset;
	/*0x08*/ struct tag *next;
	/*0x0c*/ struct defaultobj *obj;
};

struct objective { // objtype 0x17
	/*0x00*/ uint32_t unk00;
	/*0x04*/ int index;
	/*0x08*/ uint32_t text;
	/*0x0c*/ uint16_t unk0c;
	/*0x0e*/ uint8_t flags;
	/*0x0f*/ int8_t difficulties;
};

struct briefingobj { // objtype 0x23
	uint32_t unk00;
	uint32_t type;
	uint32_t text;
	struct briefingobj *next;
};

struct gasbottleobj { // objtype 0x24
	struct defaultobj base;
};

struct padlockeddoorobj { // objtype 0x26
	uint32_t unk00;
	struct doorobj *door;
	struct defaultobj *lock;
	struct padlockeddoorobj *next;
};

struct truckobj { // objtype 0x27
	struct defaultobj base;
	/*0x5c*/ uint8_t *ailist;
	/*0x60*/ uint16_t aioffset;
	/*0x62*/ int16_t aireturnlist;
	/*0x64*/ float speed;
	/*0x68*/ float wheelxrot;
	/*0x6c*/ float wheelyrot;
	/*0x70*/ float speedaim;
	/*0x74*/ float speedtime60;
	/*0x78*/ float turnrot60;
	/*0x7c*/ float roty;
	/*0x80*/ struct path *path;
	/*0x84*/ int nextstep;
};

struct heliobj { // objtype 0x28
	struct defaultobj base;
	/*0x5c*/ uint8_t *ailist;
	/*0x60*/ uint16_t aioffset;
	/*0x62*/ int16_t aireturnlist;
	/*0x64*/ float rotoryrot;
	/*0x68*/ float rotoryspeed;
	/*0x6c*/ float rotoryspeedaim;
	/*0x70*/ float rotoryspeedtime;
	/*0x74*/ float speed;
	/*0x78*/ float speedaim;
	/*0x7c*/ float speedtime60;
	/*0x80*/ float yrot;
	/*0x84*/ struct path *path;
	/*0x88*/ int nextstep;
};

struct glassobj { // objtype 0x2a
	struct defaultobj base;
	/*0x5c*/ int16_t portalnum;
};

struct safeobj { // objtype 0x2b
	struct defaultobj base;
};

struct safeitemobj {
	uint32_t unk00;
	struct defaultobj *item;
	struct safeobj *safe;
	struct doorobj *door;
	struct safeitemobj *next;
};

struct cameraposobj { // objtype 0x2e
	int type;
	float x;
	float y;
	float z;
	float theta;
	float verta;
	int pad;
};

struct tintedglassobj { // objtype 0x2f
	struct defaultobj base;
	/*0x5c*/ int16_t xludist;
	/*0x5e*/ int16_t opadist;
	/*0x60*/ int16_t opacity;
	/*0x62*/ int16_t portalnum;
	/*0x64*/ float unk64;
};

struct liftobj { // objtype 0x30
	struct defaultobj base;
	/*0x5c*/ int16_t pads[4];
	/*0x64*/ struct doorobj *doors[4];
	/*0x74*/ float dist;
	/*0x78*/ float speed;
	/*0x7c*/ float accel;
	/*0x80*/ float maxspeed;
	/*0x84*/ int8_t soundtype;
	/*0x85*/ int8_t levelcur;
	/*0x86*/ int8_t levelaim;
	/*0x88*/ struct coord prevpos;
};

struct linksceneryobj { // objtype 0x31
	uint32_t unk00;
	struct defaultobj *trigger;
	struct defaultobj *unexp;
	struct defaultobj *exp;
	struct linksceneryobj *next;
};

struct blockedpathobj { // objtype 0x32
	uint32_t unk00;
	struct defaultobj *blocker;
	int16_t waypoint1;
	int16_t waypoint2;
	struct blockedpathobj *next;
};

struct hoverbikeobj { // objtype 0x33
	struct defaultobj base;
	struct hov hov;
	/*0x09c*/ float speed[2];
	/*0x0a4*/ float prevpos[2];
	/*0x0ac*/ float w;
	/*0x0b0*/ float rels[2];
	/*0x0b8*/ float exreal;
	/*0x0bc*/ float ezreal;
	/*0x0c0*/ float ezreal2;
	/*0x0c4*/ float leanspeed;
	/*0x0c8*/ float leandiff;
	/*0x0cc*/ int maxspeedtime240;
	/*0x0d0*/ float speedabs[2];
	/*0x0d8*/ float speedrel[2];
};

struct hoverpropobj { // objtype 0x35
	struct defaultobj base;
	struct hov hov;
};

struct fanobj { // objtype 0x36
	struct defaultobj base;
	/*0x5c*/ float yrot;
	/*0x60*/ float yrotprev;
	/*0x64*/ float ymaxspeed;
	/*0x68*/ float yspeed;
	/*0x6c*/ float yaccel;
	/*0x70*/ int8_t on;
};

struct hovercarobj { // objtype 0x37
	struct defaultobj base;
	/*0x5c*/ uint8_t *ailist;
	/*0x60*/ uint16_t aioffset;
	/*0x62*/ int16_t aireturnlist;
	/*0x64*/ float speed;
	/*0x68*/ float speedaim;
	/*0x6c*/ float speedtime60;
	/*0x70*/ float turnyspeed60;
	/*0x74*/ float turnxspeed60;
	/*0x78*/ float turnrot60;
	/*0x7c*/ float roty;
	/*0x80*/ float rotx;
	/*0x84*/ float rotz;
	/*0x88*/ struct path *path;
	/*0x8c*/ int nextstep;
	/*0x90*/ int16_t status;
	/*0x92*/ int16_t dead;
	/*0x94*/ int16_t deadtimer60;
	/*0x96*/ int16_t sparkstimer60;
};

struct padeffectobj { // objtype 0x38
	uint32_t unk00;
	int effect;
	int pad;
};

struct chopperobj { // objtype 0x39
	struct defaultobj base;
	/*0x5c*/ uint8_t *ailist;
	/*0x60*/ uint16_t aioffset;
	/*0x62*/ int16_t aireturnlist;
	union {
		struct {
			/*0x64*/ float speed;
			/*0x68*/ float speedaim;
			/*0x6c*/ float speedtime60;
		};
		struct coord fall;
	};
	/*0x70*/ float turnyspeed60;
	/*0x74*/ float turnxspeed60;
	/*0x78*/ float turnrot60;
	/*0x7c*/ float roty;
	/*0x80*/ float rotx;
	/*0x84*/ float rotz;
	/*0x88*/ struct path *path;
	/*0x8c*/ int nextstep;
	/*0x90*/ int16_t weaponsarmed;
	/*0x92*/ int16_t ontarget;
	/*0x94*/ int16_t target;
	/*0x96*/ uint8_t attackmode;
	/*0x97*/ uint8_t cw;
	/*0x98*/ float vx;
	/*0x9c*/ float vy;
	/*0xa0*/ float vz;
	/*0xa4*/ float power;
	/*0xa8*/ float otx;
	/*0xac*/ float oty;
	/*0xb0*/ float otz;
	/*0xb4*/ float bob;
	/*0xb8*/ float bobstrength;
	/*0xbc*/ int targetvisible;
	/*0xc0*/ int timer60;
	/*0xc4*/ int patroltimer60;
	/*0xc8*/ float gunturnyspeed60;
	/*0xcc*/ float gunturnxspeed60;
	/*0xd0*/ float gunroty;
	/*0xd4*/ float gunrotx;
	/*0xd8*/ float barrelrotspeed;
	/*0xdc*/ float barrelrot;
	/*0xe0*/ struct fireslotthing *fireslotthing;
	/*0xe4*/ int dead;
};

struct mineobj { // objtype 0x3a
	struct defaultobj base;
};

struct escalatorobj { // objtype 0x3b
	struct defaultobj base;
	/*0x5c*/ int frame;
	/*0x60*/ struct coord prevpos;
};

struct eyespy {
	/*0x00*/ struct prop *prop;
	/*0x04*/ struct coord look;
	/*0x10*/ struct coord up;
	/*0x1c*/ float theta; // turn angle in degrees
	/*0x20*/ float costheta;
	/*0x24*/ float sintheta;
	/*0x28*/ float verta;
	/*0x2c*/ float cosverta;
	/*0x30*/ float sinverta;
	/*0x34*/ uint8_t held; // the eyespy is held by the player (opposite of deployed)
	/*0x35*/ uint8_t deployed; // the eyespy is deployed in the stage somewhere
	/*0x36*/ int8_t startuptimer60;
	/*0x37*/ int8_t active; // player is currently controlling the eyespy
	/*0x38*/ int8_t buttonheld;
	/*0x39*/ int8_t camerabuttonheld;
	/*0x3a*/ int16_t bobdir;
	/*0x3c*/ uint8_t bobtimer;
	/*0x3d*/ uint8_t bobactive;
	/*0x40*/ struct coord vel;
	/*0x4c*/ struct coord unk4c;
	/*0x58*/ float speed;
	/*0x5c*/ float oldground;
	/*0x60*/ float height; // height above ground - 30 to 160 on G5 and CI training, 80 to 160 elsewhere
	/*0x64*/ float gravity;
	/*0x68*/ int8_t camerashuttertime;
	/*0x69*/ uint8_t hit;
	/*0x6a*/ uint8_t opendoor;
	/*0x6b*/ uint8_t mode;
	/*0x6c*/ float velf[2];
	/*0x74*/ float vels[2];
	/*0x7c*/ float pitch;
};

struct sndstate {
	/*0x00*/ ALLink node;
	/*0x08*/ ALSound *sound;
	/*0x0c*/ N_ALVoice voice;
	/*0x28*/ float basepitch;
	/*0x2c*/ float pitch;
	/*0x30*/ struct sndstate **unk30;
	/*0x34*/ int unk34;
	/*0x38*/ int16_t vol;
	/*0x3a*/ int16_t envvol;
	/*0x3c*/ ALMicroTime endtime;
	/*0x40*/ uint8_t priority;
	/*0x41*/ ALPan pan;
	/*0x42*/ uint8_t fxmix;
	/*0x43*/ uint8_t fxbus;
	/*0x44*/ uint8_t flags;
	/*0x45*/ uint8_t state;
	/*0x46*/ uint16_t soundnum;
	/*0x48*/ ALMicroTime unk48;
};

/**
 * List of guns that the player has equipped and the amount of time they've been
 * equipped for. It's used to determine the weapon of choice for the endscreen.
 * The time is increased for the equipped weapon on each tick.
 *
 * The list is only 10 items long. If an 11th item is used, the least used item
 * in the list will be removed.
 */
struct gunheld {
	int weapon1;
	int weapon2;
	int totaltime240_60;
};

struct playerjo {

	// unk00.x = look vector x (-1 to +1)
	// unk00.y = always 0?
	// unk00.z = look vector z (-1 to +1)
	/*0x0338 0x036c*/ struct coord unk00;

	/*0x0344 0x0378*/ float radius; // always 30?

	/*0x0348 0x037c*/ struct coord unk10;

	// unk1c.x = affected by both left/right and up/down looking
	// unk1c.y = vertical look vector (-1 for down, 1 for up)
	// unk1c.z = affected by both left/right and up/down looking
	/*0x0354 0x0388*/ struct coord unk1c;

	// unk28.x = affected by both horiz and vertical angle
	// unk28.y = 0 when looking up or down, .999 when looking horizontal
	// unk28.z = pos.z
	/*0x0360 0x0394*/ struct coord unk28;
};

struct trackedprop {
	struct prop *prop;
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
};

struct beam {
	/*0x00*/ int8_t age;
	/*0x01*/ int8_t weaponnum;
	/*0x04*/ struct coord from;
	/*0x10*/ struct coord dir;
	/*0x1c*/ float maxdist;
	/*0x20*/ float speed;
	/*0x24*/ float mindist;
	/*0x28*/ float dist;
};

struct abmag {
	// When the gauge uses separate bars, this is zero/unused. When the gauge
	// uses merged bars, this is the same figure as displayed on the HUD.
	/*0x00*/ int16_t loadedammo;

	// Counts up to 255 when firing or reloading. It's used to determine the
	// brightness of the bar in the ammo gauge.
	/*0x02*/ int16_t timer60;

	// When firing, this is the number of bars still loaded + any bars recently
	// fired which are fading to empty.
	// When loading, this is the number of bars which are loaded and don't have
	// a fade effect on them.
	// In other words, it's a reference to where the fade effects start and is
	// also a reference for the timer.
	/*0x04*/ int8_t ref;

	// When positive, is the number of slots remaining to settle when reloading,
	// including slots which are fully empty.
	// When negative, is the number of slots remaining to settle when firing.
	/*0x05*/ int8_t change;

	/*0x06*/ uint16_t alignment;
};

// Weapon data per hand
struct hand {
	struct gset gset;
	/*0x063c*/ int8_t firing;
	/*0x063d*/ int8_t flashon;
	/*0x063e*/ uint8_t gunon;
	/*0x063f*/ int8_t visible;
	/*0x0640*/ int8_t inuse; // true if hand is holding a gun, false if not
	/*0x0644*/ bool triggeron;
	/*0x0648*/ bool triggerprev;
	/*0x064c*/ bool triggerreleased;
	/*0x0650*/ int count;
	/*0x0654*/ int count60;
	/*0x0658*/ uint32_t mode;
	/*0x065c*/ uint32_t modenext; // 0 = idle, 9 = reloading
	/*0x0660*/ uint32_t numfires;
	/*0x0664*/ uint32_t numshotguncarts;
	/*0x0668*/ uint32_t refiretime;
	/*0x066c*/ uint32_t typechange;
	/*0x0670*/ int pausetime60;
	/*0x0674*/ uint32_t pausechange;
	/*0x0678*/ uint32_t nextprevchange;
	/*0x067c*/ struct coord posstart;
	/*0x0688*/ float rotxstart;
	/*0x068c*/ struct coord posend;
	/*0x0698*/ float rotxend;
	/*0x069c*/ struct coord posoffset;
	/*0x06a8*/ float rotxoffset;
	/*0x06ac*/ Mtx posrotmtx;
	/*0x06ec*/ bool useposrot;
	/*0x06f0*/ struct coord damppos;
	/*0x06fc*/ struct coord damplook;
	/*0x0708*/ struct coord dampup;
	/*0x0714*/ struct coord damppossum;
	/*0x0720*/ struct coord damplooksum;
	/*0x072c*/ struct coord dampupsum;
	/*0x0738*/ struct coord blendpos[4];
	/*0x0768*/ struct coord blendlook[4];
	/*0x0798*/ struct coord blendup[4];
	/*0x07c8*/ int curblendpos;
	/*0x07cc*/ float dampt;
	/*0x07d0*/ float blendscale;
	/*0x07d4*/ float blendscale1;
	/*0x07d8*/ int sideflag;
	/*0x07dc*/ struct coord adjustdamp;
	/*0x07e8*/ struct coord adjustpos;
	/*0x07f4*/ float xshift;
	/*0x07f8*/ struct coord aimpos;
	/*0x0804*/ struct sndstate *audiohandle2;
	/*0x0808*/ struct sndstate *audiohandle3;
	/*0x080c*/ int allowshootframe;
	/*0x0810*/ int lastshootframe60;
	/*0x0814*/ struct beam beam;
	/*0x0840*/ float noiseradius;
	/*0x0844*/ uint32_t fingerroty;
	/*0x0848*/ float slidetrans; // 0 at rest, positive when back (struct weaponfunc_shoot.slidemax)
	/*0x084c*/ bool slideinc; // true when moving back, false when moving forward or not moving
	/*0x0850*/ struct weaponobj *rocket;
	/*0x0854*/ bool firedrocket;
	/*0x0858*/ int loadedammo[2];
	/*0x0860*/ int clipsizes[2];
	/*0x0868*/ float angledamper;
	/*0x086c*/ float lastrotangx;
	/*0x0870*/ float lastrotangy;
	/*0x0874*/ float matmot1;
	/*0x0878*/ float matmot2;
	/*0x087c*/ float matmot3;
	/*0x0880*/ uint32_t unk0880;
	/*0x0884*/ uint32_t unk0884;
	/*0x0888*/ float loadslide;
	/*0x088c*/ float upgrademult[12];
	/*0x08bc*/ float finalmult[12];
	/*0x08ec*/ Mtx cammtx;
	/*0x092c*/ Mtx posmtx;
	/*0x096c*/ Mtx prevmtx;
	/*0x09ac*/ struct coord muzzlepos;
	/*0x09b8*/ float muzzlez;
	/*0x09bc*/ struct model gunmodel;
	/*0x09e0*/ struct anim anim;
	/*0x0a6c*/ uint32_t unk0a6c[32];
	/*0x0aec*/ uint32_t handsavedata[32]; // should be 0a6c
	/*0x0b6c*/ struct model handmodel;
	/*0x0b90*/ int burstbullets;
	/*0x0b94*/ struct coord hitpos;
	/*0x0ba0*/ uint32_t unk0ba0;
	/*0x0ba4*/ uint32_t unk0ba4;
	/*0x0ba8*/ uint32_t unk0ba8;
	/*0x0bac*/ uint32_t unk0bac;
	/*0x0bb0*/ uint32_t unk0bb0;
	/*0x0bb4*/ uint32_t unk0bb4;
	/*0x0bb8*/ uint32_t unk0bb8;
	/*0x0bbc*/ uint32_t unk0bbc;
	/*0x0bc0*/ uint32_t unk0bc0;
	/*0x0bc4*/ uint32_t unk0bc4;
	/*0x0bc8*/ uint32_t unk0bc8;
	/*0x0bcc*/ uint32_t unk0bcc;
	/*0x0bd0*/ uint32_t unk0bd0;
	/*0x0bd4*/ uint32_t unk0bd4;
	/*0x0bd8*/ uint32_t unk0bd8;
	/*0x0bdc*/ uint32_t unk0bdc;
	/*0x0be0*/ uint32_t unk0be0;
	/*0x0be4*/ uint32_t unk0be4;
	/*0x0be8*/ uint32_t unk0be8;
	/*0x0bec*/ uint32_t unk0bec;
	/*0x0bf0*/ uint32_t unk0bf0;
	/*0x0bf4*/ uint32_t unk0bf4;
	/*0x0bf8*/ uint32_t unk0bf8;
	/*0x0bfc*/ uint32_t unk0bfc;
	/*0x0c00*/ uint32_t unk0c00;
	/*0x0c04*/ uint32_t unk0c04;
	/*0x0c08*/ uint32_t unk0c08;
	/*0x0c0c*/ uint32_t unk0c0c;
	/*0x0c10*/ uint32_t unk0c10;
	/*0x0c14*/ int8_t lastdirvalid;
	/*0x0c18*/ struct coord lastshootdir;
	/*0x0c24*/ struct coord lastshootpos;
	/*0x0c30*/ int shotstotake;
	/*0x0c34*/ float shotremainder;
	/*0x0c38*/ uint32_t hitspershot;
	/*0x0c3c*/ int state;
	/*0x0c40*/ int stateminor;
	/*0x0c44*/ uint32_t stateflags;
	/*0x0c48*/ uint32_t statemode;
	/*0x0c4c*/ int stateframes;
	/*0x0c50*/ int statecycles;
	/*0x0c54*/ int statelastframe;
	/*0x0c58*/ Mtx muzzlemat;
	/*0x0c98*/ float gs_float1;
	/*0x0c9c*/ float gs_float2;
	/*0x0ca0*/ float gs_float3;
	/*0x0ca4*/ float gs_float4;
	/*0x0ca8*/ int gs_int1;
	/*0x0cac*/ int gs_int2;
	/*0x0cb0*/ int gs_int3;
	/*0x0cb4*/ int gs_int4;
	/*0x0cb8*/ int animload;
	/*0x0cbc*/ int animframeinc;
	/*0x0cc0*/ uint32_t animframeincfreal;
	/*0x0cc4*/ int animmode;
	/*0x0cc8*/ uint8_t waitingForRelease : 1;
	/*0x0cc8*/ uint8_t unk0cc8_02 : 1;
	/*0x0cc8*/ uint8_t incrementalreloading : 1;
	/*0x0cc8*/ uint8_t unk0cc8_04 : 3;
	/*0x0cc8*/ uint8_t unk0cc8_07 : 1;
	/*0x0cc8*/ uint8_t unk0cc8_08 : 1;
	/*0x0cc9*/ uint8_t animloopcount;
	/*0x0ccc*/ float crosspos[2];
	/*0x0cd4*/ float guncrosspossum[2];
	/*0x0cdc*/ uint32_t statejob;
	/*0x0ce0*/ int statevar1;
	/*0x0ce4*/ int attacktype;
	/*0x0ce8*/ struct guncmd *currentGunCmd;
	/*0x0cec*/ bool hasdotinfo;
	/*0x0cf0*/ struct coord dotpos;
	/*0x0cfc*/ struct coord dotrot;
	/*0x0d08*/ float gangstarot; // frac
	/*0x0d0c*/ int16_t primetimer60;
	/*0x0d0e*/ uint8_t ejectstate : 4;
	/*0x0d0e*/ uint8_t ejecttype : 3;
	/*0x0d0e*/ uint8_t unk0d0e_07 : 1;
	/*0x0d0f*/ uint8_t createsmoke : 1;
	/*0x0d0f*/ uint8_t forcecreatesmoke : 1;
	/*0x0d0f*/ uint8_t unk0d0f_02 : 1;
	/*0x0d0f*/ uint8_t activatesecondary : 1;
	/*0x0d0f*/ uint8_t unk0d0f_04 : 4;
	/*0x0d10*/ float unk0d10;
	/*0x0d14*/ float unk0d14;
	/*0x0d18*/ float unk0d18;
	/*0x0d1c*/ float unk0d1c;
	/*0x0d20*/ struct coord unk0d20;
	/*0x0d2c*/ float unk0d2c[3][3];
	/*0x0d50*/ float unk0d50[3][3];
	/*0x0d74*/ uint16_t gunroundsspent[4]; // actually a countdown timer
	/*0x0d7c*/ int ispare1;
	/*0x0d80*/ struct guncmd *unk0d80;
	/*0x0d84*/ struct sndstate *audiohandle;
	/*0x0d88*/ uint32_t ispare4;
	/*0x0d8c*/ uint32_t ispare5;
	/*0x0d90*/ uint32_t ispare6;
	/*0x0d94*/ uint32_t ispare7;
	/*0x0d98*/ uint32_t ispare8;
	/*0x0d9c*/ uint32_t ispare9;
	/*0x0da0*/ uint32_t ispare10;
	/*0x0da4*/ float fspare1;
	/*0x0da8*/ float fspare2;
	/*0x0dac*/ float turnuprot; // Hand turning up when facing wall rotation
	/*0x0db0*/ float fspare4;
	/*0x0db4*/ float fspare5;
	/*0x0db8*/ float gunsmokepoint;
	/*0x0dbc*/ uint32_t fspare7;
	/*0x0dc0*/ uint32_t fspare8;
	/*0x0dc4*/ struct abmag abmag;
	/*0x0dcc*/ uintptr_t *unk0dcc;
	/*0x0dd0*/ uintptr_t *unk0dd0;
	/*0x0dd4*/ int unk0dd4; 
	/*0x0dd8*/ Mtx *unk0dd8;
};

struct texpool {
	uint8_t *start;
	union {
		struct tex *head; // for shared pool
		struct tex *end; // for dedicated pools
	};
	uint8_t *leftpos;
	struct tex *rightpos;
};

struct fileinfo {
	uint32_t loadedsize;
	uint32_t allocsize;
};

struct gunctrl {
	/*0x1580*/ int8_t weaponnum;
	/*0x1581*/ int8_t prevweaponnum; // previously drawn weapon, switched to when throwing Dragon/Laptop or when ammo depleted
	/*0x1582*/ int8_t switchtoweaponnum; // weaponnum to change to
	/*0x1583*/ uint8_t dualwielding : 1;
	/*0x1583*/ uint8_t prevwasdualwielding : 1;
	/*0x1583*/ uint8_t invertgunfunc : 1;
	/*0x1583*/ uint8_t gangsta : 1;
	/*0x1583*/ uint8_t throwing : 1;
	/*0x1583*/ uint8_t wantammo : 1;
	/*0x1583*/ uint8_t loadall : 1;
	/*0x1583*/ uint8_t passivemode : 1;
	/*0x1584*/ int gunmemnew; // a weapon number, -1 when not changing guns
	/*0x1588*/ int gunmemtype; // also a weapon number
	/*0x158c*/ uint8_t *gunmem;
	/*0x1590*/ struct modeldef *gunmodeldef;
	/*0x1594*/ struct modeldef *handmodeldef;
	/*0x1598*/ struct modeldef *cartmodeldef;
	/*0x159c*/ uint16_t handfilenum;
	/*0x15a0*/ uint8_t *handmemloadptr;
	/*0x15a4*/ int handmemloadremaining;
	/*0x15a8*/ uint8_t *memloadptr;
	/*0x15ac*/ uint32_t memloadremaining;
	/*0x15b0*/ uint8_t masterloadstate;
	/*0x15b1*/ uint8_t gunloadstate;
	/*0x15b2*/ uint16_t loadfilenum;
	/*0x15b4*/ struct modeldef **loadtomodeldef;
	/*0x15b8*/ uintptr_t *loadmemptr;
	/*0x15bc*/ uintptr_t*loadmemremaining;
	/*0x15c0*/ struct texpool texpool;
	/*0x15d0*/ uint32_t nexttexturetoload;
	/*0x15d4*/ struct fileinfo fileinfo;
	/*0x15dc*/ struct abmag abmag;
	/*0x15e4*/ int8_t ammotypes[2];
	/*0x15e6*/ uint8_t action;
	/*0x15e7*/ uint8_t fnfader;
	/*0x15e8*/ uint8_t upgradewant;
	/*0x15e9*/ int8_t lastmag;
	/*0x15ea*/ uint8_t gunmemowner;
	/*0x15eb*/ int8_t gunlocktimer;
	/*0x15ec*/ uint16_t curfnstr;
	/*0x15ee*/ uint8_t fnstrtimer;
	/*0x15ef*/ uint8_t guntypetimer;
	/*0x15f0*/ uint8_t guntypefader;
	/*0x15f2*/ uint16_t curgunstr;
	/*0x15f4*/ uint8_t paddingashdown;
};

struct player {
	/*0x0000*/ int cameramode;
	/*0x0004*/ struct coord memcampos; // Room that the camera is in (differs from the player's room during cutscenes and Slayer rocket)
	/*0x0010*/ uint16_t visionmode;
	/*0x0014*/ int memcamroom;
	/*0x0018*/ struct coord eraserpos;
	/*0x0024*/ float eraserpropdist;
	/*0x0028*/ float eraserbgdist;
	/*0x002c*/ float eraserdepth;
	/*0x0030*/ bool isfalling;
	/*0x0034*/ int fallstart; // lvframe60 when player started falling
	/*0x0038*/ struct coord globaldrawworldoffset;
	/*0x0064*/ Mtx *worldToScreenMtx;
	/*0x0068*/ Mtx *initProjMtx;
	/*0x006c*/ Mtx *artifactMtx;
	/*0x0070*/ float sumground;
	/*0x0074*/ float vv_manground; // Feet Y value in absolute coordinates
	/*0x0078*/ float vv_ground; // Ground Y value in absolute coordinates
	/*0x007c*/ struct coord bdeltapos; // Only y is used? Negative when falling

	// These crouch fields are related to recovering after a fall - not actual crouching
	/*0x0088*/ float sumcrouch;
	/*0x008c*/ float crouchheight; // Negative, is Y offset to regular standing height
	/*0x0090*/ int crouchtime240; // Set to 60 when landing, counts down
	/*0x0094*/ float crouchfall; // -90 when slowing the descent, increments back to 0 while returning to stand

	/*0x0098*/ int swaypos;
	/*0x009c*/ float swayoffset;
	/*0x00a0*/ float swaytarget;
	/*0x00a4*/ float swayoffset0;
	/*0x00a8*/ float swayoffset2;
	/*0x00ac*/ int crouchpos;
	/*0x00b0*/ int autocrouchpos;
	/*0x00b4*/ float crouchoffset;
	/*0x00b8*/ float crouchspeed;
	/*0x00bc*/ struct prop *prop;
	/*0x00c0*/ bool bondperimenabled;
	/*0x00c4*/ uint32_t devicesactive;
	/*0x00c8*/ int badrockettime;
	/*0x00cc*/ float gunspeed;
	/*0x00d0*/ int bondactivateorreload;
	/*0x00d4*/ struct model *model00d4;
	/*0x00d8*/ bool isdead;
	/*0x00dc*/ float bondhealth; // frac (range 0-1)
	/*0x00e0*/ struct sndstate *chokehandle;
	/*0x00e4*/ float oldhealth;
	/*0x00e8*/ float oldarmour;
	/*0x00ec*/ float apparenthealth;
	/*0x00f0*/ float apparentarmour;
	/*0x00f4*/ float damageshowtime;
	/*0x00f8*/ float healthshowtime;
	/*0x00fc*/ uint32_t healthshowmode;
	/*0x0100*/ int docentreupdown;
	/*0x0104*/ uint32_t lastupdown60;
	/*0x0108*/ int prevupdown;
	/*0x010c*/ int movecentrerelease;
	/*0x0110*/ bool lookaheadcentreenabled;
	/*0x0114*/ bool automovecentreenabled;
	/*0x0118*/ bool fastmovecentreenabled;
	/*0x011c*/ bool automovecentre;
	/*0x0120*/ bool insightaimmode;

	/*0x0124*/ bool autoyaimenabled;
	/*0x0128*/ float autoaimy;
	/*0x012c*/ struct prop *autoyaimprop;
	/*0x0130*/ int autoyaimtime60;

	/*0x0134*/ bool autoxaimenabled;
	/*0x0138*/ float autoaimx;
	/*0x013c*/ struct prop *autoxaimprop;
	/*0x0140*/ int autoxaimtime60;

	/*0x0144*/ float vv_theta;   // turn angle in degrees
	/*0x0148*/ float speedtheta; // turn speed
	/*0x014c*/ float vv_costheta;
	/*0x0150*/ float vv_sintheta;

	/*0x0154*/ float vv_verta;   // look up/down angle. 0 = horizontal, 90 = up
	/*0x0158*/ float vv_verta360;
	/*0x015c*/ float speedverta; // look up/down speed
	/*0x0160*/ float vv_cosverta;
	/*0x0164*/ float vv_sinverta;

	/*0x0168*/ float speedsideways;    // range -1 to 1
	/*0x016c*/ float speedstrafe;
	/*0x0170*/ float speedforwards;    // range -1 to 1
	/*0x0174*/ float speedboost;       // speed multiplier - ranges from 1 to 1.25 - kicks in after 3 seconds of full speed
	/*0x0178*/ int speedmaxtime60;   // amount of time player has held full forward speed
	/*0x017c*/ struct coord bondshotspeed;
	/*0x0188*/ float bondfadetime60;
	/*0x018c*/ float bondfadetimemax60;
	/*0x0190*/ float bondfadefracold;
	/*0x0194*/ float bondfadefracnew;
	/*0x0198*/ float bondbreathing;
	/*0x019c*/ int activatetimelast;
	/*0x01a0*/ int activatetimethis;
	/*0x01a4*/ struct coord moveinitspeed;
	/*0x01b0*/ int bondmovemode;
	/*0x01b4*/ float gunextraaimx;
	/*0x01b8*/ float gunextraaimy;
	/*0x01c0*/ struct anim dummyanim;
	/*0x024c*/ int16_t invdowntime;
	/*0x024e*/ int16_t usedowntime;
	/*0x0250*/ uint8_t activemenumode;
	/*0x0251*/ uint8_t ecol_1;
	/*0x0252*/ uint8_t ecol_2;
	/*0x0253*/ uint8_t ecol_3;
	/*0x0254*/ int erasertime; // related to FarSight's auto tracking
	/*0x0258*/ float autoeraserdist;
	/*0x025c*/ struct prop *autoerasertarget;
	/*0x0260*/ int aimtaptime;
	/*0x0264*/ struct weaponobj *slayerrocket;
	/*0x0268*/ bool eyesshut;
	/*0x026c*/ float eyesshutfrac;
	/*0x0270*/ uint8_t epcol_0;
	/*0x0271*/ uint8_t epcol_1;
	/*0x0272*/ uint8_t epcol_2;
	/*0x0273*/ uint8_t flashbang;
	/*0x0274*/ bool waitforzrelease;
	/*0x0278*/ float shieldshowrot;
	/*0x027c*/ uint32_t shieldshowrnd;
	/*0x0280*/ float shieldshowtime;
	/*0x0284*/ RoomNum bondprevrooms[8];
	/*0x0294*/ float liftground;
	/*0x0298*/ struct prop *lift;
	/*0x029c*/ float ladderupdown;
	/*0x02a0*/ struct coord laddernormal;
	/*0x02ac*/ bool onladder;
	/*0x02b0*/ bool inlift;
	/*0x02b4*/ struct coord posdie;
	/*0x02c0*/ struct coord bonddampeyesum;
	/*0x02cc*/ struct coord bonddampeye;
	/*0x02d8*/ int colourscreenred;
	/*0x02dc*/ int colourscreengreen;
	/*0x02e0*/ int colourscreenblue;
	/*0x02e4*/ float colourscreenfrac;
	/*0x02e8*/ float colourfadetime60;
	/*0x02ec*/ float colourfadetimemax60;
	/*0x02f0*/ int colourfaderedold;
	/*0x02f4*/ int colourfaderednew;
	/*0x02f8*/ int colourfadegreenold;
	/*0x02fc*/ int colourfadegreennew;
	/*0x0300*/ int colourfadeblueold;
	/*0x0304*/ int colourfadebluenew;
	/*0x0308*/ float colourfadefracold;
	/*0x030c*/ float colourfadefracnew;
	/*0x0310*/ struct coord bondprevpos;
	/*0x031c*/ float thetadie;
	/*0x0320*/ float vertadie;
	/*0x0324*/ uint32_t outfit; // OUTFIT constant
	/*0x0328*/ bool startnewbonddie;
	/*0x032c*/ bool redbloodfinished;
	/*0x0330*/ bool deathanimfinished;
	/*0x0334*/ int controldef;
	/*0x0338*/ struct playerjo bonddie;
	/*0x036c*/ struct playerjo bond2;
	/*0x03a0*/ bool resetheadpos;
	/*0x03a4*/ bool resetheadrot;
	/*0x03a8*/ bool resetheadtick;
	/*0x03ac*/ int headanim;
	/*0x03b0*/ float headdamp;
	/*0x03b4*/ int headwalkingtime60;
	/*0x03b8*/ float headamplitude;
	/*0x03bc*/ float sideamplitude;
	/*0x03c0*/ struct coord headpos; // Head position relative to the player's base position
	/*0x03cc*/ struct coord headlook;
	/*0x03d8*/ struct coord headup;
	/*0x03e4*/ struct coord headpossum;
	/*0x03f0*/ struct coord headlooksum;
	/*0x03fc*/ struct coord headupsum;
	/*0x0408*/ struct coord headbodyoffset;
	/*0x0414*/ float standheight;
	/*0x0418*/ struct coord standbodyoffset;
	/*0x0424*/ float standfrac;
	/*0x0428*/ struct coord standlook[2];
	/*0x0440*/ struct coord standup[2];
	/*0x0458*/ int standcnt;
	/*0x045c*/ struct model model;
	/*0x0480*/ struct eyespy *eyespy;
	/*0x0484*/ uint8_t *unk0484; // pointer to vtx buffer?
	/*0x0488*/ uint8_t *unk0488; // pointer to vtx buffer?
	/*0x048c*/ bool aborted;
	/*0x0490*/ int eyespydarts;
	/*0x0494*/ uint32_t bondheadsave[30];
	/*0x050c*/ uint32_t unk050c;
	/*0x0510*/ Mtx bondheadmatrices[4];
	/*0x0610*/ Vp viewport[NUM_FRAMEBUFFERS];
	/*0x0630*/ int16_t viewwidth;  // 320 lo-res, 640 hi-res
	/*0x0632*/ int16_t viewheight; // 220 regardless of res
	/*0x0634*/ int16_t viewleft;   // 0
	/*0x0636*/ int16_t viewtop;    // 0
	/*0x0638*/ struct hand hands[2];
	struct gunctrl gunctrl;
	/*0x15f8*/ float gunposamplitude;
	/*0x15fc*/ float gunxamplitude;
	/*0x1600*/ int doautoselect;
	/*0x1604*/ uint32_t playertriggeron;
	/*0x1608*/ uint32_t playertriggerprev;
	/*0x160c*/ int playertrigtime240;
	/*0x1610*/ int curguntofire; // 0 or 1, for dual wielding
	/*0x1614*/ uint8_t gunshadecol[4];
	/*0x1618*/ int16_t resetshadecol;
	/*0x161a*/ uint16_t floorcol;
	/*0x161c*/ uint16_t floorflags;
	/*0x161e*/ uint8_t floortype;
	/*0x1620*/ uint32_t aimtype;
	/*0x1624*/ struct trackedprop lookingatprop;
	/*0x1630*/ struct trackedprop trackedprops[4];
	/*0x1660*/ float crosspos[2];
	/*0x1668*/ float crosspossum[2];
	/*0x1670*/ float guncrossdamp;
	/*0x1674*/ float crosspos2[2];
	/*0x167c*/ float crosssum2[2];
	/*0x1684*/ float gunaimdamp;
	/*0x1688*/ struct coord aimangle;
	/*0x16d8*/ uint32_t gunammooff;
	/*0x16dc*/ float gunsync;
	/*0x16e0*/ float syncchange;
	/*0x16e4*/ float synccount;
	/*0x16e8*/ int syncoffset;
	/*0x16ec*/ float cyclesum;
	/*0x16f0*/ float gunampsum;
	/*0x16f4*/ float gunzoomfovs[3]; // saved zoom levels/fovs for sniper, farsight and horizon scanner
	/*0x1700*/ int lastroomforoffset;
	/*0x1704*/ float c_screenwidth;
	/*0x1708*/ float c_screenheight;
	/*0x170c*/ float c_screenleft;
	/*0x1710*/ float c_screentop;
	/*0x1718*/ float c_perspfovy;
	/*0x171c*/ float c_perspaspect;
	/*0x1720*/ float c_halfwidth;
	/*0x1724*/ float c_halfheight;
	/*0x1728*/ float c_scalex;
	/*0x172c*/ float c_scaley;
	/*0x1730*/ float c_recipscalex;
	/*0x1734*/ float c_recipscaley;
	/*0x1740*/ Mtx *worldtoscreenmtx;
	/*0x1744*/ int c_viewfmdynticknum;
	/*0x174c*/ Mtx *projectionmtx;
	/*0x1750*/ Mtx *perspmtxl;
	/*0x1754*/ Mtx *skyMtx;
	/*0x1758*/ Mtx *orthomtxl;
	/*0x175c*/ LookAt *lookat;
	/*0x1760*/ Mtx *prevworldtoscreenmtx;
	/*0x1764*/ int c_prevviewfmdynticknum;
	/*0x1768*/ Mtx *prevprojectionmtx;
	/*0x1774*/ float c_lodscalez;
	/*0x1794*/ float screenxminf;
	/*0x1798*/ float screenyminf;
	/*0x179c*/ float screenxmaxf;
	/*0x17a0*/ float screenymaxf;
	/*0x17a4*/ uint32_t gunsightoff;
	/*0x17a8*/ int ammoheldarr[33]; // ammo quantities not loaded into a gun
	/*0x182c*/ float hovspeed;
	/*0x1830*/ uint32_t unk1830;
	/*0x1834*/ uint32_t unk1834;
	/*0x1838*/ uint32_t unk1838;
	/*0x183c*/ uint32_t unk183c;
	/*0x1840*/ float zoomintime;
	/*0x1844*/ float zoomintimemax;
	/*0x1848*/ float zoominfovy;
	/*0x184c*/ float zoominfovyold;
	/*0x1850*/ float zoominfovynew;
	/*0x1854*/ float fovy;
	/*0x1858*/ float aspect;
	/*0x185c*/ uint32_t hudmessoff;
	/*0x1860*/ int bondmesscnt;
	/*0x1864*/ struct invitem *weapons; // circular linked list, sorted
	/*0x1868*/ struct invitem *equipment; // slots, allocated, unsorted
	/*0x186c*/ int equipmaxitems;
	/*0x1870*/ uint32_t equipallguns;
	/*0x1874*/ uint32_t equipcuritem;
	/*0x1878*/ struct gunheld gunheldarr[10];
	/*0x18f0*/ int magnetattracttime;
	/*0x18f4*/ float angleoffset;
	/*0x18f8*/ uint32_t buthist[10];
	/*0x1920*/ uint8_t buthistindex;
	/*0x1921*/ uint8_t buthistlen;
	/*0x1922*/ uint8_t invincible;
	/*0x1924*/ int healthdamagetype; // 0-7, decreases based on amount of damage taken
	/*0x1928*/ float bondleandown;
	/*0x192c*/ bool mpmenuon;
	/*0x1930*/ uint32_t mpmenumode;
	/*0x1934*/ uint32_t mpquitconfirm;
	/*0x1938*/ uint32_t mpjoywascentre;
	/*0x193c*/ int damagetype;
	/*0x1940*/ uint32_t deathcount;
	/*0x1944*/ float oldcrosspos[2];
	/*0x194c*/ int lastkilltime60;
	/*0x1950*/ int lastkilltime60_2;
	/*0x1954*/ int lastkilltime60_3;
	/*0x1958*/ int lastkilltime60_4;
	/*0x195c*/ int lifestarttime60;
	/*0x1960*/ uint32_t killsthislife;
	/*0x1964*/ uint32_t healthdisplaytime60;
	/*0x1968*/ float guncloseroffset;
	/*0x196c*/ float shootrotx;
	/*0x1970*/ float shootroty;
	/*0x1974*/ char *award1;
	/*0x1978*/ char *award2;
	/*0x197c*/ struct coord chrmuzzlelastpos[2];
	/*0x1994*/ int chrmuzzlelast[2];
	/*0x199c*/ float healthscale;
	/*0x19a0*/ float armourscale;
	/*0x19a4*/ float speedgo;
	/*0x19a8*/ int sighttimer240;
	/*0x19ac*/ int crouchoffsetreal;
	/*0x19b0*/ RoomNum floorroom;
	/*0x19b2*/ uint8_t unk19b2;
	/*0x19b3*/ uint8_t dostartnewlife;
	/*0x19b4*/ float crouchoffsetsmall;
	/*0x19b8*/ int crouchoffsetrealsmall; // 0 = standing, -90 = squatting, can be between during transition
	/*0x19bc*/ float vv_height;     // 159 when Jo, regardless of crouch state
	/*0x19c0*/ float vv_headheight; // 172 when Jo, regardless of crouch state
	/*0x19c4*/ float vv_eyeheight;  // 159 when Jo, regardless of crouch state
	/*0x19c8*/ bool haschrbody;
	/*0x19cc*/ struct geocyl periminfo;
	/*0x19e4*/ struct geocyl perimshoot;
	/*0x19fc*/ float bondprevtheta;
	/*0x1a00*/ struct coord grabbedprevpos;
	/*0x1a0c*/ float grabbedrotoffset;
	/*0x1a10*/ struct coord grabbedposoffset;
	/*0x1a1c*/ bool grabbeddoextra;
	/*0x1a20*/ float grabbedrotextra;
	/*0x1a24*/ int pausemode;
	/*0x1a28*/ int pausetime60;
	/*0x1a2c*/ struct coord grabbedposextra;
	/*0x1a38*/ float grabbedrotextrasum;
	/*0x1a3c*/ struct coord grabbedposextrasum;
	/*0x1a48*/ float bondtankthetaspeedsum;
	/*0x1a4c*/ float bondtankverta;
	/*0x1a50*/ float bondtankvertasum;
	/*0x1a54*/ float bondturrettheta;
	/*0x1a58*/ float bondturretthetasum;
	/*0x1a5c*/ float bondturretspeedsum;
	/*0x1a60*/ float bondturretside;
	/*0x1a64*/ float bondturretchange;
	/*0x1a68*/ int bondtankslowtime;
	/*0x1a6c*/ struct prop *hoverbike;
	/*0x1a70*/ struct coord bondvehicleoffset;
	/*0x1a7c*/ int bondvehiclemode;

	// All bondenter properties are related to mounting the hoverbike
	/*0x1a80*/ float bondentert;
	/*0x1a84*/ float bondentert2;
	/*0x1a88*/ uint32_t bondentertheta;
	/*0x1a8c*/ uint32_t bondenterverta;
	/*0x1a90*/ struct coord bondenterpos;
	/*0x1a9c*/ Mtx bondentermtx;
	/*0x1adc*/ struct coord bondenteraim;

	/*0x1ae8*/ float bondonground;
	/*0x1aec*/ struct prop *tank;
	/*0x1af0*/ struct prop *unk1af0;
	/*0x1af4*/ uint32_t bondonturret;
	/*0x1af8*/ int walkinitmove;
	/*0x1afc*/ struct coord walkinitpos;
	/*0x1b08*/ Mtx walkinitmtx;
	/*0x1b48*/ float walkinitt;
	/*0x1b4c*/ float walkinitt2;
	/*0x1b50*/ struct coord walkinitstart;
	/*0x1b5c*/ struct prop *grabbedprop;
	/*0x1b60*/ float bondgrabthetaspeedsum;
	/*0x1b64*/ int grabstarttime;
	/*0x1b68*/ float autoaimdamp;
	/*0x1b6c*/ struct coord bondforcespeed;
	/*0x1b78*/ bool bondtankexplode;
	/*0x1b7c*/ int bondviewlevtime60;
	/*0x1b84*/ bool tickdiefinished;
	/*0x1b88*/ int introanimnum;
	/*0x1b8c*/ int lastsighton;
	/*0x1b90*/ uint16_t targetset[4]; // related to trackedprops
	/*0x1b98*/ uint8_t sighttracktype; // eg. threat detector, follow lock-on
	/*0x1b9c*/ float speedthetacontrol;
	/*0x1ba0*/ int cam_room;
	/*0x1ba4*/ int16_t autocontrol_aimpad;
	/*0x1ba6*/ int16_t autocontrol_lookup;
	/*0x1ba8*/ int16_t autocontrol_dist;
	/*0x1baa*/ int16_t autocontrol_walkspeed;
	/*0x1bac*/ int autocontrol_turnspeed;
	/*0x1bb0*/ struct coord cam_pos;
	/*0x1bbc*/ struct coord cam_look;
	/*0x1bc8*/ struct coord cam_up;
	/*0x1bd4*/ uint8_t *gunmem2;
	/*0x1bd8*/ int autocontrol_x;
	/*0x1bdc*/ int autocontrol_y;
	/*0x1be0*/ float cachedlookahead;
	/*0x1be4*/ uint16_t lookaheadframe;
	/*0x1be6*/ uint8_t numaibuddies;
	/*0x1be7*/ uint8_t aibuddynums[MAX_BOTS];
	/*0x1bf0*/ bool bondexploding;
	/*0x1bf4*/ int bondnextexplode; // lvframe60 of next explosion
	/*0x1bf8*/ int bondcurexplode;  // Increases by 1 on each tick even when not exploding
	/*0x1bfc*/ uint8_t teleportstate;
	/*0x1bfd*/ uint8_t teleporttime;
	/*0x1bfe*/ uint16_t teleportpad;
	/*0x1c00*/ uint16_t teleportcamerapad;
	/*0x1c04*/ struct chrdata *commandingaibot;
	/*0x1c08*/ uint32_t training;
	/*0x1c0c*/ int deadtimer;
	/*0x1c10*/ bool coopcanrestart;
	/*0x1c14*/ int foot;
	/*0x1c18*/ float footstepdist;
	/*0x1c1c*/ bool usinggoggles; // using night vision or IR scanner
	/*0x1c20*/ struct sndstate *nvhum;
	/*0x1c24*/ struct sndstate *nvoverload;
	/*0x1c28*/ int overexposurered;
	/*0x1c2c*/ int overexposuregreen;
	/*0x1c30*/ int overexposureblue;
	/*0x1c34*/ int prevoverexposurered;
	/*0x1c38*/ int prevoverexposuregreen;
	/*0x1c3c*/ int prevoverexposureblue;
	/*0x1c40*/ uint32_t joybutinhibit;
	/*0x1c44*/ struct coord bondextrapos;
	/*0x1c50*/ uint8_t menuisactive : 1;
	/*0x1c51*/ uint8_t disguised : 1;
	/*0x1c54*/ uint32_t devicesinhibit;
	/*0x1c58*/ float grabbedforcez;
	/*0x1c5c*/ float stealhealth;
	/*0x1c60*/ int16_t fslastradius;
	/*0x1c62*/ int16_t fsscanline;
	/*0x1c64*/ int unk1c64;
	/*0x1c68*/ uint32_t unk1c68;
	/*0x1c6c*/ uint32_t unk1c6c;
	/*0x1c70*/ int16_t altdowntime; // for alt-modes, used like invdowntime and amdowntime
	/*0x1c72*/ int16_t amdowntime; // for alt-modes, used like invdowntime and amdowntime
	/*0x1c76*/ bool wantsgangsta; // player wants to turn weapon sideways
	/*0x1c74*/ float swivelpos[2];
	           bool hasplayeddeathmusic;
};

struct ailist {
	uint8_t *list;
	int id;
};

struct path {
	/*0x00*/ int *pads;
	/*0x04*/ uint8_t id;
	/*0x05*/ uint8_t flags;
	/*0x06*/ uint16_t len;
};

struct covercandidate {
	uint64_t sqdist;
	int covernum;
};

struct coverdefinition {
	struct coord pos;
	struct coord look;
	uint16_t flags;
};

struct cover {
	/*0x00*/ struct coord *pos;
	/*0x04*/ struct coord *look;
	/*0x08*/ RoomNum rooms[2];
	/*0x0c*/ uint16_t flags;
};

struct padsfileheader {
	int numpads;
	int numcovers;
	uintptr_t waypointsoffset;
	uintptr_t waygroupsoffset;
	uintptr_t coversoffset;
	uint16_t padoffsets[1];
};

struct stagesetup {
	/*0x00*/ struct waypoint *waypoints;
	/*0x04*/ struct waygroup *waygroups;
	/*0x08*/ void *cover;
	/*0x0c*/ int *intro;
	/*0x10*/ uint32_t *props;
	/*0x14*/ struct path *paths;
	/*0x18*/ struct ailist *ailists;
	/*0x1c*/ int8_t *padfiledata;
};

struct noisesettings {
	float minradius;
	float maxradius;
	float incradius;
	float decbasespeed;
	float decremspeed;
};

struct recoilsettings {
	float xrange;
	float yrange;
	float zrange;
	float unk0c; // not used
	uint8_t unk10;  // not used
};

struct invaimsettings {
	float zoomfov;
	float guntransup;   // gun y translation when aiming upwards
	float guntransdown; // gun y translation when aiming downwards
	float guntransside; // gun x translation when aiming sideways
	float aimdamppal;   // slowdown speed when aiming or turning (PAL)
	float aimdamp;      // slowdown speed when aiming or turning (NTSC)
	uint32_t tracktype : 4;
	uint32_t unk18_04 : 4;  // not used
	uint32_t flags;
};

struct gunviscmd {
	// See related functions: bgunExecuteGunVisCommands and bgunTestGunVisCommand

	// unk00 - Some kind of condition field
	// 0 = terminator
	// 4 = if bit in hand->unk0639 (bit index specified via unk02)
	// 5 = if in left hand
	// 6 = if in right hand
	uint8_t type;

	uint16_t param;

	// 0 = set part visible if condition passed
	// 1 = set part hidden if condition passed
	// 3 = set part visible if condition passed, or hidden if condition failed
	uint8_t op;

	uint16_t partnum;
	uint16_t unk08;
};

struct weaponfunc {
	/*0x00*/ int type;
	/*0x04*/ uint16_t name;
	/*0x06*/ uint8_t unk06; // not used
	/*0x07*/ int8_t ammoindex; // -1 = no ammo, 0 or 1 = index into weapon->ammos[]
	/*0x08*/ struct noisesettings *noisesettings;
	/*0x0c*/ struct guncmd *fire_animation;
	/*0x10*/ uint32_t flags;
};

struct weaponfunc_shoot {
	struct weaponfunc base;
	/*0x14*/ struct recoilsettings *recoilsettings;
	/*0x18*/ int8_t recoverytime60;
	/*0x1c*/ float damage;
	/*0x20*/ float spread;

	/**
	 * Setting these 4 bytes to higher values causes slower recoil animations
	 * after shooting.
	 */
	/*0x24*/ int8_t unk24;
	/*0x25*/ int8_t unk25;
	/*0x26*/ int8_t unk26;
	/*0x27*/ int8_t unk27;

	/*0x28*/ float recoildist;
	/*0x2c*/ float recoilangle;
	/*0x30*/ float slidemax;
	/*0x34*/ float impactforce;
	/*0x38*/ uint8_t duration60;
	/*0x3a*/ uint16_t shootsound;
	/*0x3c*/ uint8_t penetration;
};

struct weaponfunc_shootsingle {
	struct weaponfunc_shoot base;
};

struct weaponfunc_shootauto {
	struct weaponfunc_shoot base;
	/*0x40*/ float initialrpm; // rounds per minute
	/*0x44*/ float maxrpm; // rounds per minute
	/*0x48*/ float *vibrationstart;
	/*0x4c*/ float *vibrationmax;
	/*0x50*/ int8_t turretaccel;
	/*0x51*/ int8_t turretdecel;
};

struct weaponfunc_shootprojectile {
	struct weaponfunc_shoot base;
	/*0x40*/ int projectilemodelnum;
	/*0x44*/ uint32_t unk44; // unused
	/*0x48*/ float scale;
	/*0x4c*/ int speed;
	/*0x50*/ float unk50;
	/*0x54*/ int traveldist;
	/*0x58*/ int timer60;
	/*0x5c*/ float reflectangle;
	/*0x60*/ int16_t soundnum;
};

struct weaponfunc_throw {
	struct weaponfunc base;
	/*0x14*/ int projectilemodelnum;
	/*0x18*/ int16_t activatetime60; // time until proxies become active, or timed mine/grenade explodes
	/*0x1c*/ int recoverytime60; // time before player can throw another
	/*0x20*/ float damage;
};

struct weaponfunc_melee {
	struct weaponfunc base;
	/*0x14*/ float damage;
	/*0x18*/ float range;
	/*0x1c*/ uint32_t unk1c; // unused
	/*0x20*/ uint32_t unk20; // unused
	/*0x24*/ uint32_t unk24; // unused
	/*0x28*/ float unk28; // unused
	/*0x2c*/ float unk2c; // unused
	/*0x30*/ float unk30; // unused
	/*0x34*/ float unk34; // unused
	/*0x38*/ float unk38; // unused
	/*0x3c*/ float unk3c; // unused
	/*0x40*/ float unk40; // unused
	/*0x44*/ float unk44; // unused
	/*0x48*/ uint32_t unk48; // unused
};

struct weaponfunc_special {
	struct weaponfunc base;
	/*0x14*/ int specialfunc;
	/*0x18*/ int recoverytime60;
	/*0x1c*/ uint16_t soundnum; // unused
};

struct weaponfunc_device {
	struct weaponfunc base;
	/*0x14*/ uint32_t device;
};

struct inventory_ammo {
	uint32_t type;
	uint32_t casingeject;
	int16_t clipsize;
	struct guncmd *reload_animation;
	uint8_t flags;
};

struct modelpartvisibility {
	uint8_t part;
	uint8_t visible;
};

struct weapon {
	/*0x00*/ uint16_t hi_model;
	/*0x02*/ uint16_t lo_model;
	/*0x04*/ struct guncmd *equip_animation;
	/*0x08*/ struct guncmd *unequip_animation;
	/*0x0c*/ struct guncmd *pritosec_animation;
	/*0x10*/ struct guncmd *sectopri_animation;
	/*0x14*/ void *functions[2];
	/*0x1c*/ struct inventory_ammo *ammos[2];
	/*0x24*/ struct invaimsettings *aimsettings;
	/*0x28*/ float muzzlez;
	/*0x2c*/ float posx;
	/*0x30*/ float posy;
	/*0x34*/ float posz;
	/*0x38*/ float sway;
	/*0x3c*/ struct gunviscmd *gunviscmds;
	/*0x40*/ struct modelpartvisibility *partvisibility;
	/*0x44*/ uint16_t shortname;
	/*0x46*/ uint16_t name;
	/*0x48*/ uint16_t manufacturer;
	/*0x4a*/ uint16_t description;
	/*0x4c*/ uint32_t flags;
};

struct cutscene {
	int16_t stage;
	int16_t mission;
	uint32_t scene;
	uint16_t name;
};

struct cheat {
	uint16_t nametextid;
	uint16_t time;
	uint8_t stage_index;
	uint8_t difficulty;
	uint8_t flags;
};

struct headorbody {
	/*0x00*/ uint16_t ismale : 1;
	/*0x00*/ uint16_t unk00_01 : 1;
	/*0x00*/ uint16_t canvaryheight : 1;
	/*0x00*/ uint16_t type : 3;
	/*0x00*/ uint16_t height : 8;
	/*0x02*/ uint16_t filenum;
	/*0x04*/ float scale;
	/*0x08*/ float animscale;
	/*0x0c*/ struct modeldef *modeldef;
	/*0x10*/ uint16_t handfilenum;
};

struct stagetableentry {
	int16_t id;
	uint8_t light_type;
	uint8_t light_alpha;
	uint8_t light_width;
	uint8_t light_height;
	uint16_t unk06;
	uint16_t bgfileid;
	uint16_t tilefileid;
	uint16_t padsfileid;
	uint16_t setupfileid;
	uint16_t mpsetupfileid;
	int16_t maxXRayEdgeLength;
	int16_t eraserpropdist;
	int16_t unk30;
	int16_t alarm;
	uint16_t extragunmem;
};

struct langbank {
	uint16_t id;
	uint32_t begin;
	uint32_t end;
};

struct mpweaponset {
	/*0x00*/ uint16_t name;
	/*0x02*/ uint8_t slots[NUM_MPWEAPONSLOTS];
	/*0x08*/ uint8_t requirefeatures[4];
	/*0x0c*/ uint8_t unk0c;
	/*0x0d*/ uint8_t unk0d;
	/*0x0e*/ uint8_t unk0e;
	/*0x0f*/ uint8_t unk0f;
	/*0x10*/ uint8_t unk10;
	/*0x11*/ uint8_t unk11;
};

struct mphead {
	int16_t headnum;
	uint8_t requirefeature;
};

struct botprofile {
	/*0x00*/ uint8_t type;
	/*0x01*/ uint8_t difficulty;
	/*0x02*/ int16_t name;
	/*0x04*/ int16_t body;
	/*0x06*/ uint8_t requirefeature;
};

struct mpbody {
	int16_t bodynum;
	int16_t name;
	int16_t headnum;
	uint8_t requirefeature;
};

struct mptrack {
	uint16_t musicnum : 7;
	uint16_t duration : 9;
	int16_t name;
	int16_t unlockstage;
};

struct solostage {
	/*0x00*/ uint32_t stagenum;
	/*0x04*/ uint8_t unk04;
	/*0x06*/ uint16_t name1; // "dataDyne Central"
	/*0x08*/ uint16_t name2; // " - Defection"
	/*0x0a*/ uint16_t name3; // "dataDyne Defection"
};

struct stagemusic {
	int16_t stagenum;
	int16_t primarytrack;
	int16_t ambienttrack;
	int16_t xtrack;
};

struct shadesettings {
	float znear;
	float zfar;
	uint32_t unk08;
	uint32_t unk0c;
	float alphafar;
	float alphanear;
};

struct environment {
	int stage;
	int16_t near;
	int16_t far;
	int16_t opaperc;
	int16_t xluperc;
	int16_t refdist;
	int fogmin;
	int fogmax;
	uint8_t sky_r;
	uint8_t sky_g;
	uint8_t sky_b;
	uint8_t numsuns;
	struct sun *suns;
	uint8_t clouds_enabled;
	uint8_t clouds_r;
	uint8_t clouds_g;
	uint8_t clouds_b;
	float clouds_scale;
	int16_t clouds_type;
	float clouds_height;
	uint8_t water_enabled;
	uint8_t water_r;
	uint8_t water_g;
	uint8_t water_b;
	float water_scale;
	int16_t water_type;
	float skyredfrac;
	float skygreenfrac;
	float skybluefrac;
};

struct sun {
	uint8_t lens_flare;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	float pos[3];
	int16_t texture_size;
	int16_t orb_size;
};

struct menuitemdata_controller {
	uint8_t textfadetimer;
	uint8_t contfadetimer;
	uint8_t curmode;
	uint8_t controlgroup;
	int8_t prevmode;
};

struct menuitemdata_list {
	int16_t curoffsety;
	int16_t index;
	int16_t targetoffsety;
	int16_t viewheight;
};

struct menuitemdata_dropdown {
	struct menuitemdata_list list;
	uint16_t unk08; // unused
	uint16_t unk0a; // unused
	int16_t scrolloffset;
	uint16_t unk0e; // unused
};

struct menuitemdata_keyboard {
	char string[11];
	int8_t col;
	int8_t row;
	uint8_t capslock : 1;      // Pressed A on caps button
	uint8_t capseffective : 1; // Same as above, but inverted if holding L or R
};

struct menuitemdata_marquee {
	uint16_t totalmoved;
	uint16_t texthash;
	uint16_t viewwidth;
	uint16_t unk06; // unused
};

struct menuitemdata_ranking {
	int16_t scrolloffset;
};

struct menuitemdata_scrollable {
	int16_t scrolloffset;
	int16_t unk02; // unused
	int16_t maxscrolloffset;
	int16_t dialogheight;
};

struct menuitemdata_slider {
	int16_t multiplier;
};

union menuitemdata {
	struct menuitemdata_controller controller;
	struct menuitemdata_dropdown dropdown;
	struct menuitemdata_keyboard keyboard;
	struct menuitemdata_list list;
	struct menuitemdata_marquee marquee;
	struct menuitemdata_ranking ranking;
	struct menuitemdata_scrollable scrollable;
	struct menuitemdata_slider slider;
};

struct handlerdata_carousel {
	int value;
	uint32_t unk04;
};

struct handlerdata_checkbox {
	uint32_t value;
};

struct handlerdata_dropdown {
	uintptr_t value;
	uintptr_t unk04;
};

struct handlerdata_keyboard {
	char *string;
};

struct handlerdata_label {
	uint32_t colour1;
	uint32_t colour2;
};

struct handlerdata_list {
	union {
		uintptr_t value;
		intptr_t values32;
	};
	union {
		int unk04;
		uint32_t unk04u32;
	};
	int groupstartindex;
	int unk0c;
};

struct handlerdata_slider {
	uint32_t value;
	char *label;
};

struct menuitemrenderdata {
	int x;
	int y;
	int width;
	uint32_t colour;
	uint8_t unk10;
};

struct handlerdata_type19 {
	Gfx *gdl;
	union {
		intptr_t unk04;
		uintptr_t unk04u32;
	};
	struct menuitemrenderdata *renderdata2;
	int unk0c;
};

struct handlerdata_dialog1 {
	uint32_t preventclose;
};

struct handlerdata_dialog2 {
	struct menuinputs *inputs;
};

union handlerdata {
	struct handlerdata_carousel carousel;
	struct handlerdata_checkbox checkbox;
	struct handlerdata_list list;
	struct handlerdata_dropdown dropdown;
	struct handlerdata_keyboard keyboard;
	struct handlerdata_label label;
	struct handlerdata_slider slider;
	struct handlerdata_type19 type19;

	struct handlerdata_dialog1 dialog1;
	struct handlerdata_dialog2 dialog2;
};

struct menuitem {
	uint8_t type;
	uint8_t param;
	uint32_t flags;
	intptr_t param2;
	intptr_t param3;

	union {
		uintptr_t (*handler)(int operation, struct menuitem *item, union handlerdata *data);
		void (*handlervoid)(int operation, struct menuitem *item, union handlerdata *data);
	};
};

struct menudialogdef {
	uint8_t type;
	uintptr_t title;
	struct menuitem *items;
	int (*handler)(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
	uint32_t flags;
	struct menudialogdef *nextsibling;
};

struct surfacetype {
	uint16_t *sounds;
	uint8_t *wallhittexes;
	int16_t numsounds;
	int16_t numwallhittexes;
};

union soundnumhack {
	int16_t packed;

#ifdef PLATFORM_BIG_ENDIAN
	struct {
		uint16_t hasconfig : 1;
		uint16_t confignum : 15;
	};
	struct {
		uint16_t hasconfig2 : 1;
		uint16_t unk02 : 2;
		uint16_t mp3priority : 2;
		uint16_t id : 11;
	};
#else
	struct {
		uint16_t confignum : 15;
		uint16_t hasconfig : 1;
	};
	struct {
		uint16_t id : 11;
		uint16_t mp3priority : 2;
		uint16_t unk02 : 2;
		uint16_t hasconfig2 : 1;
	};
#endif
};

struct audiorussmapping {
	int16_t soundnum;
	uint16_t audioconfig_index;
};

struct audioconfig {
	float dist1; // full volume within dist1
	float dist2; // dist1 to dist2 -> taper volume using curve
	float dist3; // dist2 to dist3 -> scale volume linearly to zero
	float pitch;
	int volpercentage;
	int pan;
	int volchangespeed;
	uint32_t flags;
};

struct artifact {
	uint16_t type;                  // ARTIFACTTYPE_FREE or ARTIFACTTYPE_GLARE
	uint16_t losCheckResult;        // Result of line-of-sight test (0 = occluded, 1 = visible)
	float zbufferDepth;             // Used for sorting/rendering depth.
	uint16_t *zbufferPixelPtr;      // Pointer to the Z-buffer pixel this artifact maps to

	union {
		uint16_t *outputPixelPtr;   // Written to in zbufDrawArtifactsOffscreen
		struct {
			uint16_t screenY;       // Screen Y coordinate (row)
			uint16_t screenX;       // Screen X coordinate (column)
		};
	} screenPos;

	struct light *light;            // Pointer to the source light this glare comes from
	float dist;                     // Distance to player camera
};

struct credit {
	uint8_t more : 1;
	uint8_t retain : 2;
	uint8_t durationindex : 2;
	uint8_t style;
	uint16_t text1;
	uint16_t text2;
};

struct sparktype {
	uint16_t unk00;
	int16_t unk02;
	uint16_t unk04;
	uint16_t unk06;
	uint16_t unk08;
	uint16_t unk0a;
	float weight;
	uint16_t maxage;
	uint16_t unk12;
	uint16_t numsparks;
	uint32_t unk18;
	uint32_t unk1c;
	uint32_t unk20;
	float decel;
};

struct sparkgroup {
	int type;
	int numsparks;
	int age;
	int startindex; // index into g_Sparks
	int room;
	struct prop *prop;
	struct coord pos;
};

struct spark {
	struct coord pos;
	struct coord speed;
	int ttl; // time to live (number of ticks remaining)
};

struct screenbox {
	union {
		struct {
			int16_t xmin;
			int16_t ymin;
			int16_t xmax;
			int16_t ymax;
		};
		int16_t array[2][2];
	};
};

struct roomblock {
	uint8_t type;
	struct roomblock *next;
	union {
		struct { // type 0 (leaf)
			Gfx *gdl;
			Vtx *vertices;
			Col *colours;
		};
		struct { // type 1 (parent)
			struct roomblock *child;
			struct coord *unk0c; // pointer to 2 coords at least
		};
	};
};

struct roomgfxdata {
	/*0x00*/ Vtx *vertices;
	/*0x04*/ Col *colours;
	/*0x08*/ struct roomblock *opablocks;
	/*0x0c*/ struct roomblock *xlublocks;
	/*0x10*/ int16_t lightsindex;
	/*0x12*/ int16_t numlights;
	/*0x14*/ int16_t numvertices;
	/*0x16*/ int16_t numcolours;
	/*0x18*/ struct roomblock blocks[1];
};

struct vtxbatch {
	/*0x00*/ uint16_t gbicmdindex;
	/*0x02*/ uint16_t type;
	/*0x04*/ Gfx *gdl;
	/*0x08*/ struct coord bbmin;
	/*0x14*/ struct coord bbmax;
};

struct room {
	/*0x00*/ uint16_t flags;
	/*0x02*/ int16_t loaded240; // 0 when unloaded, 1 when visible, ticks up to 120 when recently visible
	/*0x04*/ uint8_t portalrecursioncount;
	/*0x05*/ int8_t numportals;
	/*0x06*/ uint8_t snakecount;
	/*0x07*/ uint8_t unk07;
	/*0x08*/ int8_t numlights;
	/*0x09*/ uint8_t numwaypoints; // note: excludes waypoints with PADFLAG_AIDROP
	/*0x0a*/ uint16_t lightindex; // index of start of this room's lights in data file
	/*0x0c*/ uint16_t firstwaypoint; // offset into g_Vars.waypoints
	/*0x0e*/ int16_t roomportallistoffset;
	/*0x10*/ int16_t roommtxindex;
	/*0x14*/ struct roomgfxdata *gfxdata;
	/*0x18*/ float bbmin[3];
	/*0x24*/ float bbmax[3];
	/*0x30*/ struct coord centre;
	/*0x3c*/ float radius; // from volume centre to the corner in 3D
	/*0x40*/ int numvtxbatches;
	/*0x44*/ struct vtxbatch *vtxbatches;

	/**
	 * br values are a brightness value (0-255).
	 */

	// The min and max values are the brightness levels when the room has all
	// its lights shot out, or all lights healthy. They come from the BG file.
	// The br_light_each value is calculated from these based on the number of
	// lights in the room.
	/*0x48*/ uint8_t br_light_min;
	/*0x49*/ uint8_t br_light_max;
	/*0x4a*/ uint8_t br_light_each;

	// The brightness level of the room, assuming no flashes are taking place,
	// and factoring in settled brightness from neighbouring rooms.
	/*0x4b*/ uint8_t br_settled_regional;

	// Generally the same as br_light_min, but is sometimes fudged a bit.
	/*0x4c*/ uint8_t br_base;

	// Unused
	/*0x4d*/ uint8_t unk4d;

	// Lightops control what a room is doing with its lighting. They can be used
	// to simply set a brightness multiplier, or transition to another
	// brightness level over several frames, or repeat a brightness sine pattern.
	// The lightop calculates lightop_cur_frac, which is a multiplier.
	/*0x4e*/ uint8_t lightop : 4;

	// Unused
	/*0x4e*/ uint8_t unk4e_04 : 4;

	// The brightness level of the room, assuming no flashes are taking place,
	// and ignoring the brightness of neighbouring rooms.
	/*0x50*/ int16_t br_settled_local;

	// The current brightness of any temporary flash of light,
	// such as gunfire or lightning.
	/*0x52*/ int16_t br_flash;
	/*0x54*/ int16_t lightop_timer240;
	/*0x56*/ uint16_t hlupdatedframe;
	/*0x58*/ Col *colours;
	/*0x5c*/ float lightop_cur_frac;
	/*0x60*/ float lightop_to_frac;
	/*0x64*/ float lightop_from_frac;
	/*0x68*/ float lightop_duration240;
	/*0x6c*/ float volume;      // in metres
	/*0x70*/ float surfacearea; // in centimetres
	/*0x74*/ float highlightfrac_r;
	/*0x78*/ float highlightfrac_g;
	/*0x7c*/ float highlightfrac_b;
	/*0x80*/ int gfxdatalen; // when inflated
	/*0x84*/ struct wallhit *opawallhits; // opaque
	/*0x88*/ struct wallhit *xluwallhits; // translucent

#ifndef PLATFORM_N64
	/*0x8c*/ uint16_t extra_flags;
#endif
};

struct fireslotthing {
	/*0x00*/ uint8_t unk00;
	/*0x01*/ uint8_t unk01;
	/*0x04*/ struct beam *beam;
	/*0x08*/ int unk08;
	/*0x0c*/ float unk0c;
	/*0x10*/ float unk10;
	/*0x14*/ float unk14;
	/*0x18*/ uint32_t unk18;
	/*0x1c*/ uint32_t unk1c;
};

struct fireslot {
	/*0x00*/ int endlvframe;
	/*0x04*/ struct beam beam;
};

struct menulayer {
	struct menudialog *siblings[5];
	int8_t numsiblings;
	int8_t cursibling;
};

struct menudialog {
	struct menudialogdef *definition;
	uint8_t colstart;
	uint8_t numcols;
	uint16_t blockstart;
	struct menuitem *focuseditem;
	/*0x0c*/ bool dimmed; // when dropdown is open or slider is active
	/*0x10*/ uint32_t unk10;
	/*0x14*/ int x;
	/*0x18*/ int y;
	/*0x1c*/ int width;
	/*0x20*/ int height;
	/*0x24*/ int contentwidth;
	/*0x28*/ int contentheight;
	/*0x2c*/ int dstx;
	/*0x30*/ int dsty;
	/*0x34*/ int dstwidth;
	/*0x38*/ int dstheight;
	/*0x3c*/ uint8_t type; // dialog type
	/*0x3d*/ uint8_t type2; // used when transitioning
	/*0x40*/ float transitionfrac; // >= 0 means transitioning from one dialog type to another
	/*0x44*/ uint32_t colourweight;
	/*0x48*/ float redrawtimer;
	/*0x4c*/ float unk4c;
	/*0x50*/ float statefrac;
	/*0x54*/ int unk54;
	/*0x58*/ uint32_t unk58;
	/*0x5c*/ int unk5c;
	/*0x60*/ uint8_t state;
	/*0x64*/ int scroll; // scroll related, 0 when at top, negative when scrolled down
	/*0x68*/ int dstscroll; // same value as unk64
	/*0x6c*/ uint8_t unk6c;
	/*0x6d*/ int8_t swipedir;
	/*0x6e*/ uint8_t unk6e;
};

struct menudfc {
	struct menuitem *item;
	float unk04;
};

struct menudata_endscreen {
	uint32_t unke1c;

	// ......xx = timed cheat ID
	// .....1.. = this stage + difficulty has a timed cheat
	// .....2.. = timed cheat just got unlocked
	// .....4.. = timed cheat already unlocked
	// .....8.. = completion cheat just got unlocked
	// ....1... = this stage has a completion cheat
	// ..xx.... = completion cheat ID
	uint32_t cheatinfo;

	bool isfirstcompletion;
	uint32_t unke28;
	uint32_t stageindex;
};

struct menudata_main {
	uint32_t unke1c;
	uint32_t controlmode;
	uint32_t mpindex;
	uint32_t unke28;
	uint32_t unke2c;
};

struct menudata_mpsetup {
	uint32_t slotindex;
	uint32_t slotcount;
	uint32_t unke24;
};

struct menudata_mppause {
	uint32_t unke1c;
	uint32_t unke20;
	uint32_t unke24;
	int weaponnum; // of selected weapon in inventory menu
	uint32_t unke2c;
	uint32_t unke30;
	uint32_t unke34;
	uint32_t unke38;
	uint32_t unke3c;
	uint8_t unke40_00 : 1;
};

struct menudata_mpend {
	uint32_t unke1c;
};

struct menudata_filemgr {
	/*0xe1c*/ uint32_t filetypeplusone;
	/*0xe20*/ uint32_t device;
	/*0xe24*/ uint32_t unke24;
	union {
		uint32_t isdeletingforsave;
		uint32_t noteindex;
	};
	/*0xe2c*/ int unke2c;
	/*0xe30*/ uint32_t unke30;
	/*0xe34*/ uint16_t errnum;
	/*0xe38*/ struct filelistfile *filetodelete;
	/*0xe3c*/ uint8_t device1;
	/*0xe3d*/ uint8_t filetypetodelete;
	/*0xe3e*/ uint8_t unke3e;
	/*0xe3f*/ uint8_t listnum;
	/*0xe40*/ uint16_t unke40_00 : 1;
	/*0xe40*/ uint16_t unke40_01 : 1;
	/*0xe41*/ uint8_t unke41;
	/*0xe42*/ uint8_t fileop;
	union {
		void *unke44;
		intptr_t mpplayernum;
	};
	/*0xe48*/ uint32_t fileid;
	/*0xe4c*/ uint32_t deviceserial;
	/*0xe50*/ uint16_t isretryingsave;
	/*0xe52*/ uint8_t device2;
	/*0xe53*/ char filename[16];
	/*0xe64*/ uint32_t unke64;
	/*0xe6c*/ int8_t device3;
};

struct menudata_training {
	uint32_t unke1c;
	struct mpconfigfull *mpconfig;
	uint32_t unke24;
	uint32_t weaponnum;
};

struct textureconfig {
	union {
		texnum_t texturenum;
		uint8_t *textureptr;
	};
	uint8_t width;
	uint8_t height;
	uint8_t level;
	uint8_t format;
	uint8_t depth;
	uint8_t s;
	uint8_t t;
	uint8_t unk0b;
};

// Used for menu 3D model turntables
struct menumodel {
	/*0x000*/ uint8_t loaddelay;
	/*0x002*/ int16_t headnum;
	/*0x004*/ uint8_t *allocstart;
	/*0x008*/ uint32_t alloclen;
	/*0x00c*/ uint32_t newparams;
	/*0x010*/ uint32_t curparams;
	/*0x014*/ Mtx mtx;
	/*0x054*/ struct modeldef *bodymodeldef;
	/*0x058*/ struct modeldef *headmodeldef;
	/*0x05c*/ int16_t newanimnum;
	/*0x05e*/ int16_t curanimnum;
	/*0x060*/ struct model bodymodel;
	/*0x084*/ struct anim bodyanim;
#ifdef PLATFORM_64BIT
	/*0x110*/ uint32_t rwdata[256+128];
#else
	/*0x110*/ uint32_t rwdata[256];
#endif
	/*0x510*/ float curposx;
	/*0x514*/ float curposy;
	/*0x518*/ float curposz;
	/*0x51c*/ float curscale;
	/*0x520*/ float currotx;
	/*0x524*/ float curroty;
	/*0x528*/ float currotz;
	/*0x52c*/ float displacex;
	/*0x530*/ float displacey;
	/*0x534*/ float displacez;
	/*0x538*/ float newposx;
	/*0x53c*/ float newposy;
	/*0x540*/ float newposz;
	/*0x544*/ float newscale;
	/*0x548*/ float newrotx;
	/*0x54c*/ float newroty;
	/*0x550*/ float newrotz;
	/*0x554*/ float zoom; // character selection: far = 100, near = 370
	/*0x564*/ float configurefrac;
	/*0x568*/ uint8_t flags;
	/*0x56a*/ int16_t bodynum;
	/*0x574*/ int zoomtimer60;
	/*0x578*/ int rottimer60;
	/*0x580*/ bool removingpiece;
	/*0x5b0*/ uint8_t perfectheadnum;
	/*0x5b1*/ uint8_t isperfecthead : 1;
	/*0x5b1*/ uint8_t reverseanim : 1;
	/*0x5b1*/ uint8_t configuring : 1;
	/*0x5b1*/ uint8_t drawbehinddialog : 1;
	/*0x5b4*/ struct modelpartvisibility *partvisibility;
};

struct menurow {
	int16_t height;
	uint8_t itemindex;
	int8_t blockindex;
};

struct menucolumn {
	int16_t width;
	int16_t height;
	uint8_t unk04;
	uint16_t rowstart;
	uint8_t numrows;
};

struct menu {
	struct menudialog dialogs[10];
	/*0x460*/ int16_t numdialogs;
	/*0x464*/ struct menulayer layers[6];
	/*0x4f4*/ int16_t depth; // index into layers. 1-indexed?
	/*0x4f8*/ struct menudialog *curdialog;
	/*0x4fc*/ struct menurow rows[88];
	/*0x65c*/ int rowend;
	/*0x660*/ struct menucolumn cols[12];
	/*0x6d8*/ int colend;
	/*0x6dc*/ uint32_t blocks[80]; // for menuitemdata
	/*0x81c*/ int blockend;
	/*0x820*/ uint8_t unk820;
	/*0x824*/ int xrepeattimer60;
	/*0x828*/ int16_t xrepeatcount;
	/*0x82a*/ int16_t xrepeatdir;
	/*0x82c*/ int16_t xrepeatmode;
	/*0x830*/ int yrepeattimer60;
	/*0x834*/ int16_t yrepeatcount;
	/*0x836*/ int16_t yrepeatdir;
	/*0x838*/ int16_t yrepeatmode;
	/*0x83b*/ uint8_t playernum;
	/*0x83c*/ uint8_t openinhibit;
	/*0x840*/ struct menumodel menumodel;
	/*0xdf8*/ int8_t bannernum;
	/*0xdfc*/ struct menudfc unkdfc[4];

	union {
		struct menudata_endscreen endscreen;
		struct menudata_main main;
		struct menudata_mpsetup mpsetup;
		struct menudata_mppause mppause;
		struct menudata_mpend mpend;
		struct menudata_filemgr fm;
		struct menudata_training training;
	};
};

struct gamefile {
	/*0x00*/ char name[11];
	/*0x0b*/ uint8_t thumbnail : 5; // stage index of the image to show on file select screen
	/*0x0b*/ uint8_t autodifficulty : 3;
	/*0x0c*/ uint8_t autostageindex;
	/*0x10*/ uint32_t totaltime;
	/*0x14*/ uint8_t flags[10];
	/*0x1e*/ uint16_t unk1e;
	/*0x20*/ uint16_t besttimes[NUM_SOLOSTAGES][3];
	/*0xa0*/ int coopcompletions[3]; // indexed by difficulty
	/*0xac*/ uint8_t firingrangescores[9];
	/*0xb5*/ uint8_t weaponsfound[6];
};

struct mpchrconfig {
	/*0x00*/ char name[15];
	/*0x0f*/ uint8_t mpheadnum;
	/*0x10*/ uint8_t mpbodynum;
	/*0x11*/ uint8_t team;
	/*0x14*/ uint32_t displayoptions;
	/*0x18*/ uint16_t unk18;
	/*0x1a*/ uint16_t unk1a;
	/*0x1c*/ uint16_t unk1c;
	/*0x1e*/ int8_t placement; // 0 = winner, 1 = second place etc
	/*0x20*/ int rankablescore;
	/*0x24*/ int16_t killcounts[MAX_MPCHRS]; // per player - each index is a chrslot
	/*0x3c*/ int16_t numdeaths;
	/*0x3e*/ int16_t numpoints;
	/*0x40*/ int16_t unk40;
};

struct mpplayerconfig {
	/*0x00*/ struct mpchrconfig base;
	/*0x44*/ uint8_t controlmode;
	/*0x45*/ int8_t contpad1;
	/*0x46*/ int8_t contpad2;
	/*0x48*/ uint16_t options;
	/*0x4c*/ struct fileguid fileguid;
	/*0x54*/ uint32_t kills;
	/*0x58*/ uint32_t deaths;
	/*0x5c*/ uint32_t gamesplayed;
	/*0x60*/ uint32_t gameswon;
	/*0x64*/ uint32_t gameslost;
	/*0x68*/ uint32_t time;
	/*0x6c*/ uint32_t distance; // 1 unit = 100 metres
	/*0x70*/ uint32_t accuracy;
	/*0x74*/ uint32_t damagedealt;
	/*0x78*/ uint32_t painreceived;
	/*0x7c*/ uint32_t headshots;
	/*0x80*/ uint32_t ammoused;
	/*0x84*/ uint32_t accuracymedals;
	/*0x88*/ uint32_t headshotmedals;
	/*0x8c*/ uint32_t killmastermedals;
	/*0x90*/ uint32_t survivormedals;
	/*0x94*/ uint8_t medals;
	/*0x95*/ uint8_t title;
	/*0x96*/ uint8_t newtitle;
	/*0x97*/ uint8_t gunfuncs[6];
	/*0x9d*/ uint8_t handicap;
};

struct mpbotconfig {
	/*0x00*/ struct mpchrconfig base;
	/*0x44*/ uint8_t unk44[3];
	/*0x47*/ uint8_t type;
	/*0x48*/ uint8_t difficulty;
};

struct missionconfig {
	uint8_t difficulty : 7;
	uint8_t pdmode : 1;

	/*0x01*/ uint8_t stagenum;
	/*0x02*/ uint8_t stageindex;

	uint8_t iscoop : 1;
	uint8_t isanti : 1;

	/*0x04*/ uint8_t pdmodereaction;
	/*0x05*/ uint8_t pdmodehealth;
	/*0x06*/ uint8_t pdmodedamage;
	/*0x07*/ uint8_t pdmodeaccuracy;
	/*0x08*/ float pdmodereactionf;
	/*0x0c*/ float pdmodehealthf;
	/*0x10*/ float pdmodedamagef;
	/*0x14*/ float pdmodeaccuracyf;
};

struct mpsetup {
	/*0x800acb88*/ char name[12];
	/*0x800acb94*/ uint32_t options;
	/*0x800acb98*/ uint8_t scenario;
	/*0x800acb99*/ uint8_t stagenum;
	/*0x800acb9a*/ uint8_t timelimit;
	/*0x800acb9b*/ uint8_t scorelimit;
	/*0x800acb9c*/ uint16_t teamscorelimit;

	/**
	 * Each bit signifies that a player or sim is participating.
	 *
	 * Bits 0x000f are for players
	 * Bits 0x0ff0 are for sims
	 * Bits 0xf000 are probably not used
	 */
	/*0x800acb9e*/ uint16_t chrslots;
	/*0x800acba0*/ uint8_t weapons[NUM_MPWEAPONSLOTS];
	/*0x800acba6*/ uint8_t paused;
	/*0x800acba8*/ struct fileguid fileguid;
};

struct bossfile {
	/*0x00*/ char teamnames[MAX_TEAMS][12];
	/*0x60*/ uint8_t locktype;
	/*0x61*/ uint8_t unk89;
	/*0x62*/ uint8_t usingmultipletunes;
	/*0x63*/ uint8_t unk8b;
	/*0x64*/ int8_t tracknum; // -1 = random
	/*0x8d*/ uint8_t multipletracknums[6];
};

struct savebuffer {
	uint32_t bitpos;
	uint8_t bytes[220];
};

struct mparena {
	int16_t stagenum;
	uint8_t requirefeature;
	uint16_t name;
};

struct filelistfile {
	int fileid;
	uint16_t deviceserial;
	char name[16];
};

// This stores information about all files of a particular filetype across all
// devices. For example, the copy file dialog might list all MP player files
// from all devices in one listing, and that information is stored here.
struct filelist {
	/*0x000*/ struct filelistfile files[30];
	/*0x2d0*/ int16_t numfiles;
	/*0x2d2*/ int8_t spacesfree[5]; // per device - controller paks then game pak
	/*0x2d8*/ struct fileguid deviceguids[5];
	/*0x300*/ int8_t devicestartindexes[5]; // game pak then controller paks
	/*0x308*/ int8_t unk305[5]; // controller paks then game pak - error count?
	/*0x30a*/ uint8_t numdevices;
	/*0x30b*/ uint8_t filetype;
	/*0x30c*/ uint8_t timeuntilupdate;
	/*0x30d*/ uint8_t unk30d;
	/*0x30e*/ uint8_t updatedthisframe;
};

struct challenge {
	/*0x00*/ uint16_t name;
	/*0x02*/ int16_t confignum;

	// Bitfield xxx4 321a
	// The number denotes if that player has the challenge available,
	// and `a` denotes if any player has the challenge available.
	/*0x04*/ uint8_t availability;

	// Same structure as availability, however each byte determines how many
	// players it was completed with. So completions[0] is for completions with
	// a single player and completions[3] is for completions with 4 players.
	/*0x05*/ uint8_t completions[MAX_PLAYERS];

	// Array of features which will become unlocked once the challenge is
	// available. The array is automatically populated at runtime based on what
	// features the challenge uses.
	/*0x09*/ uint8_t unlockfeatures[16];
};

struct scenariodata_htb {
	uint32_t unk00;
	struct prop *token; // Briefcase or chr
	struct coord pos;
	int tokenpad;
	int16_t nextindex;
	int16_t padnums[60];
};

struct htmterminal {
	uint32_t unk00;
	struct prop *prop;
	int16_t padnum;
	uint8_t team;
	uint8_t unk0b;
};

struct scenariodata_htm {
	/*0x800ac110*/ int16_t numpads;
	/*0x800ac112*/ int16_t numterminals;
	/*0x800ac114*/ int16_t padnums[60];
	/*0x800ac18c*/ struct htmterminal terminals[7]; // only the first element is used
	/*0x800ac1e0*/ int16_t dlplayernum;
	/*0x800ac1e2*/ int16_t playernuminrange;
	/*0x800ac1e4*/ int dlterminalnum;
	/*0x800ac1e8*/ int numpoints[MAX_MPCHRS];
	/*0x800ac218*/ int dltime240[MAX_MPCHRS];
	/*0x800ac248*/ uint32_t unk138;
	/*0x800ac24c*/ struct prop *uplink;
	/*0x800ac250*/ uint32_t unk140;
	/*0x800ac250*/ uint32_t unk144;
};

struct scenariodata_pac {
	int16_t unk00;
	uint16_t age240;
	int victimindex;
	int16_t victims[MAX_MPCHRS]; // shuffled list of player numbers
	int16_t killcounts[MAX_MPCHRS]; // indexed by player num
	int16_t survivalcounts[MAX_MPCHRS]; // indexed by player num
};

struct scenariodata_koh {
	/*0x800ac110*/ uint32_t unk00;
	/*0x800ac114*/ int16_t occupiedteam;
	/*0x800ac116*/ int16_t elapsed240;
	/*0x800ac118*/ int16_t movehill;
	/*0x800ac11a*/ int16_t hillindex;
	/*0x800ac11c*/ int16_t hillcount;
	/*0x800ac11e*/ RoomNum hillrooms[2];
	/*0x800ac122*/ int16_t hillpads[9];
	/*0x800ac134*/ struct coord hillpos;
	/*0x800ac140*/ float colourfracr;
	/*0x800ac144*/ float colourfracg;
	/*0x800ac148*/ float colourfracb;
};

struct ctcspawnpadsperteam {
	int16_t homepad;
	int16_t numspawnpads;
	int16_t spawnpads[6];
};

struct scenariodata_ctc {
	/*0x00*/ int16_t playercountsperteam[4];
	/*0x08*/ int16_t teamindexes[4];
	/*0x10*/ RoomNum baserooms[4];
	/*0x18*/ struct ctcspawnpadsperteam spawnpadsperteam[4];
	/*0x58*/ struct prop *tokens[4];
};

struct scenariodata {
	union {
		struct scenariodata_htb htb;
		struct scenariodata_htm htm;
		struct scenariodata_pac pac;
		struct scenariodata_koh koh;
		struct scenariodata_ctc ctc;
	};
};

struct bgportal {
	uint16_t verticesoffset;
	int16_t roomnum1;
	int16_t roomnum2;
	uint8_t flags;
};

struct portalcamcacheitem {
	uint16_t updatedframe1;
	int16_t side;
	uint16_t updatedframe2;
	int16_t bboxisvalid;
	int16_t xmin;
	int16_t ymin;
	int16_t xmax;
	int16_t ymax;
};

struct portalmetric { // related to portals
	struct coord normal;
	float min;
	float max;
};

struct trainingdata {
	uint8_t intraining : 1;
	uint8_t failed : 1;
	uint8_t completed : 1;
	uint8_t finished : 1;
	uint8_t holographedpc : 1;
	int8_t timeleft;
	int timetaken;
	struct defaultobj *obj;
	uint32_t unk0c;
};

struct activemenu {
	/*0x00*/ int8_t screenindex;
	/*0x02*/ int16_t xradius;
	/*0x04*/ int16_t slotwidth;
	/*0x06*/ int16_t selx;
	/*0x08*/ int16_t sely;
	/*0x0a*/ int16_t dstx;
	/*0x0c*/ int16_t dsty;
	/*0x0e*/ uint8_t slotnum; // 0-8, where 4 is middle
	/*0x0f*/ uint8_t fromslotnum; // when moving from one slot to another
	/*0x10*/ int cornertimer;
	/*0x14*/ int returntimer; // time before selection returns to middle after releasing control stick
	/*0x18*/ float alphafrac;
	/*0x1c*/ float selpulse; // determines the border colour of the selection box

	/**
	 * Indexes into the player's inventory. Element 0 is AM slot top left,
	 * then it goes left to right, top to bottom. Updated every tick while
	 * the active menu is open. A value of 0xff means the slot is not shown.
	 */
	/*0x20*/ uint8_t invindexes[8];

	/**
	 * Weapon numbers to slot mapping. In multiplayer this is determined at the
	 * start of the match. A value of 0xff means the slot cannot be shown in
	 * this match.
	 */
	/*0x28*/ uint8_t favourites[8];

	/*0x30*/ uint8_t togglefunc;
	/*0x31*/ uint8_t numitems; // number of items in player's inventory; can be higher than the number of AM slots
	/*0x32*/ uint8_t allbots; // when player holds R on the bot command screen
	/*0x33*/ uint8_t prevallbots; // used when opening "Pick Target" menu for attack command
	/*0x34*/ int8_t origscreennum; // original screen number before using allbots

#ifndef PLATFORM_N64
	/*    */ float mousex;
	/*    */ float mousey;
#endif
};

struct briefing {
	uint16_t briefingtextnum;
	uint16_t objectivenames[6]; // index 0 is the briefing, and the rest are objectives
	uint16_t objectivedifficulties[6]; // index 0 is unused
	uint16_t langbank;
};

struct criteria_roomentered {
	uint32_t unk00;
	uint32_t pad;
	uint32_t status;
	struct criteria_roomentered *next;
};

struct criteria_throwinroom {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t pad;
	uint32_t status;
	struct criteria_throwinroom *next;
};

struct criteria_holograph {
	uint32_t holo_unk00;
	uint32_t obj;
	uint32_t status;
	struct criteria_holograph *next;
};

struct mppreset {
	uint16_t name;
	uint32_t confignum;
	uint8_t requirefeatures[16]; // Doesn't seem to be used? All values are zero
};

struct explosiontype {
	/*0x00*/ float rangeh;
	/*0x04*/ float rangev;
	/*0x08*/ float changerateh;
	/*0x0c*/ float changeratev;
	/*0x10*/ float innersize;
	/*0x14*/ float blastradius;
	/*0x18*/ float damageradius;
	/*0x1c*/ int16_t duration;
	/*0x1e*/ int16_t propagationrate;
	/*0x20*/ float flarespeed;
	/*0x24*/ uint8_t smoketype;
	/*0x26*/ uint16_t sound;
	/*0x28*/ float damage;
};

struct explosionpart {
	struct coord pos;
	float size;
	float rot;
	int16_t frame;
	uint8_t bb;
};

struct explosionbb {
	struct coord bbmin;
	struct coord bbmax;
	RoomNum room;
	RoomNum room2;
};

struct explosion {
	struct prop *prop; // Prop of the explosion
	struct prop *source; // Prop of the thing that created the explosion
	struct explosionpart parts[40];
	/*0x3c8*/ int16_t age;
	/*0x3ca*/ RoomNum room;
	/*0x3cc*/ int8_t type;
	/*0x3cd*/ int8_t makescorch;
	/*0x3ce*/ int8_t owner;
	/*0x3cf*/ uint8_t numbb;
	/*0x3d0*/ struct coord unk3d0;
	/*0x3dc*/ struct coord unk3dc;
	/*0x3e8*/ struct explosionbb bbs[5]; // may be smaller
	/*0x474*/ uint32_t unk474;
};

struct smoketype {
	/*0x00*/ int16_t duration;
	/*0x02*/ int16_t fadespeed;
	/*0x04*/ int16_t spreadspeed;
	/*0x06*/ int16_t size;
	/*0x08*/ float bgrotatespeed;
	/*0x0c*/ uint8_t r;
	/*0x0c*/ uint8_t g;
	/*0x0c*/ uint8_t b;
	/*0x10*/ float fgrotatespeed;
	/*0x14*/ int16_t numclouds;
	/*0x18*/ float unk18;
	/*0x1c*/ float unk1c;
	/*0x20*/ float unk20;
};

struct smokepart {
	/*0x00*/ struct coord pos;
	/*0x0c*/ float size;
	/*0x10*/ float rot;
	/*0x14*/ float deltarot;
	/*0x18*/ float offset1;
	/*0x1c*/ float offset2;
	/*0x20*/ float alpha;
	/*0x24*/ int16_t count;
};

struct smoke {
	/*0x000*/ struct prop *prop; // Prop of the smoke itself
	/*0x004*/ int16_t age;
	/*0x006*/ uint16_t type : 7;

	// If source is set, option 0/1 determines whether the source is a prop/pad effect
	// If source is null, option 0/1 is the handnum (right/left)
	/*0x006*/ uint16_t option : 1;

	/*0x007*/ uint16_t unk06_08 : 8;
	/*0x008*/ struct smokepart parts[10];

	/*0x198*/
	union {
		// The thing generating smoke
		struct prop *sourceprop;
		struct padeffectobj *padeffect;
		void *source;
	};
};

struct textoverride {
	/*0x00*/ uint32_t unk00;
	/*0x04*/ int objoffset;
	/*0x08*/ int weapon;
	/*0x0c*/ uint32_t obtaintext;     // eg. "Obtain medlab 2 keycard\n"
	/*0x10*/ uint32_t ownertext;      // eg. "Guard's\n"
	/*0x14*/ uint32_t inventorytext;  // eg. "Medlab 2 key card\n"
	/*0x18*/ uint32_t inventory2text; // eg. "Medlab 2 key card\n"
	/*0x1c*/ uint32_t pickuptext;     // eg. "Picked up medlab 2 key card.\n"
	/*0x20*/ struct textoverride *next;
	/*0x24*/ struct defaultobj *obj;
};

typedef struct {
	int		status;
	int		channel;
	uint8_t		id[32];
	uint8_t		label[32];
	int		version;
	int		dir_size;
	int		inode_table;		/* block location */
	int		minode_table;		/* mirrioring inode_table */
	int		dir_table;		/* block location */
	int		inode_start_page;	/* page # */
	uint8_t		banks;
	uint8_t		activebank;
} PakPfs;

typedef struct {
	uint32_t	file_size;	/* bytes */
  	uint32_t 	game_code;
  	uint16_t 	company_code;
  	char  	ext_name[4];
  	char 	game_name[16];
} PakPfsState;


struct pakdata {
	/*0x000*/ PakPfsState notes[16];
	/*0x200*/ bool notesinuse[16];
	/*0x240*/ uint16_t pagesused;
	/*0x242*/ uint16_t pagesfree;
};

struct pakheadercache {
	int blocknum;
	uint8_t payload[0x20];
};

struct pak {
	/*0x000*/ int type;
	/*0x004*/ uint32_t rumblestate;
	/*0x008*/ uint32_t unk008;
	/*0x00c*/ uint32_t unk00c;
	/*0x010*/ int state;
	/*0x014*/ uint8_t features;
	/*0x018*/ struct pakdata pakdata;
	/*0x25c*/ uint32_t maxfileid;
	/*0x260*/ uint32_t serial;
	/*0x264*/ uint32_t plugcount;
	/*0x268*/ uint32_t unk268;
	/*0x26c*/ uint32_t unk26c;
	/*0x270*/ uint32_t unk270;
	/*0x274*/ uint32_t unk274;
	/*0x278*/ uint32_t unk278;
	/*0x27c*/ uint32_t unk27c;
	/*0x280*/ uint32_t unk280;
	/*0x284*/ int rumblepulsestopat;
	/*0x288*/ uint32_t rumblepulselen;
	/*0x28c*/ uint32_t rumblepulsetimer;  // counts up to pulselen then loops
	/*0x290*/ int notestotal; // always 16
	/*0x294*/ int notesused;
	/*0x298*/ uint32_t unk298;
	/*0x29c*/ int pdnoteindex;
	/*0x2a0*/ uint32_t pdnumbytes;
	/*0x2a4*/ uint32_t pdnumblocks;
	/*0x2a8*/ uint32_t pdnumpages;
	/*0x2ac*/ uint32_t unk2ac;
	/*0x2b0*/ uint32_t unk2b0;
	/*0x2b4*/ float rumblettl;
	/*0x2b8*/ uint8_t unk2b8_01 : 1;
	/*0x2b8*/ uint8_t unk2b8_02 : 1;
	/*0x2b8*/ uint8_t isgbcamera : 1;
	/*0x2b8*/ uint8_t isgbpd : 1;
	/*0x2b8*/ uint8_t unk2b8_05 : 1;
	/*0x2b8*/ uint8_t unk2b8_06 : 1;
	/*0x2b8*/ uint8_t showdatalost : 1;
	/*0x2b9*/ uint8_t unk2b9;
	/*0x2ba*/ uint8_t unk2ba;
	/*0x2bb*/ uint8_t unk2bb;
	/*0x2bc*/ uint8_t pdnumnotes;
	/*0x2bd*/ uint8_t unk2bd;
	/*0x2be*/ uint8_t headercachecount;
	/*0x2c0*/ struct pakheadercache *headercache;
	/*0x2c4*/ uint8_t *unk2c4; // len 4096
	/*0x2c8*/ uint32_t unk2c8;
};

typedef struct OSScTask_s {
    struct OSScTask_s   *next;          /* note: this must be first */
    uint32_t                 state;
    uint32_t			flags;
    void		*framebuffer;	/* used by graphics tasks */

    OSTask              list;
    //OSMesg              msg;
} OSScTask;

struct invitem_weap {
	int16_t weapon1;
	int16_t pickuppad;
};

struct invitem_prop {
	struct prop *prop;
};

struct invitem_dual {
	int weapon1;
	int weapon2;
};

struct invitem {
	/*0x00*/ int type;

	union {
		struct invitem_weap type_weap;
		struct invitem_prop type_prop;
		struct invitem_dual type_dual;
	};

	/*0x0c*/ struct invitem *next;
	/*0x10*/ struct invitem *prev;
};

struct chrnumaction {
	int16_t chrnum;
	uint8_t myaction;
};

struct modelstate {
	struct modeldef *modeldef;
	uint16_t fileid;
	uint16_t scale;
};

struct botdifficulty {
	uint8_t shootdelay;
	float unk04;
	float unk08;
	uint16_t unk0c;
	float unk10;
	float unk14;
	float unk18;
	int dizzyamount;
};

struct animtablerow {
	int16_t animnum;
	bool flip;
	float endframe;
	float speed;
	uint32_t unk10;
	float thudframe1;
	float thudframe2;
};

struct animtable {
	int hitpart;
	struct animtablerow *deathanims;
	struct animtablerow *injuryanims;
	int deathanimcount;
	int injuryanimcount;
};

struct headanim {
	int16_t animnum;
	float loopframe;
	float endframe;
	float translateperframe;
	float minspeed;
	float maxspeed;
};

struct vimode {
	int fbwidth;
	int fbheight;
	int width;
	float yscale;
	int xscale;
	int fullheight;
	int fulltop;
	int wideheight;
	int widetop;
	int cinemaheight;
	int cinematop;
};

struct miscbio {
	uint32_t name;
	uint32_t description;
};

struct hangarbio {
	uint32_t name;
	uint32_t description;
	uint32_t subheading;
	uint32_t unk0c;
};

struct chrbio {
	uint32_t name;
	uint32_t race;
	uint32_t age;
	uint32_t description;
};

struct ranking {
	struct mpchrconfig *mpchr;
	union {
		uint32_t teamnum;
		uint32_t chrnum;
	};
	uint32_t positionindex;
	uint8_t unk0c;
	int score;
};

struct hudmsgtype {
	/*0x00*/ uint8_t unk00;
	/*0x01*/ uint8_t unk01;
	/*0x02*/ uint8_t unk02;
	/*0x04*/ struct fontchar **unk04;
	/*0x08*/ struct font **unk08;
	/*0x0c*/ uint32_t colour;
	/*0x10*/ uint32_t unk10;
	/*0x14*/ uint8_t alignh;
	/*0x15*/ uint8_t alignv;
	/*0x16*/ int16_t unk16;
	/*0x18*/ int16_t unk18;
	/*0x1c*/ int duration;
};

struct hudmessage {
	/*0x000*/ uint8_t state;
	/*0x001*/ uint8_t boxed;
	/*0x002*/ uint8_t allowfadein;
	/*0x003*/ uint8_t flash;
	/*0x004*/ uint8_t opacity;
	/*0x006*/ uint16_t timer;
	/*0x008*/ struct fontchar *font1;
	/*0x00c*/ struct font *font2;
	/*0x010*/ uint32_t textcolour;
	/*0x014*/ uint32_t glowcolour;
	/*0x018*/ uint16_t x;
	/*0x01a*/ uint16_t y;
	/*0x01c*/ uint16_t width;
	/*0x01e*/ uint16_t height;
	/*0x020*/ char text[400];
	/*0x1b0*/ int channelnum;
	/*0x1b4*/ uint32_t type;
	/*0x1b8*/ int id;
	/*0x1bc*/ int showduration;
	/*0x1c0*/ int playernum;
	/*0x1c4*/ uint32_t flags;
	/*0x1c8*/ uint8_t alignh;
	/*0x1c9*/ uint8_t alignv;
	/*0x1cc*/ uint32_t xmarginextra;
	/*0x1d0*/ uint32_t xmargin;
	/*0x1d4*/ uint32_t ymargin;
	/*0x1d8*/ uint32_t hash;
};

struct frtarget {
	/*0x00*/ uint8_t inuse : 1;         // 1 if being used at all in this session
	/*0x00*/ uint8_t active : 1;        // 1 if target has appeared
	/*0x00*/ uint8_t destroyed : 1;
	/*0x00*/ uint8_t scriptenabled : 1;
	/*0x00*/ uint8_t rotating : 1;
	/*0x00*/ uint8_t rotateoncloak : 1;
	/*0x00*/ uint8_t frpadindex : 2;
	/*0x01*/ uint8_t maxdamage;
	/*0x02*/ uint8_t scriptindex;
	/*0x04*/ struct prop *prop;
	/*0x08*/ struct coord dstpos;
	/*0x14*/ int scriptsleep;
	/*0x18*/ int timeuntilrotate;
	/*0x1c*/ float travelspeed;
	/*0x20*/ uint8_t damage;
	/*0x21*/ uint8_t scriptoffset;
	/*0x24*/ float rotatespeed;      // Negative for reverse direction
	/*0x28*/ float angle;
	/*0x2c*/ float rotatetoangle;
	/*0x30*/ uint8_t flags;
	/*0x31*/ uint8_t silent;            // 0 if playing the hum sound while travelling
	/*0x32*/ uint8_t donestopsound;     // 1 if the clank sound has played when stopping travelling
	/*0x33*/ uint8_t travelling;
	/*0x34*/ int8_t frpadnum;
	/*0x38*/ int invincibletimer;
};

struct frdata {
	/*0x000*/ uint8_t maxactivetargets;
	/*0x002*/ uint16_t goalscore;
	/*0x004*/ uint8_t goaltargets;
	/*0x005*/ uint8_t timelimit;
	/*0x006*/ uint8_t ammolimit;
	/*0x007*/ uint8_t sdgrenadelimit;
	/*0x008*/ uint8_t goalaccuracy;
	/*0x00c*/ float speed;
	/*0x010*/ struct frtarget targets[18];
	/*0x448*/ uint8_t difficulty;
	/*0x44c*/ int timetaken;
	/*0x450*/ int score;
	/*0x454*/ uint8_t numtargets;
	/*0x455*/ uint8_t targetsdestroyed;
	/*0x456*/ uint16_t slot;
	/*0x458*/ uint16_t numshots;
	/*0x45a*/ uint8_t numshotssincetopup;
	/*0x45b*/ uint8_t failreason;
	/*0x45c*/ uint16_t numhitsbullseye;
	/*0x45e*/ uint16_t numhitsring1;
	/*0x460*/ uint16_t numhitsring2;
	/*0x462*/ uint16_t numhitsring3;
	/*0x464*/ int8_t menucountdown;
	/*0x465*/ uint8_t menutype : 3;
	/*0x465*/ uint8_t donelighting : 1;
	/*0x465*/ uint8_t donealarm : 1;
	/*0x465*/ uint8_t ammohasgrace : 1;
	/*0x466*/ uint8_t helpscriptindex;
	/*0x467*/ uint8_t helpscriptoffset;
	/*0x468*/ uint8_t helpscriptenabled;
	/*0x46c*/ int helpscriptsleep;
	/*0x470*/ uint8_t padindexoffset;
	/*0x471*/ uint8_t feedbackzone;
	/*0x472*/ int8_t feedbackttl;
	/*0x474*/ int16_t proxyendtimer;
	/*0x476*/ int16_t ammoextra;
	/*0x478*/ int16_t sdgrenadeextra;
	/*0x47c*/ uint32_t unk47c;
};

struct menudata_5d8 {
	struct fileguid fileguid;
	uint8_t unk08;
	uint8_t unk09;
	uint8_t unk0a;
	uint8_t unk0b;
};

struct menudata {
	/*0x000*/ int count;
	/*0x004*/ int root;
	/*0x008*/ int prevmenuroot; // also a menuroot constant
	/*0x00c*/ struct menudialogdef *prevmenudialog;
	/*0x010*/ float unk010;
	/*0x014*/ uint8_t bg;
	/*0x015*/ uint8_t nextbg;
	/*0x016*/ uint8_t screenshottimer;
	/*0x017*/ uint8_t playerjoinalpha[MAX_PLAYERS];
	/*0x01b*/ int8_t bannernum;
	/*0x01c*/ struct menumodel hudpiece;
	/*0x5d4*/ uint8_t unk5d4;
	/*0x5d5*/ uint8_t unk5d5_01 : 1;
	/*0x5d5*/ uint8_t unk5d5_02 : 1;
	/*0x5d5*/ uint8_t usezbuf : 1;
	/*0x5d5*/ uint8_t unk5d5_04 : 1;
	/*0x5d5*/ uint8_t unk5d5_05 : 1;
	/*0x5d5*/ uint8_t unk5d5_06 : 1;
	/*0x5d5*/ uint8_t unk5d5_07 : 1;
	/*0x5d5*/ uint8_t unk5d5_08 : 1;
	/*0x5d8*/ struct menudata_5d8 unk5d8[12];
	/*0x668*/ int8_t unk668;
	/*0x669*/ uint8_t unk669[5];
	/*0x66e*/ int8_t unk66e; // index into 669
	/*0x66f*/ uint8_t unk66f;
	/*0x670*/ int unk670;
	/*0x674*/ int unk674;
};

struct ammotype {
	int capacity;
	uint32_t unk04;
	float unk08;
};

struct weather58 {
	float unk00;
	float unk04;
	int unk08;
};

struct weatherdata {
	/*0x00*/ float windspeedx;
	/*0x04*/ float windspeedz;
	/*0x08*/ float windanglerad;
	/*0x0c*/ float newwindangle; // windanglerad transitions to this when a new wind angle is chosen
	/*0x10*/ int windangletransitiontime;
	/*0x14*/ float windspeed;
	/*0x18*/ uint32_t unk18;
	/*0x1c*/ uint32_t unk1c;
	/*0x20*/ int type;
	/*0x24*/ struct weatherparticledata *particledata[2];
	/*0x2c*/ uint32_t unk2c;
	/*0x30*/ uint32_t unk30;
	/*0x34*/ struct sndstate *audiohandles[4];
	/*0x44*/ int unk44;
	/*0x48*/ int unk48;
	/*0x4c*/ int unk4c;
	/*0x50*/ int unk50;
	/*0x54*/ int unk54;
	/*0x58*/ struct weather58 unk58[4];
	/*0x88*/ float sndcurrentvolume;
	/*0x8c*/ float snddesiredvolume;
	/*0x90*/ int sndtransitiontime;
	/*0x94*/ int unk94;
	/*0x98*/ int unk98;
	/*0x9c*/ int unk9c;
	/*0xa0*/ int unka0;
	/*0xa4*/ int unka4;
	/*0xa8*/ uint32_t unka8;
	/*0xac*/ uint32_t unkac;
	/*0xb0*/ uint32_t unkb0;
	/*0xb4*/ uint32_t unkb4;
	/*0xb8*/ float rdcurrentlength; // vertical length of the raindrop
	/*0xbc*/ float rddesiredlength;
	/*0xc0*/ int rdtransitiontime; // used to change the length of raindrops when weather intensity changes
	/*0xc4*/ float lightningchance;
	/*0xc8*/ float raindropfallspeed;
	/*0xcc*/ int intensity;
	/*0xd0*/ int numcurrentsnowflakes;
	/*0xd4*/ int numdesiredparticles;
	/*0xd8*/ uint32_t unkd8;
	/*0xdc*/ uint32_t unkdc;
	/*0xe0*/ uint32_t unke0;
	/*0xe4*/ uint32_t unke4;
	/*0xe8*/ uint32_t unke8;
	/*0xec*/ uint32_t unkec;
	/*0xf0*/ uint32_t unkf0;
	/*0xf4*/ uint32_t unkf4;
	/*0xf8*/ int16_t rainsfxindex;
	/*0xfc*/ uint32_t unkfc;
};

struct weatherparticle {
	struct coord pos;
	int active;
	struct coord inc;
	float horizspeed;
};

struct weatherparticledata {
	struct weatherparticle particles[500];
	/*0x3e80*/ struct coord unk3e80;
	/*0x3e8c*/ struct coord boundarymax;
	/*0x3e98*/ struct coord boundarymin;
	/*0x3ea4*/ struct coord boundaryrange;
	/*0x3eb0*/ uint32_t unk3eb0;
	/*0x3eb4*/ uint32_t unk3eb4;
	/*0x3eb8*/ uint32_t unk3eb8;
	/*0x3ebc*/ uint32_t unk3ebc;
	/*0x3ec0*/ uint32_t unk3ec0;
	/*0x3ec4*/ uint32_t unk3ec4;
	/*0x3ec8*/ float unk3ec8[8];
	/*0x3ee8*/ uint32_t unk3ee8;
	/*0x3eec*/ uint32_t unk3eec;
	/*0x3ef0*/ uint32_t unk3ef0;
	/*0x3ef4*/ uint32_t unk3ef4;
	/*0x3ef8*/ uint32_t unk3ef8;
	/*0x3efc*/ uint32_t unk3efc;
};

#ifndef PLATFORM_N64

struct weathercfg {
	int stagenum;
	uint32_t flags;
	float windspeed;
	float ymin;
	float ymax;
	float zmax;
	RoomNum skiprooms[WEATHERCFG_MAX_SKIPROOMS]; // if flags has WEATHERFLAG_INCLUDE, rooms that have weather, else rooms that don't
	float windanglerad; // wind fields only used if flags has WEATHERFLAG_FORCE_WINDDIR
	float windspeedx;
	float windspeedz;
};

#endif

struct texture {
	uint32_t soundsurfacetype : 4;
	uint32_t surfacetype : 4;
	uint32_t dataoffset : 24;
	uint32_t unk04_00 : 4;
	uint32_t unk04_04 : 4;
	uint32_t unk04_08 : 4;
	uint32_t unk04_0c : 4;
};

struct bgcmd {
	uint8_t type;
	uint8_t len;
	int param;
};

struct drawslot {
	RoomNum roomnum;
	uint8_t unk02;
	uint8_t draworder;
	struct screenbox box;
};

struct zrange {
	union {
		struct {
			float near;
			float far;
		};
		float f[2];
	};
};

struct bgsnakeitem {
	/*0x00*/ RoomNum roomnum;
	/*0x02*/ RoomNum fromroomnums[5];
	/*0x0c*/ uint8_t depth;
	/*0x0d*/ uint8_t numportals;
	/*0x0e*/ int16_t roomportallistoffset;
	/*0x10*/ struct screenbox screenbox;
};

struct bgsnake {
	int16_t count;
	int16_t headindex;
	int16_t tailindex;
	struct zrange zrange;
	struct bgsnakeitem items[250];
};

struct menuinputs {
	/*0x00*/ int8_t leftright;     // Both control stick and C/D buttons - set on initial press and on key repeat intervals
	/*0x01*/ int8_t updown;        // As above
	/*0x02*/ uint8_t select;        // A/Z buttons
	/*0x03*/ uint8_t back;          // B button
	/*0x04*/ int8_t xaxis;         // Control stick's current left/right position
	/*0x05*/ int8_t yaxis;         // Control stick's current up/down position
	/*0x06*/ uint8_t shoulder;      // L or R buttons
	/*0x07*/ uint8_t back2;         // Used in keyboard
	/*0x08*/ int8_t leftrightheld; // Same as leftright, but is also set between repeat intervals
	/*0x09*/ int8_t updownheld;    // As above
	/*0x0a*/ int8_t start;
	/*0x0c*/ int unk0c;
	/*0x10*/ int unk10;
	/*0x14*/ uint8_t unk14;
#ifndef PLATFORM_N64
	/*0x15*/ uint8_t mousemoved;
	/*0x16*/ int8_t mousescroll;
	/*0x18*/ int mousex;
	/*0x1c*/ int mousey;
#endif
};

struct mpconfigsim {
	uint8_t type;
	uint8_t mpheadnum;
	uint8_t mpbodynum;
	uint8_t team;
	uint8_t difficulties[MAX_PLAYERS];
};

struct mpconfig {
	struct mpsetup setup;
	struct mpconfigsim simulants[MAX_BOTS];
};

struct mpweapon {
	/*0x00*/ uint8_t weaponnum;
	/*0x01*/ int8_t priammotype;
	/*0x02*/ uint8_t priammoqty;
	/*0x03*/ int8_t secammotype;
	/*0x04*/ uint8_t secammoqty;
	/*0x05*/ uint8_t hasweapon : 1;
	/*0x05*/ uint8_t unlockfeature : 7;
	/*0x06*/ int16_t model;
	/*0x08*/ int16_t extrascale;
};

struct mpstrings {
	char description[200];
	char aibotnames[MAX_BOTS][15];
};

struct mpconfigfull {
	struct mpconfig config;
	struct mpstrings strings;
};

struct movedata {
	/*0x00*/ bool canswivelgun;
	/*0x04*/ bool canmanualaim;
	/*0x08*/ bool triggeron;
	/*0x0c*/ int btapcount;
	/*0x10*/ bool canlookahead;
	/*0x14*/ int unk14;
	/*0x18*/ bool cannaturalturn;
	/*0x1c*/ bool cannaturalpitch;
	/*0x20*/ bool digitalstepforward;
	/*0x24*/ bool digitalstepback;
	/*0x28*/ bool digitalstepleft;
	/*0x2c*/ bool digitalstepright;
	/*0x30*/ float unk30;
	/*0x34*/ float unk34;
	/*0x38*/ float speedvertadown;
	/*0x3c*/ float speedvertaup;
	/*0x40*/ float aimturnleftspeed;
	/*0x44*/ float aimturnrightspeed;
	/*0x48*/ int weaponbackoffset;
	/*0x4c*/ int weaponforwardoffset;
	/*0x50*/ uint32_t unk50;
	/*0x54*/ bool aiming;
	/*0x58*/ bool zooming;
	/*0x5c*/ float zoomoutfovpersec;
	/*0x60*/ float zoominfovpersec;
	/*0x64*/ int crouchdown;
	/*0x68*/ int crouchup;
	/*0x6c*/ bool rleanleft;
	/*0x70*/ bool rleanright;
	/*0x74*/ bool detonating;
	/*0x78*/ bool canautoaim;
	/*0x7c*/ bool farsighttempautoseek;
	/*0x80*/ bool eyesshut;
	/*0x84*/ bool invertpitch;
	/*0x88*/ bool disablelookahead;
	/*0x8c*/ int c1stickxsafe; // raw values but adjusted to remove dead zone
	/*0x90*/ int c1stickysafe;
	/*0x94*/ int c1stickxraw; // raw values from control stick
	/*0x98*/ int c1stickyraw;
	/*0x9c*/ int analogturn;
	/*0xa0*/ int analogpitch;
	/*0xa4*/ int analogstrafe;
	/*0xa8*/ int analogwalk;
	/*0xac*/ int alt1tapcount;
	/*    */ float freelookdx; // how much the mouse moved ...
	/*    */ float freelookdy; // ... scaled by sensitivity
	/*    */ float analoglean; // how much we're trying to lean

};

struct attackanimgroup {
	struct attackanimconfig *animcfg;
	int len;
};

struct animtableentry {
	/*0x00*/ uint16_t numframes;
	/*0x02*/ uint16_t bytesperframe; // in bytes
	/*0x04*/ uint32_t data;
	/*0x08*/ uint16_t headerlen;
	/*0x0a*/ uint8_t framelen;
	/*0x0b*/ uint8_t flags;
};

struct modelrenderdata {
	/*0x00*/ Mtx *unk00;
	/*0x04*/ bool zbufferenabled;
	/*0x08*/ uint32_t flags;
	/*0x0c*/ Gfx *gdl;
	/*0x10*/ Mtx *unk10;
	/*0x14*/ uint32_t unk14;
	/*0x18*/ uint32_t unk18;
	/*0x1c*/ uint32_t unk1c;
	/*0x20*/ uint32_t unk20;
	/*0x24*/ uint32_t unk24;
	/*0x28*/ uint32_t unk28;
	/*0x2c*/ uint32_t unk2c;
	/*0x30*/ int unk30;
	/*0x34*/ uint32_t envcolour;
	/*0x38*/ uint32_t fogcolour;
	/*0x3c*/ uint32_t cullmode;
};

struct rend_vidat {
	uint8_t mode;
	uint8_t unk01;
	uint8_t unk02;
	uint8_t unk03;
	int16_t x;
	int16_t y;
	float fovy;
	float aspect;
	float znear;
	float zfar;
	int16_t bufx;
	int16_t bufy;
	int16_t viewx;
	int16_t viewy;
	int16_t viewleft;
	int16_t viewtop;
	int32_t viewxreal;
	int32_t viewyreal;
	int32_t viewleftreal;
	int32_t viewtopreal;
	bool usezbuf;
	uint16_t *fb;
};

struct shieldhit {
	/*0x00*/ struct prop *prop;
	/*0x04*/ struct modelnode *node;
	/*0x08*/ struct model *model;
	/*0x0c*/ int lvframe60;
	/*0x10*/ int8_t side;
	/*0x11*/ int8_t unk011;
	/*0x12*/ int16_t unk012;
	/*0x14*/ int16_t unk014;
	/*0x14*/ int16_t unk016;
	/*0x18*/ int8_t unk018[32];
	/*0x38*/ uint8_t unk038[32];
	/*0x58*/ float shield;
};

struct bgroom {
	uintptr_t ptr_gfxdata;
	struct coord pos;
	uint8_t br_light_min;
	uint8_t br_light_max;
};

struct damagetype {
	float flashstartframe;
	float flashfullframe;
	float flashendframe;
	float maxalpha;
	int red;
	int green;
	int blue;
};

struct healthdamagetype {
	int openendframe;
	int updatestartframe;
	int updateendframe;
	int closestartframe;
	int closeendframe;
};

struct optiongroup {
	int offset;
	uint16_t name;
};

struct musicevent {
	int tracktype;
	union {
		int tracknum;
		int timer240;
	};
	bool keepafterfade;
	float unk0c;
	int16_t volume;
	uint16_t eventtype;
	int16_t id;
	uint16_t failcount;
	uint16_t numattempts;
};

struct casing {
	/*0x00*/ float ground;
	/*0x04*/ struct coord pos;
	/*0x10*/ struct coord speed;
	/*0x1c*/ int16_t rot[3][3];
	/*0x28*/ int16_t rotspeed[3][3];
	/*0x40*/ struct modeldef *modeldef;
};

struct mplockinfo {
	int8_t lockedplayernum;
	int8_t lastwinner;
	int8_t lastloser;
	int8_t unk03;
	int unk04;
};

struct boltbeam {
	union {
		intptr_t unk00;
		struct prop *unk00_prop;
	};

	uint8_t unk04;
	uint8_t tickmode;
	uint32_t unk08;
	uint32_t unk0c;
	uint32_t unk10;
	struct coord headpos;
	struct coord tailpos;
	float speed;
};

struct drawslotpointer {
	uint16_t updatedframe;
	int16_t slotnum;
};

struct seqchannel {
	int tracktype;
	int inuse;
	int keepafterfade;
	int unk0c;
};

struct seqinstance {
	/*0x000*/ ALCSeq seq;
	/*0x0f8*/ N_ALCSPlayer *seqp;
	/*0x0fc*/ uint8_t *data;
	/*0x100*/ uint16_t volume;
	/*0x104*/ int tracknum;
};

struct lasersight {
	int id;
	struct coord beamnear;
	struct coord beamfar;
	float unk28;
	struct coord dotpos;
	struct coord dotrot;
	uint32_t unk44;
};

struct vec3s16 {
	union {
		struct {
			int16_t x;
			int16_t y;
			int16_t z;
		};
		int16_t s[3];
	};
};

struct light {
	/*0x00*/ uint16_t roomnum;
	/*0x02*/ uint16_t colour; // 4/4/4/4
	/*0x04*/ uint8_t brightness;
	/*0x05*/ uint8_t sparkable : 1;
	/*0x05*/ uint8_t healthy : 1;
	/*0x05*/ uint8_t on : 1;
	/*0x05*/ uint8_t sparking : 1;
	/*0x05*/ uint8_t vulnerable : 1;
	/*0x06*/ uint8_t brightnessmult;
	/*0x07*/ int8_t dirx;
	/*0x08*/ int8_t diry;
	/*0x09*/ int8_t dirz;
	/*0x0a*/ struct vec3s16 bbox[4];
};

struct lightvisdata {
	uint8_t *portalvis_compressed;
	uint8_t *roomvis_compressed;
};

struct menurendercontext {
	int16_t x;
	int16_t y;
	int16_t width;
	int16_t height;
	struct menuitem *item;
	bool focused;
	struct menudialog *dialog;
	union menuitemdata *data;
	bool unk18;
};

struct menucolourpalette {
	/*0x00*/ uint32_t dialog_border1;
	/*0x04*/ uint32_t dialog_titlebg;
	/*0x08*/ uint32_t dialog_border2;
	/*0x0c*/ uint32_t dialog_titlefg;
	/*0x10*/ uint32_t dialog_bodybg;
	/*0x14*/ uint32_t unused14;
	/*0x18*/ uint32_t item_unfocused;
	/*0x1c*/ uint32_t item_disabled;
	/*0x20*/ uint32_t item_focused_inner;
	/*0x24*/ uint32_t checkbox_checked_unfocused;
	/*0x28*/ uint32_t item_focused_outer;
	/*0x2c*/ uint32_t listgroup_headerbg;
	/*0x30*/ uint32_t listgroup_headerfg;
	/*0x34*/ uint32_t unused34;
	/*0x38*/ uint32_t unused38;
};

struct bytelist {
	uint8_t b0;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
	uint8_t b4;
	uint8_t b5;
	uint8_t b6;
	uint8_t b7;
	uint8_t b8;
	uint8_t b9;
	uint8_t b10;
	uint8_t b11;
	uint8_t b12;
};

struct stageallocation {
	int stagenum;
	char *string;
};

struct guncmd {
	uint8_t type;
	uint8_t unk01;
	uint16_t unk02;
	intptr_t unk04;
};

struct pakthing {
	char unk00[4]; // len unknown
	uint32_t unk04;
	uint32_t unk08;
	uint32_t unk0c;
	uint16_t unk10;
};

struct pakfileheader {
	uint16_t headersum[2];       // checksum from filetype to end of header
	uint16_t bodysum[2];
	uint32_t filetype : 9;       // PAKFILETYPE constant
	uint32_t bodylen : 11;       // not aligned
	uint32_t filelen : 12;       // aligned to 0x10
	uint32_t deviceserial : 13;
	uint32_t fileid : 7;
	uint32_t generation : 9;     // increments by 1 each time the same file is saved
	uint32_t occupied : 1;
	uint32_t writecompleted : 1; // 0 while writing data, then updated to 1 afterwards
	uint32_t version : 1;        // 0, but can be set to 1 using -forceversion argument
};

struct animsmovement {
	int16_t animnum;
	float value;
};

struct shard {
	RoomNum room;
	int age60;
	struct coord pos;
	struct coord rot;
	struct coord vel;
	struct coord rotspeed;
	VtxF vertices[3];
	Col colours[3];
	uint8_t type;
};

struct pschannel {
	/*0x00*/ struct sndstate *audiohandle;
	/*0x04*/ int16_t targetvol;
	/*0x06*/ int16_t currentvol;
	/*0x08*/ int16_t currentpan;
	/*0x0a*/ int16_t targetpan;
	/*0x0c*/ int16_t targetfx;
	/*0x0e*/ int16_t currentfx;
	/*0x10*/ int16_t vol10;
	/*0x12*/ int16_t vol12;
	/*0x14*/ int16_t degrees;
	/*0x16*/ int16_t unk16;
	/*0x18*/ int16_t volchangespeed;
	/*0x1a*/ uint8_t fxbus;
	/*0x1c*/ int volchangetimer60;
	/*0x20*/ int pitchchangespeed;
	/*0x24*/ int16_t padnum;
	/*0x26*/ int16_t soundnum26;
	/*0x28*/ int16_t type;
	/*0x2a*/ uint16_t unk2a; // unused
	/*0x2c*/ int16_t soundnum2c;
	/*0x2e*/ int16_t channelnum;
	/*0x30*/ uint16_t flags;
	/*0x32*/ uint16_t flags2;
	/*0x34*/ float dist1; // full volume within dist1
	/*0x38*/ float dist2; // dist1 to dist2 -> taper volume using curve
	/*0x3c*/ float dist3; // dist2 to dist3 -> scale volume linearly to zero
	/*0x40*/ float unk40; // unused
	/*0x44*/ float targetpitch;
	/*0x48*/ float currentpitch;
	/*0x4c*/ float distance;
	/*0x50*/ struct prop *prop;
	/*0x54*/ struct coord *posptr;
	/*0x58*/ struct coord pos;
	/*0x64*/ RoomNum rooms[8];
	/*0x74*/ uint32_t uuid;
};

struct var8007e3d0_data {
	void *unk00;
	struct modelnode *node;
	int level;
	int16_t count;
	int16_t unk0e;
};

struct vtxstoretype {
	int valifsp;
	int numifsp;
	int valifmp;
	int numifmp;
	int valifspecial;
	int numifspecial;
	int unk18;
	int unk1c;
	int unk20;
	struct var8007e3d0_data *unk24;
	int val1;
	int val2;
	int numallocated;
};

struct wallhit {
	/*0x00*/ Vtx vertices[4];
	/*0x30*/ Col basecolours[4];  // without room lighting applied
	/*0x40*/ Col finalcolours[4]; // with room lighting applied
	/*0x50*/ struct coord relpos; // position relative to room or prop's pos
	/*0x5c*/ struct prop *chrprop;
	/*0x60*/ struct prop *objprop;
	/*0x64*/ Vtx *vertices2; // overridden vertices for when blood is expanding
	/*0x68*/ RoomNum roomnum;
	/*0x6a*/ uint8_t texturenum;
	/*0x6b*/ uint8_t unk6b;
	/*0x6c*/ uint8_t mtxindex;
	/*0x6d*/ uint8_t timermax;
	/*0x6e*/ uint8_t timercur;
	/*0x6f*/ uint8_t inuse : 1;
	/*0x6f*/ uint8_t unk6f_01 : 1;
	/*0x6f*/ uint8_t fading : 1;
	/*0x6f*/ uint8_t expanding : 1;
	/*0x6f*/ uint8_t xlu : 1;
	/*0x6f*/ uint8_t unk6f_05 : 1;
	/*0x70*/ uint32_t createdframe : 28;
	/*0x70*/ uint32_t timerspeed : 4;
	/*0x74*/ struct wallhit *globalnext; // for the used/free linked lists
	/*0x78*/ struct wallhit *localnext; // for the room/prop specific linked list
};

/**
 * This struct is used as a pretend linked list.
 *
 * The final item in the propnums array is not a propnum; it's the index of the
 * next chunk in the chunks array.
 *
 * The first item in the propnums array is -2 if this chunk is unallocated.
 */
struct roomproplistchunk {
	int16_t propnums[MAX_PROPSPERROOMCHUNK + 1];
};

struct nbomb {
	struct coord pos;
	int age240;
	float radius;
	int16_t rotAmount;
	struct prop *ownerprop;
	struct sndstate *audiohandle20;
	struct sndstate *audiohandle24;
	int spawnframe240; // spawned on this frame
};

struct roomacousticdata {
	float surfacearea;
	float unk04;
	float unk08;
	float roomvolume;
};

struct var8009dd78 {
	int16_t unk00;
	float unk04;
};

struct texturepair {
	texnum_t texturenum1;
	texnum_t texturenum2;
};

struct collision {
	struct geo *geo;
	bool intile;
	int vertexindex;
	struct prop *prop;
	int room;
};

struct escastepkeyframe {
	int frame;
	struct coord pos;
};

struct fontchar {
	uint8_t index;
	int8_t baseline;
	uint8_t height;
	uint8_t width;
	int kerningindex;
	uint8_t *pixeldata;
};

struct font {
	int kerning[13 * 13];
	struct fontchar chars[94];
};

typedef struct AudioInfo_s {
	short         *data;          /* Output data pointer */
	short         frameSamples;   /* # of samples synthesized in this frame */
	OSScTask      task;           /* scheduler structure */
} AudioInfo;

typedef struct {
	Acmd          *ACMDList[2];
	AudioInfo     *audioInfo[3];
	//OSThread      thread;
	//OSMesgQueue   audioFrameMsgQ;
	//OSMesg        audioFrameMsgBuf[8];
	//OSMesgQueue   audioReplyMsgQ;
	//OSMesg        audioReplyMsgBuf[8];
	N_ALGlobals   g;
} AMAudioMgr;

union audioparam {
	int s32;
	float f32;
};

struct animationdefinition {
	uint16_t numkeyframes;
	uint16_t numkeyframebytes;
	void *data;
	uint16_t unk08;
	uint8_t unk0a;
	uint8_t flags;
};

struct portalvertices {
	uint8_t count;
	struct coord vertices[1];
};

struct aibotweaponpreference {
	uint8_t unk00;
	uint8_t unk01;
	uint8_t unk02;
	uint8_t unk03;
	uint16_t haspriammogoal : 1;
	uint16_t hassecammogoal : 1;
	uint16_t pridistconfig : 4;
	uint16_t secdistconfig : 4;
	uint16_t targetammopri;
	uint16_t targetammosec;
	uint16_t criticalammopri;
	uint16_t criticalammosec;
	uint16_t reloaddelay : 3; // in seconds
	uint16_t allowpartialreloaddelay : 1;
};

struct handweaponinfo {
	int weaponnum;
	struct weapon *definition;
	struct gunctrl *gunctrl;
};

struct seqtableentry {
	uint32_t romaddr;
	uint16_t binlen;
	uint16_t ziplen;
};

struct seqtable {
	uint16_t count;
	struct seqtableentry entries[1];
};

struct mp3thing {
	uint16_t unk00[580];
};

struct mp3vars {
#ifdef PLATFORM_N64
	/*0x00*/ int romaddr;
#else
	/*0x00*/ uintptr_t romaddr;
#endif
	/*0x04*/ struct asistream *var8009c394;
	/*0x08*/ uint8_t *var8009c398;
	/*0x0c*/ int16_t var8009c39c;
	/*0x0e*/ int16_t var8009c39e;
	/*0x10*/ int16_t ivol1;
	/*0x12*/ int16_t ivol2;
	/*0x14*/ int16_t var8009c3a4;
	/*0x16*/ int16_t var8009c3a6;
	/*0x18*/ uint16_t ratel1;
	/*0x1a*/ int16_t ratem1;
	/*0x1c*/ int16_t var8009c3ac;
	/*0x1e*/ uint16_t ratel2;
	/*0x20*/ int16_t ratem2;
	/*0x22*/ int16_t var8009c3b2;
	/*0x24*/ int16_t var8009c3b4;
	/*0x28*/ int samples;
	/*0x2c*/ int var8009c3bc;
	/*0x30*/ int filesize;
	/*0x34*/ int var8009c3c4;
	/*0x38*/ struct mp3thing *var8009c3c8;
	/*0x3c*/ int var8009c3cc;
	/*0x40*/ int var8009c3d0;
	/*0x44*/ uint32_t *var8009c3d4[1];
	/*0x48*/ uint32_t var8009c3d8;
	/*0x4c*/ void *var8009c3dc;
	/*0x50*/ uint32_t var8009c3e0;
	/*0x54*/ uint32_t var8009c3e4;
	/*0x58*/ uint32_t var8009c3e8;
	/*0x5c*/ int16_t var8009c3ec;
	/*0x5e*/ int16_t var8009c3ee;
	/*0x60*/ uint8_t var8009c3f0;
	/*0x61*/ uint8_t var8009c3f1;
#ifndef PLATFORM_N64
	/*0x62*/ uint8_t reset;
#endif
};

struct rdptask {
	OSScTask sctask;
	uint16_t *framebuffer;
	uint32_t unk5c;
};

struct warpparams {
	/*0x00*/ uint32_t unk00;
	/*0x04*/ struct coord pos;
	/*0x10*/ float look[2];
	/*0x18*/ int pad;
};

struct hitthing {
	struct coord pos; // world pos
	struct coord unk0c;
	Vtx *point1;
	Vtx *point2;
	Vtx *point3;
	Gfx *tricmd;
	int16_t unk28;
	int16_t texturenum;
	int16_t unk2c;
};

struct hit {
	/*0x00*/ float distance;
	/*0x04*/ struct prop *prop;
	/*0x08*/ int hitpart;
	/*0x0c*/ struct modelnode *bboxnode;
	/*0x10*/ struct hitthing hitthing;
	/*0x40*/ int mtxindex;
	/*0x44*/ struct modelnode *dlnode;
	/*0x48*/ struct model *model;
	/*0x4c*/ int8_t slowsbullet;
	/*0x4d*/ int8_t bulletproof;
	/*0x50*/ struct coord pos;
	/*0x5c*/ struct coord dir;
};

struct shotdata {
	struct coord gunpos2d;
	struct coord gundir2d;
	struct gset gset;
	struct coord gunpos3d;
	struct coord gundir3d;
	float distance;
	int penetration;
	struct hit hits[10];
};

struct hatposition {
	float x;
	float y;
	float z;
	float unk0c;
	float unk10;
	float unk14;
};

struct var80062960 {
	/*0x000*/ struct prop *prop;
	/*0x004*/ bool unk004;
	/*0x008*/ float unk008;
	/*0x00c*/ bool unk00c;
	/*0x010*/ struct modelrodata_bbox bbox;
	/*0x02c*/ Mtx unk02c;
	/*0x06c*/ Mtx unk06c;
	/*0x0ac*/ Mtx unk0ac;
	/*0x0ec*/ Mtx unk0ec;
	/*0x12c*/ float unk12c;
	/*0x130*/ uint32_t unk130;
	/*0x134*/ float unk134[2];
	/*0x13c*/ float unk13c[2];
	/*0x144*/ float unk144[2];
	/*0x14c*/ float unk14c;
	/*0x150*/ float unk150;
	/*0x154*/ float unk154;
	/*0x158*/ float unk158;
};

struct awardmetrics {
	/*0x00*/ int numshots;
	/*0x04*/ int numheadshots;
	/*0x08*/ int numkills;
	/*0x0c*/ int numdeaths;
	/*0x10*/ int numsuicides;
	/*0x14*/ float ksratio; // kills/shots
	/*0x18*/ float kdratio; // kills/deaths
	/*0x1c*/ int backshotcount;
	/*0x20*/ int drawplayercount;
	/*0x24*/ float avgkmperhour; // average kilometres
	/*0x28*/ float armourcount;
	/*0x2c*/ uint32_t awards;
	/*0x30*/ int longestlife;
	/*0x34*/ int shortestlife;
	/*0x38*/ float accuracyfrac;
};

struct tex {
	/*0x00*/ uint16_t texturenum : 12;
	/*0x04*/ uint8_t *data;
	/*0x08*/ uint8_t width;
	/*0x09*/ uint8_t height;
	/*0x0a*/ uint8_t texTlutTmemOffset; // Offset into TMEM for TLUT (palette) data
	/*0x0b*/ uint8_t numlods : 3;
	/*0x0b*/ uint8_t gbiformat : 3;
	/*0x0b*/ uint8_t depth : 2;
	/*0x0c*/ uint32_t lutmodeindex : 2;
	/*0x0c*/ uint32_t hasloddata : 1;
	/*0x0c*/ uintptr_t next;
};

struct texcacheitem {
	int16_t texturenum;
	uint8_t widths[7];
	uint8_t heights[7];
};

struct skyvtx3d {
	float x;
	float y;
	float z;
	float s;
	float t;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct skyvtx2d {
	float unk00;
	float unk04;
	float unk08;
	float unk0c;
	float r;
	float g;
	float b;
	float a;
	float s;
	float t;
	float x;
	float y;
	float unk30;
	float unk34;
};

struct hovtype {
	float bobymid;
	float bobyminradius;
	float bobyrandradius;
	float bobyaccel;
	float bobymaxspeed;
	float bobpitchminangle;
	float bobpitchrandangle;
	float bobpitchaccel;
	float bobpitchmaxspeed;
	float bobrollminangle;
	float bobrollrandangle;
	float bobrollaccel;
	float bobrollmaxspeed;
};

struct modelrwdatabinding {
	struct model *model;
	void *rwdata;
};

struct portalthing2 {
	struct coord coord;
	bool behind;
};

struct var800a6538 {
	int vtxbatchindex;
	float unk04;
};

struct xraydata {
	int unk000;
	int unk004;
	int unk008;
	float unk00c;
	float unk010;
	float unk014;
	float unk018;
	float unk01c;
	int maxEdgeLength;
	int maxEdgeLengthSq;
	int16_t vertices[16][3];
	uint32_t colours[16];
	int16_t tris[64][3];
	int16_t numvertices;
	int16_t numtris;
};

struct widthxz {
	float width;
	float x;
	float z;
};

struct xz {
	float x;
	float z;
};

struct extplayerconfig {
	float fovy;
	float fovzoommult;
	int fovzoom;
	int mouseaimmode;
	float mouseaimspeedx;
	float mouseaimspeedy;
	int crouchmode;
	float radialmenuspeed;
	float crosshairsway;
	int extcontrols;
	uint32_t crosshaircolour;
	uint32_t crosshairsize;
	int crosshairhealth;
	int usereloads;
};

#pragma pack(1)

// Used for importing .bmp files to replace texture files in the ROM
typedef struct {
    uint16_t type;      // File type ("BM" for BMP)
    uint32_t size;      // File size in bytes
    uint16_t reserved1; // Unused (must be 0)
    uint16_t reserved2; // Unused (must be 0)
    uint32_t offset;    // Offset where pixel data starts
} BMPFileHeader;

typedef struct {
    uint32_t size;          // Header size (40 bytes)
    int32_t width;          // Image width
    int32_t height;         // Image height
    uint16_t planes;        // Must be 1
    uint16_t bitCount;      // Bits per pixel (24 for RGB)
    uint32_t compression;   // Compression type (0 = none)
    uint32_t imageSize;     // Image data size (can be 0 if uncompressed)
    int32_t xPixelsPerMeter; // Horizontal resolution
    int32_t yPixelsPerMeter; // Vertical resolution
    uint32_t colorsUsed;     // Number of colors in the palette (0 = all)
    uint32_t colorsImportant;// Important colors (0 = all)
} BMPInfoHeader;

//#define bool int

#pragma pack()
