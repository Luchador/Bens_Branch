
/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo.

        $RCSfile: os_motor.h,v $
        $Revision: 1.1 $
        $Date: 1998/10/09 08:01:15 $
 *---------------------------------------------------------------------*/

#ifndef _OS_MOTOR_H_
#define	_OS_MOTOR_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include "os_pfs.h"


#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Type definitions
 *
 */


#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

/**************************************************************************
 *
 * Global definitions
 *
 */


#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Macro definitions
 *
 */


/**************************************************************************
 *
 * Extern variables
 *
 */


/**************************************************************************
 *
 * Function prototypes
 *
 */

/* Rumble PAK interface */
#define	osMotorStart(x)		__osMotorAccess((x), 1)
#define	osMotorStop(x)		__osMotorAccess((x), 0)
extern s32 __osMotorAccess(OSPfs *, s32);
s32 osMotorProbe(OSPfs* pfs, s32 channel);


#endif  /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_MOTOR_H_ */
