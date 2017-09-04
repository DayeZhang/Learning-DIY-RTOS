#ifndef TMUTEX_H
#define TMUTEX_H

#include "tEvent.h"

typedef struct _tMutex
{
	tEvent event;
	uint32_t lockedCount;
	tTask * owner;
	uint32_t ownerOriginalPrio;
}tMutex;

typedef struct _tMutexInfo
{
	uint32_t taskCount;
	uint32_t ownerPrio;
	uint32_t inheritedPrio;
	tTask * owner;
	uint32_t lockedCount;
}tMutexInfo;

void tMutexInit (tMutex * mutex);
uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks);
uint32_t tMutexNoWaitGet (tMutex * mutex);
uint32_t tMutexNotify (tMutex * mutex);
uint32_t tMutexDestroy (tMutex * mutex);
void tMutexGetInfo (tMutex * mutex, tMutexInfo * info);

#endif
