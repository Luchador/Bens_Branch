#ifndef _IN_LIB_AUDIOMGR_H
#define _IN_LIB_AUDIOMGR_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void amgrCreate(ALSynConfig *config);
void amgrStartThread(void);
void amgrFrame(void);

#endif
