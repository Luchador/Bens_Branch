/*====================================================================
 *
 * Copyright 1993, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

#ifndef __N_LIBAUDIO__
#define __N_LIBAUDIO__

#include <PR/libaudio.h>

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include <PR/mbi.h>
#include <stdint.h>
#include "platform.h"

struct N_SpeakerType {
	uint8_t surround;
	uint8_t mono;
	uint8_t headphone;
	uint8_t unk03;
};

extern struct N_SpeakerType N_SpeakerType;
extern uint8_t var8009c344[2];
extern uint8_t var8009c346[2];
extern uint8_t var8009c348[4];

/*
 * Synthesis driver stuff
 */
typedef struct N_ALVoice_s {
    ALLink              node;
    struct N_PVoice_s     *pvoice;
    ALWaveTable         *table;
    void                *clientPrivate;
    int16_t                 state;
    int16_t                 priority;
    int16_t                 fxBus;
    int16_t                 unityPitch;
} N_ALVoice;

typedef struct {
    ALPlayer    *head;          /* client list head                     */
    ALLink      pFreeList;      /* list of free physical voices         */
    ALLink      pAllocList;     /* list of allocated physical voices    */
    ALLink      pLameList;      /* list of voices ready to be freed     */
    int         paramSamples;
    int         curSamples;     /* samples from start of game           */
    ALDMANew    dma;
    ALHeap      *heap;
    struct ALParam_s    *paramList;
    struct N_ALMainBus_s  *mainBus;
    struct N_ALAuxBus_s   *auxBus;
    int                 numPVoices;
    int                 maxAuxBusses;
    int                 outputRate;
    int                 maxOutSamples;
    uintptr_t           sv_dramout;
    int                 sv_first;
} N_ALSynth;


void    n_alSynAddPlayer(ALPlayer *client);
void    n_alSynAddSndPlayer(ALPlayer *client);
void    n_alSynAddSeqPlayer(ALPlayer *client);

ALFxRef n_alSynAllocFX( int16_t bus,ALSynConfig *c, ALHeap *hp);
int     n_alSynAllocVoice( N_ALVoice *voice, ALVoiceConfig *vc);


void    n_alSynFreeVoice(N_ALVoice *voice);
ALFxRef n_alSynGetFXRef( int16_t bus, int16_t index);
int16_t     n_alSynGetPriority( N_ALVoice *voice);
void    n_alSynRemovePlayer( ALPlayer *client);
void    n_alSynSetFXMix(N_ALVoice *v, uint8_t fxmix);
void    n_alSynSetFXParam(ALFxRef fx, int16_t paramID, void *param);
void    n_alSynFreeFX(ALFxRef *fx);
void    n_alSynSetPan(N_ALVoice *v, uint8_t pan);
void    n_alSynSetPitch( N_ALVoice *v, float pitch);
void    n_alSynSetPriority( N_ALVoice *voice, int16_t priority);
void    n_alSynSetVol( N_ALVoice *v, int16_t volume, ALMicroTime t);
void    n_alSynStartVoice(N_ALVoice *v, ALWaveTable *table);
void    n_alSynStartVoiceParams(N_ALVoice *v, ALWaveTable *w,float pitch, int16_t vol,
				ALPan pan, uint8_t fxmix, uint8_t arg6, float arg7, uint8_t arg8, ALMicroTime t);
void    n_alSynStopVoice( N_ALVoice *v);

void    n_alSynFilter11(N_ALVoice *v, uint8_t channel);
void    n_alSynFilter12(N_ALVoice *v, uint8_t arg1);
void    n_alSynFilter13(N_ALVoice *v, float arg1);

void    n_alSynNew(ALSynConfig *c);
void    n_alSynDelete(void);


/*
 * Audio Library (AL) stuff
 */
typedef struct {
    N_ALSynth     drvr;
} N_ALGlobals;

extern N_ALGlobals *n_alGlobals;
extern N_ALSynth *n_syn;

void n_alInit(N_ALGlobals *g, ALSynConfig *c);
void n_alClose(N_ALGlobals *glob);
Acmd *n_alAudioFrame(Acmd *cmdList, int *cmdLen,
			int16_t *outBuf, int outLen);


