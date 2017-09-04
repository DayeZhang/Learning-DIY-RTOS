#include "tinyOS.h"

void tMutexInit (tMutex * mutex)
{
	tEventInit(&mutex->event, tEventTypeMutex);
	
	mutex->lockedCount = 0;
	mutex->owner = (tTask *)0;
	mutex->ownerOriginalPrio = TINYOS_PRO_COUNT;
}

uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOriginalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		if (mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		else
		{
			if (currentTask->prio < mutex->owner->prio)
			{
				tTask * owner = mutex->owner;
				if (owner->state == TINYOS_TASK_STATE_RDY)
				{
					tTaskSchedUnRdy(owner);
					owner->prio = currentTask->prio;
					tTaskSchedRdy(owner);
				}
				else
				{
					owner->prio = currentTask->prio;
				}
			}
			
			tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks);
			tTaskExitCritical(status);
			
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}

uint32_t tMutexNoWaitGet (tMutex * mutex)
{
	uint32_t stauts = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOriginalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(stauts);
		return tErrorNoError;
	}
	else
	{
		if (mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(stauts);
			return tErrorNoError;
		}
		
		tTaskExitCritical(stauts);
		return tErrorResourceUnavaliable;
	}
}

uint32_t tMutexNotify (tMutex * mutex)
{
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	if (mutex->owner != currentTask)
	{
		tTaskExitCritical(status);
		return tErrorOwner;
	}
	
	if (--mutex->lockedCount > 0)
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	if (mutex->ownerOriginalPrio != mutex->owner->prio)
	{
		if (mutex->owner->state == TINYOS_TASK_STATE_RDY)
		{
			tTaskSchedUnRdy(mutex->owner);
			currentTask->prio = mutex->ownerOriginalPrio;
			tTaskSchedRdy(mutex->owner);
		}
		else 
		{
			currentTask->prio = mutex->ownerOriginalPrio;
		}
	}
	
	if (tEventWaitCount(&mutex->event) > 0)
	{
		tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);
		
		mutex->owner = task;
		mutex->ownerOriginalPrio = task->prio;
		mutex->lockedCount++;
		
		if (task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return tErrorNoError;
	
	
}


