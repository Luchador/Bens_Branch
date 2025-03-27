#ifndef IN_GAME_DEBUG_H
#define IN_GAME_DEBUG_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

u32 dprint();
s32 debug_log(const char *message, s32 num);
s32 debug_log_float(const char *message, f32 num);
s32 debug_erase();

#endif
