#ifndef TFLAGGROUP_H
#define	TFLAGGROUP_H

#include "tEvent.h"

typedef struct _tFlagGroup
{
	tEvent event;
	uint32_t flag;
}tFlagGroup;

void tFlagGroupInit (tFlagGroup * flagGroup, uint32_t flags);

#endif
