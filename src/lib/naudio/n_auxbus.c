#include <ultra64.h>
#include <stdint.h>
#include "n_synthInternals.h"

Acmd *n_alAuxBusPull(int sampleOffset, Acmd *cmdptr, int fxBus, int *numpulls)
{
	Acmd *cmd = cmdptr;
	struct N_ALAuxBus_s *bus = &n_syn->auxBus[fxBus];
	N_PVoice **sources = bus->sources;
	uint32_t i;
	int sp34 = 0;
	uint32_t sp30 = 1;
	uint32_t sp2c;

	aClearBuffer(cmd++, 1984, 736);

	*numpulls = 0;

	for (i = 0; i < bus->sourceCount; i++) {
		if (sources[i]->vvoice && sources[i]->vvoice->fxBus == fxBus && sources[i]->vvoice->pvoice && sources[i]->vvoice->pvoice->unk8c >= 64) {
			cmd = n_alEnvmixerPull(sources[i], sampleOffset, cmd);
			*numpulls = *numpulls + 1;
			sp34++;

			if (sources[i]->vvoice->pvoice->unk8c > sp30) {
				sp30 = sources[i]->vvoice->pvoice->unk8c;
			}
		}
	}

	if (sp34) {
		sp30 -= 62;

		if (sp30 >= 7) {
			sp2c = 26755.0f / sqrtf(sp30);
		} else {
			sp2c = 65536.0f / sp30;
		}

		n_aNoop(cmd++, 1248, sp2c, sp30);
		n_aNoop(cmd++, 1616, sp2c, sp30);
	}

	for (i = 0; i < bus->sourceCount; i++) {
		if ((sources[i]->vvoice && sources[i]->vvoice->fxBus == fxBus && (sources[i]->vvoice->pvoice == NULL || sources[i]->vvoice->pvoice->unk8c < 64))
				|| (sources[i]->vvoice == 0 && !fxBus)) {
			Acmd *prev = cmd;
			cmd = n_alEnvmixerPull(sources[i], sampleOffset, cmd);

			if (cmd != prev) {
				*numpulls = *numpulls + 1;
			}
		}
	}

	return cmd;
}
