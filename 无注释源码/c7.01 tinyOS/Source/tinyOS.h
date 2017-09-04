#ifndef TINYOS_H
#define TINYOS_H

#include <stdint.h>

#include "tLib.h"

#include "tConfig.h"

#include "tTask.h"

#include "tEvent.h"

#include "tSem.h"

#include "tMbox.h"

typedef enum _tError {
	tErrorNoError = 0,
	tErrorTimeout,
	tErrorResourceUnavaliable,
	tErrorDel,
}tError;

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
void tTaskSchedRemove (tTask * task);
void tTaskSched (void);
void tTimeTaskWait (tTask * task, uint32_t ticks);
void tTimeTaskWakeUp (tTask * task);
void tTimeTaskRemove (tTask * task);
void tTaskSystemTickHandler (void);
void tTaskDelay (uint32_t delay);

void tSetSysTickPeriod (uint32_t ms);
void tInitApp (void);

#endif
