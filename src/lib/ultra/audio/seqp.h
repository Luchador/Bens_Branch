

#define KILL_TIME	50000	/* 50 ms */

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

ALVoiceState    *__mapVoice(ALSeqPlayer *, uint8_t, uint8_t, uint8_t);
void            __unmapVoice(ALSeqPlayer *seqp, ALVoice *voice);
char		__voiceNeedsNoteKill(ALSeqPlayer *seqp, ALVoice *voice, ALMicroTime killTime);	/* sct 1/5/96 */

ALVoiceState    *__lookupVoice(ALSeqPlayer *, uint8_t, uint8_t);
ALSound         *__lookupSound(ALSeqPlayer *, uint8_t, uint8_t, uint8_t);
ALSound         *__lookupSoundQuick(ALSeqPlayer *, uint8_t, uint8_t, uint8_t);

int16_t             __vsVol(ALVoiceState *voice, ALSeqPlayer *seqp);
ALMicroTime     __vsDelta(ALVoiceState *voice, ALMicroTime t);
ALPan           __vsPan(ALVoiceState *voice, ALSeqPlayer *seqp);

void		__initFromBank(ALSeqPlayer *seqp, ALBank *b);
void            __initChanState(ALSeqPlayer *seqp);
void            __resetPerfChanState(ALSeqPlayer *seqp, int chan);
void            __setInstChanState(ALSeqPlayer *seqp, ALInstrument *inst, int chan);

void            __seqpPrintVoices(ALSeqPlayer *);
void		__seqpReleaseVoice(ALSeqPlayer *seqp, ALVoice *voice, ALMicroTime deltaTime);

void            __seqpStopOsc(ALSeqPlayer *seqp, ALVoiceState *vs);

void		__postNextSeqEvent(ALSeqPlayer *seqp);			/* sct 11/7/95 */

