#ifndef IN_GAME_ACTIVEMENU_H
#define IN_GAME_ACTIVEMENU_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void amTick(void);

void amOpenPickTarget(void);
MenuDialogHandlerResult menudialog000fcd48(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult amPickTargetMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
void amSetAiBuddyTemperament(bool aggressive);
void amSetAiBuddyStealth(void);
int amGetFirstBuddyIndex(void);
void amApply(int slot);
void amGetSlotDetails(int slot, int32_t *flags, char *label);
void amReset(void);
int16_t amCalculateSlotWidth(void);
void amChangeScreen(int step);
void amAssignWeaponSlots(void);
void amOpen(void);
void amClose(void);
bool amIsCramped(void);
void amCalculateSlotPosition(int16_t column, int16_t row, int16_t *x, int16_t *y);
Gfx *amRenderText(Gfx *gdl, char *text, int32_t colour, int16_t left, int16_t top);
Gfx *amRenderAibotInfo(Gfx *gdl, int buddynum);
Gfx *amRenderSlot(Gfx *gdl, char *text, int16_t x, int16_t y, int mode, int flags);
Gfx *amRender(Gfx *gdl);

#endif
