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

void SwTimerRegisterCallback(SwTimer_t* const swTimer, SwTimerHandler_t callback)
{
    ASSERT(swTimer != NULL);

    swTimer->callback = callback;
}

void SwTimerTick(void)
{
    for (uint8_t i = 0; i < SW_TIMER_MAX; i++)
    {
        SwTimer_t* t = m_swTimers[i];

        if (!t || !t->active)
        {
            continue;
        }

        if (--t->remaining == 0)
        {
            if (t->callback)
            {
                t->callback();
            }

            if (t->mode == SW_TIMER_PERIODIC)
            {
                t->remaining = t->period;
            }
            else
            {
                t->active = false;
            }
        }
    }
}
