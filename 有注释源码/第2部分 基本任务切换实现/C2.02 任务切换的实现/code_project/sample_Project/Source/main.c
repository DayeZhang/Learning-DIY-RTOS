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

// 当前任务：记录当前是哪个任务正在运行
tTask * currentTask;

// 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
tTask * nextTask;

// 所有任务的指针数组：简单起见，只使用两个任务
tTask * taskTable[2];

/**********************************************************************************************************
** Function name        :   tTaskInit
** Descriptions         :   初始化任务结构
** parameters           :   task        要初始化的任务结构
** parameters           :   entry       任务的入口函数
** parameters           :   param       传递给任务的运行参数
** Returned value       :   无
***********************************************************************************************************/
void tTaskInit (tTask * task, void (*entry)(void *), void *param, uint32_t * stack)
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
}

/**********************************************************************************************************
** Function name        :   tTaskSched
** Descriptions         :   任务调度接口。tinyOS通过它来选择下一个具体的任务，然后切换至该任务运行。
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tTaskSched () 
{    
    // 这里的算法很简单。
    // 一共有两个任务。选择另一个任务，然后切换过去
    if (currentTask == taskTable[0]) 
    {
        nextTask = taskTable[1];
    }
    else 
    {
        nextTask = taskTable[0];
    }
    
    tTaskSwitch();
}

/**********************************************************************************************************
** 应用示例
** 有两个任务，分别执行task1Entry和task2Entry。功能是分别对相应的变量进行周期性置0置1，每次完成从1->0的切换后，
** 自动切换至另一个任务中运行。这样便实现了两个任务交替性的使用一段时间CPU，对相应变量值修改。
**********************************************************************************************************/
void delay (int count) 
{
    while (--count > 0);
}

int task1Flag;
void task1Entry (void * param) 
{
    for (;;) 
    {
        task1Flag = 1;
        delay(100);
        task1Flag = 0;
        delay(100);
        tTaskSched();
    }
}

int task2Flag;
void task2Entry (void * param) 
{
    for (;;) 
    {
        task2Flag = 1;
        delay(100);
        task2Flag = 0;
        delay(100);
        tTaskSched();
    }
}

// 任务1和任务2的任务结构，以及用于堆栈空间
tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];     
tTaskStack task2Env[1024];

int main () 
{
    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1Entry, (void *)0x11111111, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void *)0x22222222, &task2Env[1024]);
    
    // 接着，将任务加入到任务表中
    taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
    
    // 我们期望先运行tTask1, 也就是void task1Entry (void * param) 
    nextTask = taskTable[0];

    // 切换到nextTask， 这个函数永远不会返回
    tTaskRunFirst();
    return 0;
}
