#pragma once

#include "data.h"
#include "types.h"

char *argParseString(char *str);
void argSetString(char *string);
bool argsParseDebugArgs(void);
char *argFindByPrefix(int occurrence, char *string);
void argGetLevel(int *stagenum);
