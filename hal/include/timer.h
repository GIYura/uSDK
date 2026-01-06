#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ONE_SHOT = 0,
    PERIODIC,
} TIMER_MODES;

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
        TIMER_MODES mode;
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
    void (*open)(TimerHandle_t* const handle, uint32_t timeoutMs, TIMER_MODES mode);

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
    void (*interrupt)(TimerHandle_t* const handle, TimerIrqHandler handler);
};

/*Brief:
* [in] - none
* [out] - pointer to timer operations
* */
const TimerOps_t* TimerGetOps(void);

#endif /* TIMER_H */
