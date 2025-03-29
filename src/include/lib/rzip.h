#ifndef _IN_LIB_RZIP_H
#define _IN_LIB_RZIP_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

int rzipInflate(void *src, void *dst, void *scratch);
uint32_t rzipInit(void);
bool rzipIs1173(void *buffer);
void *rzipGetSomething(void);

#endif
