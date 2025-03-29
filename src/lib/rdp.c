#include <ultra64.h>
#include "constants.h"
#include "types.h"
#include "data.h"
#include "bss.h"
#include "lib/memp.h"
#include "lib/sched.h"
#include "video.h"

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

void rdpCreateTask(Gfx *gdlstart, Gfx *gdlend)
{
	OSScTask *sctask;
	OSTask *task;

	sctask = &g_RdpCurTask->sctask;
	task = &sctask->list;

	task->t.data_ptr = (uint64_t *)gdlstart;
	task->t.data_size = (gdlend - gdlstart) * sizeof(Gfx);

	videoSubmitCommands((Gfx *)task->t.data_ptr);

	// Toggle between g_RdpTaskA and g_RdpTaskB using XOR trick
	g_RdpCurTask = (struct rdptask *)((uintptr_t)g_RdpCurTask ^ (uintptr_t)&g_RdpTaskA ^ (uintptr_t)&g_RdpTaskB);
}
