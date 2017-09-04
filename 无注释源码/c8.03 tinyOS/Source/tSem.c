#include "tSem.h"
#include "tinyOS.h"

void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount)
{
	tEventInit(&sem->event, tEventTypeSem);
	
	sem->maxCount = maxCount;
	if (maxCount == 0)
	{
		sem->count = startCount;
	}
	else 
	{
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
}

uint32_t tSemWait (tSem * sem, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	
	if (sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		tEventWait(&sem->event, currentTask, (void *)0, tEventTypeSem, waitTicks);
		tTaskExitCritical(status);
		
		tTaskSched();
		
		return currentTask->waitEventResult;
	}
}

uint32_t tSemNoWaitGet (tSem * sem)
{
	uint32_t status = tTaskEnterCritical();
	
	if (sem->count > 0)
	{
		--sem->count;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else 
	{
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}


void tSemNotify (tSem * sem)
{
	uint32_t status = tTaskEnterCritical();
	
	if (tEventWaitCount(&sem->event) > 0)
	{
		tTask * task = tEventWakeUp(&sem->event, (void *)0, tErrorNoError);
		if (task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	else
	{
		++sem->count;
		if ((sem->maxCount != 0) && (sem->count > sem->maxCount))
		{
			sem->count = sem->maxCount;
		}
	}
	tTaskExitCritical(status);
}

void tSemGetInfo (tSem * sem, tSemInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->count = sem->count;
	info->maxCount = sem->maxCount;
	info->taskCount = tEventWaitCount(&sem->event);
	
	tTaskExitCritical(status);
}

uint32_t tSemDestroy (tSem * sem)
{
	uint32_t status = tTaskEnterCritical();
	uint32_t count = tEventRemoveAll(&sem->event, (void *)0, tErrorDel);
	sem->count = 0;
	tTaskExitCritical(status);
	
	if (count > 0)
	{
		tTaskSched();
	}
	
	return count;
}

