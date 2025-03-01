#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "data.h"
#include "types.h"

// Ben's comment: Takes a float and converts it to an unsigned integer. The line result = -result; is unncessary because the return value is an unsigned int. I've rewritten it slightly.
/*u32 FloatToUInt32(f32 arg0)
{
	u32 result;

	if (arg0 > 32767.9f) {
		arg0 = 32767.9f;
	}

	if (arg0 < -32767.9f) {
		arg0 = -32767.9f;
	}

	if (arg0 < 0) {
		result = arg0 * -65536;
		result = -result;
	} else {
		result = 65536 * arg0;
	}

	return result;
}*/

u32 FloatToUInt32(f32 arg0)
{
	if (arg0 > 32767.9f) {
		arg0 = 32767.9f;
	}

	if (arg0 < -32767.9f) {
		arg0 = -32767.9f;
	}

	return (u32)(arg0 * 65536);
}

Gfx *func0f153134(Gfx *gdl)
{
	gDPPipeSync(gdl++);
	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
	gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
	gDPSetRenderMode(gdl++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

	return gdl;
}
