#include <stddef.h>
#include <stdbool.h>

#include "custom-assert.h"
#include "timer.h"


static void GptOpen(TimerHandle_t* const handle, uint32_t timeout)
{

}

static void GptClose(TimerHandle_t* const handle)
{

}

static void GptStart(const TimerHandle_t* const handle)
{

}

static void GptStop(const TimerHandle_t* const handle)
{

}

static void GptSetInterrupt(TimerHandle_t* const handle, TimerIrqHandler handler)
{

}

/* Timer operations */
static const TimerOps_t m_TimerOps = {
    .open = &GptOpen,
    .close = &GptClose,
    .start = &GptStart,
    .stop = &GptStop,
    .interrupt = &GptSetInterrupt
};

const TimerOps_t* TimerGetOps(void)
{
    return &m_TimerOps;
}