/*
 * Sequence Player stuff
 */
typedef struct {
    struct N_ALVoice_s    *voice;
} N_ALNoteEvent;


typedef struct {
    struct N_ALVoice_s    *voice;
    ALMicroTime         delta;
    uint8_t                  vol;
} N_ALVolumeEvent;


typedef struct {
    struct N_ALVoiceState_s      *vs;
    void                       *oscState;
    uint8_t                         chan;
} N_ALOscEvent;

typedef struct {
    float unk00;
    float unk04;
} N_AL18Event;

typedef struct {
    uint8_t unk00;
    uint8_t unk01;
    uint8_t unk02;
    uint8_t unk03;
    uint8_t param;
} N_AL19Event;

typedef struct {
    struct sndstate *sndstate;
    int data;
    int data2;
} N_ALGenericEvent;

typedef struct {
    int16_t                 	type;
    union {
		ALMIDIEvent     	midi;
		ALTempoEvent    	tempo;
		ALEndEvent      	end;
		N_ALNoteEvent     	note;
		N_ALVolumeEvent   	vol;
		ALSeqpLoopEvent 	loop;
		ALSeqpVolEvent  	spvol;
		ALSeqpPriorityEvent	sppriority;
		ALSeqpSeqEvent		spseq;
		ALSeqpBankEvent		spbank;
		N_ALOscEvent      	osc;
		N_AL18Event      	evt18;
		N_AL19Event      	evt19;
		N_ALGenericEvent    generic;
    } msg;
} N_ALEvent;


typedef struct {
    ALLink      node;
    ALMicroTime delta;
    N_ALEvent     evt;
} N_ALEventListItem;

void            n_alEvtqNew(ALEventQueue *evtq, N_ALEventListItem *items, int itemCount);
ALMicroTime     n_alEvtqNextEvent(ALEventQueue *evtq, N_ALEvent *evt);
void            n_alEvtqPostEvent(ALEventQueue *evtq, N_ALEvent *evt, ALMicroTime delta, int arg3);
void        	n_alEvtqFlushType(ALEventQueue *evtq, int16_t type);

struct oscstate {
	u32 unk00;
	uint8_t unk04;
	u32 unk08;
	float unk0c;
	float unk10;
};

struct fx {
	int16_t unk00;
	int16_t unk02;
	int unk04;
	int16_t unk08[16];
};

typedef struct N_ALVoiceState_s {
    struct N_ALVoiceState_s *next;/* MUST be first                */
    N_ALVoice   voice;
    ALSound    *sound;
    ALMicroTime envEndTime;     /* time of envelope segment end */
    float         pitch;          /* currect pitch ratio          */
    float         vibrato;        /* current value of the vibrato */
    uint8_t          envGain;        /* current envelope gain        */
    uint8_t          channel;        /* channel assignment           */
    uint8_t          key;            /* note on key number           */
    uint8_t          velocity;       /* note on velocity             */
    uint8_t          envPhase;       /* what envelope phase          */
    uint8_t          phase;
    uint8_t          tremelo;        /* current value of the tremelo */
    uint8_t          flags;          /* bit 0 tremelo flag
                                   bit 1 vibrato flag           */
	void *oscState;
	struct oscstate *oscState2;
} N_ALVoiceState;

