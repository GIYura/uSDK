#ifndef BUTTON_H
#define BUTTON_H

#include "gpio.h"

typedef void (*ButtonEventHandler)(void);

typedef struct
{
    GpioHandle_t* gpio;
} Button_t;

/*Brief: Button initialization
* [in] - button - pointer to button object
* [in] - gpio - pointer to GPIO object
* [in] - pin - GPIO pin
* [out] - none
* */
void ButtonInit(Button_t* const button, GpioHandle_t* const gpio, uint8_t pin);

/*Brief: Button de-initialization
* [in] - button - pointer to button object
* [out] - none
* */
void ButtonDeinit(Button_t* const button);

/*Brief: Register button press handler
 * [in] - button - pointer to button object
 * [in] - callback - callback function
 * [out] - none
 * */
void ButtonRegisterHandler(const Button_t* const button, ButtonEventHandler callback);

#endif /* BUTTON_H */
