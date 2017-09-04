#include "tinyOS.h"

void tMboxInit (tMbox * mbox, void **msgBufer, uint32_t maxCount)
{
	tEventInit(&mbox->event, tEventTypeMbox);
	
	mbox->msgBuffer = msgBufer;
	mbox->maxCount = maxCount;
	mbox->read = 0;
	mbox->write = 0;
	mbox->count = 0;
}
