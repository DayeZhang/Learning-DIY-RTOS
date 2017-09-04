#ifndef TMBOX_H
#define	TMBOX_H

#include "tEvent.h"

typedef struct _tMbox
{
	tEvent event;
	uint32_t count;
	uint32_t read;
	uint32_t write;
	uint32_t maxCount;
	void ** msgBuffer;
}tMbox;

void tMboxInit (tMbox * mbox, void **msgBufer, uint32_t maxCount);

#endif
