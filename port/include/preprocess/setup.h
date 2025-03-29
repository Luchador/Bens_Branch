#ifndef _IN_SETUP_DECL
#define _IN_SETUP_DECL

#include "preprocess/common.h"
#include <stdint.h>

struct n64_stagesetup {
	uint32_t ptr_waypoints;
	uint32_t ptr_waygroups;
	uint32_t ptr_cover;
	uint32_t ptr_intro;
	uint32_t ptr_props;
	uint32_t ptr_paths;
	uint32_t ptr_ailists;
	uint32_t ptr_padfiledata;
};

struct n64_coord {
	float x;
	float y;
	float z;
};

struct n64_defaultobj {
	uint16_t extrascale;
	uint8_t hidden2;
	uint8_t type;
	int16_t modelnum;
	int16_t pad;
	uint32_t flags;
	uint32_t flags2;
	uint32_t flags3;
	uint32_t ptr_prop;
	uint32_t ptr_model;
	float realrot[3][3];
	uint32_t hidden;
	union {
		uint32_t ptr_geotilef;
		uint32_t ptr_geoblock;
		uint32_t ptr_geocyl;
		uint32_t ptr_unkgeo;
	};
	union {
		uint32_t ptr_projectile;
		uint32_t ptr_embedment;
	};
	int16_t damage;
	int16_t maxdamage;
	uint8_t shadecol[4];
	uint8_t nextcol[4];
	uint16_t floorcol;
	int8_t geocount;
	int8_t _pad_;
};

struct n64_doorobj {
	struct n64_defaultobj base;
	float maxfrac;
	float perimfrac;
	float accel;
	float decel;
	float maxspeed;
	uint16_t doorflags;
	uint16_t doortype;
	uint32_t keyflags;
	int autoclosetime;
	float frac;
	float fracspeed;
	int8_t mode;
	int8_t glasshits;
	int16_t fadealpha;
	int16_t xludist;
	int16_t opadist;
	struct n64_coord startpos;
	float mtx98[3][3];
	uint32_t ptr_sibling;
	int lastopen60;
	int16_t portalnum;
	int8_t soundtype;
	int8_t fadetime60;
	int lastcalc60;
	uint8_t laserfade;
	uint8_t unusedmaybe[3];
	uint8_t shadeinfo1[4];
	uint8_t shadeinfo2[4];
	uint8_t actual1;
	uint8_t actual2;
	uint8_t extra1;
	uint8_t extra2;
};

struct n64_doorscaleobj {
	uint32_t unk00;
	int scale;
};

struct n64_keyobj {
	struct n64_defaultobj base;
	uint32_t keyflags;
};

struct n64_alarmobj {
	struct n64_defaultobj base;
};

struct n64_cctvobj {
	struct n64_defaultobj base;

	int16_t lookatpadnum;
	int16_t toleft;
	//Mtxf camrotm;
	float m[4][4];
	float yzero;
	float yrot;
	float yleft;
	float yright;
	float yspeed;
	float ymaxspeed;
	int seebondtime60;
	float maxdist;
	float xzero;
};

struct n64_ammocrateobj {
	struct n64_defaultobj base;
	int ammotype;
};

struct n64_gset {
	uint8_t weaponnum;
	uint8_t unk0639;
	uint8_t unk063a;
	uint8_t weaponfunc;
};

struct n64_weaponobj {
	struct n64_defaultobj base;

	union {
		struct n64_gset gset;
		struct {
			uint8_t weaponnum;
			int8_t unk5d;
			int8_t unk5e;
			uint8_t gunfunc;
		};
	};

	int8_t fadeouttimer60;
	int8_t dualweaponnum;

	union {
		int16_t timer240;
		int16_t team;
	};

	uint32_t ptr_dualweapon;
};

