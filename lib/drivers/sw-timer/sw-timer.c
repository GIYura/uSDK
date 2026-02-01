#include <stddef.h>

#include "sw-timer.h"
#include "custom-assert.h"

#define SW_TIMER_MAX   16

static SwTimer_t* m_swTimers[SW_TIMER_MAX] = { NULL };

void SwTimerInit(SwTimer_t* const swTimer, uint32_t timeoutTicks, SW_TIMER_MODES mode)
{
    ASSERT(swTimer != NULL);

    swTimer->period = timeoutTicks;
    swTimer->remaining = timeoutTicks;
    swTimer->mode = mode;
    swTimer->active = false;
    swTimer->callback = NULL;

    /* register SW timer in table */
    for (uint8_t i = 0; i < SW_TIMER_MAX; i++)
    {
        if (m_swTimers[i] == NULL)
        {
            m_swTimers[i] = swTimer;
            return;
        }
    }

    /* no free slot */
    ASSERT(false);
}

void SwTimerStart(SwTimer_t* const swTimer)
{
    ASSERT(swTimer != NULL);

    swTimer->remaining = swTimer->period;
    swTimer->active = true;
}

void SwTimerStop(SwTimer_t* const swTimer)
{
    ASSERT(swTimer != NULL);

    swTimer->active = false;
}

void SwTimerRegisterCallback(SwTimer_t* const swTimer, SwTimerHandler_t callback, void* context)
{
    ASSERT(swTimer != NULL);

    swTimer->callback = callback;
    swTimer->context = context;
}

void SwTimerTick(void)
{
    for (uint8_t i = 0; i < SW_TIMER_MAX; i++)
    {
        SwTimer_t* timer = m_swTimers[i];

        if (!timer || !timer->active)
        {
            continue;
        }

        if (--timer->remaining == 0)
        {
            if (timer->callback)
            {
                timer->callback(timer->context);
            }

            if (timer->mode == SW_TIMER_PERIODIC)
            {
                timer->remaining = timer->period;
            }
            else
            {
                timer->active = false;
            }
        }
    }
}
