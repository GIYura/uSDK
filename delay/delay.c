#include "delay.h"
#include "cmsis-device.h"

#define SYSTICK_FREQ_HZ     SystemCoreClock
#define TICKS_IN_US         ((SYSTICK_FREQ_HZ) / 1000000U)

void DelaySec(uint32_t sec)
{
    uint32_t usec = sec * 1000000;
    DelayUs(usec);
}

void DelayMs(uint32_t msec)
{
    uint32_t usec = msec * 1000;
    DelayUs(usec);
}

void DelayUs(uint32_t usec)
{
    SysTick->LOAD = usec * TICKS_IN_US - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);

    SysTick->CTRL = 0;
}

