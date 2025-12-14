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

/* LEDs */
static Led_t m_ledGreenExt;
static Led_t m_ledGreen;
static Led_t m_ledYellow;
static Led_t m_ledRed;

void Board_Init(void)
{
    const GpioOps_t* ops = GpioGetOps();

    m_gpioLedGreenExt.ops = ops;
    m_gpioLedGreen.ops = ops;
    m_gpioLedRed.ops = ops;
    m_gpioLedYellow.ops = ops;

    LedInit(&m_ledGreen, &m_gpioLedGreen, PIN_GPIOA1_3);
    LedInit(&m_ledGreenExt, &m_gpioLedGreenExt, PIN_GPIOA2_1);
    LedInit(&m_ledYellow, &m_gpioLedYellow, PIN_GPIOA1_2);
    LedInit(&m_ledRed, &m_gpioLedRed, PIN_GPIOA1_1);
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

