#include <libaudio.h>
#include <stdint.h>
#include "n_libaudio.h"
#include "types.h"

uint32_t __n_alCSeqGetTrackEvent(ALCSeq *seq, uint32_t track, N_ALEvent *event, int arg3);
uint8_t __getTrackByte(ALCSeq *seq, uint32_t track);
uint32_t __readVarLen(ALCSeq *seq,uint32_t track);

void n_alCSeqNew(ALCSeq *seq, uint8_t *ptr)
{
	uint32_t i, tmpOff, flagTmp;

	/* load the seqence pointed to by ptr   */
	seq->base = (ALCMidiHdr*)ptr;
	seq->validTracks = 0;
	seq->lastDeltaTicks = 0;
	seq->lastTicks = 0;
	seq->deltaFlag = 1;

	for (i = 0; i < 16; i++) {
		seq->lastStatus[i] = 0;
		seq->curBUPtr[i] = 0;
		seq->curBULen[i] = 0;
		tmpOff = seq->base->trackOffset[i];

		if (tmpOff) {
			flagTmp = 1 << i;
			seq->validTracks |= flagTmp;
			seq->curLoc[i] = (uint8_t*)((uintptr_t)ptr + tmpOff);
			seq->evtDeltaTicks[i] = __readVarLen(seq,i);
		} else {
			seq->curLoc[i] = 0;
		}
	}

	seq->qnpt = 1.0f / (float)seq->base->division;
}

void n_alCSeqNextEvent(ALCSeq *seq, N_ALEvent *evt, int arg2)
{
	uint32_t i;
	uint32_t firstTime = 0xffffffff;
	uint32_t firstTrack;
	uint32_t lastTicks = seq->lastDeltaTicks;

	for (i = 0; i < 16; i++) {
		if ((seq->validTracks >> i) & 1) {
			if (seq->deltaFlag) {
				seq->evtDeltaTicks[i] -= lastTicks;
			}

			if (seq->evtDeltaTicks[i] < firstTime) {
				firstTime = seq->evtDeltaTicks[i];
				firstTrack = i;
			}
		}
	}

	__n_alCSeqGetTrackEvent(seq, firstTrack, evt, arg2);

	evt->msg.midi.ticks = firstTime;
	seq->lastTicks += firstTime;
	seq->lastDeltaTicks = firstTime;

	if (evt->type != AL_TRACK_END) {
		seq->evtDeltaTicks[firstTrack] += __readVarLen(seq, firstTrack);
	}

	seq->deltaFlag = 1;
}

