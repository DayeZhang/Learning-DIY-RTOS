#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>

#include "tLib.h"

#include "tConfig.h"

#define	TINYOS_TASK_STATE_RDY					0
#define	TINYOS_TASK_STATE_DELAYED				(1 << 1)
#define	TINYOS_TASK_STATE_SUSPEND				(1 << 2)

typedef uint32_t tTaskStack;

typedef struct _tTask {
	tTaskStack * stack;
	tNode linkNode;
	uint32_t delayTicks;
	tNode delayNode;
	uint32_t prio;
	uint32_t state;
	uint32_t slice;
	uint32_t suspendCount;
}tTask;

extern tTask * currentTask;
extern tTask * nextTask;

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);
	
void tTaskRunFirst (void);
void tTaskSwitch (void);

void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);
void tTaskSchedRdy (tTask * task);
void tTaskSchedUnRdy (tTask * task);
void tTaskSched (void);
void tTimeTaskWait (tTask * task, uint32_t ticks);
void tTimeTaskWakeUp (tTask * task);
void tTaskSystemTickHandler (void);
void tTaskDelay (uint32_t delay);
void tTaskInit (tTask * task, void (*entry)(void *), void * param, uint32_t prio, tTaskStack * stack);
void tTaskSuspend (tTask * task);
void tTaskWakeUp (tTask * task);
void tSetSysTickPeriod (uint32_t ms);
void tInitApp (void);

#endif
