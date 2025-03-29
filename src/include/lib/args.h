#ifndef _IN_LIB_ARGS_H
#define _IN_LIB_ARGS_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

char *argParseString(char *str);
void argSetString(char *string);
int argsParseDebugArgs(void);
char *argFindByPrefix(int occurrence, char *string);
void argGetLevel(int *stagenum);

#endif
