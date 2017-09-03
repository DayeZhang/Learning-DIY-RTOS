/*************************************** Copyright (c)******************************************************
** File name            :   tinyOS.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的核心头文件。包含了所有关键数据类型的定义，还有核心的函数。
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
#ifndef TINYOS_H
#define TINYOS_H

// 标准头文件，里面包含了常用的类型定义，如uint32_t
#include <stdint.h>

// tinyOS的内核库文件
#include "tLib.h"

// TinyOS的配置文件
#include "tConfig.h"

#define TINYOS_TASK_STATE_RDY                   0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define TINYOS_TASK_STATE_DELAYED               (1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

// Cortex-M的堆栈单元类型：堆栈单元的大小为32位，所以使用uint32_t
typedef uint32_t tTaskStack;

// 任务结构：包含了一个任务的所有信息
typedef struct _tTask {
	// 任务所用堆栈的当前堆栈指针。每个任务都有他自己的堆栈，用于在运行过程中存储临时变量等一些环境参数
	// 在tinyOS运行该任务前，会从stack指向的位置处，会读取堆栈中的环境参数恢复到CPU寄存器中，然后开始运行
	// 在切换至其它任务时，会将当前CPU寄存器值保存到堆栈中，等待下一次运行该任务时再恢复。
	// stack保存了最后保存环境参数的地址位置，用于后续恢复
    tTaskStack * stack;

    // 连接结点
    tNode linkNode;

    // 任务延时计数器
    uint32_t delayTicks;

    // 延时结点：通过delayNode就可以将tTask放置到延时队列中
    tNode delayNode;

    // 任务的优先级
    uint32_t prio;

    // 任务当前状态
    uint32_t state;

    // 当前剩余的时间片
    uint32_t slice;

    // 被挂起的次数
    uint32_t suspendCount;

    // 任务被删除时调用的清理函数
    void (*clean) (void * param);

    // 传递给清理函数的参数
    void * cleanParam;

    // 请求删除标志，非0表示请求删除
    uint8_t requestDeleteFlag;

 }tTask;

// 当前任务：记录当前是哪个任务正在运行
extern tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern tTask * nextTask;

/**********************************************************************************************************
** Function name        :   tTaskEnterCritical
** Descriptions         :   进入临界区
** parameters           :   无
** Returned value       :   进入之前的临界区状态值
***********************************************************************************************************/
uint32_t tTaskEnterCritical (void);

/**********************************************************************************************************
** Function name        :   tTaskExitCritical
** Descriptions         :   退出临界区,恢复之前的临界区状态
** parameters           :   status 进入临界区之前的CPU
** Returned value       :   进入临界区之前的临界区状态值
***********************************************************************************************************/
void tTaskExitCritical (uint32_t status);

/**********************************************************************************************************
** Function name        :   tTaskRunFirst
** Descriptions         :   在启动tinyOS时，调用该函数，将切换至第一个任务运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskRunFirst (void); 

/**********************************************************************************************************
** Function name        :   tTaskSwitch
** Descriptions         :   进行一次任务切换，tinyOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至
**                          nextTask运行
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSwitch (void);

/**********************************************************************************************************
** Function name        :   tTaskHighestReady
** Descriptions         :   获取当前最高优先级且可运行的任务
** parameters           :   无
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
tTask * tTaskHighestReady (void);

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedDisable (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedEnable
** Descriptions         :   允许任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedEnable (void);

/**********************************************************************************************************
** Function name        :   tTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedRdy (tTask * task);

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    ÒªÒÆ³ýµÄÈÎÎñ¿é
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedUnRdy (tTask * task);

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedRemove
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    等待移除的任务
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedRemove (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSched (void);

/**********************************************************************************************************
** Function name        :   tTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWait (tTask * task, uint32_t ticks);

/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWakeUp (tTask * task);

/**********************************************************************************************************
** Function name        :   tTimeTaskRemove
** Descriptions         :   将延时的任务从延时队列中移除
** input parameters     :   task  需要移除的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskRemove (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskDelay
** Descriptions         :   使当前任务进入延时状态。
** parameters           :   delay 延时多少个ticks
** Returned value       :   无
***********************************************************************************************************/
void tTaskDelay (uint32_t delay);

/**********************************************************************************************************
** Function name        :   tTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSystemTickHandler (void);

/**********************************************************************************************************
** Function name        :   tTaskInit
** Descriptions         :   初始化任务结构
** parameters           :   task        要初始化的任务结构
** parameters           :   entry       任务的入口函数
** parameters           :   param       传递给任务的运行参数
** Returned value       :   无
***********************************************************************************************************/
void tTaskInit (tTask * task, void (*entry)(void *), void *param, uint32_t prio, uint32_t * stack);

/**********************************************************************************************************
** Function name        :   tTaskSuspend
** Descriptions         :   挂起指定的任务
** parameters           :   task        待挂起的任务
** Returned value       :   无
***********************************************************************************************************/
void tTaskSuspend (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskWakeUp
** Descriptions         :   唤醒被挂起的任务
** parameters           :   task        待唤醒的任务
** Returned value       :   无
***********************************************************************************************************/
void tTaskWakeUp (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskSetCleanCallFunc
** Descriptions         :   设置任务被删除时调用的清理函数
** parameters           :   task  待设置的任务
** parameters           :   clean  清理函数入口地址
** parameters           :   param  传递给清理函数的参数
** Returned value       :   无
***********************************************************************************************************/
void tTaskSetCleanCallFunc (tTask * task, void (*clean)(void * param), void * param);

/**********************************************************************************************************
** Function name        :   tTaskForceDelete
** Descriptions         :   强制删除指定的任务
** parameters           :   task  需要删除的任务
** Returned value       :   无
***********************************************************************************************************/
void tTaskForceDelete (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskRequestDelete
** Descriptions         :   请求删除某个任务，由任务自己决定是否删除自己
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskRequestDelete (tTask * task);

/**********************************************************************************************************
** Function name        :   tTaskIsRequestedDelete
** Descriptions         :   是否已经被请求删除自己
** parameters           :   task  需要删除的任务
** Returned value       :   非0表示请求删除，0表示无请求
***********************************************************************************************************/
uint8_t tTaskIsRequestedDelete (void);

/**********************************************************************************************************
** Function name        :   tTaskDeleteSelf
** Descriptions         :   删除自己
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskDeleteSelf (void);

/**********************************************************************************************************
** Function name        :   tInitApp
** Descriptions         :   初始化应用接口
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tInitApp (void);

/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms);

#endif /* TINYOS_H */
