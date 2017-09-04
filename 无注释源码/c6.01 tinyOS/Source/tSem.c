#include "tSem.h"

void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount)
{
	tEventInit(&sem->event, tEventTypeSem);
	
	sem->maxCount = maxCount;
	if (maxCount == 0)
	{
		sem->count = startCount;
	}
	else 
	{
		sem->count = (startCount > maxCount) ? maxCount : startCount;
	}
}

