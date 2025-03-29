#ifndef IN_GAME_FILEMGR_H
#define IN_GAME_FILEMGR_H
#include <ultra64.h>
#include <stdint.h>
#include "data.h"
#include "types.h"

extern struct menudialogdef g_ChooseLanguageMenuDialog;

char *filemgrGetDeviceName(int index);
char *filemgrMenuTextDeviceName(struct menuitem *item);
void filemgrGetSelectName(char *buffer, struct filelistfile *file, uint32_t filetype);
char *filemgrMenuTextDeleteFileName(struct menuitem *item);
void filemgrSetDevice1BySerial(int deviceserial);
void filemgrSetDevice1ByFile(struct filelistfile *file);
void filemgrSetFileToDelete(struct filelistfile *file, int filetype);
char *filemgrMenuTextFailReason(struct menuitem *item);
char *filemgrMenuTextDeviceNameForError(struct menuitem *item);
void filemgrPushErrorDialog(uint16_t errnum);
uintptr_t filemgrGetDeviceNameOrStartIndex(int listnum, int operation, int optionindex);
char *filemgrMenuTextErrorTitle(struct menuitem *item);
char *filemgrMenuTextFileType(struct menuitem *item);
void func0f10898c(void);
void filemgrHandleSuccess(void);
void filemgrEraseCorruptFile(void);
char *filemgrMenuTextInsertOriginalPak(struct menuitem *item);
void filemgrRetrySave(int arg0);
bool filemgrAttemptOperation(int device, bool closeonsuccess);
bool filemgrSaveOrLoad(struct fileguid *guid, int fileop, uintptr_t playernum);
void filemgrDeleteCurrentFile(void);
void func0f1097d0(int device);
void filemgrSaveGameToDevice(int device);
void filemgrGetFileName(char *dst, struct filelistfile *file);
void filemgrGetRenameName(char *buffer);
void filemgrSetRenameName(char *name);
bool filemgrIsNameAvailable(int arg0);
void filemgrSaveToDevice(void);
char *filemgrMenuTextDeviceNameContainingDuplicateFile(struct menuitem *item);
char *filemgrMenuTextDuplicateFileName(struct menuitem *item);
char *filemgrMenuTextLocationName2(struct menuitem *item);
char *filemgrMenuTextSaveLocationSpaces(struct menuitem *item);
void filemgrPushSelectLocationDialog(int arg0, uint32_t filetype);
char *filemgrMenuTextFileInUseDescription(struct menuitem *item);
bool filemgrIsFileInUse(struct filelistfile *file);
MenuItemHandlerResult filemgrFileToCopyOrDeleteListMenuHandler(int operation, struct menuitem *item, union handlerdata *data, bool isdelete);
void filemgrPushDeleteFileDialog(int listnum);
char *pakMenuTextPagesFree(struct menuitem *item);
char *pakMenuTextPagesUsed(struct menuitem *item);
char *pakMenuTextStatusMessage(struct menuitem *item);
char *pakMenuTextEditingPakName(struct menuitem *item);
bool filemgrConsiderPushingFileSelectDialog(void);
void bootmenuReset(void);
MenuItemHandlerResult filemgrChooseAgentListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakGameNoteListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileToCopyListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileToDeleteListMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuDialogHandlerResult filemgrInsertOriginalPakMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult filemgrCopyOrDeleteListMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult pakGameNotesMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult pakChoosePakMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuDialogHandlerResult filemgrMainMenuDialog(int operation, struct menudialogdef *dialogdef, union handlerdata *data);
MenuItemHandlerResult filemgrDeviceNameMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrFileNameMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDeviceNameForErrorMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrRetrySaveMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrCancelSave2MenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrAcknowledgeFileLostMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrReinsertedOkMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrReinsertedCancelMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDuplicateRenameMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDuplicateCancelMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrCancelSaveMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrDeleteFilesForSaveMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrConfirmDeleteMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakDeleteGameNoteMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult pakSelectionMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrAgentNameKeyboardMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrOpenCopyFileMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrOpenDeleteFileMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrConfirmRenameMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrSaveElsewhereYesMenuHandler(int operation, struct menuitem *item, union handlerdata *data);
MenuItemHandlerResult filemgrSelectLocationMenuHandler(int operation, struct menuitem *item, union handlerdata *data);

#endif