uint32_t __n_alCSeqGetTrackEvent(ALCSeq *seq, uint32_t track, N_ALEvent *event, int arg3)
{
	uint32_t offset;
	uint8_t status, loopCt, curLpCt, *tmpPtr;

	status = __getTrackByte(seq, track);

	if (status == AL_MIDI_Meta) {
		uint8_t type = __getTrackByte(seq, track);

		if (type == AL_MIDI_META_TEMPO) {
			event->type = AL_TEMPO_EVT;
			event->msg.tempo.status = status;
			event->msg.tempo.type = type;
			event->msg.tempo.byte1 = __getTrackByte(seq, track);
			event->msg.tempo.byte2 = __getTrackByte(seq, track);
			event->msg.tempo.byte3 = __getTrackByte(seq, track);
			seq->lastStatus[track] = 0;  /* lastStatus not supported after meta */
		} else if (type == AL_MIDI_META_EOT) {
			uint32_t flagMask;

			flagMask = 1 << track;
			seq->validTracks = seq->validTracks ^ flagMask;

			if (seq->validTracks) { /* there is music left don't end */
				event->type = AL_TRACK_END;
			} else {       /* no more music send AL_SEQ_END_EVT msg */
				event->type = AL_SEQ_END_EVT;
			}
		} else if (type == AL_CMIDI_LOOPSTART_CODE) {
			status = __getTrackByte(seq, track);
			event->msg.loop.count = status << 8;

			status = __getTrackByte(seq, track);
			event->msg.loop.count += status;

			seq->lastStatus[track] = 0;
			event->type = AL_CSP_LOOPSTART;
		} else if (type == AL_CMIDI_LOOPEND_CODE) {
			tmpPtr = seq->curLoc[track];
			loopCt = *tmpPtr++;
			curLpCt = *tmpPtr;

			if (curLpCt == 0 || !arg3) {
				*tmpPtr = loopCt; /* reset current loop count */
				seq->curLoc[track] = tmpPtr + 5; /* move pointer to end of event */
			} else {
				if (curLpCt != 0xff) { /* not a loop forever */
					*tmpPtr = curLpCt - 1;   /* decrement current loop count */
				}

				tmpPtr++;                    /* get offset from end of event */
				offset = (*tmpPtr++) << 24;
				offset += (*tmpPtr++) << 16;
				offset += (*tmpPtr++) << 8;
				offset += *tmpPtr++;
				seq->curLoc[track] = tmpPtr - offset;
			}

			seq->lastStatus[track] = 0;
			event->type = AL_CSP_LOOPEND;
		}
	} else {
		event->type = AL_SEQ_MIDI_EVT;

		if (status & 0x80) {
			event->msg.midi.status = (status & 0xf0) | track;
			event->msg.midi.byte1 = __getTrackByte(seq,track);
			seq->lastStatus[track] = event->msg.midi.status;
		} else {    /* running status */
			event->msg.midi.status = seq->lastStatus[track];
			event->msg.midi.byte1 = status;
		}

		if ((event->msg.midi.status & 0xf0) != AL_MIDI_ProgramChange
				&& (event->msg.midi.status & 0xf0) != AL_MIDI_ChannelPressure) {
			event->msg.midi.byte2 = __getTrackByte(seq,track);

			if ((event->msg.midi.status & 0xf0) == AL_MIDI_NoteOn) {
				event->msg.midi.duration = __readVarLen(seq,track);
			}
		} else {
			event->msg.midi.byte2 = 0;
		}
	}

	return 1;
}

int alCSeqGetTicks(ALCSeq *seq)
{
	return seq->lastTicks;
}

void alCSeqSetLoc(ALCSeq *seq, ALCSeqMarker *m)
{
	int i;

	seq->validTracks    = m->validTracks;
	seq->lastTicks      = m->lastTicks;
	seq->lastDeltaTicks = m->lastDeltaTicks;

	for (i = 0; i < 16; i++) {
		seq->curLoc[i]        = m->curLoc[i];
		seq->curBUPtr[i]      = m->curBUPtr[i];
		seq->curBULen[i]      = m->curBULen[i];
		seq->lastStatus[i]    = m->lastStatus[i];
		seq->evtDeltaTicks[i] = m->evtDeltaTicks[i];
	}
}

void alCSeqGetLoc(ALCSeq *seq, ALCSeqMarker *m)
{
	int i;

	m->validTracks    = seq->validTracks;
	m->lastTicks      = seq->lastTicks;
	m->lastDeltaTicks = seq->lastDeltaTicks;

	for (i = 0; i < 16; i++) {
		m->curLoc[i]        = seq->curLoc[i];
		m->curBUPtr[i]      = seq->curBUPtr[i];
		m->curBULen[i]      = seq->curBULen[i];
		m->lastStatus[i]    = seq->lastStatus[i];
		m->evtDeltaTicks[i] = seq->evtDeltaTicks[i];
	}
}

void n_alCSeqNewMarker(ALCSeq *seq, ALCSeqMarker *m, uint32_t ticks)
{
	N_ALEvent evt;
	ALCSeq tempSeq;
	int i;

	n_alCSeqNew(&tempSeq, (uint8_t*)seq->base);

	do {
		m->validTracks    = tempSeq.validTracks;
		m->lastTicks      = tempSeq.lastTicks;
		m->lastDeltaTicks = tempSeq.lastDeltaTicks;

		for (i = 0; i < 16; i++) {
			m->curLoc[i]        = tempSeq.curLoc[i];
			m->curBUPtr[i]      = tempSeq.curBUPtr[i];
			m->curBULen[i]      = tempSeq.curBULen[i];
			m->lastStatus[i]    = tempSeq.lastStatus[i];
			m->evtDeltaTicks[i] = tempSeq.evtDeltaTicks[i];
		}

		n_alCSeqNextEvent(&tempSeq, &evt, 0);

		if (evt.type == AL_SEQ_END_EVT) {
			break;
		}
	} while (tempSeq.lastTicks < ticks);
}