typedef struct {
    ALPlayer            node;          /* note: must be first in structure */
    N_ALSynth          *drvr;          /* reference to the client driver   */
    ALSeq              *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    int                 uspt;           /* microseconds per tick            */
    int                 nextDelta;      /* microseconds to next callback    */
    int                 state;
    u16                 chanMask;       /* active channels                  */
    int16_t                 vol;            /* overall sequence volume          */
    uint8_t                  maxChannels;    /* number of MIDI channels          */
    uint8_t                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
    ALSeqMarker        *loopStart;
    ALSeqMarker        *loopEnd;
    intptr_t            loopCount;      /* -1 = loop forever, 0 = no loop   */
    uint8_t unk88;
    uint8_t unk89;
} N_ALSeqPlayer;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    N_ALSynth          *drvr;           /* reference to the client driver   */
    ALCSeq             *target;         /* current sequence                 */
    ALMicroTime         curTime;
    ALBank             *bank;           /* current ALBank                   */
    int                 uspt;           /* microseconds per tick            */
    int                 nextDelta;      /* microseconds to next callback    */
    int                 state;
    u16                 chanMask;       /* active channels                  */
    int16_t                 vol;            /* overall sequence volume          */
    uint8_t                  maxChannels;    /* number of MIDI channels          */
    uint8_t                  debugFlags;     /* control which error get reported */
    N_ALEvent           nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState        *chanState;      /* 16 channels for MIDI             */
    N_ALVoiceState     *vAllocHead;     /* list head for allocated voices   */
    N_ALVoiceState     *vAllocTail;     /* list tail for allocated voices   */
    N_ALVoiceState     *vFreeList;      /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;

#ifdef PLATFORM_64BIT
    int _pad1_;
    float unk7c;
    int _pad2_;
    float unk80;
#else
    float unk7c;
    float unk80;
#endif

    void *queue;

    uint8_t unk88;
    uint8_t unk89;
} N_ALCSPlayer;


/*
 * Sequence data representation routines
 */
void    n_alSeqNextEvent(ALSeq *seq, N_ALEvent *event);

void    n_alCSeqNew(ALCSeq *seq, uint8_t *ptr);
void    n_alCSeqNextEvent(ALCSeq *seq, N_ALEvent *evt, int arg2);
void    n_alCSeqNewMarker(ALCSeq *seq, ALCSeqMarker *m, u32 ticks);


/*
 * Sequence Player routines
 */
void    n_alSeqpNew(N_ALSeqPlayer *seqp, ALSeqpConfig *config);
void    n_alSeqpDelete(N_ALSeqPlayer *seqp);
uint8_t	n_alSeqpGetChlVol(N_ALSeqPlayer *seqp, uint8_t chan);
uint8_t      n_alSeqpGetChlFXMix(N_ALSeqPlayer *seqp, uint8_t chan);
ALPan   n_alSeqpGetChlPan(N_ALSeqPlayer *seqp, uint8_t chan);
uint8_t      n_alSeqpGetChlPriority(N_ALSeqPlayer *seqp, uint8_t chan);
int     n_alSeqpGetChlProgram(N_ALSeqPlayer *seqp, uint8_t chan);
ALSeq  *n_alSeqpGetSeq(N_ALSeqPlayer *seqp);
int	n_alSeqpGetState(N_ALSeqPlayer *seqp);
int     n_alSeqpGetTempo(N_ALSeqPlayer *seqp);
int16_t     n_alSeqpGetVol(N_ALSeqPlayer *seqp);		/* Master volume control */
void    n_alSeqpPlay(N_ALSeqPlayer *seqp);
void    n_alSeqpSendMidi(N_ALSeqPlayer *seqp, int ticks, uint8_t status, uint8_t byte1, uint8_t byte2);
void    n_alSeqpSetBank(N_ALSeqPlayer *seqp, ALBank *b);
void	n_alSeqpSetChlVol(N_ALSeqPlayer *seqp, uint8_t chan, uint8_t vol);
void    n_alSeqpSetChlFXMix(N_ALSeqPlayer *seqp, uint8_t chan, uint8_t fxmix);
void    n_alSeqpSetChlPan(N_ALSeqPlayer *seqp, uint8_t chan, ALPan pan);
void    n_alSeqpSetChlPriority(N_ALSeqPlayer *seqp, uint8_t chan, uint8_t priority);
void    n_alSeqpSetChlProgram(N_ALSeqPlayer *seqp, uint8_t chan, uint8_t prog);
void    n_alSeqpSetSeq(N_ALSeqPlayer *seqp, ALSeq *seq);
void    n_alSeqpSetTempo(N_ALSeqPlayer *seqp, int tempo);
void    n_alSeqpSetVol(N_ALSeqPlayer *seqp, int16_t vol);
void    n_alSeqpStop(N_ALSeqPlayer *seqp);
void    n_alSeqpLoop(N_ALSeqPlayer *seqp, ALSeqMarker *start, ALSeqMarker *end, int count);
void    n_alSeqpSetFilter11(N_ALSeqPlayer *seqp);


