/*************************************** Copyright (c)******************************************************
** File name            :   tTimer.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的软定时器实现
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
#include "tinyOS.h"
#include "tTimer.h"

// "硬"定时器列表
static tList tTimerHardList;

// "软"定时器列表
static tList tTimerSoftList;

// 用于访问软定时器列表的信号量
static tSem tTimerProtectSem;

// 用于软定时器任务与中断同步的计数信号量
static tSem tTimerTickSem;

/**********************************************************************************************************
** Function name        :   tTimerInit
** Descriptions         :   初始化定时器
** parameters           :   timer 等待初始化的定时器
** parameters           :   delayTicks 定时器初始启动的延时ticks数。
** parameters           :   durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
** parameters           :   timerFunc 定时器回调函数
** parameters           :   arg 传递给定时器回调函数的参数
** parameters           :   timerFunc 定时器回调函数
** parameters           :   config 定时器的初始配置
** Returned value       :   无
***********************************************************************************************************/
void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
    tNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;

    // 如果初始启动延时为0，则使用周期值
    if (delayTicks == 0)
    {
        timer->delayTicks = durationTicks;
    }
    else
    {
        timer->delayTicks = timer->startDelayTicks;
    }
    timer->state = tTimerCreated;
}

/**********************************************************************************************************
** Function name        :   tTimerStart
** Descriptions         :   启动定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerStart (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerCreated:
        case tTimerStopped:
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
            timer->state = tTimerStarted;

            // 根据定时器类型加入相应的定时器列表
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = tTaskEnterCritical();

                // 加入硬定时器列表
                tListAddLast(&tTimerHardList, &timer->linkNode);

                tTaskExitCritical(status);
            }
            else
            {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                tSemWait(&tTimerProtectSem, 0);
                tListAddLast(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
            break;
        default:
            break;
    }
}

/**********************************************************************************************************
** Function name        :   tTimerStop
** Descriptions         :   终止定时器
** parameters           :   timer 等待启动的定时器
** Returned value       :   无
***********************************************************************************************************/
void tTimerStop (tTimer * timer)
{
    switch (timer->state)
    {
        case tTimerStarted:
        case tTimerRunning:
            // 如果已经启动，判断定时器类型，然后从相应的延时列表中移除
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = tTaskEnterCritical();

                // 从硬定时器列表中移除
                tListRemove(&tTimerHardList, &timer->linkNode);

                tTaskExitCritical(status);
            }
            else
            {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                tSemWait(&tTimerProtectSem, 0);
                tListRemove(&tTimerSoftList, &timer->linkNode);
                tSemNotify(&tTimerProtectSem);
            }
            timer->state = tTimerStopped;
            break;
        default:
            break;
    }
}

/**********************************************************************************************************
** Function name        :   tTimerCallFuncList
** Descriptions         :   遍历指定的定时器列表，调用各个定时器处理函数
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static void tTimerCallFuncList (tList * timerList)
{
    tNode * node;
    
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNodeParent(node, tTimer, linkNode);

        // 如果延时已到，则调用定时器处理函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            // 切换为正在运行状态
            timer->state = tTimerRunning;

            // 调用定时器处理函数
            timer->timerFunc(timer->arg);

            // 切换为已经启动状态
            timer->state = tTimerStarted;

            if (timer->durationTicks > 0)
            {
                // 如果是周期性的，则重复延时计数值
                timer->delayTicks = timer->durationTicks;
            }
            else
            {
                // 否则，是一次性计数器，中止定时器
                tListRemove(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
}

/**********************************************************************************************************
** Function name        :   tTimerSoftTask
** Descriptions         :   处理软定时器列表的任务
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
static tTask tTimeTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerSoftTask (void * param)
{
    for (;;)
    {
        // 等待系统节拍发送的中断事件信号
        tSemWait(&tTimerTickSem, 0);

        // 获取软定时器列表的访问权限
        tSemWait(&tTimerProtectSem, 0);

        // 处理软定时器列表
        tTimerCallFuncList(&tTimerSoftList);

        // 释放定时器列表访问权限
        tSemNotify(&tTimerProtectSem);
    }
}

/**********************************************************************************************************
** Function name        :   tTimerModuleTickNotify
** Descriptions         :   通知定时模块，系统节拍tick增加
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimerModuleTickNotify (void)
{
    uint32_t status = tTaskEnterCritical();

    // 处理硬定时器列表
    tTimerCallFuncList(&tTimerHardList);

    tTaskExitCritical(status);

    // 通知软定时器节拍变化
    tSemNotify(&tTimerTickSem);
}

/**********************************************************************************************************
** Function name        :   tTimerModuleInit
** Descriptions         :   定时器模块初始化
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimerModuleInit (void)
{
    tListInit(&tTimerHardList);
    tListInit(&tTimerSoftList);
    tSemInit(&tTimerProtectSem, 1, 1);
    tSemInit(&tTimerTickSem, 0, 0);

#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
    #error "The proprity of timer task must be greater then (TINYOS_PRO_COUNT - 1)"
#endif
    tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0,
        TINYOS_TIMERTASK_PRIO, &tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]);
}
