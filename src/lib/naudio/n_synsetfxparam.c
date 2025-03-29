#include "n_synthInternals.h"
#include <stdint.h>

void n_alSynSetFXParam(ALFxRef fx, int16_t paramID, void *param)
{
	ALFx *f = (ALFx *)fx;

	n_alFxParamHdl(f, (int)paramID, param);
}

void func0003ba64(struct fx *fx, float outputrate);

void func0003e674(struct fx *fx, int16_t arg1, void *param)
{
	if (arg1 == 8) {
		fx->unk02 = (*(int *)param * 0.1f);
	} else if (arg1 == 9) {
		fx->unk00 = *(int *)param;
	}

	func0003ba64(fx, n_syn->outputRate);
}
