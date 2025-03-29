#include <libaudio.h>
#include "types.h"

void _bnkfPatchBank(ALBank *bank, uintptr_t offset, uintptr_t table);
void _bnkfPatchInst(ALInstrument* inst, uintptr_t offset, uintptr_t table);
void _bnkfPatchSound(ALSound* s, uintptr_t offset, uintptr_t table);
void _bnkfPatchWaveTable(ALWaveTable* w, uintptr_t offset, uintptr_t table);

void alSeqFileNew(ALSeqFile *file, uint8_t *base)
{
	uintptr_t offset = (uintptr_t) base;
	int i;

	/*
	 * patch the file so that offsets are pointers
	 */
	for (i = 0; i < file->seqCount; i++) {
		file->seqArray[i].offset = (uint8_t *)((uint8_t *)file->seqArray[i].offset + offset);
	}
}

void alBnkfNew(ALBankFile *file, uint8_t *table)
{
	uintptr_t offset = (uintptr_t) file;
	uintptr_t woffset = (uintptr_t) table;

	int i;

	/*
	 * check the file format revision in debug libraries
	 */
	ALFailIf(file->revision != AL_BANK_VERSION, ERR_ALBNKFNEW);

	/*
	 * patch the file so that offsets are pointers
	 */
	for (i = 0; i < file->bankCount; i++) {
		file->bankArray[i] = (ALBank *)((uint8_t *)file->bankArray[i] + offset);

		if (file->bankArray[i]) {
			_bnkfPatchBank(file->bankArray[i], offset, woffset);
		}
	}
}

void _bnkfPatchBank(ALBank *bank, uintptr_t offset, uintptr_t table)
{
	int i;

	if (bank->flags) {
		return;
	}

	bank->flags = 1;

	if (bank->percussion) {
		bank->percussion = (ALInstrument *)((uint8_t *)bank->percussion + offset);
		_bnkfPatchInst(bank->percussion, offset, table);
	}

	for (i = 0; i < bank->instCount; i++) {
		bank->instArray[i] = (ALInstrument *)((uint8_t *)bank->instArray[i] + offset);

		if (bank->instArray[i]) {
			_bnkfPatchInst(bank->instArray[i], offset, table);
		}
	}
}

void _bnkfPatchInst(ALInstrument *inst, uintptr_t offset, uintptr_t table)
{
	int i;

	if (inst->flags) {
		return;
	}

	inst->flags = 1;

	for (i = 0; i < inst->soundCount; i++) {
		inst->soundArray[i] = (ALSound *)((uint8_t *)inst->soundArray[i] + offset);
		_bnkfPatchSound(inst->soundArray[i], offset, table);
	}
}

void _bnkfPatchSound(ALSound *s, uintptr_t offset, uintptr_t table)
{
	if (s->flags) {
		return;
	}

	s->flags = 1;

	s->envelope = (ALEnvelope *)((uint8_t *)s->envelope + offset);
	s->keyMap = (ALKeyMap *)((uint8_t *)s->keyMap + offset);

	s->wavetable = (ALWaveTable *)((uint8_t *)s->wavetable + offset);
	_bnkfPatchWaveTable(s->wavetable, offset, table);
}

void _bnkfPatchWaveTable(ALWaveTable *w, uintptr_t offset, uintptr_t table)
{
	if (w->flags) {
		return;
	}

	w->flags = 1;

	w->base += table;

	if (w->type == AL_ADPCM_WAVE) {
		w->waveInfo.adpcmWave.book  = (ALADPCMBook *)((uint8_t *)w->waveInfo.adpcmWave.book + offset);

		if (w->waveInfo.adpcmWave.loop) {
			w->waveInfo.adpcmWave.loop = (ALADPCMloop *)((uint8_t *)w->waveInfo.adpcmWave.loop + offset);
		}
	} else if (w->type == AL_RAW16_WAVE) {
		if (w->waveInfo.rawWave.loop) {
			w->waveInfo.rawWave.loop = (ALRawLoop *)((uint8_t *)w->waveInfo.rawWave.loop + offset);
		}
	}
}
