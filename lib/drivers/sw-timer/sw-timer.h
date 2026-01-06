#ifndef SW_TIMER_H
#define SW_TIMER_H

#include "timer.h"

typedef void (*SwTimerHandler_t)(void);

typedef struct
{
    TimerHandle_t* timer;
    TIMER_MODES mode;
    SwTimerHandler_t callback;
} SwTimer_t;

/*Brief: SW Timer initialization
* [in] - swTimer - pointer to SW timer object
* [in] - timer - pointer to HW timer object
* [in] - timeoutMs - timeout in ms
* [in] - mode - 0 - one shot; 1 - periodic
* [out] - none
* */
void SwTimerInit(SwTimer_t* const swTimer, TimerHandle_t* const timer, uint32_t timeoutMs, TIMER_MODES mode);

/*Brief: SW Timer start
* [in] - swTimer - pointer to SW timer object
* [out] - none
* */
void SwTimerStart(SwTimer_t* const swTimer);

/*Brief: SW Timer stop
* [in] - swTimer - pointer to SW timer object
* [out] - none
* */
void SwTimerStop(SwTimer_t* const swTimer);

/*Brief: Register SW Timer callback
* [in] - swTimer - pointer to SW timer object
* [in] - callback - SW timer callback
* [out] - none
* */
void SwTimerRegisterCallback(SwTimer_t* const swTimer, SwTimerHandler_t callback);

#endif /* SW_TIMER */

