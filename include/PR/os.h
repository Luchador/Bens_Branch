
/*====================================================================
 * os.h
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
        
        $RCSfile: os.h,v $
        $Revision: 1.167 $
        $Date: 1999/01/18 13:17:43 $
 *---------------------------------------------------------------------*/

#ifndef _OS_H_
#define	_OS_H_

#include <stdint.h>

#define	ALIGN(s, align)	(((uint32_t)(s) + ((align)-1)) & ~((align)-1))

#include <PR/os_cont.h>
#include <PR/os_pfs.h>
#include <PR/os_eeprom.h>
#include <PR/os_motor.h>
#include <PR/os_libc.h>

#endif /* !_OS_H */
