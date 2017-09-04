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

#endif
