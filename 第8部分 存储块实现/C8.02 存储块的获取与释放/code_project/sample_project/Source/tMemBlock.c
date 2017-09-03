/*************************************** Copyright (c)******************************************************
** File name            :   tMemBlock.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的存储块的实现
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
#include "tMemBlock.h"

#include "tinyOS.h"

/**********************************************************************************************************
** Function name        :   tMemBlockInit
** Descriptions         :   初始化存储控制块
** parameters           :   memBlock 等待初始化的存储控制块
** parameters           :   memStart 存储区的起始地址
** parameters           :   blockSize 每个块的大小
** parameters           :   blockCnt 总的块数量
** Returned value       :   唤醒的任务数量
***********************************************************************************************************/
void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)
{
	uint8_t * memBlockStart = (uint8_t *)memStart;
	uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;

	// 每个存储块需要来放置链接指针，所以空间至少要比tNode大
	// 即便如此，实际用户可用的空间并没有少
	if (blockSize < sizeof(tNode))
	{
		return;
	}

	tEventInit(&memBlock->event, tEventTypeMemBlock);

	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;

	tListInit(&memBlock->blockList);
	while (memBlockStart < memBlockEnd)
	{
		tNodeInit((tNode *)memBlockStart);
		tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);

		memBlockStart += blockSize;
	}
}

/**********************************************************************************************************
** Function name        :   tMemBlockWait
** Descriptions         :   等待存储块
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** parameters           :   waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t ** mem, uint32_t waitTicks)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tListCount(&memBlock->blockList) > 0)
    {
    	// 如果有的话，取出一个
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
    	tTaskExitCritical(status);
    	return tErrorNoError;
    }
    else
    {
         // 然后将任务插入事件队列中
        tEventWait(&memBlock->event, currentTask, (void *)0,  tEventTypeMemBlock, waitTicks);
        tTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *mem = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   tMemBlockNoWaitGet
** Descriptions         :   获取存储块，如果没有存储块，则立即退回
** parameters           :   memBlock 等待的存储块
** parameters			:   mem 存储块存储的地址
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (tListCount(&memBlock->blockList) > 0)
    {
    	// 如果有的话，取出一个
        *mem = (uint8_t *)tListRemoveFirst(&memBlock->blockList);
    	tTaskExitCritical(status);
    	return tErrorNoError;
    }
    else
    {
     	// 否则，返回资源不可用
        tTaskExitCritical(status);
    	return tErrorResourceUnavaliable;
    }
}

/**********************************************************************************************************
** Function name        :   tMemBlockNotify
** Descriptions         :   通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
** parameters           :   memBlock 操作的信号量
** Returned value       :   无
***********************************************************************************************************/
void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem)
{
    uint32_t status = tTaskEnterCritical();

    // 检查是否有任务等待
    if (tEventWaitCount(&memBlock->event) > 0)
    {
    	// 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask * task = tEventWakeUp(&memBlock->event, (void *)mem, tErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched();
    	}
    }
    else
    {
    	// 如果没有任务等待的话，将存储块插入到队列中
    	tListAddLast(&memBlock->blockList, (tNode *)mem);
   	}

	tTaskExitCritical(status);
}
