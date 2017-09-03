/*************************************** Copyright (c)******************************************************
** File name            :   app.c
** Latest modified Date :   2016-06-01
** Latest Version       :   0.1
** Descriptions         :   应用代码
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
** 应用示例
** 有4个任务，i演示了软定时器的启动与停止
**********************************************************************************************************/
// 任务1和任务2的任务结构，以及用于堆栈空间
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

tTimer timer1;
tTimer timer2;
tTimer timer3;
uint32_t bit1 = 0;
uint32_t bit2 = 0;
uint32_t bit3 = 0;

void timerFunc (void * arg)
{
    // 简单的将最低位取反，输出高低翻转的信号
    uint32_t * ptrBit = (uint32_t *)arg;
    *ptrBit ^= 0x1;
}

int task1Flag;
void task1Entry (void * param)
{
    uint32_t stopped = 0;

    tSetSysTickPeriod(10);

    // 定时器1：100个tick后启动，以后每10个tick启动一次
    tTimerInit(&timer1, 100, 10, timerFunc, (void *)&bit1, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer1);

    // 定时器2：200个tick后启动，以后每20个tick启动一次
    tTimerInit(&timer2, 200, 20, timerFunc, (void *)&bit2, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer2);

    // 定时器1：300个tick后启动，启动之后关闭
    tTimerInit(&timer3, 300, 0, timerFunc, (void *)&bit3, TIMER_CONFIG_TYPE_HARD);
    tTimerStart(&timer3);
    for (;;)
    {
        task1Flag = 1;
        tTaskDelay(1);
        task1Flag = 0;
        tTaskDelay(1);

        // 200个tick后，手动关闭定时器1
        if (stopped == 0)
        {
            tTaskDelay(200);
            tTimerStop(&timer1);
            stopped = 1;
        }
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

int task3Flag;
void task3Entry (void * param) 
{
	for (;;)
    {
        task3Flag = 1;
        tTaskDelay(1);
        task3Flag = 0;
        tTaskDelay(1);
    }
}

int task4Flag;
void task4Entry (void * param) 
{
    for (;;) 
    {
        task4Flag = 1;
        tTaskDelay(1);
        task4Flag = 0;
        tTaskDelay(1);
    }
}

/**********************************************************************************************************
** Function name        :   tInitApp
** Descriptions         :   初始化应用接口
** parameters           :   无
** Returned value       :   无
***********************************************************************************************************/
void tInitApp (void) 
{
    // 初始化任务1和任务2结构，传递运行的起始地址，想要给任意参数，以及运行堆栈空间
    tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3, task3Entry, (void *)0x33333333, 1, &task3Env[1024]);
    tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);
}


