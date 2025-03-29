/*====================================================================
 * libaudio.h
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

/**************************************************************************
 *
 *  $Revision: 1.173 $
 *  $Date: 1997/12/01 12:42:21 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/libaudio.h,v $
 *
 **************************************************************************/

#ifndef __LIB_AUDIO__
#define __LIB_AUDIO__

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <stdint.h>
#include <PR/abi.h>
#include <PR/mbi.h>

/***********************************************************************
 * misc defines
 ***********************************************************************/

#ifndef NULL
#define NULL 0
#endif

#define AL_FX_BUFFER_SIZE       8192
#define AL_FRAME_INIT           -1
#define AL_USEC_PER_FRAME       16000
#define AL_MAX_PRIORITY         127
#define AL_GAIN_CHANGE_TIME     1000

typedef int     ALMicroTime;
typedef uint8_t      ALPan;

#define AL_PAN_CENTER    64
#define AL_PAN_LEFT      0
#define AL_PAN_RIGHT     127
#define _AL_VOL_FULL     127    /* SDK/internal value */
#define AL_VOL_FULL      0x7fff /* But the game uses this mostly */
#define AL_KEY_MIN       0
#define AL_KEY_MAX       127
#define AL_DEFAULT_FXMIX 0
#define AL_SUSTAIN       63

/***********************************************************************
 * Error handling
 ***********************************************************************/

#define ALFailIf(condition, error)	\
            if (condition) {		\
                return; }

#define ALFlagFailIf(condition, flag, error)	\
            if (condition) {		\
                return; }

/***********************************************************************
 * Audio Library global routines
 ***********************************************************************/
typedef struct ALLink_s {
    struct ALLink_s      *next;
    struct ALLink_s      *prev;
} ALLink;

void    alUnlink(ALLink *element);
void    alLink(ALLink *element, ALLink *after);

typedef uintptr_t (*ALDMAproc)(uintptr_t addr, int len, void *state);
typedef ALDMAproc (*ALDMANew)(void *state);

void    alCopy(void *src, void *dest, int len);

typedef struct {
    uint8_t          *base;
    uint8_t          *cur;
    int         len;
    int         count;
} ALHeap;

void    alHeapInit(ALHeap *hp, uint8_t *base, int len);
void    *alHeapDBAlloc(uint8_t *file, int line, ALHeap *hp, int num, int size);

#define alHeapAlloc(hp, elem ,size) alHeapDBAlloc(0, 0,(hp),(elem),(size))

/***********************************************************************
 * FX Stuff
 ***********************************************************************/
#define    AL_FX_NONE          0
#define    AL_FX_SMALLROOM     1
#define    AL_FX_BIGROOM       2
#define    AL_FX_CHORUS        3
#define    AL_FX_FLANGE        4
#define    AL_FX_ECHO          5
#define    AL_FX_CUSTOM        6

typedef uint8_t      ALFxId;
typedef void    *ALFxRef;

/***********************************************************************
 * data structures for sound banks
 ***********************************************************************/

#define AL_BANK_VERSION    0x4231	/* 'B1' */

/* Possible wavetable types */
enum    {AL_ADPCM_WAVE = 0,
         AL_RAW16_WAVE};

typedef struct {
    int order;
    int npredictors;
    int16_t book[128];        /* Actually variable size. Must be 8-byte aligned */
} ALADPCMBook;

typedef struct {
    uint32_t         start;
    uint32_t         end;
    uint32_t         count;
    ADPCM_STATE state;
} ALADPCMloop;

typedef struct {
    uint32_t         start;
    uint32_t         end;
    uint32_t         count;
} ALRawLoop;

typedef struct {
    ALMicroTime attackTime;
    ALMicroTime decayTime;
    ALMicroTime releaseTime;
    uint8_t          attackVolume;
    uint8_t          decayVolume;
} ALEnvelope;

typedef struct {
    uint8_t          velocityMin;
    uint8_t          velocityMax;
    uint8_t          keyMin;
    uint8_t          keyMax;
    uint8_t          keyBase;
    int8_t          detune;
} ALKeyMap;

typedef struct {
    ALADPCMloop *loop;
    ALADPCMBook *book;
} ALADPCMWaveInfo;

typedef struct {
    ALRawLoop *loop;
} ALRAWWaveInfo;

typedef struct ALWaveTable_s {
    uint8_t          *base;          /* ptr to start of wave data    */
    int         len;            /* length of data in bytes      */
    uint8_t          type;           /* compression type             */
    uint8_t          flags;          /* offset/address flags         */
    union {
        ALADPCMWaveInfo adpcmWave;
        ALRAWWaveInfo   rawWave;
    } waveInfo;
} ALWaveTable;

