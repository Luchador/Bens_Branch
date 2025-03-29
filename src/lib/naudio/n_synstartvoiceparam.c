#include "n_synthInternals.h"

void n_alSynStartVoiceParams(N_ALVoice *v, ALWaveTable *w, float pitch, int16_t vol,
		ALPan pan, uint8_t fxmix, uint8_t arg6, float arg7, uint8_t arg8, ALMicroTime t)
{
	ALStartParamAlt *update;

	if (v->pvoice) {
		/*
		 * get new update struct from the free list
		 */
		update = (ALStartParamAlt *)__n_allocParam();
		ALFailIf(update == 0, ERR_ALSYN_NO_UPDATE);

		/*
		 * set offset and fxmix data
		 */
		update->delta   = n_syn->paramSamples + v->pvoice->offset;
		update->next    = 0;
		update->type    = AL_FILTER_START_VOICE_ALT;
		update->unity   = v->unityPitch;
		update->pan     = pan;
		update->volume  = vol;
		update->fxMix   = fxmix;
		update->pitch   = pitch;
		update->unk14   = arg8;
		update->unk15   = arg6;
		update->unk18   = arg7;
		update->samples = _n_timeToSamples(t);
		update->wave    = w;

		n_alEnvmixerParam(v->pvoice, AL_FILTER_ADD_UPDATE, update);
	}
}
