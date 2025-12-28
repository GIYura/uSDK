#ifndef CMSIS_DEVICE_H
#define CMSIS_DEVICE_H

#if defined(PLATFORM_STM32F411)
    #include "stm32f411xe.h"
#elif defined(PLATFORM_CC3220SF)
    #include "cc3220sf-cmsis.h"
    #include "system_cc32xx.h"
#else
    #error "Unsupported platform"
#endif

#endif /* CMSIS_DEVICE_H */
