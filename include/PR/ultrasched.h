/*====================================================================
 * sched.h
 *
 * Synopsis:
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
 *  $Revision: 1.7 $
 *  $Date: 1997/02/11 08:32:02 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/sched.h,v $
 *
 **************************************************************************/

#ifndef __sched__
#define __sched__

#include <ultra64.h>
#include <stdint.h>

typedef struct OSScTask_s {
    struct OSScTask_s   *next;          /* note: this must be first */
    uint32_t                 state;
    uint32_t			flags;
    void		*framebuffer;	/* used by graphics tasks */

    OSTask              list;
    //OSMesg              msg;
} OSScTask;

#endif

