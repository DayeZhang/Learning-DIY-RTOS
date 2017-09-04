/*************************************** Copyright (c)******************************************************
** File name            :   tMBox.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的事件控制结构实现
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
#include "tMBox.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_MBOX == 1

/**********************************************************************************************************
** Function name        :   tMboxInit
** Descriptions         :   初始化邮箱
** parameters           :   mbox 等待初始化的邮箱
** parameters           :   msgBuffer 消息存储缓冲区
** parameters           :   maxCount 最大计数
** Returned value       :   无
***********************************************************************************************************/
void tMboxInit (tMbox * mbox, void ** msgBuffer, uint32_t maxCount) 
{
	tEventInit(&mbox->event, tEventTypeMbox);

	mbox->msgBuffer = msgBuffer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
}

/**********************************************************************************************************
** Function name        :   tMboxWait
** Descriptions         :   等待邮箱, 获取一则消息
** parameters           :   mbox 等待的邮箱
** parameters           :   msg 消息存储缓存区
** parameters           :   waitTicks 最大等待的ticks数，为0表示无限等待
** Returned value       :   等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
***********************************************************************************************************/
uint32_t tMboxWait (tMbox * mbox, void **msg, uint32_t waitTicks) 
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
    	// 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount) 
        {
        	mbox->read = 0;
        }
    	tTaskExitCritical(status); 
    	return tErrorNoError;
    }
    else                                                                
    { 
         // 然后将任务插入事件队列中
        tEventWait(&mbox->event, currentTask, (void *)0,  tEventTypeMbox, waitTicks);
        tTaskExitCritical(status);
        
        // 最后再执行一次事件调度，以便于切换到其它任务
        tTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *msg = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**********************************************************************************************************
** Function name        :   tMboxNoWaitGet
** Descriptions         :   获取一则消息，如果没有消息，则立即退回
** parameters           :   mbox 获取消息的邮箱
** parameters           :   msg 消息存储缓存区
** Returned value       :   获取结果, tErrorResourceUnavaliable.tErrorNoError
***********************************************************************************************************/
uint32_t tMboxNoWaitGet (tMbox * mbox, void **msg)
{
    uint32_t status = tTaskEnterCritical();

    // 首先检查消息计数是否大于0
    if (mbox->count > 0)
    {
    	// 如果大于0的话，取出一个
        --mbox->count;
        *msg = mbox->msgBuffer[mbox->read++];

        // 同时读取索引前移，如果超出边界则回绕
        if (mbox->read >= mbox->maxCount) 
        {
        	mbox->read = 0;
        }
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
** Function name        :   tMboxNotify
** Descriptions         :   通知消息可用，唤醒等待队列中的一个任务，或者将消息插入到邮箱中
** parameters           :   mbox 操作的信号量
** parameters           :   msg 发送的消息
** parameters           :   notifyOption 发送的选项
** Returned value       :   tErrorResourceFull
***********************************************************************************************************/
uint32_t tMboxNotify (tMbox * mbox, void * msg, uint32_t notifyOption)
{
    uint32_t status = tTaskEnterCritical();        
    
    // 检查是否有任务等待
    if (tEventWaitCount(&mbox->event) > 0)
    {
    	// 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        tTask * task = tEventWakeUp(&mbox->event, (void *)msg, tErrorNoError );
        
        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio)
        {
            tTaskSched(); 
    	}
    }
    else
    {
    	// 如果没有任务等待的话，将消息插入到缓冲区中
    	if (mbox->count >= mbox->maxCount) 
    	{
		    tTaskExitCritical(status);
		    return tErrorResourceFull;
    	}

		// 可以选择将消息插入到头，这样后面任务获取的消息的时候，优先获取该消息
		if (notifyOption & tMBOXSendFront)
		{
			if (mbox->read <= 0) 
			{
				mbox->read = mbox->maxCount - 1;
			} 
			else 
			{
				--mbox->read;
			}
			mbox->msgBuffer[mbox->read] = msg;
		}
		else 
		{
			mbox->msgBuffer[mbox->write++] = msg;
			if (mbox->write >= mbox->maxCount)
			{
				mbox->write = 0;
			}
		}

		// 增加消息计数
		mbox->count++;
   	}

	tTaskExitCritical(status);
	return tErrorNoError;
}

/**********************************************************************************************************
** Function name        :   tMboxFlush
** Descriptions         :   清空邮箱中所有消息
** parameters           :   mbox 等待清空的邮箱
** Returned value       :   无
***********************************************************************************************************/
void tMboxFlush (tMbox * mbox)
{
    uint32_t status = tTaskEnterCritical();        
    
    // 如果队列中有任务等待，说明邮箱已经是空的了，不需要再清空
    if (tEventWaitCount(&mbox->event) == 0) 
    {
        mbox->read = 0;
        mbox->write = 0;
        mbox->count = 0;
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tMboxDestroy
** Descriptions         :   销毁邮箱
** parameters           :   mbox 需要销毁的邮箱
** Returned value       :   因销毁该信号量而唤醒的任务数量
***********************************************************************************************************/
uint32_t tMboxDestroy (tMbox * mbox)
{       
    uint32_t status = tTaskEnterCritical(); 

    // 清空事件控制块中的任务
    uint32_t count = tEventRemoveAll(&mbox->event, (void *)0, tErrorDel);  
    
    tTaskExitCritical(status);
    
    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) 
    {
        tTaskSched();
    } 
    return count;  
}

/**********************************************************************************************************
** Function name        :   tMboxGetInfo
** Descriptions         :   查询状态信息
** parameters           :   mbox 查询的邮箱
** parameters           :   info 状态查询存储的位置
** Returned value       :   无
***********************************************************************************************************/
void tMboxGetInfo (tMbox * mbox, tMboxInfo * info)
{
    uint32_t status = tTaskEnterCritical();

    // 拷贝需要的信息
    info->count = mbox->count;
    info->maxCount = mbox->maxCount;
    info->taskCount = tEventWaitCount(&mbox->event);

    tTaskExitCritical(status);
}

#endif



