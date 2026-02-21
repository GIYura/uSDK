#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*TimerIrqHandler)(void);

typedef struct TimerOps TimerOps_t;

typedef struct
{
    const TimerOps_t* ops;

    TimerIrqHandler irqHandler;

    struct
    {
        void* instance;
        uint32_t timeoutMs;
    } timer;

    bool initialized;

} TimerHandle_t;

struct TimerOps
{
/*Brief: Timer open
* [in] - handle - pointer to timer object
* [in] - timeoutMs - timeout in ms
* [in] - mode - 0 - one shot; 1 - periodic
* [out] - none
* */
    void (*open)(TimerHandle_t* const handle, uint32_t timeoutMs);

/*Brief: Timer start
* [in] - handle - pointer to timer object
* [out] - none
* */
    void (*start)(const TimerHandle_t* const handle);

/*Brief: Timer stop
* [in] - handle - pointer to timer object
* */
    void (*stop)(const TimerHandle_t* const handle);

/*Brief: Timer close
* [in] - handle - pointer to timer object
* [out] - none
* */
    void (*close)(TimerHandle_t* const handle);

/*Brief: Timer IRQ initialization
* [in] - handle - pointer to timer object
* [in] - handler - callback function pointer
* [out] - none
* */
#if 0
/*
 * uSDK Timer Interrupt Context Contract
 *
 * - Timer callbacks are executed in ISR (interrupt) context.
 * - The callback runs inside the hardware timer IRQ handler.
 *
 * - If a callback calls an RTOS "...FromISR()" API (e.g. FreeRTOS),
 *   the corresponding hardware interrupt priority must be
 *   numerically >= the RTOS system call interrupt priority threshold
 *   (e.g. configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY).
 *
 * - Using too high an interrupt priority (e.g. 0) while calling
 *   RTOS API from the callback will lead to configASSERT failure
 *   or undefined behavior.
 *
 *   Refer for details:
 *   https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/ARM-Cortex/RTOS-Cortex-M3-M4#cortex-m-hardware-details
 */
#endif
    void (*interrupt)(TimerHandle_t* const handle, TimerIrqHandler handler, uint8_t priority);
};

/*Brief:
* [in] - none
* [out] - pointer to timer operations
* */
const TimerOps_t* TimerGetOps(void);

#endif /* TIMER_H */
