#include "Arduino.h"
#include "ZWSupport.h"
#include "ZWCCTimer.h"
#include "ZWCCSwitchColor.h"
#include "ZWCCSwitchMultilevel.h"
#include "ZWCCSwitchBinary.h"

void *zuno_CCTimerFind(size_t channel, void *b, void *e, size_t step) {
	ZunoTimerDimmingStart_t				*lp_b;
	ZunoTimerDimmingStart_t				*lp_e;

	lp_b = (ZunoTimerDimmingStart_t *)b;
	lp_e = (ZunoTimerDimmingStart_t *)e;
	channel++;
	while (lp_b < lp_e) {
		if (lp_b->channel == channel)
			return (lp_b);
		lp_b = (ZunoTimerDimmingStart_t *)((size_t)lp_b + step);
	}
	lp_b = (ZunoTimerDimmingStart_t *)b;
	while (lp_b < lp_e) {
		if ((lp_b->bMode & ZUNO_TIMER_SWITCH_DIMMING) == 0)
			return (lp_b);
		lp_b = (ZunoTimerDimmingStart_t *)((size_t)lp_b + step);
	}
	return (0);
}

size_t zuno_CCTimerTicksTable7(size_t duration) {// Get the step for dimming in ticks
	if (duration == 0)
		return (0);
	if (duration == 0xFF)
		return (ZUNO_TIMER_SWITCH_DEFAULT_DURATION * (1000 / ZUNO_SYSTIMER_PERIOD_MC));// Set By Default
	if ((duration & (1 << 7)) != 0)
		duration = (duration ^ (1 << 7)) * 60;
	return (duration * (1000 / ZUNO_SYSTIMER_PERIOD_MC));
}

void zunoSendReportHandler(uint32_t ticks);

// Main timer for CC purposes
ZunoTimer_t g_zuno_timer;

void zuno_CCTimer(uint32_t ticks) {
	g_zuno_timer.ticks = ticks;
	#if defined(WITH_CC_SWITCH_BINARY) || defined(WITH_CC_SWITCH_MULTILEVEL) || defined(WITH_CC_SWITCH_COLOR)
	zunoEnterCritical();
	#endif
	#ifdef WITH_CC_SWITCH_BINARY
	zuno_CCSwitchBinaryTimer(ticks);
	#endif
	#ifdef WITH_CC_SWITCH_MULTILEVEL
	zuno_CCSwitchMultilevelTimer(ticks);
	#endif
	#ifdef WITH_CC_SWITCH_COLOR
	zuno_CCSwitchColorTimer(ticks);
	#endif
	#if defined(WITH_CC_SWITCH_BINARY) || defined(WITH_CC_SWITCH_MULTILEVEL) || defined(WITH_CC_SWITCH_COLOR)
	zunoExitCritical();
	#endif
	if((ticks & 0x1F) == 0) // Once in ~320ms 
		zunoSendReportHandler(ticks);
}