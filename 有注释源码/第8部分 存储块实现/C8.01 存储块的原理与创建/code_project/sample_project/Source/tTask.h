/*************************************** Copyright (c)******************************************************
** File name            :   tTask.h
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   tinyOS的任务管理
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
#ifndef TTASK_H
#define TTASK_H

#define	TINYOS_TASK_STATE_RDY					0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define	TINYOS_TASK_STATE_DELAYED				(1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

#define TINYOS_TASK_WAIT_MASK                   (0xFF << 16)

// 前置声明
struct _tEvent;

// Cortex-M的堆栈单元类型：堆栈单元的大小为32位，所以使用uint32_t
typedef uint32_t tTaskStack;

// 任务结构：包含了一个任务的所有信息
typedef struct _tTask {
	// 任务所用堆栈的当前堆栈指针。每个任务都有他自己的堆栈，用于在运行过程中存储临时变量等一些环境参数
	// 在tinyOS运行该任务前，会从stack指向的位置处，会读取堆栈中的环境参数恢复到CPU寄存器中，然后开始运行
	// 在切换至其它任务时，会将当前CPU寄存器值保存到堆栈中，等待下一次运行该任务时再恢复。
	// stack保存了最后保存环境参数的地址位置，用于后续恢复
    uint32_t * stack;

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

    // 任务正在等待的事件类型
    struct _tEvent * waitEvent;

    // 等待事件的消息存储位置
    void * eventMsg;

    // 等待事件的结果
    uint32_t waitEventResult;

 }tTask;

// 任务相关信息结构
typedef struct _tTaskInfo {
    // 任务延时计数器
    uint32_t delayTicks;

    // 任务的优先级
    uint32_t prio;

    // 任务当前状态
    uint32_t state;

    // 当前剩余的时间片
    uint32_t slice;

    // 被挂起的次数
    uint32_t suspendCount;

 }tTaskInfo;

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
** Function name        :   tTaskGetInfo
** Descriptions         :   获取任务相关信息
** parameters           :   task 需要查询的任务
** parameters           :   info 任务信息存储结构
** Returned value       :   无
***********************************************************************************************************/
void tTaskGetInfo (tTask * task, tTaskInfo * info);

#endif /* TTASK_H */ 
