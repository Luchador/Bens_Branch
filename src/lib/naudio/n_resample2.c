#include "n_synthInternals.h"
#include <math.h>
#include <os.h>
#include "types.h"
#include <stdint.h>

void func0003ba64(struct fx *fx, float outputrate);

Acmd *n_alResamplePull2(N_PVoice *e, int16_t *outp, int outCount, Acmd *p)
{
	Acmd *ptr = p;
	float sp28;

	ptr = n_alResamplePull(e, outp, p);

	if (e->unk8c >= 0 && e->unk8c < 64) {
		if (e->unk8c >= 6) {
			sp28 = 26755;
		} else {
			sp28 = 65536 / (e->unk8c + 1.0f);
		}

		if (sp28 < 7723) {
			sp28 = 7723;
		}

		n_aNoop(ptr++, *outp, sp28, e->unk8c + 1);
	}

	if (e->fx.unk02 > 0) {
		if (e->unkb8 != 0) {
			func0003ba64(&e->fx, 22050);
		}

		n_aLoadADPCM(ptr++, 32, (uintptr_t)(e->fx.unk08))

		if (e->unkb8 == 2) {
			e->unkb8 = 0;
		}

		n_aPoleFilter(ptr++, e->unkb8, *outp, 0, (uintptr_t)(e->unkbc) & 0xffffff);

		e->unkb8 = 0;
	}

	return ptr;
}

int n_alResampleParam2(N_PVoice *filter, int paramID, void *param)
{
	float *f = (float *) &param;

	switch (paramID) {
	case (AL_FILTER_RESET):
		filter->fx.unk02 = 0;
		n_alLoadParam(filter, AL_FILTER_RESET, param);
		break;
	case (AL_FILTER_12):
		filter->fx.unk02 = (int) param;
		filter->unkb8 |= 2;
		break;
	case (AL_FILTER_13):
		filter->fx.unk00 = *f;
		filter->unkb8 |= 2;
		break;
	case (AL_FILTER_11):
		filter->unk8c = (uint8_t)param;
		break;
	default:
		n_alLoadParam(filter, paramID, param);
		break;
	}

	return 0;
}
