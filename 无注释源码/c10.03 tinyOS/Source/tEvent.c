#include "tinyOS.h"

void tEventInit (tEvent * event, tEventType type)
{
	event->type = tEventTypeUnknow;
	tListInit(&event->waitList);
}

void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout)
{
	uint32_t status = tTaskEnterCritical();
	
	task->state |= state << 16;
	task->waitEvent = event;
	task->eventMsg = msg;
	task->waitEventResult = tErrorNoError;
	
	tTaskSchedUnRdy(task);
	
	tListAddLast(&event->waitList, &task->linkNode);
	
	if (timeout)
	{
		tTimeTaskWait(task, timeout);
	}
	
	tTaskExitCritical(status);
}

tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	tTask * task = (tTask *)0;
	
	uint32_t status = tTaskEnterCritical();
	
	if ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
	{
		task = (tTask *)tNodeParent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		
		if (task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task);
		}
		
		tTaskSchedRdy(task);
	}
	
	tTaskExitCritical(status);
	
	return task;
}

tTask * tEventWakeUpTask (tEvent * event, tTask * task, void * msg, uint32_t result)
{	
	uint32_t status = tTaskEnterCritical();
	
	tListRemove(&event->waitList, &task->linkNode);
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->waitEventResult = result;
	task->state &= ~TINYOS_TASK_WAIT_MASK;
		
	if (task->delayTicks != 0)
	{
		tTimeTaskWakeUp(task);
	}
		
	tTaskSchedRdy(task);
	
	tTaskExitCritical(status);

	return task;
}

void tEventRemoveTask (tTask *task, void * msg, uint32_t result)
{
	uint32_t status = tTaskEnterCritical();
	
	tListRemove(&task->waitEvent->waitList, &task->linkNode);
	task->waitEvent = (tEvent *)0;
	task->eventMsg = msg;
	task->waitEventResult = result;
	task->state &= ~TINYOS_TASK_WAIT_MASK;

	tTaskExitCritical(status);
}

uint32_t tEventRemoveAll (tEvent * event, void * msg, uint32_t result)
{
	tNode * node;
	uint32_t count = 0;
	
	uint32_t status = tTaskEnterCritical();
	
	count = tListCount(&event->waitList);
	
	while ((node = tListRemoveFirst(&event->waitList)) != (tNode *)0)
	{
		tTask * task = (tTask *)tNodeParent(node, tTask, linkNode);
		task->waitEvent = (tEvent *)0;
		task->eventMsg = msg;
		task->waitEventResult = result;
		task->state &= ~TINYOS_TASK_WAIT_MASK;
		
		if (task->delayTicks != 0)
		{
			tTimeTaskWakeUp(task);
		}
		
		tTaskSchedRdy(task);
	}
	
	tTaskExitCritical(status);
	
	return count;
}

uint32_t tEventWaitCount (tEvent * event)
{
	uint32_t count = 0;
	
	uint32_t status = tTaskEnterCritical();
	count = tListCount(&event->waitList);
	tTaskExitCritical(status);
	
	return count;
}

