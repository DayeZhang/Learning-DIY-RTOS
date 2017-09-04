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

#define NVIC_INT_CTRL       0xE000ED04      // 中断控制及状态寄存器
#define NVIC_PENDSVSET      0x10000000      // 触发软件中断的值
#define NVIC_SYSPRI2        0xE000ED22      // 系统优先级寄存器
#define NVIC_PENDSV_PRI     0x000000FF      // 配置优先级

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)

void triggerPendSVC (void)
{
    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;   // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为最低优先级
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}

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
    task->stack = stack;                                // 保存最终的值
}

typedef struct _BlockType_t
{
    unsigned long * stackPtr;
}BlockType_t;

BlockType_t * blockPtr;

void delay (int count)
{
    while (--count > 0);
}

int flag;

unsigned long stackBuffer[1024];
BlockType_t block;

// 任务1和任务2的任务结构，以及用于堆栈空间
tTask tTask1;
tTask tTask2;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

void task1 (void * param)
{
    for (;;)
    {
    }
}

void task2 (void * param)
{
    for (;;)
    {
    }
}

int main ()
{
    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1, (void *)0x11111111, &task1Env[1024]);
    tTaskInit(&tTask2, task2, (void *)0x22222222, &task2Env[1024]);

    block.stackPtr = &stackBuffer[1024];
    blockPtr = &block;

    for (;;) {
        flag = 0;
        delay(100);
        flag = 1;
        delay(100);

        triggerPendSVC();
    }
}