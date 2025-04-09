#pragma once

#include "data.h"
#include "types.h"

uint32_t dprint();
int debug_log(const char *message, int num);
int debug_log_float(const char *message, float num);
int debug_log_string(const char *message);
int debug_erase();
void debug_log_coord(const struct coord *pos);
void writeVertexShaderToFile(const char *vs_buf);