#include <stddef.h>

#include "custom-assert.h"
#include "board.h"
#include "gpio.h"
#include "gpio-name.h"

/* LEDs gpios */
static GpioHandle_t m_gpioLedGreen;
static GpioHandle_t m_gpioLedGreenExt;
static GpioHandle_t m_gpioLedYellow;
static GpioHandle_t m_gpioLedRed;

static GpioHandle_t m_gpioButtonSW2;
static GpioHandle_t m_gpioButtonSW3;

/* LEDs */
static Led_t m_ledGreenExt;
static Led_t m_ledGreen;
static Led_t m_ledYellow;
static Led_t m_ledRed;

static uint8_t m_counterSW2 = 0;
static uint8_t m_counterSW3 = 0;

void OnButtonSW2(void)
{
    ++m_counterSW2;
}

void OnButtonSW3(void)
{
    ++m_counterSW3;
}

void Board_Init(void)
{
    const GpioOps_t* ops = GpioGetOps();

    m_gpioLedGreenExt.ops = ops;
    m_gpioLedGreen.ops = ops;
    m_gpioLedRed.ops = ops;
    m_gpioLedYellow.ops = ops;

    m_gpioButtonSW2.ops = ops;
    m_gpioButtonSW3.ops = ops;

    LedInit(&m_ledGreen, &m_gpioLedGreen, PIN_GPIOA1_3);
    LedInit(&m_ledGreenExt, &m_gpioLedGreenExt, PIN_GPIOA2_1);
    LedInit(&m_ledYellow, &m_gpioLedYellow, PIN_GPIOA1_2);
    LedInit(&m_ledRed, &m_gpioLedRed, PIN_GPIOA1_1);

    m_gpioButtonSW2.ops->open(&m_gpioButtonSW2, PIN_GPIOA1_5, PIN_MODE_INPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, 1);
    m_gpioButtonSW3.ops->open(&m_gpioButtonSW3, PIN_GPIOA2_6, PIN_MODE_INPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, 1);

#if 0
NOTE: initialization sequence matters.
1. initialize button gpio
2. enable interrupts
NOTE: bare in mind, an interrupt might be triggered while GPIO is not configured completely yet.
#endif
    m_gpioButtonSW2.ops->interrupt(&m_gpioButtonSW2, PIN_IRQ_RISING, 0, &OnButtonSW2);
    m_gpioButtonSW3.ops->interrupt(&m_gpioButtonSW3, PIN_IRQ_RISING, 1, &OnButtonSW3);
}

Led_t* Board_GetLed(BOARD_LED_ID id)
{
    ASSERT(id < BOARD_LED_COUNT);

    switch (id)
    {
    case BOARD_LED_GREEN_EXT:
        return &m_ledGreenExt;
        break;

    case BOARD_LED_GREEN:
        return &m_ledGreen;
        break;

    case BOARD_LED_YELLOW:
        return &m_ledYellow;
        break;

    case BOARD_LED_RED:
        return &m_ledRed;
        break;

    default:
        /* never reach here */
        return NULL;
        break;
    }
}

