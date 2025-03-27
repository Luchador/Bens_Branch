#ifndef IN_GAME_PROP_H
#define IN_GAME_PROP_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

extern float g_AutoAimScale;

void propsTick(void);

void propsStop(void);

void propsSort(void);
void propEnable(struct prop *prop);
void propDisable(struct prop *prop);
struct prop *propAllocate(void);
void propFree(struct prop *prop);
void propActivate(struct prop *prop);
void propActivateThisFrame(struct prop *prop);
void propDelist(struct prop *prop);
void propReparent(struct prop *mover, struct prop *adopter);
void propDetach(struct prop *prop);
Gfx *propRender(Gfx *gdl, struct prop *prop, bool xlupass);
Gfx *propsRender(Gfx *gdl, RoomNum renderroomnum, int renderpass, RoomNum *roomnumsbyprop);
void weaponPlayWhooshSound(int weaponnum, struct prop *prop);
void weaponPlayMeleeHitSound(int weaponnum, struct prop *prop);
struct prop *shotCalculateHits(int handnum, bool isshooting, struct coord *gunpos2d, struct coord *gundir2d, struct coord *gunpos3d, struct coord *gundir3d, u32 arg6, float distance, bool arg8);
struct prop *propFindAimingAt(int handnum, bool isshooting, u32 context);
void shotCreate(int handnum, bool arg1, bool dorandom, int numshots, bool arg4);
void hitCreate(struct shotdata *shotdata, struct prop *prop, float hitdistance, int hitpart, struct modelnode *bboxnode, struct hitthing *hitthing, int arg6, struct modelnode *dlnode, struct model *model, bool slowsbullet, bool bulletproof, struct coord *arg11, struct coord *arg12);
void handInflictMeleeDamage(int handnum, struct gset *gset, bool arg2);
void handTickAttack(int handnum);
void handsTickAttack(void);
void propExecuteTickOperation(struct prop *prop, int op);
struct prop *propFindForInteract(bool eyespy);
void propFindForUplink(void);
bool currentPlayerInteract(bool eyespy);
void propPause(struct prop *prop);
void propUnpause(struct prop *prop);
void propsTickPlayer(bool islastplayer);
void propsTickPadEffects(void);
void propSetPerimEnabled(struct prop *prop, bool enable);
void propsTestForPickup(void);
float func0f06438c(struct prop *prop, struct coord *arg1, float *arg2, float *arg3, float *arg4, bool throughobjects, bool cangangsta, int arg7);
void farsightChooseTarget(void);
void autoaimTick(void);
u32 propDoorGetCdTypes(struct prop *prop);
bool propIsOfCdType(struct prop *prop, u32 types);
void roomsCopy(RoomNum *srcrooms, RoomNum *dstrooms);
void roomsAppend(RoomNum *newrooms, RoomNum *dstrooms, int maxlen);
bool arrayIntersects(RoomNum *a, RoomNum *b);
bool propTryAddToChunk(int16_t propnum, int chunkindex);
int roomAllocatePropListChunk(int room, int arg1);
void propRegisterRoom(struct prop *prop, RoomNum room);
void propDeregisterRoom(struct prop *prop, RoomNum room);
void propDeregisterRooms(struct prop *prop);
void propRegisterRooms(struct prop *prop);
void func0f065d1c(struct coord *pos, RoomNum *rooms, struct coord *newpos, RoomNum *newrooms, RoomNum *morerooms, u32 arg5);
void func0f065dd8(struct coord *pos, RoomNum *rooms, struct coord *newpos, RoomNum *newrooms);
void func0f065dfc(struct coord *pos, RoomNum *rooms, struct coord *newpos, RoomNum *newrooms, RoomNum *morerooms, u32 arg5);
void func0f065e74(struct coord *pos, RoomNum *rooms, struct coord *newpos, RoomNum *newrooms);
void func0f065e98(struct coord *pos, RoomNum *rooms, struct coord *pos2, RoomNum *rooms2);
void roomGetProps(RoomNum *room, int16_t *propnums, int len);
void propsDefragRoomProps(void);
void propGetBbox(struct prop *prop, float *radius, float *ymax, float *ymin);
bool propUpdateGeometry(struct prop *prop, uint8_t **start, uint8_t **end);

#ifndef PLATFORM_N64
bool shotTestLos(struct coord *gunpos2d, struct coord *gundir2d, struct coord *gunpos3d, struct coord *gundir3d, struct coord *endpos3d);
#endif

#endif