typedef struct ALSound_s {
    ALEnvelope  *envelope;
    ALKeyMap    *keyMap;
    ALWaveTable *wavetable;     /* offset to wavetable struct           */
    ALPan       samplePan;
    uint8_t          sampleVolume;
    uint8_t          flags;
} ALSound;

typedef struct {
    uint8_t          volume;         /* overall volume for this instrument   */
    ALPan       pan;            /* 0 = hard left, 127 = hard right      */
    uint8_t          priority;       /* voice priority for this instrument   */
    uint8_t          flags;
    uint8_t          tremType;       /* the type of tremelo osc. to use      */
    uint8_t          tremRate;       /* the rate of the tremelo osc.         */
    uint8_t          tremDepth;      /* the depth of the tremelo osc         */
    uint8_t          tremDelay;      /* the delay for the tremelo osc        */
    uint8_t          vibType;        /* the type of tremelo osc. to use      */
    uint8_t          vibRate;        /* the rate of the tremelo osc.         */
    uint8_t          vibDepth;       /* the depth of the tremelo osc         */
    uint8_t          vibDelay;       /* the delay for the tremelo osc        */
    int16_t         bendRange;      /* pitch bend range in cents            */
    int16_t         soundCount;     /* number of sounds in this array       */
    ALSound     *soundArray[1];
} ALInstrument;

typedef struct ALBank_s {
    int16_t                 instCount;      /* number of programs in this bank */
    uint8_t                  flags;
    uint8_t                  pad;
    int                 sampleRate;     /* e.g. 44100, 22050, etc...       */
    ALInstrument        *percussion;    /* default percussion for GM       */
    ALInstrument        *instArray[1];  /* ARRAY of instruments            */
} ALBank;

typedef struct {                /* Note: sizeof won't be correct        */
    int16_t         revision;       /* format revision of this file         */
    int16_t         bankCount;      /* number of banks                      */
    ALBank      *bankArray[1];  /* ARRAY of bank offsets                */
} ALBankFile;

void    alBnkfNew(ALBankFile *f, uint8_t *table);

/***********************************************************************
 * Sequence Files
 ***********************************************************************/
#define AL_SEQBANK_VERSION    'S1'

typedef struct {
    uint8_t          *offset;
    int         len;
} ALSeqData;

typedef struct {                /* Note: sizeof won't be correct        */
    int16_t         revision;       /* format revision of this file         */
    int16_t         seqCount;       /* number of sequences                  */
    ALSeqData   seqArray[1];    /* ARRAY of sequence info               */
} ALSeqFile;

void    alSeqFileNew(ALSeqFile *f, uint8_t *base);

/***********************************************************************
 * Synthesis driver stuff
 ***********************************************************************/
typedef ALMicroTime (*ALVoiceHandler)(void *);

typedef struct {
    int                 maxVVoices;     /* obsolete */
    int                 maxPVoices;
    int                 maxUpdates;
    int                 maxFXbusses;
    void                *dmaproc;
    ALHeap              *heap;
    int                 outputRate;     /* output sample rate */
    ALFxId              fxTypes[4];
    int                 *params[2];
} ALSynConfig;

typedef struct ALPlayer_s {
    struct ALPlayer_s   *next;
    void                *clientData;    /* storage for client callback  */
    ALVoiceHandler      handler;        /* voice handler for player     */
    ALMicroTime         callTime;       /* usec requested callback      */
    int                 samplesLeft;    /* usec remaining to callback   */
} ALPlayer;

typedef struct ALVoice_s {
    ALLink              node;
    struct PVoice_s     *pvoice;
    ALWaveTable         *table;
    void                *clientPrivate;
    int16_t                 state;
    int16_t                 priority;
    int16_t                 fxBus;
    int16_t                 unityPitch;
} ALVoice;

typedef struct ALVoiceConfig_s {
    int16_t                 priority;       /* voice priority               */
    int16_t                 fxBus;          /* bus assignment               */
    uint8_t                  unityPitch;     /* unity pitch flag             */
} ALVoiceConfig;

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

    struct ALMainBus_s  *mainBus;
    struct ALAuxBus_s   *auxBus;        /* ptr to array of aux bus structs */
    struct ALFilter_s   *outputFilter;  /* last filter in the filter chain */

    int                 numPVoices;
    int                 maxAuxBusses;
    int                 outputRate;     /* output sample rate */
    int                 maxOutSamples;  /* Maximum samples rsp can generate
                                           at one time at output rate */
} ALSynth;

