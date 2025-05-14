#include <ultra64.h>
#include <stdint.h>
#include "constants.h"
#include "bss.h"
#include "lib/vi.h"
#include "data.h"
#include "gfx.h"
#include "types.h"

Gfx *titleClear(Gfx *gdl)
{
	gfx_Set_Cycle_Type(gdl++, G_CYC_FILL);
	gfx_Set_Color_Image(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, viGetWidth(), (uintptr_t)((void *)viGetBackBuffer()));
	RGBA fillColor = {0, 0, 0, 1};
	gfx_Set_Fill_Color(gdl++, fillColor);
	gdl += gfx_Fill_Rectangle(gdl, 0, 0, viGetWidth() - 1, viGetHeight() - 1);

	return gdl;
}
