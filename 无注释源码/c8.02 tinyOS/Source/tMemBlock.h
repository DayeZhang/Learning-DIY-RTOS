#ifndef TMEMBLOCK_H
#define	TMEMBLOCK_H

#include "tEvent.h"

typedef struct _tMemBlock
{
	tEvent event;
	void * memStart;
	uint32_t blockSize;
	uint32_t maxCount;
	tList blockList;
}tMemBlock;

void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t tMemBlockWait (tMemBlock * memBlock, uint8_t **mem, uint32_t waitTicks);
uint32_t tMemBlockNoWaitGet (tMemBlock * memBlock, void ** mem);
void tMemBlockNotify (tMemBlock * memBlock, uint8_t * mem);

#endif
