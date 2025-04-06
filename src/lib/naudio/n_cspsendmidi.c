#include <libaudio.h>
#include "n_libaudio.h"

void n_alCSPSendMidi(N_ALCSPlayer *seqp, int ticks, uint8_t status, uint8_t byte1, uint8_t byte2)
{
	N_ALEvent evt;
	ALMicroTime deltaTime;

	evt.type = AL_SEQP_MIDI_EVT;
	evt.msg.midi.ticks = 0;
	evt.msg.midi.status = status;
	evt.msg.midi.byte1 = byte1;
	evt.msg.midi.byte2 = byte2;
	evt.msg.midi.duration = 0;

	deltaTime = ticks;

	n_alEvtqPostEvent(&seqp->evtq, &evt, deltaTime, 0);
}
