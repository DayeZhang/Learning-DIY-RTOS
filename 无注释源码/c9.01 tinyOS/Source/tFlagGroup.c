#include "tinyOS.h"

void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags)
{
	tEventInit(&flagGroup->event, tEventTypeFlagGroup);
	flagGroup->flag = flags;
}


