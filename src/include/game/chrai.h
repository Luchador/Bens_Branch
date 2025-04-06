#pragma once

#include "data.h"
#include "types.h"

int chraiGetListIdByList(uint8_t *ailist, bool *is_global);
uint32_t chraiGoToLabel(uint8_t *ailist, uint32_t aioffset, uint8_t label);
void chraiExecute(void *entity, int proptype);
uint32_t chraiGetCommandLength(uint8_t *ailist, uint32_t aioffset);