void    alSynNew(ALSynth *s, ALSynConfig *config);
void    alSynDelete(ALSynth *s);

void    alSynAddPlayer(ALSynth *s, ALPlayer *client);
void    alSynRemovePlayer(ALSynth *s, ALPlayer *client);

int     alSynAllocVoice(ALSynth *s, ALVoice *v, ALVoiceConfig *vc);
void    alSynFreeVoice(ALSynth *s, ALVoice *voice);

void    alSynStartVoice(ALSynth *s, ALVoice *voice, ALWaveTable *w);
void    alSynStartVoiceParams(ALSynth *s, ALVoice *voice, ALWaveTable *w,
                              float pitch, int16_t vol, ALPan pan, uint8_t fxmix,
                              ALMicroTime t);
void    alSynStopVoice(ALSynth *s, ALVoice *voice);

void    alSynSetVol(ALSynth *s, ALVoice *v, int16_t vol, ALMicroTime delta);
void    alSynSetPitch(ALSynth *s, ALVoice *voice, float ratio);
void    alSynSetPan(ALSynth *s, ALVoice *voice, ALPan pan);
void    alSynSetFXMix(ALSynth *s, ALVoice *voice, uint8_t fxmix);
void    alSynSetPriority(ALSynth *s, ALVoice *voice, int16_t priority);
int16_t     alSynGetPriority(ALSynth *s, ALVoice *voice);

ALFxRef *alSynAllocFX(ALSynth *s, int16_t bus, ALSynConfig *c, ALHeap *hp);
ALFxRef alSynGetFXRef(ALSynth *s, int16_t bus, int16_t index);
void    alSynFreeFX(ALSynth *s, ALFxRef *fx);
void    alSynSetFXParam(ALSynth *s, ALFxRef fx, int16_t paramID, void *param);

/***********************************************************************
 * Audio Library (AL) stuff
 ***********************************************************************/
typedef struct {
    ALSynth     drvr;
} ALGlobals;

extern ALGlobals *alGlobals;

void    alInit(ALGlobals *glob, ALSynConfig *c);
void    alClose(ALGlobals *glob);

Acmd    *alAudioFrame(Acmd *cmdList, int *cmdLen, int16_t *outBuf, int outLen);

/***********************************************************************
 * Sequence Player stuff
 ***********************************************************************/

/*
 * Play states
 */
#define AL_STOPPED  0
#define AL_PLAYING  1
#define AL_STOPPING 2
#define AL_STATE3   3
#define AL_STATE4   4
#define AL_STATE5   5
#define AL_STARTING 6

#define AL_DEFAULT_PRIORITY     5
#define AL_DEFAULT_VOICE        0
#define AL_MAX_CHANNELS         16

/*
 * Audio Library event type definitions
 */
enum ALMsg {
    /*0x00*/ AL_SEQ_REF_EVT,	/* Reference to a pending event in the sequence. */
    /*0x01*/ AL_SEQ_MIDI_EVT,
    /*0x02*/ AL_SEQP_MIDI_EVT,
    /*0x03*/ AL_TEMPO_EVT,
    /*0x04*/ AL_SEQ_END_EVT,
    /*0x05*/ AL_NOTE_END_EVT,
    /*0x06*/ AL_SEQP_ENV_EVT,
    /*0x07*/ AL_SEQP_META_EVT,
    /*0x08*/ AL_SEQP_PROG_EVT,
    /*0x09*/ AL_SEQP_API_EVT,
    /*0x0a*/ AL_SEQP_VOL_EVT,
    /*0x0b*/ AL_SEQP_LOOP_EVT,
    /*0x0c*/ AL_SEQP_PRIORITY_EVT,
    /*0x0d*/ AL_SEQP_SEQ_EVT,
    /*0x0e*/ AL_SEQP_BANK_EVT,
    /*0x0f*/ AL_SEQP_PLAY_EVT,
    /*0x10*/ AL_SEQP_STOP_EVT,
    /*0x11*/ AL_SEQP_STOPPING_EVT,
    /*0x12*/ AL_TRACK_END,
    /*0x13*/ AL_CSP_LOOPSTART,
    /*0x14*/ AL_CSP_LOOPEND,
    /*0x15*/ AL_CSP_NOTEOFF_EVT,
    /*0x16*/ AL_TREM_OSC_EVT,
    /*0x17*/ AL_VIB_OSC_EVT,
    /*0x18*/ AL_18_EVT,
    /*0x19*/ AL_19_EVT
};

/*
 * Midi event definitions
 */
