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

void tMutexInit (tMutex * mutex);
uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks);
uint32_t tMutexNoWaitGet (tMutex * mutex);
uint32_t tMutexNotify (tMutex * mutex);

#endif
