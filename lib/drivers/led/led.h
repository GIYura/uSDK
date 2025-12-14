#ifndef LED_H
#define LED_H

#include "gpio.h"

typedef struct
{
    GpioHandle_t* gpio;
} Led_t;

/*Brief: LED initialization
* [in] - led  - pointer to LED object
* [in] - gpio  - pointer to GPIO object
* [in] - pin  - GPIO pin
* [out] - none
* */
void LedInit(Led_t* const led, GpioHandle_t* const gpio, uint8_t pin);

/*Brief: LED ON
* [in] - led  - pointer to LED object
* [out] - none
* */
void LedOn(const Led_t* const led);

/*Brief: LED OFF
* [in] - led  - pointer to LED object
* [out] - none
* */
void LedOff(const Led_t* const led);

/*Brief: LED Toggle
* [in] - led  - pointer to LED object
* [out] - none
* */
void LedToggle(const Led_t* const led);

#endif /* LED_H */
