#include <stddef.h>

#include "custom-assert.h"
#include "led.h"

void LedInit(LedHandle_t* const led, GpioHandle_t* const gpio, uint8_t pin)
{
    ASSERT(led != NULL);
    ASSERT(gpio != NULL);

    led->gpio = gpio;

    led->gpio->ops->open(led->gpio, pin, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_LOW);
}

void LedOn(const LedHandle_t* const led)
{
    ASSERT(led != NULL);

    led->gpio->ops->write(led->gpio, PIN_STATE_HIGH);
}

void LedOff(const LedHandle_t* const led)
{
    ASSERT(led != NULL);

    led->gpio->ops->write(led->gpio, PIN_STATE_LOW);
}

void LedToggle(const LedHandle_t* const led)
{
    ASSERT(led != NULL);

    led->gpio->ops->toggle(led->gpio);
}

void LedDeinit(const LedHandle_t* const led)
{
    ASSERT(led != NULL);

    led->gpio->ops->close(led->gpio);
}