#define AL_EVTQ_END     0x7fffffff

enum AL_MIDIstatus {
    /* For distinguishing channel number from status */
    AL_MIDI_ChannelMask         = 0x0F,
    AL_MIDI_StatusMask          = 0xF0,

    /* Channel voice messages */
    AL_MIDI_ChannelVoice        = 0x80,
    AL_MIDI_NoteOff             = 0x80,
    AL_MIDI_NoteOn              = 0x90,
    AL_MIDI_PolyKeyPressure     = 0xA0,
    AL_MIDI_ControlChange       = 0xB0,
    AL_MIDI_ChannelModeSelect   = 0xB0,
    AL_MIDI_ProgramChange       = 0xC0,
    AL_MIDI_ChannelPressure     = 0xD0,
    AL_MIDI_PitchBendChange     = 0xE0,

    /* System messages */
    AL_MIDI_SysEx               = 0xF0, /* System Exclusive */

    /* System common */
    AL_MIDI_SystemCommon            = 0xF1,
    AL_MIDI_TimeCodeQuarterFrame    = 0xF1,
    AL_MIDI_SongPositionPointer     = 0xF2,
    AL_MIDI_SongSelect              = 0xF3,
    AL_MIDI_Undefined1              = 0xF4,
    AL_MIDI_Undefined2              = 0xF5,
    AL_MIDI_TuneRequest             = 0xF6,
    AL_MIDI_EOX                     = 0xF7, /* End of System Exclusive */

    /* System real time */
    AL_MIDI_SystemRealTime  = 0xF8,
    AL_MIDI_TimingClock     = 0xF8,
    AL_MIDI_Undefined3      = 0xF9,
    AL_MIDI_Start           = 0xFA,
    AL_MIDI_Continue        = 0xFB,
    AL_MIDI_Stop            = 0xFC,
    AL_MIDI_Undefined4      = 0xFD,
    AL_MIDI_ActiveSensing   = 0xFE,
    AL_MIDI_SystemReset     = 0xFF,
    AL_MIDI_Meta            = 0xFF      /* MIDI Files only */
};

enum AL_MIDIctrl {
    AL_MIDI_VOLUME_CTRL         = 0x07,
    AL_MIDI_PAN_CTRL            = 0x0A,
    AL_MIDI_PRIORITY_CTRL       = 0x10, /* use general purpose controller for priority */
    AL_MIDI_FX_CTRL_0           = 0x14,
    AL_MIDI_FX_CTRL_1           = 0x15,
    AL_MIDI_FX_CTRL_2           = 0x16,
    AL_MIDI_FX_CTRL_3           = 0x17,
    AL_MIDI_FX_CTRL_4           = 0x18,
    AL_MIDI_FX_CTRL_5           = 0x19,
    AL_MIDI_FX_CTRL_6           = 0x1A,
    AL_MIDI_FX_CTRL_7           = 0x1B,
    AL_MIDI_FX_CTRL_8           = 0x1C,
    AL_MIDI_FX_CTRL_9           = 0x1D,
    AL_MIDI_SUSTAIN_CTRL        = 0x40,
    AL_MIDI_FX1_CTRL            = 0x5B,
    AL_MIDI_FX3_CTRL            = 0x5D
};

enum AL_MIDImeta {
    AL_MIDI_META_TEMPO          = 0x51,
    AL_MIDI_META_EOT            = 0x2f
};


#define AL_CMIDI_BLOCK_CODE           0xFE
#define AL_CMIDI_LOOPSTART_CODE       0x2E
#define AL_CMIDI_LOOPEND_CODE         0x2D
#define AL_CMIDI_CNTRL_LOOPSTART      102
#define AL_CMIDI_CNTRL_LOOPEND        103
#define AL_CMIDI_CNTRL_LOOPCOUNT_SM   104
#define AL_CMIDI_CNTRL_LOOPCOUNT_BIG  105

typedef struct {
    uint8_t          *curPtr;                /* ptr to the next event */
    int         lastTicks;              /* sequence clock ticks (used by alSeqSetLoc) */
    int	       	curTicks;		/* sequence clock ticks of next event (used by loop end test) */
    int16_t         lastStatus;             /* the last status msg */
} ALSeqMarker;

typedef struct {
    int         ticks;    /* MIDI, Tempo and End events must start with ticks */
    uint8_t          status;
    uint8_t          byte1;
    uint8_t          byte2;
    uint32_t         duration;
} ALMIDIEvent;

