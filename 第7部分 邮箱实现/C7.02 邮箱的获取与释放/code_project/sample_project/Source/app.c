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
** 有4个任务，i演示了邮箱的等待与恢复
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

int task1Flag;

tMbox mbox1;
tMbox mbox2;
void * mbox1MsgBuffer[20];
void * mbox2MsgBuffer[20];
uint32_t msg[20];

void task1Entry (void * param) 
{
    tSetSysTickPeriod(10);

    // 初始化邮箱
    tMboxInit(&mbox1, mbox1MsgBuffer, 20);

    for (;;) 
    {
        // 依次加入邮箱中
        uint32_t i = 0;
        for (i = 0; i < 20; i++) 
        {
            msg[i] = i;
            tMboxNotify(&mbox1, &msg[i], tMBOXSendNormal);
        }
        tTaskDelay(100);

        // 后发的消息具有更高优先级
        // 也许你会期望task2~task3得到的消息值会从19/18/...1递减
        // 但是如果队列中已经存在等待任务的话，每发一次消息，都会消耗掉该消息
        // 导致最开始的顺序会有所变化
        for (i = 0; i < 20; i++) 
        {
            msg[i] = i;
            tMboxNotify(&mbox1, &msg[i], tMBOXSendFront);
        }   
        tTaskDelay(100);

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
        void * msg;
        uint32_t err = tMboxWait(&mbox1, &msg, 10);
        if (err == tErrorNoError) 
        {
            uint32_t value = *(uint32_t*)msg;
            task2Flag = value;
            tTaskDelay(1);
        }
    }
}

int task3Flag;
void task3Entry (void * param) 
{
    tMboxInit(&mbox2, mbox2MsgBuffer, 20);
    for (;;) 
    {
        void * msg;
        tMboxWait(&mbox2, &msg, 100);

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


