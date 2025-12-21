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

/*Brief: Register button press handler
 * [in] - callback - callback function
 * [out] - none
 * */
void ButtonRegisterPressHandler(ButtonEventHandler callback);

#endif /* BUTTON_H */