typedef struct {
    int         ticks;
    uint8_t          status;
    uint8_t          type;
    uint8_t          len;
    uint8_t          byte1;
    uint8_t          byte2;
    uint8_t          byte3;
} ALTempoEvent;

typedef struct {
    int         ticks;
    uint8_t          status;
    uint8_t          type;
    uint8_t          len;
} ALEndEvent;

typedef struct {
    struct ALVoice_s    *voice;
} ALNoteEvent;

typedef struct {
    struct ALVoice_s    *voice;
    ALMicroTime         delta;
    uint8_t                  vol;
} ALVolumeEvent;

typedef struct {
    int16_t                 vol;
} ALSeqpVolEvent;

typedef struct {
    ALSeqMarker         *start;
    ALSeqMarker         *end;
    int                 count;
} ALSeqpLoopEvent;

typedef struct {
    uint8_t			chan;
    uint8_t			priority;
} ALSeqpPriorityEvent;

typedef struct {
    void		*seq;	/* pointer to a seq (could be an ALSeq or an ALCSeq). */
} ALSeqpSeqEvent;

typedef struct {
    ALBank		*bank;
} ALSeqpBankEvent;

typedef struct {
    struct ALVoiceState_s      *vs;
    void                       *oscState;
    uint8_t                         chan;
} ALOscEvent;

typedef struct {
    int16_t                 	type;
    union {
        ALMIDIEvent     	midi;
        ALTempoEvent    	tempo;
        ALEndEvent      	end;
        ALNoteEvent     	note;
        ALVolumeEvent   	vol;
        ALSeqpLoopEvent 	loop;
        ALSeqpVolEvent  	spvol;
	ALSeqpPriorityEvent	sppriority;
	ALSeqpSeqEvent		spseq;
	ALSeqpBankEvent		spbank;
        ALOscEvent      	osc;
    } msg;
} ALEvent;

typedef struct {
    ALLink      node;
    ALMicroTime delta;
    ALEvent     evt;
} ALEventListItem;

typedef struct {
    ALLink      freeList;
    ALLink      allocList;
    int         eventCount;
} ALEventQueue;

void            alEvtqNew(ALEventQueue *evtq, ALEventListItem *items,
                          int itemCount);
ALMicroTime     alEvtqNextEvent(ALEventQueue *evtq, ALEvent *evt);
void            alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt,
                                ALMicroTime delta);
void        	alEvtqFlush(ALEventQueue *evtq);
void        	alEvtqFlushType(ALEventQueue *evtq, int16_t type);


#define AL_PHASE_ATTACK         0
#define AL_PHASE_NOTEON         0
#define AL_PHASE_DECAY          1
#define AL_PHASE_SUSTAIN        2
#define AL_PHASE_RELEASE        3
#define AL_PHASE_SUSTREL        4

typedef struct ALVoiceState_s {
    struct ALVoiceState_s *next;/* MUST be first                */
    ALVoice     voice;
    ALSound     *sound;
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
} ALVoiceState;

typedef struct {
    ALInstrument        *instrument;    /* instrument assigned to this chan */
    int16_t                 bendRange;      /* pitch bend range in cents        */
    ALFxId              fxId;           /* type of fx assigned to this chan */
    ALPan               pan;            /* overall pan for this chan        */
    uint8_t                  priority;       /* priority for this chan           */
    uint8_t                  vol;            /* current volume for this chan     */
    uint8_t                  fxmix;          /* current fx mix for this chan     */
    uint8_t                  unk0b;
    uint8_t                  sustain;        /* current sustain pedal state      */
    uint8_t unk0d;
    uint8_t unk0e;
    uint8_t unk0f;
    uint8_t unk10;
    uint8_t unk11;
    uint8_t unk12;
    uint8_t unk13;
    float pitchBend;      /* current pitch bend val in cents  */
    ALMicroTime attackTime;
    ALMicroTime decayTime;
    ALMicroTime releaseTime;
    uint8_t unk24;
    uint8_t attackVolume;
    uint8_t decayVolume;
    int8_t unk27;
    uint8_t tremType;
    uint8_t tremRate;
    uint8_t tremDepth;
    uint8_t tremDelay;
    uint8_t vibType;
    uint8_t vibRate;
    uint8_t vibDepth;
    uint8_t vibDelay;
    uint8_t unk30;
    uint8_t unk31;
    uint8_t unk32;
} ALChanState;

typedef struct ALSeq_s {
    uint8_t          *base;                  /* ptr to start of sequence file   */
    uint8_t          *trackStart;            /* ptr to first MIDI event         */
    uint8_t          *curPtr;                /* ptr to next event to read       */
    int         lastTicks;              /* MIDI ticks for last event       */
    int         len;                    /* length of sequence in bytes     */
    float         qnpt;                   /* qrter notes / tick (1/division) */
    int16_t         division;               /* ticks per quarter note          */
    int16_t         lastStatus;             /* for running status              */
} ALSeq;

