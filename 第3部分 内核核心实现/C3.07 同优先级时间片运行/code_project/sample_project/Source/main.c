/*************************************** Copyright (c)******************************************************
** File name            :   main.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   主文件，包含应用代码
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
#include "ARMCM3.h"

// 当前任务：记录当前是哪个任务正在运行
tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
tTask * nextTask;

// 空闲任务
tTask * idleTask;

// 任务优先级的标记位置结构
tBitmap taskPrioBitmap;

// 所有任务的指针数组：简单起见，只使用两个任务
tList taskTable[TINYOS_PRO_COUNT];

// 调度锁计数器
uint8_t schedLockCount;

// 延时队列
tList tTaskDelayedList;

/**********************************************************************************************************
** Function name        :   tTaskInit
** Descriptions         :   初始化任务结构
** parameters           :   task        要初始化的任务结构
** parameters           :   entry       任务的入口函数
** parameters           :   param       传递给任务的运行参数
** Returned value       :   无
***********************************************************************************************************/
void tTaskInit (tTask * task, void (*entry)(void *), void *param, uint32_t prio, uint32_t * stack)
{
    // 为了简化代码，tinyOS无论是在启动时切换至第一个任务，还是在运行过程中在不同间任务切换
    // 所执行的操作都是先保存当前任务的运行环境参数（CPU寄存器值）的堆栈中(如果已经运行运行起来的话)，然后再
    // 取出从下一个任务的堆栈中取出之前的运行环境参数，然后恢复到CPU寄存器
    // 对于切换至之前从没有运行过的任务，我们为它配置一个“虚假的”保存现场，然后使用该现场恢复。

    // 注意以下两点：
    // 1、不需要用到的寄存器，直接填了寄存器号，方便在IDE调试时查看效果；
    // 2、顺序不能变，要结合PendSV_Handler以及CPU对异常的处理流程来理解
    *(--stack) = (unsigned long)(1<<24);                // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
    *(--stack) = (unsigned long)entry;                  // 程序的入口地址
    *(--stack) = (unsigned long)0x14;                   // R14(LR), 任务不会通过return xxx结束自己，所以未用
    *(--stack) = (unsigned long)0x12;                   // R12, 未用
    *(--stack) = (unsigned long)0x3;                    // R3, 未用
    *(--stack) = (unsigned long)0x2;                    // R2, 未用
    *(--stack) = (unsigned long)0x1;                    // R1, 未用
    *(--stack) = (unsigned long)param;                  // R0 = param, 传给任务的入口函数
    *(--stack) = (unsigned long)0x11;                   // R11, 未用
    *(--stack) = (unsigned long)0x10;                   // R10, 未用
    *(--stack) = (unsigned long)0x9;                    // R9, 未用
    *(--stack) = (unsigned long)0x8;                    // R8, 未用
    *(--stack) = (unsigned long)0x7;                    // R7, 未用
    *(--stack) = (unsigned long)0x6;                    // R6, 未用
    *(--stack) = (unsigned long)0x5;                    // R5, 未用
    *(--stack) = (unsigned long)0x4;                    // R4, 未用

    task->slice = TINYOS_SLICE_MAX;                     // 初始化任务的时间片计数
    task->stack = stack;                                // 保存最终的值
    task->delayTicks = 0;
    task->prio = prio;                                  // 设置任务的优先级
    task->state = TINYOS_TASK_STATE_RDY;                // 设置任务为就绪状态

    tNodeInit(&(task->delayNode));                      // 初始化延时结点
    
    tNodeInit(&(task->linkNode));                       // 初始化链接结点
    tListAddLast(&taskTable[prio], &(task->linkNode));  // 插入对应的优先级队列中

    tBitmapSet(&taskPrioBitmap, prio);                  // 标记优先级位置中的相应位
}

/**********************************************************************************************************
** Function name        :   tTaskHighestReady
** Descriptions         :   获取当前最高优先级且可运行的任务
** parameters           :   无
** Returned value       :   优先级最高的且可运行的任务
***********************************************************************************************************/
tTask * tTaskHighestReady (void) 
{
    uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
    tNode * node = tListFirst(&taskTable[highestPrio]);
    return (tTask *)tNodeParent(node, tTask, linkNode);
}

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void)
{
    int i = 0;

    schedLockCount = 0;
    tBitmapInit(&taskPrioBitmap);
    for (i = 0; i < TINYOS_PRO_COUNT; i++)
    {
        tListInit(&taskTable[i]);
    }
}

