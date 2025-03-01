#include <ultra64.h>
#include "constants.h"
#include "game/game_097a50.h"
#include "game/game_097aa0.h"
#include "bss.h"
#include "data.h"
#include "types.h"

void ScaleVector2D(f32 *a, f32 *b, f32 *dst)
{
	dst[0] = b[0] * a[0];
	dst[1] = b[1] * a[1];
}

void Lerp2D(f32 *a, f32 *b, f32 scale, f32 *dst)
{
	dst[0] = (b[0] - a[0]) * scale + a[0];
	dst[1] = (b[1] - a[1]) * scale + a[1];
}
