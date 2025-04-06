#pragma once

#include "data.h"
#include "types.h"

int osEepromLongWrite(uint8_t address, uint8_t *buffer, int nbytes);
