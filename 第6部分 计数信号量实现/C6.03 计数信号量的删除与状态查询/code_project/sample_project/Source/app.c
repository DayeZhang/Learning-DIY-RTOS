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
** 有4个任务，演示了信号量的使用
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

tSem sem1;
tSem sem2;

int task1Flag;

void task1Entry (void * param) 
{
    uint32_t status = 0;

    tSetSysTickPeriod(10);

    // 最大10个信号量计数，初始化值为0
    tSemInit(&sem1, 0, 10);
    
    // 等待信号量，sem1将由task2删除后，才能继续恢复运行
    status = tSemWait(&sem1, 0);
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
    tSemInfo semInfo;
    int destroyed = 0;

    for (;;) 
    {
        task2Flag = 1;
        tTaskDelay(1);
        task2Flag = 0;
        tTaskDelay(1);
    
        // 删除信号量，将唤醒task1
        if (!destroyed) {
            tSemGetInfo(&sem1, &semInfo);
            tSemDestroy(&sem1);
            destroyed = 1;
        }
    }
}

int task3Flag;
void task3Entry (void * param) 
{
    // 不限信号量计数，初始值为0
    tSemInit(&sem2, 0, 0);

    for (;;) 
    {
        // 超时等待
        tSemWait(&sem2, 10);

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


