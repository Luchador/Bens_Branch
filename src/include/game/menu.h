#pragma once

#include "data.h"
#include "types.h"

extern const struct menucolourpalette g_MenuColours[];
extern const struct menucolourpalette g_MenuWave1Colours[];
extern const struct menucolourpalette g_MenuWave2Colours[];

void menuTick(void);

void menuStop(void);

void menuPlaySound(int menusound);
bool menuIsSoloMissionOrMp(void);
bool currentPlayerIsMenuOpenInSoloOrMp(void);
bool func0f0f0c68(void);
void menuSetBanner(int bannernum, bool allplayers);
Gfx *menuRenderBanner(Gfx *gdl, int x1, int y1, int x2, int y2, bool big, int msgnum, int arg7, int arg8);
struct menudfc *func0f0f1338(struct menuitem *item);
void func0f0f139c(struct menuitem *item, float arg1);
void func0f0f13ec(struct menuitem *item);
void func0f0f1418(void);
void func0f0f1494(void);
char *menuResolveText(uintptr_t thing, void *dialogoritem);
char *menuResolveParam2Text(struct menuitem *item);
char *menuResolveDialogTitle(struct menudialogdef *dialogdef);
void menuGetItemBlocksRequired(struct menuitem *item, int *arg1);
void menuCalculateItemSize(struct menuitem *item, int16_t *width, int16_t *height, struct menudialog *dialog);
void func0f0f1d6c(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu);
void dialog0f0f1ef4(struct menudialog *dialog);
void dialogCalculateContentSize(struct menudialogdef *dialogdef, struct menudialog *dialog, struct menu *menu);
int dialogFindItem(struct menudialog *dialog, struct menuitem *item, int *rowindex, int *colindex);
bool menuIsScrollableUnscrollable(struct menuitem *item);
bool menuIsItemDisabled(struct menuitem *item, struct menudialog *dialog);
bool menuIsItemFocusable(struct menuitem *item, struct menudialog *dialog, int arg2);
struct menuitem *dialogFindItemAtColY(int targety, int colindex, struct menudialogdef *dialogdef, int *rowindexptr, struct menudialog *dialog);
struct menuitem *dialogFindFirstItem(struct menudialog *dialog);
struct menuitem *dialogFindFirstItemRight(struct menudialog *dialog);
void dialogChangeItemFocusVertically(struct menudialog *dialog, int updown);
int dialogChangeItemFocusHorizontally(struct menudialog *dialog, int leftright);
int dialogChangeItemFocus(struct menudialog *dialog, struct menuinputs *inputs);
void menuOpenDialog(struct menudialogdef *dialogdef, struct menudialog *arg1, struct menu *menu);
void menuPushDialog(struct menudialogdef *dialogdef);
bool menuTrySavePlayerData(int index);
void menuCloseDialog(void);
void menuUpdateCurFrame(void);
void menuPopDialog(void);
void func0f0f3704(struct menudialogdef *dialogdef);
void menuConfigureModel(struct menumodel *menumodel, float x, float y, float z, float rotx, float roty, float rotz, float scale, uint8_t flags, float frac);
void menuUnsetModel(struct menumodel *menumodel);
Gfx *menuRenderModel(Gfx *gdl, struct menumodel *menumodel, int modeltype);
void menuGetTeamTitlebarColours(uint32_t *top, uint32_t *middle, uint32_t *bottom);
Gfx *menuApplyScissor(Gfx *gdl);
Gfx *dialogRender(Gfx *gdl, struct menudialog *dialog, struct menu *menu);
void menuGetContPads(int8_t *contpadnum1, int8_t *contpadnum2);
void func0f0f7594(int arg0, int *vdir, int *hdir);
void menuFindAvailableSize(int *xmin, int *ymin, int *xmax, int *ymax);
void dialogCalculatePosition(struct menudialog *dialog);
void menuClose(void);
void menuFinalizePlayerDataAndPopDialogs(void);
void menuResetAllDialogsAndSetNewRoot(struct menudialogdef *dialogdef, int root);
void menuSetBackground(int bg);
void func0f0f8300(void);
void menuPushRootDialog(struct menudialogdef *dialogdef, int arg1);
void func0f0f85e0(struct menudialogdef *dialogdef, int root);
Gfx *menuRenderDialog(Gfx *gdl, struct menudialog *dialog, struct menu *menu);
Gfx *menuRenderDialogs(Gfx *gdl);
void menuResetModel(struct menumodel *menumodel, uint32_t allocationlen, bool allocate);
void menuReset(void);
void menuSwipe(int direction);
void dialogTick(struct menudialog *dialog, struct menuinputs *inputs, uint32_t tickflags);
void dialogInitItems(struct menudialog *dialog);
void func0f0fa6ac(void);
void menuProcessInput(void);
Gfx *menuRenderBackgroundLayer1(Gfx *gdl, uint8_t bg, float frac);
Gfx *menuRenderBackgroundLayer2(Gfx *gdl, uint8_t bg, float frac);
Gfx *menuRender(Gfx *gdl);
uint32_t menuChooseMusic(void);
uint32_t menuGetRoot(void);
void menuPushPakDialogForPlayer(struct menudialogdef *dialogdef, int playernum, int arg2);
char *menuTextSaveDeviceName(struct menuitem *item);
int menuPakNumToPlayerNum(int paknum);
bool menuIsReadyForPakError(int device, int arg1);
void menuPushPakErrorDialog(int arg0, int arg1);
void func0f0fd494(struct coord *pos);
void func0f0fd548(int arg0);
struct menudialog *menuIsDialogOpen(struct menudialogdef *dialogdef);
struct chrdata *currentPlayerGetCommandingAibot(void);

MenuItemHandlerResult menuhandler000fcc34(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult amPickTargetMenuList(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerRepairPak(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerRetrySavePak(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerWarnRepairPak(int operation, struct menuitem *item, union handlerdata *data);