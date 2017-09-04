#include "tinyOS.h"

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
	uint32_t * ptrBit = (uint32_t *)arg;
	*ptrBit ^= 0x1;
}

int task1Flag;

void task1Entry (void * param)
{		
	uint32_t destroyed = 0;
	tTimerInfo timerInfo;
	
	tSetSysTickPeriod(10);
	
	tTimerInit(&timer1, 100, 10, timerFunc, (void *)&bit1, TIMER_CONFIG_TYPE_HARD);
	tTimerStart(&timer1);
	tTimerInit(&timer2, 200, 20, timerFunc, (void *)&bit2, TIMER_CONFIG_TYPE_HARD);
	tTimerStart(&timer2);
	tTimerInit(&timer3, 300, 0, timerFunc, (void *)&bit3, TIMER_CONFIG_TYPE_SOFT);
	tTimerStart(&timer3);
	for (;;)
	{	
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
		
		if (destroyed == 0)
		{
			tTaskDelay(200);
			tTimerDestroy(&timer1);
			destroyed = 1;
		}
		
		tTimerGetInfo(&timer2, &timerInfo);
	}
}

int task2Flag;
void task2Entry (void * param)
{					
	for (;;)
	{					
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
	}
}

int task3Flag;
void task3Entry (void * param)
{
	for (;;)
	{				
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
