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
tTask * taskTable[TINYOS_PRO_COUNT];

// 调度锁计数器
uint8_t schedLockCount;

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

    task->stack = stack;                                // 保存最终的值
    task->delayTicks = 0;
    task->prio = prio;                                  // 设置任务的优先级

    taskTable[prio] = task;                             // 填入任务优先级表
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
    return taskTable[highestPrio];
}

/**********************************************************************************************************
** Function name        :   初始化调度器
** Descriptions         :   无
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSchedInit (void)
{
    schedLockCount = 0;
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

    // 找到优先级最高的任务，如果其优先级比当前任务的还高，那么就切换到这个任务
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
** Function name        :   tTaskSystemTickHandler
** Descriptions         :   系统时钟节拍处理。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSystemTickHandler () 
{
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    int i;   
    uint32_t status = tTaskEnterCritical();

    for (i = 0; i < TINYOS_PRO_COUNT; i++) 
    {
        if (taskTable[i] == (tTask*)0) {
            continue;
        }

        if (taskTable[i]->delayTicks > 0)
        {
            taskTable[i]->delayTicks--;
        }
        else 
        {
            tBitmapSet(&taskPrioBitmap, i);
        }
    }
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
   // 配置好当前要延时的ticks数
    uint32_t status = tTaskEnterCritical();
    currentTask->delayTicks = delay;
    tBitmapClear(&taskPrioBitmap, currentTask->prio);
    tTaskExitCritical(status);

    // 然后进行任务切换，切换至另一个任务，或者空闲任务
    // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。
    tTaskSched();
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
** 有两个任务，分别执行task1Entry和task2Entry。功能是分别对相应的变量进行周期性置0置1.
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

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
        task2Flag = 1;
        tTaskDelay(1);
        task2Flag = 0;
        tTaskDelay(1);
    }
}

// 任务1和任务2的任务结构，以及用于堆栈空间
tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

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

    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);

    // 创建空闲任务
    tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, &idleTaskEnv[1024]);
    
    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = tTaskHighestReady();

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
