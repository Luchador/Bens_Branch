#pragma once

#include "data.h"
#include "types.h"

char *frMenuTextFailReason(struct menuitem *item);
char *frMenuTextDifficultyName(struct menuitem *item);
char *frMenuTextTimeTakenValue(struct menuitem *item);
char *frMenuTextScoreValue(struct menuitem *item);
char *frMenuTextWeaponName(struct menuitem *item);
char *frMenuTextTargetsDestroyedValue(struct menuitem *item);
char *frMenuTextAccuracyValue(struct menuitem *item);
char *frMenuTextGoalScoreLabel(struct menuitem *item);
char *frMenuTextGoalScoreValue(struct menuitem *item);
char *frMenuTextMinAccuracyOrTargetsLabel(struct menuitem *item);
char *frMenuTextMinAccuracyOrTargetsValue(struct menuitem *item);
char *frMenuTextTimeLimitLabel(struct menuitem *item);
char *frMenuTextTimeLimitValue(struct menuitem *item);
char *frMenuTextAmmoLimitLabel(struct menuitem *item);
char *frMenuTextAmmoLimitValue(struct menuitem *item);
char *ciMenuTextChrBioName(struct menuitem *item);
char *ciMenuTextChrBioAge(struct menuitem *item);
char *ciMenuTextChrBioRace(struct menuitem *item);
char *ciMenuTextMiscBioName(struct menuitem *item);
char *dtMenuTextName(struct menuitem *item);
char *dtMenuTextOkOrResume(struct menuitem *item);
char *dtMenuTextCancelOrAbort(struct menuitem *item);
char *dtMenuTextTimeTakenValue(struct menuitem *item);
char *htMenuTextName(struct menuitem *item);
char *htMenuTextOkOrResume(struct menuitem *item);
char *htMenuTextCancelOrAbort(struct menuitem *item);
char *htMenuTextTimeTakenValue(struct menuitem *item);
char *bioMenuTextName(struct menuitem *item);
char *ciMenuTextHangarBioSubheading(struct menuitem *item);
struct menudialogdef *ciGetFrWeaponListMenuDialog(void);
MenuDialogHandlerResult frTrainingInfoMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult frTrainingStatsMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult ciCharacterProfileMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult dtTrainingDetailsMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialogDeviceTrainingResults(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialog001a6aa4(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult menudialogFiringRangeResults(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult ciHangarHolographMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult frDetailsOkMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult frAbortMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult frWeaponListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult frScoringMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerFrFailedContinue(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult ciOfficeInformationMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult dtDeviceListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandlerDtOkOrResume(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler001a6514(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult htHoloListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler001a6a34(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult menuhandler001a6a70(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult ciHangarInformationMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult ciHangarTitleMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult frDifficultyMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
