#ifndef SW_TIMER_H
#define SW_TIMER_H

#include "timer.h"

typedef enum
{
    SW_TIMER_ONE_SHOT = 0,
    SW_TIMER_PERIODIC,
} SW_TIMER_MODES;

typedef void (*SwTimerHandler_t)(void);

typedef struct
{
    uint32_t period;        /* period in ticks */
    uint32_t remaining;     /* remaining ticks */
    SW_TIMER_MODES mode;
    bool active;
    SwTimerHandler_t callback;
} SwTimer_t;

/*Brief: SW Timer initialization
* [in] - swTimer - pointer to SW timer object
* [in] - timeoutTicks - timeout in ticks of HW timer
* [in] - mode - 0 - one shot; 1 - periodic
* [out] - none
* */
void SwTimerInit(SwTimer_t* const swTimer, uint32_t timeoutTicks, SW_TIMER_MODES mode);

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

/*Brief: Update active SW Timers
* [in] - none
* [out] - none
* */
void SwTimerTick(void);

#endif /* SW_TIMER */

