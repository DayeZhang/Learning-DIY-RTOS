#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;
tTask * nextTask;
tTask * idleTask;

tBitmap taskPrioBitmap;
tList taskTable[TINYOS_PRO_COUNT];

uint8_t schedLockCount;

uint32_t tickCount;
tList tTaskDelayedList;

uint32_t idleCount;
uint32_t idleMaxCount;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1

static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);

#endif

tTask * tTaskHighestReady (void)
{
	uint32_t highestPrio = tBitmapGetFirstSet(&taskPrioBitmap);
	tNode * node = tListFirst(&taskTable[highestPrio]);
	return tNodeParent(node, tTask, linkNode);
}

void tTaskSchedInit (void)
{
	int i;
	
	schedLockCount = 0;
	tBitmapInit(&taskPrioBitmap);
	for (i = 0; i < TINYOS_PRO_COUNT; i++)
	{
		tListInit(&taskTable[i]);
	}
}

void tTaskSchedDisable (void)
{
	uint32_t status = tTaskEnterCritical();
	
	if (schedLockCount < 255)
	{
		schedLockCount++;
	}
	
	tTaskExitCritical(status);
}

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

void tTaskSchedRdy (tTask * task)
{
	tListAddFirst(&(taskTable[task->prio]), &(task->linkNode));
	tBitmapSet(&taskPrioBitmap, task->prio);
}

void tTaskSchedUnRdy (tTask * task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if (tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSchedRemove (tTask * task)
{
	tListRemove(&taskTable[task->prio], &(task->linkNode));
	if (tListCount(&taskTable[task->prio]) == 0)
	{
		tBitmapClear(&taskPrioBitmap, task->prio);
	}
}

void tTaskSched ()
{
	tTask * tempTask;
	uint32_t status = tTaskEnterCritical();
	
	if (schedLockCount > 0)
	{
		tTaskExitCritical(status);
		return;
	}
	
	tempTask = tTaskHighestReady();
	if (tempTask != currentTask)
	{
		nextTask = tempTask;
		tTaskSwitch();
	}
		
	tTaskExitCritical(status);
}

void tTaskDelayedInit (void)
{
	tListInit(&tTaskDelayedList);
}

void tTimeTaskWait (tTask * task, uint32_t ticks)
{
	task->delayTicks = ticks;
	tListAddLast(&tTaskDelayedList, &(task->delayNode));
	task->state |= TINYOS_TASK_STATE_DELAYED;
}

void tTimeTaskWakeUp (tTask * task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
	task->state &= ~TINYOS_TASK_STATE_DELAYED;
}

void tTimeTaskRemove (tTask * task)
{
	tListRemove(&tTaskDelayedList, &(task->delayNode));
}

void tTimeTickInit (void)
{
	tickCount = 0;
}

void tTaskSystemTickHandler ()
{
	tNode * node;
	
	uint32_t status = tTaskEnterCritical();
	
	for (node = tTaskDelayedList.headNode.nextNode; node != &(tTaskDelayedList.headNode); node = node->nextNode)
	{
		tTask * task = tNodeParent(node, tTask, delayNode);
		if (--task->delayTicks == 0)
		{
			if (task->waitEvent)
			{
				tEventRemoveTask(task, (void *)0, tErrorTimeout);
			}
			
			tTimeTaskWakeUp(task);
			
			tTaskSchedRdy(task);
		}
	}
	
	if (--currentTask->slice == 0)
	{
		if (tListCount(&taskTable[currentTask->prio]) > 0)
		{
			tListRemoveFirst(&taskTable[currentTask->prio]);
			tListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
			
			currentTask->slice = TINYOS_SLICE_MAX;
		}
	}
	
	tickCount++;

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	checkCpuUsage();
#endif
	
	tTaskExitCritical(status);
	
#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleTickNotify();
#endif
	
	tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
static float cpuUsage;
static uint32_t enableCpuUsageState;

static void initCpuUsageStat (void)
{
	idleCount = 0;
	idleMaxCount = 0;
	cpuUsage = 0.0f;
	enableCpuUsageState = 0;
}

static void checkCpuUsage (void)
{
	if (enableCpuUsageState == 0)
	{
		enableCpuUsageState = 1;
		tickCount = 0;
		return;
	}
	
	if (tickCount == TICKS_PER_SEC)
	{
		idleMaxCount = idleCount;
		idleCount = 0;
		
		tTaskSchedEnable();
	} 
	else if (tickCount % TICKS_PER_SEC == 0)
	{
		cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
		idleCount = 0;
	}
}

static void cpuUsageSyncWithSysTick (void)
{
	while (enableCpuUsageState == 0)
	{
		;;
	}
}

float tCpuUsageGet (void)
{
	float usage = 0;
	
	uint32_t status = tTaskEnterCritical();
	usage = cpuUsage;
	tTaskExitCritical(status);
	
	return usage;
}
#endif

tTask tTaskIdle;
tTaskStack idleTaskEnv[TINYOS_IDLETASK_STACK_SIZE];

void idleTaskEntry (void * param) {
	tTaskSchedDisable();
	
	tInitApp();
	
#if TINYOS_ENABLE_TIMER == 1
	tTimerInitTask();
#endif

	tSetSysTickPeriod(TINYOS_SYSTICK_MS);
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	cpuUsageSyncWithSysTick();
#endif
	
	for (;;)
	{
		uint32_t status = tTaskEnterCritical();
		idleCount++;
		tTaskExitCritical(status);
	}
}

int main ()
{
	tTaskSchedInit();
	
	tTaskDelayedInit();
	
#if TINYOS_ENABLE_TIMER == 1
	tTimerModuleInit();
#endif
	
	tTimeTickInit();
	
#if TINYOS_ENABLE_CPUUSAGE_STAT == 1
	initCpuUsageStat();
#endif
	
	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, TINYOS_PRO_COUNT - 1, idleTaskEnv, TINYOS_IDLETASK_STACK_SIZE);
	idleTask = &tTaskIdle;

	nextTask = tTaskHighestReady();
	
	tTaskRunFirst();
	
	return 0;
}
