#ifndef IN_GAME_DEBUG_H
#define IN_GAME_DEBUG_H
#include "data.h"
#include "types.h"

uint32_t dprint();
int debug_log(const char *message, int num);
int debug_log_float(const char *message, float num);
int debug_erase();

#endif