typedef struct {
    uint32_t      trackOffset[16];
    uint32_t      division;
} ALCMidiHdr;

typedef struct ALCSeq_s {
    ALCMidiHdr    *base;             /* ptr to start of sequence file         */
    uint32_t           validTracks;       /* set of flags, showing valid tracks    */
    float           qnpt;              /* qrter notes / tick (1/division)       */
    uint32_t           lastTicks;         /* keep track of ticks incase app wants  */
    uint32_t           lastDeltaTicks;    /* number of delta ticks of last event   */
    uint32_t		  deltaFlag;	     /* flag: set if delta's not subtracted   */
    uint8_t            *curLoc[16];       /* ptr to current track location,        */
                                     /* may point to next event, or may point */
                                     /* to a backup code                      */
    uint8_t            *curBUPtr[16];     /* ptr to next event if in backup mode   */
    uint8_t            curBULen[16];      /* if > 0, then in backup mode           */
    uint8_t            lastStatus[16];    /* for running status                    */
    uint32_t           evtDeltaTicks[16]; /* delta time to next event              */
} ALCSeq;

typedef struct {
    uint32_t         validTracks;
    int         lastTicks;
    uint32_t         lastDeltaTicks;
    uint8_t          *curLoc[16];
    uint8_t          *curBUPtr[16];
    uint8_t          curBULen[16];
    uint8_t          lastStatus[16];
    uint32_t         evtDeltaTicks[16];
} ALCSeqMarker;

#define NO_SOUND_ERR_MASK          0x01
#define NOTE_OFF_ERR_MASK          0x02
#define NO_VOICE_ERR_MASK          0x04

typedef struct {
    int         maxVoices;         /* max number of voices to alloc    */
    int         maxEvents;         /* max internal events to support   */
    uint8_t          maxChannels;       /* max MIDI channels to support (16)*/
    uint8_t          debugFlags;        /* control which error get reported */
    ALHeap      *heap;             /* ptr to initialized heap          */
    void        *initOsc;
    void        *updateOsc;
    void        *stopOsc;
} ALSeqpConfig;

typedef ALMicroTime   (*ALOscInit)(void **oscState,float *initVal, uint8_t oscType,
                                   uint8_t oscRate, uint8_t oscDepth, uint8_t oscDelay, uint8_t unk07);
typedef ALMicroTime   (*ALOscUpdate)(void *oscState, float *updateVal);
typedef void          (*ALOscStop)(void *oscState);

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALSynth             *drvr;          /* reference to the client driver   */
    ALSeq               *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank              *bank;          /* current ALBank                   */
    int                 uspt;           /* microseconds per tick            */
    int                 nextDelta;      /* microseconds to next callback    */
    int                 state;
    uint16_t                 chanMask;       /* active channels                  */
    int16_t                 vol;            /* overall sequence volume          */
    uint8_t                  maxChannels;    /* number of MIDI channels          */
    uint8_t                  debugFlags;     /* control which error get reported */
    ALEvent             nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState         *chanState;     /* 16 channels for MIDI             */
    ALVoiceState        *vAllocHead;    /* list head for allocated voices   */
    ALVoiceState        *vAllocTail;    /* list tail for allocated voices   */
    ALVoiceState        *vFreeList;     /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
    ALSeqMarker         *loopStart;
    ALSeqMarker         *loopEnd;
    int                 loopCount;      /* -1 = loop forever, 0 = no loop   */
} ALSeqPlayer;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALSynth             *drvr;          /* reference to the client driver   */
    ALCSeq              *target;        /* current sequence                 */
    ALMicroTime         curTime;
    ALBank              *bank;          /* current ALBank                   */
    int                 uspt;           /* microseconds per tick            */
    int                 nextDelta;      /* microseconds to next callback    */
    int                 state;
    uint16_t                 chanMask;       /* active channels                  */
    int16_t                 vol;            /* overall sequence volume          */
    uint8_t                  maxChannels;    /* number of MIDI channels          */
    uint8_t                  debugFlags;     /* control which error get reported */
    ALEvent             nextEvent;
    ALEventQueue        evtq;
    ALMicroTime         frameTime;
    ALChanState         *chanState;     /* 16 channels for MIDI             */
    ALVoiceState        *vAllocHead;    /* list head for allocated voices   */
    ALVoiceState        *vAllocTail;    /* list tail for allocated voices   */
    ALVoiceState        *vFreeList;     /* list of free voice state structs */
    ALOscInit           initOsc;
    ALOscUpdate         updateOsc;
    ALOscStop           stopOsc;
} ALCSPlayer;

