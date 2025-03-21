/**************************************************************************
 *									  *
 *		 Copyright (C) 1995, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/**************************************************************************
 *
 *  $Revision: 1.13 $
 *  $Date: 1997/02/11 08:15:34 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/R4300.h,v $
 *
 **************************************************************************/

#ifndef __R4300_H__
#define __R4300_H__

#include <PR/ultratypes.h>
#include "platform.h"

/*
 * Address conversion macros
 */
#ifdef _LANGUAGE_ASSEMBLY

#define	K0_TO_K1(x)	((x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((x)&0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* kseg1 to physical */
#define	KDM_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* direct mapped to physical */
#define	PHYS_TO_K0(x)	((x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((x)|0xA0000000)	/* physical to kseg1 */

#else /* _LANGUAGE_C */

#ifdef PLATFORM_N64

#define	K0_TO_K1(x)	((u32)(x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((u32)(x)&0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* kseg1 to physical */
#define	KDM_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* direct mapped to physical */
#define	PHYS_TO_K0(x)	((u32)(x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((u32)(x)|0xA0000000)	/* physical to kseg1 */

#else /* PLATFORM_N64 */

#ifdef PLATFORM_64BIT
typedef u64 k_ptr_t;
#else
typedef u32 k_ptr_t;
#endif

#define	K0_TO_K1(x)	((k_ptr_t)(x))	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((k_ptr_t)(x))	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((k_ptr_t)(x))	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((k_ptr_t)(x))	/* kseg1 to physical */
#define	KDM_TO_PHYS(x)	((k_ptr_t)(x))	/* direct mapped to physical */
#define	PHYS_TO_K0(x)	((k_ptr_t)(x))	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((k_ptr_t)(x))	/* physical to kseg1 */

#endif /* PLATFORM_N64 */

#endif	/* _LANGUAGE_ASSEMBLY */

#endif /* __R4300_H */
