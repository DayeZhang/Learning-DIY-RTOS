#include "tinyOS.h"

tTask * currentTask;
tTask * nextTask;
tTask * taskTable[2];

void tTaskInit (tTask * task, void (*entry)(void *), void * param, tTaskStack * stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)entry;
	*(--stack) = (unsigned long)0x14;
	*(--stack) = (unsigned long)0x12;
	*(--stack) = (unsigned long)0x3;
	*(--stack) = (unsigned long)0x2;
	*(--stack) = (unsigned long)0x1;
	*(--stack) = (unsigned long)param;
	
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x9;
	*(--stack) = (unsigned long)0x8;
	*(--stack) = (unsigned long)0x7;
	*(--stack) = (unsigned long)0x6;
	*(--stack) = (unsigned long)0x5;
	*(--stack) = (unsigned long)0x4;
	
	task->stack = stack;
}

void tTaskSched ()
{
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

void delay (int count)
{
	while (--count > 0);
}

tTask tTask1;
tTask tTask2;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

int task1Flag;
void task1Entry (void * param)
{
	unsigned long value = *(unsigned long *)param;
	value++;
	for (;;)
	{
		task1Flag = 0;
		delay(100);
		task1Flag = 1;
		delay(100);
		
		tTaskSched();
	}
}

int task2Flag;
void task2Entry (void * param)
{
	for (;;)
	{
		task2Flag = 0;
		delay(100);
		task2Flag = 1;
		delay(100);
		
		tTaskSched();
	}
}

int main ()
{
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, &task2Env[1024]);
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;
	
	nextTask = taskTable[0];
	
	tTaskRunFirst();
	
	return 0;
}