/*
 * Sequence data representation routines
 */
void    alSeqNew(ALSeq *seq, uint8_t *ptr, int len);
void    alSeqNextEvent(ALSeq *seq, ALEvent *event);
int     alSeqGetTicks(ALSeq *seq);
float     alSeqTicksToSec(ALSeq *seq, int ticks, uint32_t tempo);
uint32_t     alSeqSecToTicks(ALSeq *seq, float sec, uint32_t tempo);
void    alSeqNewMarker(ALSeq *seq, ALSeqMarker *m, uint32_t ticks);
void    alSeqSetLoc(ALSeq *seq, ALSeqMarker *marker);
void    alSeqGetLoc(ALSeq *seq, ALSeqMarker *marker);
/*
 * Compact Sequence data representation routines
 */
void    alCSeqNew(ALCSeq *seq, uint8_t *ptr);
void    alCSeqNextEvent(ALCSeq *seq,ALEvent *evt);
int     alCSeqGetTicks(ALCSeq *seq);
float     alCSeqTicksToSec(ALCSeq *seq, int ticks, uint32_t tempo);
uint32_t     alCSeqSecToTicks(ALCSeq *seq, float sec, uint32_t tempo);
void    alCSeqNewMarker(ALCSeq *seq, ALCSeqMarker *m, uint32_t ticks);
void    alCSeqSetLoc(ALCSeq *seq, ALCSeqMarker *marker);
void    alCSeqGetLoc(ALCSeq *seq, ALCSeqMarker *marker);

/*
 * Sequence Player routines
 */
float     alCents2Ratio(int cents);

void    alSeqpNew(ALSeqPlayer *seqp, ALSeqpConfig *config);
void    alSeqpDelete(ALSeqPlayer *seqp);
void    alSeqpSetSeq(ALSeqPlayer *seqp, ALSeq *seq);
ALSeq   *alSeqpGetSeq(ALSeqPlayer *seqp);
void    alSeqpPlay(ALSeqPlayer *seqp);
void    alSeqpStop(ALSeqPlayer *seqp);
int	alSeqpGetState(ALSeqPlayer *seqp);
void    alSeqpSetBank(ALSeqPlayer *seqp, ALBank *b);
void    alSeqpSetTempo(ALSeqPlayer *seqp, int tempo);
int     alSeqpGetTempo(ALSeqPlayer *seqp);
int16_t     alSeqpGetVol(ALSeqPlayer *seqp);		/* Master volume control */
void    alSeqpSetVol(ALSeqPlayer *seqp, int16_t vol);
void    alSeqpLoop(ALSeqPlayer *seqp, ALSeqMarker *start, ALSeqMarker *end, int count);

void    alSeqpSetChlProgram(ALSeqPlayer *seqp, uint8_t chan, uint8_t prog);
int     alSeqpGetChlProgram(ALSeqPlayer *seqp, uint8_t chan);
void    alSeqpSetChlFXMix(ALSeqPlayer *seqp, uint8_t chan, uint8_t fxmix);
uint8_t      alSeqpGetChlFXMix(ALSeqPlayer *seqp, uint8_t chan);
void	alSeqpSetChlVol(ALSeqPlayer *seqp, uint8_t chan, uint8_t vol);
uint8_t	alSeqpGetChlVol(ALSeqPlayer *seqp, uint8_t chan);
void    alSeqpSetChlPan(ALSeqPlayer *seqp, uint8_t chan, ALPan pan);
ALPan   alSeqpGetChlPan(ALSeqPlayer *seqp, uint8_t chan);
void    alSeqpSetChlPriority(ALSeqPlayer *seqp, uint8_t chan, uint8_t priority);
uint8_t      alSeqpGetChlPriority(ALSeqPlayer *seqp, uint8_t chan);


/* Maintain backwards compatibility with old routine names. */
#define alSeqpSetProgram		alSeqpSetChlProgram
#define alSeqpGetProgram		alSeqpGetChlProgram
#define alSeqpSetFXMix			alSeqpSetChlFXMix
#define alSeqpGetFXMix			alSeqpGetChlFXMix
#define alSeqpSetPan			alSeqpSetChlPan
#define	alSeqpGetPan			alSeqpGetChlPan
#define alSeqpSetChannelPriority	alSeqpSetChlPriority
#define alSeqpGetChannelPriority	alSeqpGetChlPriority



