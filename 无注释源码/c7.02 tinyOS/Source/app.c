#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

tMbox mbox1;
tMbox mbox2;
void * mbox1MsgBuffer[20];
void * mbox2MsgBuffer[20];

uint32_t msg[20];

int task1Flag;
void task1Entry (void * param)
{		
	tSetSysTickPeriod(10);
	
	tMboxInit(&mbox1, (void *)mbox1MsgBuffer, 20);
	for (;;)
	{	
		uint32_t i = 0;
		for (i = 0; i < 20; i++)
		{
			msg[i] = i;
			tMboxNotify(&mbox1, &msg[i], tMBOXSendNormal);
		}
		
		tTaskDelay(100);
		
		for (i = 0; i < 20; i++)
		{
			msg[i] = i;
			tMboxNotify(&mbox1, &msg[i], tMBOXSendFront);
		}
		
		tTaskDelay(100);
		
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
	}
}

int task2Flag;
void task2Entry (void * param)
{		
	for (;;)
	{			
		void * msg;
		
		uint32_t err = tMboxWait(&mbox1, &msg, 0);
		if (err == tErrorNoError)
		{
			uint32_t value = *(uint32_t *)msg;
			task2Flag = value;
			tTaskDelay(1);
		}
		
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
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
		
		task3Flag = 0;
		tTaskDelay(1);
		task3Flag = 1;
		tTaskDelay(1);
	}
}
int task4Flag;
void task4Entry (void * param)
{	
	for (;;)
	{		
		task4Flag = 0;
		tTaskDelay(1);
		task4Flag = 1;
		tTaskDelay(1);
	}
}

void tInitApp (void)
{
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
	tTaskInit(&tTask3, task3Entry, (void *)0x22222222, 1, &task3Env[1024]);	
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);	
}
