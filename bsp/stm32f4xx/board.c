#include <stddef.h>

#include "custom-assert.h"
#include "board.h"
#include "gpio.h"
#include "gpio-name.h"

/* LEDs gpios */
static GpioHandle_t m_gpioLedGreen;
static GpioHandle_t m_gpioLedYellow;
static GpioHandle_t m_gpioLedWhite;

/* LEDs */
static Led_t m_ledGreen;
static Led_t m_ledYellow;
static Led_t m_ledWhite;

void Board_Init(void)
{
    const GpioOps_t* ops = GpioGetOps();

    m_gpioLedGreen.ops = ops;
    m_gpioLedWhite.ops = ops;
    m_gpioLedYellow.ops = ops;

    LedInit(&m_ledGreen, &m_gpioLedGreen, PA_5);
    LedInit(&m_ledYellow, &m_gpioLedYellow, PC_3);
    LedInit(&m_ledWhite, &m_gpioLedWhite, PC_4);
}

Led_t* Board_GetLed(BOARD_LED_ID id)
{
    ASSERT(id < BOARD_LED_COUNT);

    switch (id)
    {
    case BOARD_LED_GREEN:
        return &m_ledGreen;
        break;

    case BOARD_LED_YELLOW:
        return &m_ledYellow;
        break;

    case BOARD_LED_WHITE:
        return &m_ledWhite;
        break;

    default:
        /* never reach here */
        return NULL;
        break;
    }
}