void func00039718(ALCSeq *seq, ALCSeqMarker *m, uint32_t ticks, uint32_t arg3)
{
	N_ALEvent evt;
	ALCSeq tempSeq;
	int i;
	int j;
	ALCSeqMarker m2;

	n_alCSeqNew(&tempSeq, (uint8_t*)seq->base);

	for (j = 0; j < ticks; j++) {
		m[j].lastTicks = 0;
	}

	do {
		m2.validTracks    = tempSeq.validTracks;
		m2.lastTicks      = tempSeq.lastTicks;
		m2.lastDeltaTicks = tempSeq.lastDeltaTicks;

		for (i = 0; i < 16; i++) {
			m2.curLoc[i]        = tempSeq.curLoc[i];
			m2.curBUPtr[i]      = tempSeq.curBUPtr[i];
			m2.curBULen[i]      = tempSeq.curBULen[i];
			m2.lastStatus[i]    = tempSeq.lastStatus[i];
			m2.evtDeltaTicks[i] = tempSeq.evtDeltaTicks[i];
		}

		n_alCSeqNextEvent(&tempSeq, &evt, 0);

		if (evt.type == AL_CSP_LOOPSTART) {
			if ((evt.msg.loop.count >> 8) >= arg3 && (evt.msg.loop.count >> 8) < arg3 + ticks) {
				if (m[(evt.msg.loop.count >> 8) - arg3].lastTicks == 0) {
					m[(evt.msg.loop.count >> 8) - arg3] = m2;

					if (--j <= 0) {
						return;
					}
				}
			}
		}
	} while (evt.type != AL_SEQ_END_EVT);
}

uint8_t __getTrackByte(ALCSeq *seq, uint32_t track)
{
	uint8_t theByte;

	if (seq->curBULen[track]) {
		theByte = *seq->curBUPtr[track];
		seq->curBUPtr[track]++;
		seq->curBULen[track]--;
	} else /* need to handle backup mode */ {
		theByte = *seq->curLoc[track];
		seq->curLoc[track]++;

		if (theByte == AL_CMIDI_BLOCK_CODE) {
			uint8_t loBackUp, hiBackUp, theLen, nextByte;
			uint32_t backup;

			nextByte = *seq->curLoc[track];
			seq->curLoc[track]++;

			if (nextByte != AL_CMIDI_BLOCK_CODE) {
				/**
				 * if here, then got a backup section. get the amount of
				 * backup, and the len of the section. Subtract the amount of
				 * backup from the curLoc ptr, and subtract four more, since
				 * curLoc has been advanced by four while reading the codes.
				 */
				hiBackUp = nextByte;
				loBackUp = *seq->curLoc[track];
				seq->curLoc[track]++;
				theLen = *seq->curLoc[track];
				seq->curLoc[track]++;
				backup = (uint32_t)hiBackUp;
				backup = backup << 8;
				backup += loBackUp;
				seq->curBUPtr[track] = seq->curLoc[track] - (backup + 4);
				seq->curBULen[track] = (uint32_t)theLen;

				/* now get the byte */
				theByte = *seq->curBUPtr[track];
				seq->curBUPtr[track]++;
				seq->curBULen[track]--;
			}
		}
	}

	return theByte;
}

uint32_t __readVarLen(ALCSeq *seq, uint32_t track)
{
	uint32_t value;
	uint32_t c;

	value = (uint32_t)__getTrackByte(seq, track);

	if (value & 0x80) {
		value &= 0x7f;

		do {
			c = (uint32_t)__getTrackByte(seq, track);
			value = (value << 7) + (c & 0x7f);
		} while (c & 0x80);
	}

	return value;
}