/*
 * Compressed Sequence Player routines
 */
void    n_alCSPNew(N_ALCSPlayer *seqp, ALSeqpConfig *config);
void    n_alCSPDelete(N_ALCSPlayer *seqp);
uint8_t	n_alCSPGetChlVol(N_ALCSPlayer *seqp, uint8_t chan);
uint8_t      n_alCSPGetChlFXMix(N_ALCSPlayer *seqp, uint8_t chan);
ALPan   n_alCSPGetChlPan(N_ALCSPlayer *seqp, uint8_t chan);
uint8_t      n_alCSPGetChlPriority(N_ALCSPlayer *seqp, uint8_t chan);
int     n_alCSPGetChlProgram(N_ALCSPlayer *seqp, uint8_t chan);
ALCSeq *n_alCSPGetSeq(N_ALCSPlayer *seqp);
int	n_alCSPGetState(N_ALCSPlayer *seqp);
int     n_alCSPGetTempo(N_ALCSPlayer *seqp);
int16_t     n_alCSPGetVol(N_ALCSPlayer *seqp);
void    n_alCSPPlay(N_ALCSPlayer *seqp);
void    n_alCSPSendMidi(N_ALCSPlayer *seqp, int ticks, uint8_t status, uint8_t byte1, uint8_t byte2);
void    n_alCSPSetBank(N_ALCSPlayer *seqp, ALBank *b);
void	n_alCSPSetChlVol(N_ALCSPlayer *seqp, uint8_t chan, uint8_t vol);
void    n_alCSPSetChlFXMix(N_ALCSPlayer *seqp, uint8_t chan, uint8_t fxmix);
void    n_alCSPSetChlPan(N_ALCSPlayer *seqp, uint8_t chan, ALPan pan);
void    n_alCSPSetChlPriority(N_ALCSPlayer *seqp, uint8_t chan, uint8_t priority);
void    n_alCSPSetChlProgram(N_ALCSPlayer *seqp, uint8_t chan, uint8_t prog);
void    n_alCSPSetSeq(N_ALCSPlayer *seqp, ALCSeq *seq);
void    n_alCSPSetTempo(N_ALCSPlayer *seqp, int tempo);
void    n_alCSPSetVol(N_ALCSPlayer *seqp, int16_t vol);


/*
 * Sound Player stuff
 */
typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALEventQueue        evtq;
    N_ALEvent           nextEvent;
    N_ALSynth           *drvr;          /* reference to the client driver   */
    struct sndstate     *target;
    void                *sndState;
    int                 maxSounds;
    ALMicroTime         frameTime;
    ALMicroTime         nextDelta;      /* microseconds to next callback    */
    ALMicroTime         curTime;
} N_ALSndPlayer;

void     n_alSndpNew(ALSndpConfig *c);
void     n_alSndpDelete(void);
ALSndId  n_alSndpAllocate(ALSound *sound);
void     n_alSndpDeallocate(ALSndId id);
int      n_alSndpGetState(void);
void     n_alSndpPlay(void);
void     n_alSndpPlayAt(ALMicroTime delta);
void     n_alSndpSetFXMix(uint8_t mix);
void     n_alSndpSetPan(ALPan pan);
void     n_alSndpSetPitch(float pitch);
void     n_alSndpSetPriority(ALSndId id, uint8_t priority);
void     n_alSndpSetVol(int16_t vol);
void     n_alSndpStop(void);
ALSndId  n_alSndpGetSound(void);
void     n_alSndpSetSound(ALSndId id);

float func0003b9d4(int arg0);
void func0003ba64(struct fx *fx, float outputrate);
int16_t _getRate(float vol, float tgt, int count, u16 *ratel);
int16_t _getVol(int16_t ivol, int samples, int16_t ratem, u16 ratel);

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* __N_LIBAUDIO__ */
