#include <stddef.h>

#include "custom-assert.h"
#include "button.h"

static ButtonEventHandler m_onButton = NULL;
static void OnButtonEvent(void);

void ButtonInit(Button_t* const button, GpioHandle_t* const gpio, uint8_t pin)
{
    ASSERT(button != NULL);

    button->gpio = gpio;

    button->gpio->ops->open(button->gpio, pin, PIN_MODE_INPUT, PIN_TYPE_PULL_UP, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_HIGH);

    button->gpio->ops->interrupt(button->gpio, PIN_IRQ_FALLING, 0, OnButtonEvent);
}

void ButtonRegisterPressHandler(ButtonEventHandler callback)
{
    m_onButton = callback;
}

static void OnButtonEvent(void)
{
    if (m_onButton != NULL)
    {
        (*m_onButton)();
    }
}

