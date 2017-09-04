/*************************************** Copyright (c)******************************************************
** File name            :   tFlagGroup.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的事件标志实现
**
**--------------------------------------------------------------------------------------------------------
** Created by           :   01课堂 lishutong
** Created date         :   2016-06-01
** Version              :   1.0
** Descriptions         :   The original version
**
**--------------------------------------------------------------------------------------------------------
** Copyright            :   版权所有，禁止用于商业用途
** Author Blog          :   http://ilishutong.com
**********************************************************************************************************/
#include "tFlagGroup.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_FLAGGROUP == 1

/**********************************************************************************************************
** Function name        :   tFlagGroupInit
** Descriptions         :   初始化事件标志组
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   flags 初始的事件标志
** Returned value       :   无
***********************************************************************************************************/
void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags)
{
	tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	flagGroup->flags = flags;
}

/**********************************************************************************************************
** Function name        :   tFlagGroupCheckAndConsume
** Descriptions         :   辅助函数。检查并消耗掉事件标志
** parameters           :   flagGroup 等待初始化的事件标志组
** parameters           :   type 事件标志检查类型
** parameters           :   flags 待检查事件标志存储地址和检查结果存储位置
** Returned value       :   tErrorNoError 事件匹配；tErrorResourceUnavaliable 事件未匹配
***********************************************************************************************************/
static uint32_t tFlagGroupCheckAndConsume (tFlagGroup * flagGroup, uint32_t type, uint32_t * flags)
{
    uint32_t srcFlags = *flags;
	uint32_t isSet = type & TFLAGGROUP_SET;
	uint32_t isAll = type & TFLAGGROUP_ALL;
	uint32_t isConsume = type & TFLAGGROUP_CONSUME;

	// 有哪些类型的标志位出现
	// flagGroup->flags & flags：计算出哪些位为1
	// ~flagGroup->flags & flags:计算出哪位为0
	uint32_t calcFlag = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);

	// 所有标志位出现, 或者做任意标志位出现，满足条件
	if (((isAll != 0) && (calcFlag == srcFlags)) || ((isAll == 0) && (calcFlag != 0)))
	{
		// 是否消耗掉标志位
		if (isConsume)
		{
			if (isSet)
			{
				// 清除为1的标志位，变成0
				flagGroup->flags &= ~srcFlags;
			}
			else 
			{
				// 清除为0的标志位，变成1
				flagGroup->flags |= srcFlags;
			}
		}
		*flags = calcFlag;
		return tErrorNoError;
	}

	*flags = calcFlag;
	return tErrorResourceUnavaliable;
}

/**********************************************************************************************************
** Function name        :   tFlagGroupWait
** Descriptions         :   等待事件标志组中特定的标志
** parameters           :   flagGroup 等待的事件标志组
** parameters           :   waitType 等待的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** parameters           :   waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tFlagGroupWait (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag,
						uint32_t * resultFlag, uint32_t waitTicks)
{
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    if (result != tErrorNoError)
    {
	    // 如果事件标志不满足条件，则插入到等待队列中
		currentTask->waitFlagsType = waitType;
		currentTask->eventFlags = requestFlag;
		tEventWait(&flagGroup->event, currentTask, (void *)0,  tEventTypeFlagGroup, waitTicks);

        tTaskExitCritical(status);

		// 再执行一次事件调度，以便于切换到其它任务
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

/**********************************************************************************************************
** Function name        :   tFlagGroupNoWaitGet
** Descriptions         :   获取事件标志组中特定的标志
** parameters           :   flagGroup 获取的事件标志组
** parameters           :   waitType 获取的事件类型
** parameters           :   requstFlag 请求的事件标志
** parameters           :   resultFlag 等待标志结果
** Returned value       :   获取结果,tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tFlagGroupNoWaitGet (tFlagGroup * flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t * resultFlag)
{
    uint32_t flags = requestFlag;

    uint32_t status = tTaskEnterCritical();
    uint32_t result = tFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
	tTaskExitCritical(status);

	*resultFlag = flags;
	return status;
}

/**********************************************************************************************************
** Function name        :   tFlagGroupNotify
** Descriptions         :   通知事件标志组中的任务有新的标志发生
** parameters           :   flagGroup 事件标志组
** parameters           :   isSet 是否是设置事件标志
** parameters           :   flags 产生的事件标志
***********************************************************************************************************/
void tFlagGroupNotify (tFlagGroup * flagGroup, uint8_t isSet, uint32_t flags)
{
    tList *waitList;
    tNode * node;
    tNode * nextNode;
    uint8_t sched = 0;

    uint32_t status = tTaskEnterCritical();

    if (isSet) {
        flagGroup->flags |= flags;     // 置1事件
    } else {
        flagGroup->flags &= ~flags;    // 清0事件
    }

    // 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
    waitList = &flagGroup->event.waitList;
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode) {
        uint32_t result;
        tTask *task = tNodeParent(node, tTask, linkNode);
        uint32_t flags = task->eventFlags;
        nextNode = node->nextNode;

        // 检查标志
        result = tFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        if (result == tErrorNoError) {
            // 唤醒任务
            task->eventFlags = flags;
            tEventWakeUpTask(&flagGroup->event, task, (void *)0, tErrorNoError);
            sched = 1;
        }
    }

    // 如果有任务就绪，则执行一次调度
    if (sched)
    {
        tTaskSched();
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tFlagGroupGetInfo
** Descriptions         :   查询事件标志组的状态信息
** parameters           :   flagGroup 事件标志组
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tFlagGroupGetInfo (tFlagGroup * flagGroup, tFlagGroupInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->flags = flagGroup->flags;
    info->taskCount = tEventWaitCount(&flagGroup->event);

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tFlagGroupDestroy
** Descriptions         :   销毁事件标志组
** parameters           :   flagGroup 事件标志组
** Returned value       :   因销毁该存储控制块而唤醒的任务数量
***********************************************************************************************************/
uint32_t tFlagGroupDestroy (tFlagGroup * flagGroup)
{
    uint32_t status = tTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&flagGroup->event, (void *)0, tErrorDel);

    tTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0)
    {
        tTaskSched();
    }
    return count;
}

#endif
















