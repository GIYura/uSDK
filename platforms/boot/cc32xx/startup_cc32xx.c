#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t isr_vector;

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

    SCB_VTOR = (uint32_t)&isr_vector;

    while (dst < &_ebss)
    {
        *dst++ = 0;
    }

    (void)main();

    while (1);
}
