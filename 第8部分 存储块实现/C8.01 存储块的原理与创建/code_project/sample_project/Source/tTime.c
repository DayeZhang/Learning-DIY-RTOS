/*************************************** Copyright (c)******************************************************
** File name            :   tTime.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的时间管理接口
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

/**********************************************************************************************************
** Function name        :   tTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void tTaskDelay (uint32_t delay) {
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();
 
    // 设置延时值，插入延时队列
    tTimeTaskWait(currentTask, delay);
 
    // 将任务从就绪表中移除
    tTaskSchedUnRdy(currentTask);

    // 然后进行任务切换，切换至另一个任务，或者空闲任务
    // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。
    tTaskSched();

    // 退出临界区
    tTaskExitCritical(status); 
}

