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
			1,               // type
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
			1,               // type
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

void rdpCreateTask(Gfx *gdlstart, Gfx *gdlend, uint32_t arg2)
{
	OSTask *task;

	task = &g_RdpCurTask->sctask.list;

	task->t.data_ptr = (uint64_t *) gdlstart;
	task->t.data_size = (gdlend - gdlstart) * sizeof(Gfx);

	if (gdlstart && gdlend && gdlend > gdlstart) {
		videoSubmitCommands((Gfx *)task->t.data_ptr);
	}

	// Swap g_RdpCurTask
	g_RdpCurTask = (struct rdptask *)((uintptr_t) g_RdpCurTask ^ (uintptr_t) &g_RdpTaskA ^ (uintptr_t) &g_RdpTaskB);
}
