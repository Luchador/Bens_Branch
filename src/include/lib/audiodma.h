#pragma once

#include "data.h"
#include "types.h"

struct admastate;

void *admaNew(struct admastate **state);
void admaBeginFrame(void);