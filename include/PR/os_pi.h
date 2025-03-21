
/*====================================================================
 * os_pi.h
 *
 * Copyright 1995, Silicon Graphics, Inc.
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

/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo. (Originated by SGI)
        
        $RCSfile: os_pi.h,v $
        $Revision: 1.1 $
        $Date: 1998/10/09 08:01:16 $
 *---------------------------------------------------------------------*/

#ifndef _OS_PI_H_
#define	_OS_PI_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include "os_thread.h"
#include "os_message.h"
#include <stdint.h>

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Type definitions
 *
 */

/*
 * Structure for Enhanced PI interface
 */

/*
 * OSTranxInfo is set up for Leo Disk DMA. This info will be maintained
 * by exception handler. This is how the PIMGR and the ISR communicate.
 */

typedef struct {
	u32		errStatus;	/* error status */
        void     	*dramAddr;      /* RDRAM buffer address (DMA) */
	void		*C2Addr;	/* C2 buffer address */
	u32		sectorSize;	/* size of transfering sector */
	u32		C1ErrNum;	/* total # of C1 errors */
	u32		C1ErrSector[4];	/* error sectors */
} __OSBlockInfo;

typedef struct {
	u32     	cmdType;       	/* for disk only */
	u16     	transferMode;   /* Block, Track, or sector?   */
	u16		blockNum;	/* which block is transfering */
	s32     	sectorNum;      /* which sector is transfering */
	u32     	devAddr;        /* Device buffer address */
	u32		bmCtlShadow;	/* asic bm_ctl(510) register shadow ram */
	u32		seqCtlShadow;	/* asic seq_ctl(518) register shadow ram */
	__OSBlockInfo	block[2];	/* bolck transfer info */
} __OSTranxInfo;


typedef struct OSPiHandle_s {
        struct OSPiHandle_s     *next;  /* point to next handle on the table */
        u8                      type;   /* DEVICE_TYPE_BULK for disk */
        u8                      latency;        /* domain latency */
        u8                      pageSize;       /* domain page size */
        u8                      relDuration;    /* domain release duration */
        u8                      pulse;          /* domain pulse width */
	u8			domain;		/* which domain */
        u32                     baseAddress;    /* Domain address */
        u32                     speed;          /* for roms only */
        /* The following are "private" elements" */
        __OSTranxInfo           transferInfo;	/* for disk only */
} OSPiHandle;

/*
 * Structure for I/O message block
 */
typedef struct {
        u16 		type;		/* Message type */
        u8 		pri;		/* Message priority (High or Normal) */
        u8		status;		/* Return status */
	OSMesgQueue	*retQueue;	/* Return message queue to notify I/O 
					 * completion */
} OSIoMesgHdr;

typedef struct {
	OSIoMesgHdr	hdr;		/* Message header */
	void *		dramAddr;	/* RDRAM buffer address (DMA) */
	u32		devAddr;	/* Device buffer address (DMA) */
	u32 		size;		/* DMA transfer size in bytes */
	OSPiHandle	*piHandle;	/* PI device handle */
} OSIoMesg;


#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */


#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)


/**************************************************************************
 *
 * Function prototypes
 *
 */

/* Peripheral interface (Pi) */
extern s32		osPiStartDma(OSIoMesg *, uintptr_t, void *, u32,
				     OSMesgQueue *);

/* Enhanced PI interface */


#endif  /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_PI_H_ */
