#include "tinyOS.h"


void tTaskInit (tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack, uint32_t size)
{
	uint32_t * stackTop;
	
	task->stackBase = stack;
	task->stackSize = size;
	memset(stack, 0, size);
	
	stackTop = stack + size / sizeof(tTaskStack);
	
	*(--stackTop) = (unsigned long)(1 << 24);
	*(--stackTop) = (unsigned long)entry;
	*(--stackTop) = (unsigned long)0x14;
	*(--stackTop) = (unsigned long)0x12;
	*(--stackTop) = (unsigned long)0x3;
	*(--stackTop) = (unsigned long)0x2;
	*(--stackTop) = (unsigned long)0x1;
	*(--stackTop) = (unsigned long)param;
	
	*(--stackTop) = (unsigned long)0x11;
	*(--stackTop) = (unsigned long)0x10;
	*(--stackTop) = (unsigned long)0x9;
	*(--stackTop) = (unsigned long)0x8;
	*(--stackTop) = (unsigned long)0x7;
	*(--stackTop) = (unsigned long)0x6;
	*(--stackTop) = (unsigned long)0x5;
	*(--stackTop) = (unsigned long)0x4;
	
	task->slice = TINYOS_SLICE_MAX;
	task->stack = stackTop;
	task->delayTicks = 0;
	task->prio = prio;
	task->state = TINYOS_TASK_STATE_RDY;
	task->suspendCount = 0;
	task->clean = (void (*)(void *))0;
	task->cleanParam = (void *)0;
	task->requestDeleteFlag = 0;
	
	tNodeInit(&(task->delayNode));
	tNodeInit(&(task->linkNode));

	tTaskSchedRdy(task);
	
#if TINYOS_ENABLE_HOOKS == 1
	tHooksTaskInit(task);
#endif
}

void tTaskSuspend (tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	if (!(task->state & TINYOS_TASK_STATE_DELAYED))
	{
		if (++task->suspendCount <= 1)
		{
			task->state |= TINYOS_TASK_STATE_SUSPEND;
			tTaskSchedUnRdy(task);
			if (task == currentTask)
			{
				tTaskSched();
			}
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskWakeUp (tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	if (task->state & TINYOS_TASK_STATE_SUSPEND)
	{
		if (--task->suspendCount == 0)
		{
			task->state &= ~TINYOS_TASK_STATE_SUSPEND;
			tTaskSchedRdy(task);
			tTaskSched();
		}
	}
	
	tTaskExitCritical(status);
}

void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param)
{
	task->clean = clean;
	task->cleanParam = param;
}

void tTaskForceDelete (tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	if (task->state & TINYOS_TASK_STATE_DELAYED)
	{
		tTimeTaskRemove(task);
	}
	else if (!(task->state & TINYOS_TASK_STATE_SUSPEND))
	{
		tTaskSchedRemove(task);
	}
	
	if (task->clean)
	{
		task->clean(task->cleanParam);
	}
	
	if (currentTask == task)
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

void tTaskRequestDelete (tTask * task)
{
	uint32_t status = tTaskEnterCritical();
	
	task->requestDeleteFlag = 1;
	
	tTaskExitCritical(status);
}

uint8_t tTaskIsRequestedDeleted (void)
{
	uint8_t delete;
	
	uint32_t status = tTaskEnterCritical();
	
	delete = currentTask->requestDeleteFlag;
	
	tTaskExitCritical(status);
	
	return delete;
}

void tTaskDeleteSelf (void)
{
	uint32_t status = tTaskEnterCritical();
	
	tTaskSchedRemove(currentTask);
	
	if (currentTask->clean)
	{
		currentTask->clean(currentTask->cleanParam);
	}
	
	tTaskSched();
	
	tTaskExitCritical(status);
}

void tTaskGetInfo (tTask * task, tTaskInfo * info)
{
	uint32_t * stackEnd;
	uint32_t status = tTaskEnterCritical();
	
	info->delayTicks = task->delayTicks;
	info->prio = task->prio;
	info->state = task->state;
	info->slice = task->slice;
	info->suspendCount = task->suspendCount;
	info->stackSize = task->stackSize;
	
	info->stackFree = 0;
	stackEnd = task->stackBase;
	while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(tTaskStack)))
	{
		info->stackFree++;
	}
	info->stackFree *= sizeof(tTaskStack);
	
	tTaskExitCritical(status);
}


