#include <ultra64.h>
#include "constants.h"
#include "game/mtxutils.h"
#include "game/smoke.h"
#include "bss.h"
#include "data.h"
#include "types.h"

void smokesInit(void)
{
	mtxScale(&var800a3448, 0.1, 0.1, 0.1);
	mtxScale(&var800a3488, 0.2, 0.2, 0.2);
}
