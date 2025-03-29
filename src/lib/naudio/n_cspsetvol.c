#include <libaudio.h>
#include <stdint.h>
#include "n_libaudio.h"

void n_alCSPSetVol(N_ALCSPlayer *seqp, int16_t vol)
{
	N_ALEvent evt;

	evt.type = AL_SEQP_VOL_EVT;
	evt.msg.spvol.vol = vol;

	n_alEvtqPostEvent(&seqp->evtq, &evt, 0, 0);
}
