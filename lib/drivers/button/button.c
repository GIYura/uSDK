#include <stddef.h>

#include "custom-assert.h"
#include "button.h"

#if 1
/*
 * NOTE: for test only
 * */
#include "gpio-name.h"
static GpioHandle_t m_gpio1;
#endif

static void Button_OnGpioIrq(void* context)
{
#if 1
    m_gpio1.ops->toggle(&m_gpio1);
#endif

    Button_t* button = (Button_t*)context;

    if (button->debouncing)
    {
        return;
    }

    button->debouncing = true;

    SwTimerStop(button->debounceTimer);
    SwTimerStart(button->debounceTimer);
}

static void Button_OnDebounce(void* context)
{
#if 1
    m_gpio1.ops->toggle(&m_gpio1);
#endif

    Button_t* button = (Button_t*)context;

    button->debouncing = false;

    uint16_t buttonState = button->gpio->ops->read(button->gpio);

    if (buttonState == 0)
    {
        if (button->handler != NULL)
        {
            (*button->handler)();
        }
    }
}

void ButtonInit(Button_t* const button, GpioHandle_t* const gpio, SwTimer_t* const swTimer, uint32_t debounceTicks)
{
    ASSERT(button != NULL);
    ASSERT(gpio != NULL);
    ASSERT(swTimer != NULL);

    button->gpio = gpio;
    button->debounceTimer = swTimer;
    button->debounceTicks = debounceTicks;
    button->debouncing = false;
    button->handler = NULL;

    button->gpio->ops->interrupt(button->gpio, PIN_IRQ_FALLING, 0, &Button_OnGpioIrq, button);

    SwTimerInit(swTimer, debounceTicks, SW_TIMER_ONE_SHOT);
    SwTimerRegisterCallback(swTimer, &Button_OnDebounce, button);

#if 1
    const GpioOps_t* gpioOps = GpioGetOps();
    m_gpio1.ops = gpioOps;
    m_gpio1.ops->open(&m_gpio1, PC_4, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_LOW);
#endif
}

void ButtonDeinit(Button_t* const button)
{
    ASSERT(button != NULL);

    button->gpio->ops->close(button->gpio);
}

void ButtonRegisterHandler(Button_t* const button, ButtonEventHandler callback)
{
    ASSERT(button != NULL);

    button->handler = callback;
}

