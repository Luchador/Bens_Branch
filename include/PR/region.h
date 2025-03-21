
/**************************************************************************
 *									  *
 *		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
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
 *  Module: region.h
 *
 *  $Revision: 1.8 $
 *  $Date: 1997/11/26 00:30:56 $
 *  $Author: mitu $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/region.h,v $
 *
 *  Description:
 *      This file contains macros and structure definitions for the region
 *	library.
 *
 **************************************************************************/


#ifndef _REGION_H_
#define _REGION_H_


#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>

/***************************************
 *
 * Macro definitions
 *
 */

/* Perform alignment on input 's' */
#define	ALIGN(s, align)	(((u32)(s) + ((align)-1)) & ~((align)-1))


#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif


#endif  /* _REGION_H_ */


