#pragma once

#include "data.h"
#include "types.h"

int rzipInflate(void *src, void *dst, void *scratch);
bool rzipIs1173(void *buffer);
void *rzipGetSomething(void);
