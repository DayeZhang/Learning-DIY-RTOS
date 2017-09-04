#include "tinyOS.h"


static tList tTimerHardList;
static tList tTimerSoftList;
static tSem tTimerProtectSem;
static tSem tTimerTickSem;

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
		void (*timerFunc) (void * arg), void * arg, uint32_t config)
{
	tNodeInit(&timer->linkNode);
	timer->startDelayTicks = delayTicks;
	timer->durationTicks = durationTicks;
	timer->timerFunc = timerFunc;
	timer->arg = arg;
	timer->config = config;
	
	if (delayTicks == 0)
	{
		timer->delayTicks = durationTicks;
	}
	else
	{
		timer->delayTicks = timer->startDelayTicks;
	}
	
	timer->state = tTimerCreated;
}

void tTimerStart (tTimer * timer)
{
	switch (timer->state)
	{
		case tTimerCreated:
		case tTimerStopped:
			timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
			timer->state = tTimerStarted;
		
			if (timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTaskEnterCritical();
				tListAddFirst(&tTimerHardList, &timer->linkNode);
				tTaskExitCritical(status);
			}
			else
			{
				tSemWait(&tTimerProtectSem, 0);
				tListAddLast(&tTimerSoftList, &timer->linkNode);
				tSemNotify(&tTimerProtectSem);
			}
			break;
		default:
		
			break;
	}
}

void tTimerStop (tTimer * timer)
{
	switch (timer->state)
	{
		case tTimerStarted:
		case tTimerRunning:
			if (timer->config & TIMER_CONFIG_TYPE_HARD)
			{
				uint32_t status = tTaskEnterCritical();
				
				tListRemove(&tTimerHardList, &timer->linkNode);
				
				tTaskExitCritical(status);
			}
			else
			{
				tSemWait(&tTimerProtectSem, 0);
				tListRemove(&tTimerSoftList, &timer->linkNode);
				tSemNotify(&tTimerProtectSem);
			}
			break;
		default:
			break;
	}
}

static void tTimerCallFuncList (tList * timerList)
{
	tNode * node;
	for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
	{
		tTimer * timer = tNodeParent(node, tTimer, linkNode);
		if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
		{
			timer->state = tTimerRunning;
			timer->timerFunc(timer->arg);
			timer->state = tTimerStarted;
			
			if (timer->durationTicks > 0)
			{
				timer->delayTicks = timer->durationTicks;
			}
			else
			{
				tListRemove(timerList, &timer->linkNode);
				timer->state = tTimerStopped;
			}
		}
	}
}

static tTask tTimeTask;
static tTaskStack tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE];

static void tTimerSoftTask (void * param)
{
	for (;;)
	{
		tSemWait(&tTimerTickSem, 0);
		
		tSemWait(&tTimerProtectSem, 0);
		
		tTimerCallFuncList(&tTimerSoftList);
		
		tSemNotify(&tTimerProtectSem);
		
	}
}

void tTimerModuleTickNotify (void)
{
	uint32_t status = tTaskEnterCritical();
	
	tTimerCallFuncList(&tTimerHardList);
	
	tTaskExitCritical(status);
	
	tSemNotify(&tTimerTickSem);
}

void tTimerModuleInit (void)
{
	tListInit(&tTimerHardList);
	tListInit(&tTimerSoftList);
	tSemInit(&tTimerProtectSem, 1, 1);
	tSemInit(&tTimerTickSem, 0, 0);
	
#if TINYOS_TIMERTASK_PRIO >= (TINYOS_PRO_COUNT - 1)
	#error "The proprity of timer tasker must be greater then (TINYOS_PRO_COUNT - 1)"
#endif
	tTaskInit(&tTimeTask, tTimerSoftTask, (void *)0, TINYOS_TIMERTASK_PRIO, &tTimerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]);
}
