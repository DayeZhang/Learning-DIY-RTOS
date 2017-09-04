#ifndef TSEM_H
#define	TSEM_H

#include "tEvent.h"

typedef struct _tSem
{
	tEvent event;
	uint32_t count;
	uint32_t maxCount;
}tSem;

void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait (tSem * sem, uint32_t waitTicks);
uint32_t tSemNoWaitGet (tSem * sem);
void tSemNotify (tSem * sem);

#endif