struct n64_packedchr {
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

struct n64_tvscreen {
	uint32_t ptr_cmdlist;
	uint16_t offset;
	int16_t pause60;
	uint32_t ptr_tconfig;
	float rot;
	float xscale;
	float xscalefrac;
	float xscaleinc;
	float xscaleold;
	float xscalenew;
	float yscale;
	float yscalefrac;
	float yscaleinc;
	float yscaleold;
	float yscalenew;
	float xmid;
	float xmidfrac;
	float xmidinc;
	float xmidold;
	float xmidnew;
	float ymid;
	float ymidfrac;
	float ymidinc;
	float ymidold;
	float ymidnew;
	uint8_t red;
	uint8_t redold;
	uint8_t rednew;
	uint8_t green;
	uint8_t greenold;
	uint8_t greennew;
	uint8_t blue;
	uint8_t blueold;
	uint8_t bluenew;
	uint8_t alpha;
	uint8_t alphaold;
	uint8_t alphanew;
	float colfrac;
	float colinc;
};

struct n64_singlemonitorobj {
	struct n64_defaultobj base;
	struct n64_tvscreen screen;
	int16_t owneroffset;
	int8_t ownerpart;
	uint8_t imagenum;
};

struct n64_multimonitorobj {
	struct n64_defaultobj base;
	struct n64_tvscreen screens[4];
	uint8_t imagenums[4];
};

struct n64_hangingmonitorsobj {
	struct n64_defaultobj base;
};

struct n64_autogunobj {
	struct n64_defaultobj base;
	int16_t targetpad;
	int8_t firing;
	uint8_t firecount;
	float yzero;
	float ymaxleft;
	float ymaxright;
	float yrot;
	float yspeed;
	float xzero;
	float xrot;
	float xspeed;
	float maxspeed;
	float aimdist;
	float barrelspeed;
	float barrelrot;
	int lastseebond60;
	int lastaimbond60;
	int allowsoundframe;
	uint32_t ptr_beam;
	float shotbondsum;
	uint32_t ptr_target;
	uint8_t targetteam;
	uint8_t ammoquantity;
	int16_t nextchrtest;
};

struct n64_linkgunsobj {
	uint32_t unk00;
	int16_t offset1;
	int16_t offset2;
};

struct n64_debrisobj {
	struct n64_defaultobj base;
};

struct n64_hatobj {
	struct n64_defaultobj base;
};

struct n64_grenadeprobobj { // objtype 0x12
	uint32_t unk00;
	int16_t chrnum;
	uint16_t probability;
};

struct n64_linkliftdoorobj {
	uint32_t unk00;
	uint32_t ptr_door;
	uint32_t ptr_lift;
	uint32_t ptr_next;
	int stopnum;
};

struct n64_multiammocrateslot {
	uint16_t modelnum;
	uint16_t quantity;
};

struct n64_multiammocrateobj {
	struct n64_defaultobj base;
	struct n64_multiammocrateslot slots[19];
};

struct n64_shieldobj {
	struct n64_defaultobj base;
	float initialamount;
	float amount;
	uint32_t unk64;
};

struct n64_tag {
	uint32_t identifier;
	uint16_t tagnum;
	int16_t cmdoffset;
	uint32_t ptr_next;
	uint32_t ptr_obj;
};

struct n64_objective {
	uint32_t unk00;
	int index;
	uint32_t text;
	uint16_t unk0c;
	uint8_t flags;
	int8_t difficulties;
};

struct n64_stdobjective {
	uint32_t cmd;
};

struct n64_objectivecmd {
	uint8_t cmd0[4];
	uint32_t cmd1;
};

struct n64_briefingobj {
	uint32_t unk00;
	uint32_t type;
	uint32_t text;
	uint32_t ptr_next;
};

struct n64_padlockeddoorobj {
	uint32_t unk00;
	uint32_t ptr_door;
	uint32_t ptr_lock;
	uint32_t ptr_next;
};

struct n64_truckobj {
	struct n64_defaultobj base;
	uint32_t ptr_ailist;
	uint16_t aioffset;
	int16_t aireturnlist;
	float speed;
	float wheelxrot;
	float wheelyrot;
	float speedaim;
	float speedtime60;
	float turnrot60;
	float roty;
	uint32_t ptr_path;
	int nextstep;
};

struct n64_tankobj {
	uint8_t _pad_[128];
};

struct n64_heliobj {
	struct n64_defaultobj base;
	uint32_t ptr_ailist;
	uint16_t aioffset;
	int16_t aireturnlist;
	float rotoryrot;
	float rotoryspeed;
	float rotoryspeedaim;
	float rotoryspeedtime;
	float speed;
	float speedaim;
	float speedtime60;
	float yrot;
	uint32_t ptr_path;
	int nextstep;
};

struct n64_glassobj {
	struct n64_defaultobj base;
	int16_t portalnum;
	int16_t _pad_;
};

struct n64_safeobj {
	struct n64_defaultobj base;
};

struct n64_safeitemobj {
	uint32_t unk00;
	uint32_t ptr_item;
	uint32_t ptr_safe;
	uint32_t ptr_door;
	uint32_t ptr_next;
};

struct n64_cameraposobj {
	int type;
	float x;
	float y;
	float z;
	float theta;
	float verta;
	int pad;
};

struct n64_tintedglassobj {
	struct n64_defaultobj base;
	int16_t xludist;
	int16_t opadist;
	int16_t opacity;
	int16_t portalnum;
	float unk64;
};

struct n64_liftobj {
	struct n64_defaultobj base;
	int16_t pads[4];
	uint32_t ptr_doors[4];
	float dist;
	float speed;
	float accel;
	float maxspeed;
	int8_t soundtype;
	int8_t levelcur;
	int8_t levelaim;
	int8_t _pad_;
	struct n64_coord prevpos;
};

struct n64_linksceneryobj {
	uint32_t unk00;
	uint32_t ptr_trigger;
	uint32_t ptr_unexp;
	uint32_t ptr_exp;
	uint32_t ptr_next;
};

struct n64_blockedpathobj {
	uint32_t unk00;
	uint32_t ptr_blocker;
	int16_t waypoint1;
	int16_t waypoint2;
	uint32_t ptr_next;
};

struct n64_hov {
	uint8_t type;
	uint8_t flags;
	int16_t _pad_;
	float bobycur;
	float bobytarget;
	float bobyspeed;
	float yrot;
	float bobpitchcur;
	float bobpitchtarget;
	float bobpitchspeed;
	float bobrollcur;
	float bobrolltarget;
	float bobrollspeed;
	float groundpitch;
	float y;
	float ground;
	int prevframe60;
	int prevgroundframe60;
};

struct n64_hoverbikeobj {
	struct n64_defaultobj base;
	struct n64_hov hov;
	float speed[2];
	float prevpos[2];
	float w;
	float rels[2];
	float exreal;
	float ezreal;
	float ezreal2;
	float leanspeed;
	float leandiff;
	int maxspeedtime240;
	float speedabs[2];
	float speedrel[2];
};

struct n64_hoverpropobj {
	struct n64_defaultobj base;
	struct n64_hov hov;
};

struct n64_fanobj {
	struct n64_defaultobj base;
	float yrot;
	float yrotprev;
	float ymaxspeed;
	float yspeed;
	float yaccel;
	int8_t on;
	uint8_t _pad_[3];
};

struct n64_hovercarobj {
	struct n64_defaultobj base;
	uint32_t ptr_ailist;
	uint16_t aioffset;
	int16_t aireturnlist;
	float speed;
	float speedaim;
	float speedtime60;
	float turnyspeed60;
	float turnxspeed60;
	float turnrot60;
	float roty;
	float rotx;
	float rotz;
	uint32_t ptr_path;
	int nextstep;
	int16_t status;
	int16_t dead;
	int16_t deadtimer60;
	int16_t sparkstimer60;
};

struct n64_chopperobj {
	struct n64_defaultobj base;
	uint32_t ptr_ailist;
	uint16_t aioffset;
	int16_t aireturnlist;
	union {
		struct {
			float speed;
			float speedaim;
			float speedtime60;
		};
		struct n64_coord fall;
	};
	float turnyspeed60;
	float turnxspeed60;
	float turnrot60;
	float roty;
	float rotx;
	float rotz;
	uint32_t ptr_path;
	int nextstep;//
	int16_t weaponsarmed;
	int16_t ontarget;
	int16_t target;
	uint8_t attackmode;
	uint8_t cw;
	float vx;
	float vy;
	float vz;
	float power;
	float otx;
	float oty;
	float otz;
	float bob;
	float bobstrength;
	int targetvisible;
	int timer60;
	int patroltimer60;
	float gunturnyspeed60;
	float gunturnxspeed60;
	float gunroty;
	float gunrotx;
	float barrelrotspeed;
	float barrelrot;
	uint32_t ptr_fireslotthing;
	int dead;
};

struct n64_escalatorobj {
	struct n64_defaultobj base;
	int frame;
	struct n64_coord prevpos;
};

struct n64_textoverride {
	uint32_t unk00;
	int objoffset;
	int weapon;
	uint32_t obtaintext;
	uint32_t ownertext;
	uint32_t inventorytext;
	uint32_t inventory2text;
	uint32_t pickuptext;
	uint32_t ptr_next;
	uint32_t ptr_obj;
};

struct n64_criteria_roomentered {
	uint32_t unk00;
	uint32_t pad;
	uint32_t status;
	uint32_t ptr_next;
};

struct n64_criteria_throwinroom {
	uint32_t unk00;
	uint32_t unk04;
	uint32_t pad;
	uint32_t status;
	uint32_t ptr_next;
};

struct n64_criteria_holograph {
	uint32_t unk00;
	uint32_t obj;
	uint32_t status;
	uint32_t ptr_next;
};

struct n64_ailist {
	uint32_t ptr_list;
	int id;
};

struct n64_path {
	uint32_t ptr_pads;
	uint8_t id;
	uint8_t flags;
	uint16_t len;
};

#endif
