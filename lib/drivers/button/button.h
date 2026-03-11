#ifndef BUTTON_H
#define BUTTON_H

#include "gpio.h"
#include "sw-timer.h"

typedef void (*ButtonEventHandler)(void);

typedef struct
{
    GpioHandle_t* gpio;
    SwTimer_t* debounceTimer;
    uint32_t debounceTicks;
    ButtonEventHandler handler;
    volatile bool debouncing;
} ButtonHandle_t;

/*Brief: Button initialization
* [in] - button - pointer to button object
* [in] - gpio - pointer to GPIO object
* [in] - pin - GPIO pin
* [out] - none
* */
void ButtonInit(ButtonHandle_t* const button, GpioHandle_t* const gpio, SwTimer_t* const swTimer, uint32_t debounceTicks);

/*Brief: Button de-initialization
* [in] - button - pointer to button object
* [out] - none
* */
void ButtonDeinit(ButtonHandle_t* const button);

/*Brief: Register button press handler
 * [in] - button - pointer to button object
 * [in] - callback - callback function
 * [out] - none
 * */
void ButtonRegisterHandler(ButtonHandle_t* const button, ButtonEventHandler callback);

#endif /* BUTTON_H */