/*
 * Compressed Sequence Player routines
 */
void    alCSPNew(ALCSPlayer *seqp, ALSeqpConfig *config);
void    alCSPDelete(ALCSPlayer *seqp);
void    alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq);
ALCSeq  *alCSPGetSeq(ALCSPlayer *seqp);
void    alCSPPlay(ALCSPlayer *seqp);
void    alCSPStop(ALCSPlayer *seqp);
int	alCSPGetState(ALCSPlayer *seqp);
void    alCSPSetBank(ALCSPlayer *seqp, ALBank *b);
int     alCSPGetTempo(ALCSPlayer *seqp);
int16_t     alCSPGetVol(ALCSPlayer *seqp);
void    alCSPSetVol(ALCSPlayer *seqp, int16_t vol);

void    alCSPSetChlProgram(ALCSPlayer *seqp, uint8_t chan, uint8_t prog);
int     alCSPGetChlProgram(ALCSPlayer *seqp, uint8_t chan);
void    alCSPSetChlFXMix(ALCSPlayer *seqp, uint8_t chan, uint8_t fxmix);
uint8_t      alCSPGetChlFXMix(ALCSPlayer *seqp, uint8_t chan);
void    alCSPSetChlPan(ALCSPlayer *seqp, uint8_t chan, ALPan pan);
ALPan   alCSPGetChlPan(ALCSPlayer *seqp, uint8_t chan);
void	alCSPSetChlVol(ALCSPlayer *seqp, uint8_t chan, uint8_t vol);
uint8_t	alCSPGetChlVol(ALCSPlayer *seqp, uint8_t chan);
void    alCSPSetChlPriority(ALCSPlayer *seqp, uint8_t chan, uint8_t priority);
uint8_t      alCSPGetChlPriority(ALCSPlayer *seqp, uint8_t chan);
void    alCSPSendMidi(ALCSPlayer *seqp, int ticks, uint8_t status,
                       uint8_t byte1, uint8_t byte2);


/* Maintain backwards compatibility with old routine names. */
#define alCSPSetProgram		alCSPSetChlProgram
#define alCSPGetProgram		alCSPGetChlProgram
#define alCSPSetFXMix		alCSPSetChlFXMix
#define alCSPGetFXMix		alCSPGetChlFXMix
#define alCSPSetPan		alCSPSetChlPan
#define	alCSPGetPan		alCSPGetChlPan
#define alCSPSetChannelPriority	alCSPSetChlPriority
#define alCSPGetChannelPriority	alCSPGetChlPriority



/***********************************************************************
 * Sound Player stuff
 ***********************************************************************/

typedef struct {
    int         maxStates;
    int         maxEvents;
	int         maxSounds;
    ALHeap      *heap;
    uint16_t         unk10;
} ALSndpConfig;

typedef struct {
    ALPlayer            node;           /* note: must be first in structure */
    ALEventQueue        evtq;
    ALEvent             nextEvent;
    ALSynth             *drvr;          /* reference to the client driver   */
    int                 target;
    void                *sndState;
    int                 maxSounds;
    ALMicroTime         frameTime;
    ALMicroTime         nextDelta;      /* microseconds to next callback    */
    ALMicroTime         curTime;
} ALSndPlayer;

typedef int16_t   ALSndId;

void            alSndpNew(ALSndPlayer *sndp, ALSndpConfig *c);
void            alSndpDelete(ALSndPlayer *sndp);

ALSndId         alSndpAllocate(ALSndPlayer *sndp, ALSound *sound);
void            alSndpDeallocate(ALSndPlayer *sndp, ALSndId id);

void            alSndpSetSound(ALSndPlayer *sndp, ALSndId id);
ALSndId         alSndpGetSound(ALSndPlayer *sndp);

void            alSndpPlay(ALSndPlayer *sndp);
void            alSndpPlayAt(ALSndPlayer *sndp, ALMicroTime delta);
void            alSndpStop(ALSndPlayer *sndp);

void            alSndpSetVol(ALSndPlayer *sndp, int16_t vol);
void            alSndpSetPitch(ALSndPlayer *sndp, float pitch);
void            alSndpSetPan(ALSndPlayer *sndp, ALPan pan);
void            alSndpSetPriority(ALSndPlayer *sndp, ALSndId id, uint8_t priority);

void            alSndpSetFXMix(ALSndPlayer *sndp, uint8_t mix);
int             alSndpGetState(ALSndPlayer *sndp);

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !__LIB_AUDIO__ */



