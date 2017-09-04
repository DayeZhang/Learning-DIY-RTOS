#ifndef TTIMER_H
#define	TTIMER_H

#include "tEvent.h"

typedef enum _tTimerState
{
	tTimerCreated,
	tTimerStarted,
	tTimerRunning,
	tTimerStopped,
	tTimerDestroyed
}tTimerState;

typedef struct _tTimer
{
	tNode linkNode;
	uint32_t startDelayTicks;
	uint32_t durationTicks;
	uint32_t delayTicks;
	void (*timerFunc) (void * arg);
	void * arg;
	uint32_t config;
	
	tTimerState state;
}tTimer;

#define	TIMER_CONFIG_TYPE_HARD		(1 << 0)
#define	TIMER_CONFIG_TYPE_SOFT		(0 << 0)

void tTimerInit (tTimer * timer, uint32_t delayTicks, uint32_t durationTicks,
		void (*timerFunc) (void * arg), void * arg, uint32_t config);
void tTimerStart (tTimer * timer);
void tTimerStop (tTimer * timer);
void tTimerModuleTickNotify (void);
void tTimerModuleInit (void);

#endif
