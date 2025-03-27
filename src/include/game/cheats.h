#ifndef IN_GAME_CHEATS_H
#define IN_GAME_CHEATS_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_CheatsMenuDialog;
extern struct menudialogdef g_CheatsFunMenuDialog;
extern struct menudialogdef g_CheatsGameplayMenuDialog;
extern struct menudialogdef g_CheatsSoloWeaponsMenuDialog;
extern struct menudialogdef g_CheatsClassicWeaponsMenuDialog;
extern struct menudialogdef g_CheatsWeaponsMenuDialog;
extern struct menudialogdef g_CheatsBuddiesMenuDialog;

uint32_t cheatIsUnlocked(int cheat_id);
bool cheatIsActive(int cheat_id);
void cheatActivate(int cheat_id);
void cheatDeactivate(int cheat_id);
void cheatsInit(void);
void cheatsReset(void);
char *cheatGetNameIfUnlocked(struct menuitem *item);
char *cheatGetMarquee(struct menuitem *item);
int cheatGetByTimedStageIndex(int stage_index, int difficulty);
int cheatGetByCompletedStageIndex(int stage_index);
int cheatGetTime(int cheat_id);
char *cheatGetName(int cheat_id);
MenuDialogHandlerResult cheatMenuHandleDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult cheatCheckboxMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult cheatMenuHandleBuddyCheckbox(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult cheatMenuHandleTurnOffAllCheats(int operation, struct menuitem *item, union handlerdata *data);

#endif
