#include "tinyOS.h"

void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags)
{
	tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	flagGroup->flag = flags;
}

static uint32_t tFlagGroupCheckAndConsume (tFlagGroup * flagGroup, uint32_t type, uint32_t * flags)
{
	uint32_t srcFlag = *flags;
	uint32_t isSet = type & TFLAGGROUP_SET;
	uint32_t isAll = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;
	
	uint32_t calFlag = isSet ? (flagGroup->flag & srcFlag) : (~flagGroup->flag & srcFlag);
	
	if (((isAll != 0) && (calFlag == srcFlag)) || ((isAll == 0) && (calFlag != 0)))
	{
		if (isConsume)
		{
			if (isSet)
			{
				flagGroup->flag &= ~srcFlag;
			}
			else
			{
				flagGroup->flag |= srcFlag;
			}
		}
		
		*flags = calFlag;
		return tErrorNoError;
	}
	
	*flags = calFlag;
	return tErrorResourceUnavaliable;
}

uint32_t tFlagGroupWait (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag, uint32_t waitTicks)
{
	uint32_t result;
	uint32_t flags = requestFlag;
	uint32_t status = tTaskEnterCritical();
	
	result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	if (result != tErrorNoError)
	{
		currentTask->waitFlagsType = waitType;
		currentTask->eventFlags = requestFlag;
		tEventWait(&flagGroup->event, currentTask, (void *)0, tEventTypeFlagGroup, waitTicks);
		
		tTaskExitCritical(status);
		
		tTaskSched();
		
		*resultFlag = currentTask->eventFlags;
		result = currentTask->waitEventResult;
	}
	else
	{
		*resultFlag = flags;
		tTaskExitCritical(status);
	}
	return result;
}

uint32_t tFlagGroupNoWaitGet (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag)
{
	uint32_t flags = requestFlag;
	
	uint32_t status = tTaskEnterCritical();
	uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	tTaskExitCritical(status);
	
	*resultFlag = flags;
	return tErrorNoError;
}

void tFlagGroupNotify (tFlagGroup * flagGroup, uint8_t isSet, uint32_t flag)
{
	tList * waitList;
	tNode * node;
	tNode * nextNode;
	uint8_t sched = 0;
	uint32_t status = tTaskEnterCritical();
	
	if (isSet)
	{
		flagGroup->flag |= flag;
	}
	else
	{
		flagGroup->flag &= ~flag;
	}
	
	waitList = &flagGroup->event.waitList;
	for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode)
	{
		uint32_t result;
		tTask * task = tNodeParent(node, tTask, linkNode);
		uint32_t flags = task->eventFlags;
		nextNode = node->nextNode;
		
		result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
		if (result == tErrorNoError)
		{
			task->eventFlags = flags;
			tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
			sched = 1;
		}
	}
	
	if (sched)
	{
		tTaskSched();
	}
	
	tTaskExitCritical(status);
}

void tFlagGroupGetInfo (tFlagGroup * flagGroup, tFlagGroupInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->flags = flagGroup->flag;
	info->taskCount = tEventWaitCount(&flagGroup->event);
	
	tTaskExitCritical(status);
}

uint32_t tFlagGroupDestroy (tFlagGroup * flagGroup)
{
	uint32_t status = tTaskEnterCritical();
	uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);
	tTaskExitCritical(status);
	
	if (count > 0)
	{
		tTaskSched();
	}
	
	return count;
}

