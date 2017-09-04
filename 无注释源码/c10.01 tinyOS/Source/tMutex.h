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

#endif
