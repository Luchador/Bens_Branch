#ifndef _GU_H_
#define _GU_H_

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
 *  $Revision: 1.46 $
 *  $Date: 1997/11/26 00:30:53 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/gu.h,v $
 *
 **************************************************************************/

#include <PR/mbi.h>
#include <stdint.h>
#include <PR/sptask.h>
/*
 * matrix operations:
 *
 * The 'F' version is floating point, in case the application wants
 * to do matrix manipulations and convert to fixed-point at the last
 * minute.
 */

/*
 *  Math functions
 */

extern signed short sins (unsigned short angle);
extern signed short coss (unsigned short angle);
extern float sqrtf(float value);

#endif /* !_GU_H_ */
