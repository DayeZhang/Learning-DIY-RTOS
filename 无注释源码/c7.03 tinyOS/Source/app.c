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
void * mbox1MsgBuffer[20];
uint32_t msg1[20];

tMbox mbox2;
void * mbox2MsgBuffer[20];
uint32_t msg2[20];

int task1Flag;
void task1Entry (void * param)
{		
	void * msg;
	
	tSetSysTickPeriod(10);
	
	tMboxInit(&mbox1, mbox1MsgBuffer, 20);
	tMboxInit(&mbox2, mbox2MsgBuffer, 20);
	
	tMboxWait(&mbox1, (void *)msg, 0);
	for (;;)
	{	
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
	}
}

int task2Flag;
void task2Entry (void * param)
{		
	int destroyed = 0;
	
	for (;;)
	{			
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
		
		if (!destroyed)
		{
			tMboxDestroy(&mbox1);
			destroyed = 1;
		}
	}
}

int task3Flag;
void task3Entry (void * param)
{
	for (;;)
	{				
		int i = 0;
		for (i = 0; i < 20; i++)
		{
			msg2[i] = i;
			tMboxNotify(&mbox2, &msg2[i], tMBOXSendNormal);
		}
		
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
		uint32_t * msg;
		
		tMboxWait(&mbox2, (void *)&msg, 0);
		task4Flag = *msg;
		
		tMboxFlush(&mbox2);
	}
}

void tInitApp (void)
{
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, &task2Env[1024]);
	tTaskInit(&tTask3, task3Entry, (void *)0x22222222, 1, &task3Env[1024]);	
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, &task4Env[1024]);	
}
