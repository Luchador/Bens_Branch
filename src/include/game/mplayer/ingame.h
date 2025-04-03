#ifndef _IN_GAME_MPLAYER_INGAME_H
#define _IN_GAME_MPLAYER_INGAME_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

MenuItemHandlerResult mpStatsForPlayerDropdownHandler(int operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextInGameLimit(struct menuitem *item);
char *menutextPauseOrUnpause(int arg0);
char *menutextMatchTime(int arg0);
char *mpMenuTextWeaponDescription(struct menuitem *item);
char *mpMenuTitleStatsFor(struct menudialogdef *dialogdef);
char *mpMenuTextWeaponOfChoiceName(struct menuitem *item);
char *mpMenuTextAward1(struct menuitem *item);
char *mpMenuTextAward2(struct menuitem *item);
char *mpMenuTextPlacementWithSuffix(struct menuitem *item);
MenuItemHandlerResult mpPlacementMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult mpAwardsMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult mpPlayerTitleMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
char *mpMenuTextPlayerTitle(int arg0);
MenuItemHandlerResult mpConfirmPlayerNameHandler(int operation, struct menuitem *item, union handlerdata *data);
void mpPushPauseDialog(void);
void mpPushEndscreenDialog(u32 arg0, u32 playernum);
MenuItemHandlerResult menuhandlerMpEndGame(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler00178018(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMpInGameLimitLabel(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerMpPause(int operation, struct menuitem *item, union handlerdata *data);

#endif
