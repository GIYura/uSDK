#include <stddef.h>

#include "custom-assert.h"
#include "button.h"

void ButtonInit(Button_t* const button, GpioHandle_t* const gpio, uint8_t pin)
{
    ASSERT(button != NULL);

    button->gpio = gpio;

    button->gpio->ops->open(button->gpio, pin, PIN_MODE_INPUT, PIN_TYPE_PULL_UP, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_HIGH);
}

void ButtonRegisterHandler(const Button_t* const button, ButtonEventHandler callback)
{
    button->gpio->ops->interrupt(button->gpio, PIN_IRQ_FALLING, 0, callback);
}

