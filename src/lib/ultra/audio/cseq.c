#include "synthInternals.h"

/**
 * Note: If there are no valid tracks (ie. all tracks have
 * reached the end of their data stream), then return FALSE
 * to indicate that there is no next event.
 */
char __alCSeqNextDelta(ALCSeq *seq, int *pDeltaTicks)
{
	uint32_t i;
	uint32_t	firstTime = 0xffffffff;
	uint32_t lastTicks = seq->lastDeltaTicks;

	if (!seq->validTracks) {
		return 0;
	}

	for (i = 0; i < 16; i++) {
		if ((seq->validTracks >> i) & 1) {
			if (seq->deltaFlag) {
				seq->evtDeltaTicks[i] -= lastTicks;
			}

			if (seq->evtDeltaTicks[i] < firstTime) {
				firstTime = seq->evtDeltaTicks[i];
			}
		}
	}

	seq->deltaFlag = 0;
	*pDeltaTicks = firstTime;

	return 1;
}
