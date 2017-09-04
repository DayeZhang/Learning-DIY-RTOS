#include "tinyOS.h"


void tTaskInit (tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x3;
	*(--stack) = (unsigned long)0x2;
	*(--stack) = (unsigned long)0x1;
	*(--stack) = (unsigned long)param;
	
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x9;
	*(--stack) = (unsigned long)0x8;
	*(--stack) = (unsigned long)0x7;
	*(--stack) = (unsigned long)0x6;
	*(--stack) = (unsigned long)0x5;
	*(--stack) = (unsigned long)0x4;
	
	task->slice = TINYOS_SLICE_MAX;
	task->stack = stack;
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
	uint32_t status = tTaskEnterCritical();
	
	info->delayTicks = task->delayTicks;
	info->prio = task->prio;
	info->state = task->state;
	info->slice = task->slice;
	info->suspendCount = task->suspendCount;
	
	tTaskExitCritical(status);
}


