#ifndef SYSTEM_CC32XX_H
#define SYSTEM_CC32XX_H

#include <stdint.h>

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#endif /* SYSTEM_CC32XX_H */
