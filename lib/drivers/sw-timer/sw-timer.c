#include <stddef.h>

#include "sw-timer.h"
#include "custom-assert.h"

#if 0
/*
 * NOTE:
 * Current implementation uses 1 HW timer → 1 SW timer mapping.
 * This is intentional and used to validate RTC wakeup timer logic.
 * Will be extended to support multiple SW timers.
 */
#endif

static SwTimer_t* g_swTimer = NULL;

static void SwTimerIrq(void)
{
    ASSERT(g_swTimer != NULL);

    if (g_swTimer->mode == PERIODIC)
    {
        g_swTimer->timer->ops->start(g_swTimer->timer);
    }

    if (g_swTimer->callback)
    {
        g_swTimer->callback();
    }
}

void SwTimerInit(SwTimer_t* const swTimer, TimerHandle_t* const timer, uint32_t timeoutMs, TIMER_MODES mode)
{
    ASSERT(swTimer != NULL);

    swTimer->timer = timer;
    swTimer->mode = mode;
    swTimer->callback = NULL;

    g_swTimer = swTimer;

    swTimer->timer->ops->open(swTimer->timer, timeoutMs, mode);
    swTimer->timer->ops->interrupt(swTimer->timer, SwTimerIrq);
}

void SwTimerStart(SwTimer_t* const swTimer)
{
    ASSERT(swTimer != NULL);

    swTimer->timer->ops->start(swTimer->timer);
}

void SwTimerStop(SwTimer_t* const swTimer)
{
    ASSERT(swTimer != NULL);

    swTimer->timer->ops->close(swTimer->timer);
}

void SwTimerRegisterCallback(SwTimer_t* const swTimer, SwTimerHandler_t callback)
{
    ASSERT(swTimer != NULL);

    swTimer->callback = callback;
}

