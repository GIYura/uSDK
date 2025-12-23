#ifndef CC3220SF_CMSIS_H
#define CC3220SF_CMSIS_H

#include <stdint.h>
#include "ti/devices/cc32xx/inc/hw_ints.h"

#define __NVIC_PRIO_BITS          4U       /*!< STM32F4XX uses 4 Bits for the Priority Levels */

/* Cortex-M4 Processor Exceptions */
typedef enum
{
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn     = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn      = -11,
    UsageFault_IRQn    = -10,
    SVCall_IRQn        = -5,
    DebugMonitor_IRQn  = -4,
    PendSV_IRQn        = -2,
    SysTick_IRQn       = -1,

    /* CC3220SF interrupts */
    GPIOA0_IRQn = INT_GPIOA0,
    GPIOA1_IRQn = INT_GPIOA1,
    UARTA0_IRQn = INT_UARTA0,
    UARTA1_IRQn = INT_UARTA1,
    TIMERA0_IRQn = INT_TIMERA0A,

} IRQn_Type;

#include "core_cm4.h"

#endif /* CC3220SF_CMSIS_H */
