#include <ultra64.h>
#include "constants.h"
#include "types.h"
#include "data.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/sched.h"

struct rdptask g_RdpTaskA = {
	{
		// OSScTask
		NULL,
		0,
		0,
		0,
		{
			// OSTask
			M_GFXTASK,               // type
			0x0002,         // flags
			NULL,
			0,
			NULL,
			4096,           // ucode_size
			NULL,
			2048,      // ucode_data_size
			NULL, // dram_stack
			1024,     // dram_stack_size
		}
	}
};

struct rdptask g_RdpTaskB = {
	{
		// OSScTask
		NULL,
		0,
		0,
		0,
		{
			// OSTask
			M_GFXTASK,               // type
			0x0002,         // flags
			NULL,
			0,
			NULL,
			4096,           // ucode_size
			NULL,
			2048,      // ucode_data_size
			NULL, // dram_stack
			1024,     // dram_stack_size
		}
	}
};

struct rdptask *g_RdpCurTask = &g_RdpTaskA;

void rdpCreateTask(Gfx *gdlstart, Gfx *gdlend, u32 arg2)
{
	OSScTask *sctask;
	OSTask *task;

	sctask = &g_RdpCurTask->sctask;
	task = &sctask->list;

	//task->t.output_buff = (u64 *)g_RdpOutBufferStart;
	//task->t.output_buff_size = (u64 *)g_RdpOutBufferEnd;
	task->t.data_ptr = (u64 *) gdlstart;
	task->t.data_size = (gdlend - gdlstart) * sizeof(Gfx);
	//task->t.yield_data_ptr = (u64 *)&g_RdpYieldData;
	//task->t.yield_data_size = sizeof(g_RdpYieldData);

	//sctask->next = NULL;
	//sctask->flags = OS_SC_NEEDS_RSP | OS_SC_SWAPBUFFER;
	//sctask->msgQ = &g_MainMesgQueue;
	//sctask->msg = (void *) msg;
	//sctask->framebuffer = g_RdpCurTask->framebuffer;

	// Used on PC port
	schedSubmitTask(sctask);

	// Swap g_RdpCurTask
	g_RdpCurTask = (struct rdptask *)((uintptr_t) g_RdpCurTask ^ (uintptr_t) &g_RdpTaskA ^ (uintptr_t) &g_RdpTaskB);
}
