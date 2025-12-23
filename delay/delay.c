#include "delay.h"
#include "cmsis-device.h"

/*
 * TODO:
 * 1. ticks should not be hard-coded
 * 2. for TI there should be SystemInit() analogue as for STM32
 * */
#define TICKS_IN_US     16

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