/**********************************************************************************************************
** Function name        :   tTaskSchedDisable
** Descriptions         :   禁止任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedDisable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount < 255) 
    {
        schedLockCount++;
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskSchedEnable
** Descriptions         :   允许任务调度
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedEnable (void) 
{
    uint32_t status = tTaskEnterCritical();

    if (schedLockCount > 0) 
    {
        if (--schedLockCount == 0) 
        {
            tTaskSched(); 
        }
    }

    tTaskExitCritical(status);
}

/**********************************************************************************************************
** Function name        :   tTaskSchedRdy
** Descriptions         :   将任务设置为就绪状态
** input parameters     :   task    等待设置为就绪状态的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedRdy (tTask * task)
{
    tListAddLast(&taskTable[task->prio], &(task->linkNode));
    tBitmapSet(&taskPrioBitmap, task->prio);
}

/************************************************************************************************************ Function name        :   tSchedulerUnRdyTask
** Descriptions         :   tTaskSchedUnRdy
** Descriptions         :   将任务从就绪列表中移除
** input parameters     :   task    ÒªÒÆ³ýµÄÈÎÎñ¿é
** output parameters    :   None
** Returned value       :   None
***********************************************************************************************************/
void tTaskSchedUnRdy (tTask * task)
{
    tListRemove(&taskTable[task->prio], &(task->linkNode));

    // 队列中可能存在多个任务。只有当没有任务时，才清除位图标记
    if (tListCount(&taskTable[task->prio]) == 0)
    {
        tBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**********************************************************************************************************
** Function name        :   tTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSched (void) 
{   
    tTask * tempTask;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();

    // 如何调度器已经被上锁，则不进行调度，直接退bm
    if (schedLockCount > 0) 
    {
        tTaskExitCritical(status);
        return;
    }

    // 找到优先级最高的任务。这个任务的优先级可能比当前低低
    // 但是当前任务是因为延时才需要切换，所以必须切换过去，也就是说不能再通过判断优先级来决定是否切换
    // 只要判断不是当前任务，就立即切换过去
    tempTask = tTaskHighestReady();
    if (tempTask != currentTask) 
    {
        nextTask = tempTask;
        tTaskSwitch();   
    }

    // 退出临界区
    tTaskExitCritical(status); 
}

/**********************************************************************************************************
** Function name        :   tTaskDelayedInit
** Descriptions         :   初始化任务延时机制
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskDelayedInit (void) 
{
    tListInit(&tTaskDelayedList);
}

/**********************************************************************************************************
** Function name        :   tTimeTaskWait
** Descriptions         :   将任务加入延时队列中
** input parameters     :   task    需要延时的任务
**                          ticks   延时的ticks
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWait (tTask * task, uint32_t ticks)
{
    task->delayTicks = ticks;
    tListAddLast(&tTaskDelayedList, &(task->delayNode)); 
    task->state |= TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   tTimeTaskWakeUp
** Descriptions         :   将延时的任务从延时队列中唤醒
** input parameters     :   task  需要唤醒的任务
** output parameters    :   无
** Returned value       :   无
***********************************************************************************************************/
void tTimeTaskWakeUp (tTask * task)
{
    tListRemove(&tTaskDelayedList, &(task->delayNode));
    task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

/**********************************************************************************************************
** Function name        :   tTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSystemTickHandler (void)
{
    tNode * node;
    
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = tTaskEnterCritical();
    
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
    {
        tTask * task = tNodeParent(node, tTask, delayNode);
        if (--task->delayTicks == 0) 
        {
            // 将任务从延时队列中移除
            tTimeTaskWakeUp(task);

            // 将任务恢复到就绪状态
            tTaskSchedRdy(task);            
        }
    }

    // 检查下当前任务的时间片是否已经到了
    if (--currentTask->slice == 0)
    {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (tListCount(&taskTable[currentTask->prio]) > 0)
        {
            tListRemoveFirst(&taskTable[currentTask->prio]);
            tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));

            // 重置计数器
            currentTask->slice = TINYOS_SLICE_MAX;
        }
    }

     // 退出临界区
    tTaskExitCritical(status); 

    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    tTaskSched();
}

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

/*********************************************************************************************************
** 系统时钟节拍定时器System Tick配置
** 在我们目前的环境（模拟器）中，系统时钟节拍为12MHz
** 请务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
**********************************************************************************************************/
void tSetSysTickPeriod(uint32_t ms)
{
  SysTick->LOAD  = ms * SystemCoreClock / 1000 - 1; 
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
  SysTick->VAL   = 0;                           
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk; 
}

/**********************************************************************************************************
** Function name        :   SysTick_Handler
** Descriptions         :   SystemTick的中断处理函数。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void SysTick_Handler () 
{
    tTaskSystemTickHandler();
}

/**********************************************************************************************************
** 应用示例
** 有两个任务，分别执行task1Entry和task2Entry
** 其中task1Entry的优先级更高，只要它不处于延时状态，那么就运行它
** task2Entry和task3Entry相同优先级，按时间片切换运行，且在task1Entry延时时才能运行
**********************************************************************************************************/
int task1Flag;
void task1Entry (void * param) 
{
    tSetSysTickPeriod(10);

    for (;;) 
    {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);
    }
}

void delay ()
{
    int i;
    for (i = 0; i < 0xFF; i++) {}
}

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
        task2Flag = 1;
        delay();
        task2Flag = 0;
        delay();
    }
}

int task3Flag;
void task3Entry (void * param)
{
    for (;;)
    {
        task3Flag = 1;
        delay();
        task3Flag = 0;
        delay();
    }
}

// 任务1和任务2的任务结构，以及用于堆栈空间
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];

// 用于空闲任务的任务结构和堆栈空间
tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

void idleTaskEntry (void * param) {
    for (;;)
    {
        // 空闲任务什么都不做
    }
}

int main () 
{
    // 优先初始化tinyOS的核心功能
    tTaskSchedInit();

    // 初始化延时队列
    tTaskDelayedInit();

    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);

    // 创建空闲任务
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, &idleTaskEnv[1024]);
    
    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = tTaskHighestReady();

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
