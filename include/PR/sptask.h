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
 *  $Revision: 1.9 $
 *  $Date: 1998/03/05 06:40:29 $
 *  $Source: /hosts/gate3/exdisk2/cvs/N64OS/Master/cvsmdev2/PR/include/sptask.h,v $
 *
 **************************************************************************/

#ifndef _SPTASK_H_
#define	_SPTASK_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

/**************************************************************************
 *
 * Type definitions
 *
 */

/*
 * Task List Structure.
 *
 * Things an app might pass to the SP via the task list.
 * Not every task ucode would need/use every field, but
 *
 *	- type (audio, gfx, video, ...)
 *	- flags
 *		- wait for DP to drain before running new task
 *		- SEE BIT DEFINITIONS UNDER "Task Flags field"
 *	- pointer to boot ucode
 *	- size of boot ucode
 *	- pointer to ucode
 *	- size of ucode
 *	- pointer to initial DMEM data
 *	- size of initial DMEM data
 *	- pointer to DRAM stack
 *	- size of DRAM stack (max)
 *	- pointer to output buffer
 *	- pointer to store output buffer length
 *	- generic data pointer (for display list, etc.)
 *	- generic data length (for display list, etc.)
 *	- pointer to buffer where to store saved DMEM (in yield case)
 *	- size of buffer to store saved DMEM.
 *
 * IMPORTANT!!! Watch alignment issues.
 *
 * IMPORTANT!!! Watch data cache issues.  The RCP may write data into the
 * dram_stack, output_buff, output_buff_size, and the yield_data_ptr areas.
 * These buffers should be cache aligned and use the entire line (16 bytes) to
 * avoid corruption by writebacks by the CPU (cache tearing).
 *
 * IMPORTANT!!! all addresses are virtual addresses. Library does
 * any necessary translation.
 *
 */
typedef struct {
	uint32_t	type;
	uint32_t	flags;

	uint64_t	*ucode_boot;
	uint32_t	ucode_boot_size;

	uint64_t	*ucode;
	uint32_t	ucode_size;

	uint64_t	*ucode_data;
	uint32_t	ucode_data_size;

	uint64_t	*dram_stack;
	uint32_t	dram_stack_size;

	uint64_t	*output_buff;
	uint64_t	*output_buff_size;

	uint64_t	*data_ptr;
	uint32_t	data_size;

	uint64_t	*yield_data_ptr;
	uint32_t	yield_data_size;

} OSTask_t;

typedef union {
    OSTask_t		t;
    long long int	force_structure_alignment;
} OSTask;

/**************************************************************************
 *
 * Global definitions
 *
 */



#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

#endif /* _LANGUAGE_C */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_SPTASK_H */
