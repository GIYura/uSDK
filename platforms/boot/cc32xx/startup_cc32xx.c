#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sbss;
extern uint32_t _ebss;

/*
 * VTOR - Vector Table Offset Register
 * Arm v7-M Architecture Reference Manual
 * ARM DDI 0403E B3.2.2 System control and ID registers
 * */
#define SCB_VTOR (*(volatile uint32_t*)0xE000ED08)

int main(void);

void Reset_Handler(void);

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
};

void Reset_Handler(void)
{
    uint32_t* dst = &_sbss;

    /* assign VTOR to vector table located in SRAM (see linker script cc32xx.ld)*/
    SCB_VTOR = (uint32_t)g_pfnVectors;

    while (dst < &_ebss)
    {
        *dst++ = 0;
    }

    (void)main();

    while (1);
}
