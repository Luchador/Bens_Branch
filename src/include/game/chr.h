#ifndef _IN_GAME_CHR_CHR_H
#define _IN_GAME_CHR_CHR_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

void chrSetChrnum(struct chrdata *chr, s16 chrnum);
void chrDeregister(s32 chrnum);
void chrCalculatePushPos(struct chrdata *chr, struct coord *pos, RoomNum *rooms, bool arg3);

bool chr0f01f264(struct chrdata *chr, struct coord *pos, RoomNum *rooms, float arg3, bool arg4);
bool chr0f01f378(struct model *model, struct coord *arg1, struct coord *arg2, f32 *mangroundptr);
int chrsGetNumFree(void);
s16 chrsGetNextUnusedChrnum(void);
struct prop *chr0f020b14(struct prop *prop, struct model *model, struct coord *pos, RoomNum *rooms, f32 arg3, u8 *ailist);
void chrRemove(struct prop *prop, bool free);
void chrUpdateAimProperties(struct chrdata *chr);
void chrFlinchBody(struct chrdata *chr);
void chrFlinchHead(struct chrdata *chr, f32 arg1);
f32 chrGetFlinchAmount(struct chrdata *chr);
void chrFindEnteredRooms(struct chrdata *chr, struct coord *pos, RoomNum *rooms);
void chrAdvanceAnims(struct chrdata *chr, s32 lvupdate240, bool arg2);
void chr0f022214(struct chrdata *chr, struct prop *child, bool fulltick);
void chrUpdateCloak(struct chrdata *chr);
s32 chrGetCloakAlpha(struct chrdata *chr);
void chrSetPoisoned(struct chrdata *chr, struct prop *poisonprop);
void chrTickPoisoned(struct chrdata *chr);
bool chrTickBeams(struct prop *prop);
s32 chrTick(struct prop *prop);
void chrDropConcealedItems(struct chrdata *chr);
void chrSetHudpieceVisible(struct chrdata *chr, bool visible);
void chrDropItemsForOwnerReap(struct chrdata *chr);
void chrSetBloodColour(u8 *arg0);
bool chr0f024738(struct chrdata *chr);
bool chr0f024b18(struct model *model, struct modelnode *node);
void chrRenderAttachedObject(struct prop *prop, struct modelrenderdata *renderdata, bool xlupass, struct chrdata *chr);
void chrGetBloodColour(s16 bodynum, u8 *colour1, u32 *colour2);
Gfx *chrRender(struct prop *prop, Gfx *gdl, bool xlupass);
void chrEmitSparks(struct chrdata *chr, struct prop *prop, s32 hitpart, struct coord *coord, struct coord *coord2, struct chrdata *chr2);
void chr0f0260c4(struct model *model, s32 hitpart, struct modelnode *node, struct coord *arg3);
void chrBruise(struct model *model, s32 hitpart, struct modelnode *node, struct coord *arg3);
void chrDisfigure(struct chrdata *chr, struct coord *exppos, f32 damageradius);
f32 chrGetHitRadius(struct chrdata *chr);
void chrTestHit(struct prop *prop, struct shotdata *shotdata, bool isshooting, bool cheap);
void chrHit(struct shotdata *shotdata, struct hit *hit);
void chrsCheckForNoise(f32 noiseradius);
bool chrCalculateAutoAim(struct prop *prop, struct coord *arg1, f32 *arg2, f32 *arg3);
s32 chr0f028e18(struct prop *arg0, struct modelnode *node, struct model *model, struct prop *arg3);
bool chr0f028e6c(s32 arg0, struct prop *prop, struct prop **propptr, struct modelnode **nodeptr, struct model **modelptr);
void shieldhitCreate(struct prop *prop, f32 shield, struct prop *arg2, struct modelnode *node, struct model *model, s32 side, s16 *arg6);
void shieldhitRemove(struct shieldhit *shieldhit);
void shieldhitsRemoveByProp(struct prop *prop);
int chr0f02932c(struct prop *prop, int arg1);
int chr0f0293ec(struct prop *prop, int arg1);
int chr0f0294cc(struct prop *prop, int arg1);
void chrCalcShieldColor(float arg0, int *arg1, int *arg2, int *arg3);
float propGetShieldThing(struct prop **propptr);
Gfx *chrRenderShieldComponent(Gfx *gdl, struct shieldhit *hit, struct prop *prop, struct model *model, struct modelnode *node, s32 side, s32 arg6, s32 arg7, s32 alpha);
Gfx *shieldhitRender(Gfx *gdl, struct prop *prop1, struct prop *prop2, s32 alpha, bool arg4, s32 cmnum1, s32 cmnum2, s32 cmnum3, s32 cmnum4);
Gfx *chrRenderCloak(Gfx *gdl, struct prop *chr1, struct prop *chr2);
Gfx *chrRenderShield(Gfx *gdl, struct chrdata *chr, unsigned int alpha);
void shieldhitsTick(void);
void chrSetDrCarollImages(struct chrdata *drcaroll, int imageleft, int imageright);
int chrsGetNumSlots(void);
void chrRegister(int chrnum, int chrindex);
Vtx *chrAllocateVertices(int numvertices);
void chrSetPerimEnabled(struct chrdata *chr, bool enable);
void chrSetMaxDamage(struct chrdata *chr, float maxdamage);
float chrGetMaxDamage(struct chrdata *chr);
void chrAddHealth(struct chrdata *chr, float health);
float chrGetArmor(struct chrdata *chr);
void chrInit(struct prop *prop, u8 *ailist);
struct prop *chrAllocate(struct model *model, struct coord *pos, RoomNum *rooms, f32 faceangle, u8 *ailist);
void chrClearReferences(int propnum);
void chrFindEnteredRoomsWithRoomNum(struct chrdata *chr, RoomNum *room);
void chrUpdateRooms(struct chrdata *chr);
void chrCloak(struct chrdata *chr, bool arg1);
void chrUncloak(struct chrdata *chr, bool value);
void chrUncloakTemporarily(struct chrdata *chr);
void chr0f02472c(void);
struct chrdata *chrFindByLiteralId(s32 chrnum);
struct prop *chrGetHeldProp(struct chrdata *chr, s32 hand);
struct prop *chrGetHeldUsableProp(struct chrdata *chr, s32 hand);
struct prop *chrGetTargetProp(struct chrdata *chr);
bool chrUpdateGeometry(struct prop *prop, u8 **start, u8 **end);
void chrGetBbox(struct prop *prop, f32 *radius, f32 *ymax, f32 *ymin);

#endif
