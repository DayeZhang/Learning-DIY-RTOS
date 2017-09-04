#include "tinyOS.h"

void tMemBlockInit (tMemBlock * memBlock, uint8_t * memStart, uint32_t blockSize, uint32_t blockCnt)
{
	uint8_t * memBlockStart = (uint8_t *)memStart;
	uint8_t * memBlockEnd = memBlockStart + blockSize * blockCnt;
	
	if (blockSize < sizeof(tNode))
	{
		return;
	}
	
	tEventInit(&memBlock->event, tEventTypeMemBlock);
	memBlock->memStart = memStart;
	memBlock->blockSize = blockSize;
	memBlock->maxCount = blockCnt;
	
	tListInit(&memBlock->blockList);
	while (memBlockStart < memBlockEnd)
	{
		tNodeInit((tNode *)memBlockStart);
		tListAddLast(&memBlock->blockList, (tNode *)memBlockStart);
		
		memBlockStart += blockSize;
	}
}

