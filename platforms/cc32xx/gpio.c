#include <stddef.h>

#include "custom-assert.h"

#include "gpio.h"
#include "gpio-name.h"

#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>
#include <ti/devices/cc32xx/inc/hw_ints.h>

#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/devices/cc32xx/driverlib/pin.h>
#include <ti/devices/cc32xx/driverlib/prcm.h>
#include <ti/devices/cc32xx/driverlib/interrupt.h>

static GpioHandle_t* m_GpioIrq[PIN_GPIO_MAX] = { NULL };

typedef struct
{
    uint32_t base;      /* GPIOAx_BASE [x = 0 - 3] */
    uint8_t mask;       /* GPIO_PIN_x [x = 0 - 7] */
    uint8_t pin;        /* PIN_xx (x = 0 - 63) defined in driverlib/pin.h */
} GpioMap_t;

static const GpioMap_t m_GPIO_MAP[PIN_GPIO_MAX] = {
    [PIN_GPIOA0_0] = { GPIOA0_BASE, GPIO_PIN_0, PIN_50 },
    [PIN_GPIOA0_1] = { GPIOA0_BASE, GPIO_PIN_1, PIN_55 },
    [PIN_GPIOA0_2] = { GPIOA0_BASE, GPIO_PIN_2, PIN_57 },
    [PIN_GPIOA0_3] = { GPIOA0_BASE, GPIO_PIN_3, PIN_58 },
    [PIN_GPIOA0_4] = { GPIOA0_BASE, GPIO_PIN_4, PIN_59 },
    [PIN_GPIOA0_5] = { GPIOA0_BASE, GPIO_PIN_5, PIN_60 },
    [PIN_GPIOA0_6] = { GPIOA0_BASE, GPIO_PIN_6, PIN_61 },
    [PIN_GPIOA0_7] = { GPIOA0_BASE, GPIO_PIN_7, PIN_62 },
    [PIN_GPIOA1_0] = { GPIOA1_BASE, GPIO_PIN_0, PIN_63 },
    [PIN_GPIOA1_1] = { GPIOA1_BASE, GPIO_PIN_1, PIN_64 },
    [PIN_GPIOA1_2] = { GPIOA1_BASE, GPIO_PIN_2, PIN_01 },
    [PIN_GPIOA1_3] = { GPIOA1_BASE, GPIO_PIN_3, PIN_02 },
    [PIN_GPIOA1_4] = { GPIOA1_BASE, GPIO_PIN_4, PIN_03 },
    [PIN_GPIOA1_5] = { GPIOA1_BASE, GPIO_PIN_5, PIN_04 },
    [PIN_GPIOA1_6] = { GPIOA1_BASE, GPIO_PIN_6, PIN_05 },
    [PIN_GPIOA1_7] = { GPIOA1_BASE, GPIO_PIN_7, PIN_06 },
    [PIN_GPIOA2_0] = { GPIOA2_BASE, GPIO_PIN_0, PIN_07 },
    [PIN_GPIOA2_1] = { GPIOA2_BASE, GPIO_PIN_1, PIN_08 },
    [PIN_GPIOA2_6] = { GPIOA2_BASE, GPIO_PIN_6, PIN_15 },
    [PIN_GPIOA2_7] = { GPIOA2_BASE, GPIO_PIN_7, PIN_16 },
    [PIN_GPIOA3_0] = { GPIOA3_BASE, GPIO_PIN_0, PIN_17 },
    [PIN_GPIOA3_1] = { GPIOA3_BASE, GPIO_PIN_1, PIN_21 },
    [PIN_GPIOA3_4] = { GPIOA3_BASE, GPIO_PIN_4, PIN_18 },
    [PIN_GPIOA3_5] = { GPIOA3_BASE, GPIO_PIN_5, PIN_20 },
    [PIN_GPIOA3_6] = { GPIOA3_BASE, GPIO_PIN_6, PIN_53 },
    [PIN_GPIOA3_7] = { GPIOA3_BASE, GPIO_PIN_7, PIN_45 },
};

static uint8_t GpioGetPinIndex(uint32_t base, uint8_t pinMask)
{
    for (uint8_t i = 0; i < PIN_GPIO_MAX; i++)
    {
        if (m_GPIO_MAP[i].base == base && m_GPIO_MAP[i].mask == pinMask)
        {
            return i;
        }
    }

    return 0xFF;
}

static void GpioDisableClocks(uint32_t base)
{
    switch (base)
    {
        case GPIOA0_BASE:
            PRCMPeripheralClkDisable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA1_BASE:
            PRCMPeripheralClkDisable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA2_BASE:
            PRCMPeripheralClkDisable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA3_BASE:
            PRCMPeripheralClkDisable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
            break;

        default:
            /* should never reach here */
            ASSERT(false);
            break;
    }
}

static void GpioEnableClocks(uint32_t base)
{
    switch (base)
    {
        case GPIOA0_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA1_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA2_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
            break;

        case GPIOA3_BASE:
            PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
            break;

        default:
            /* should never reach here */
            ASSERT(false);
            break;
    }
}

static uint32_t GpioMapStrength(PIN_STRENGTH strength)
{
    switch (strength)
    {
        case PIN_STRENGTH_LOW:
            return PIN_STRENGTH_2MA;

        case PIN_STRENGTH_MEDIUM:
            return PIN_STRENGTH_4MA;

        case PIN_STRENGTH_HIGH:
            return PIN_STRENGTH_6MA;

        default:
            ASSERT(false);
            return PIN_STRENGTH_2MA;
    }
}

static uint32_t GpioMapPinType(PIN_TYPES pull, PIN_CONFIGS cfg)
{
    if (cfg == PIN_CONFIG_OPEN_DRAIN)
    {
        switch (pull)
        {
            case PIN_TYPE_PULL_UP:
                return PIN_TYPE_OD_PU;

            case PIN_TYPE_PULL_DOWN:
                return PIN_TYPE_OD_PD;

            default:
                return PIN_TYPE_OD;
        }
    }
    else
    {
        switch (pull)
        {
            case PIN_TYPE_PULL_UP:
                return PIN_TYPE_STD_PU;

            case PIN_TYPE_PULL_DOWN:
                return PIN_TYPE_STD_PD;

            default:
                return PIN_TYPE_STD;
        }
    }
}

static uint8_t GpioMapEdge(PIN_IRQ_MODES mode)
{
    switch (mode)
    {
        case PIN_IRQ_RISING:
            return GPIO_RISING_EDGE;

        case PIN_IRQ_FALLING:
            return GPIO_FALLING_EDGE;

        case PIN_IRQ_BOTH:
            return GPIO_BOTH_EDGES;

        default:
            ASSERT(false);
            return GPIO_FALLING_EDGE;
    }
}

static uint8_t GpioMapInterruptPriority(uint8_t priority)
{
    switch (priority)
    {
        case 0:
            return INT_PRIORITY_LVL_0;

        case 1:
            return INT_PRIORITY_LVL_1;

        case 2:
            return INT_PRIORITY_LVL_2;

        case 3:
            return INT_PRIORITY_LVL_3;

        case 4:
            return INT_PRIORITY_LVL_4;

        case 5:
            return INT_PRIORITY_LVL_5;

        case 6:
            return INT_PRIORITY_LVL_6;

        case 7:
            return INT_PRIORITY_LVL_7;

        default:
            /* never reach here */
            ASSERT(false);
            return 0xFF;
    }
}

static uint8_t GpioMapInterrupt(uint32_t base)
{
    switch (base)
    {
        case GPIOA0_BASE:
            return INT_GPIOA0;
            break;

        case GPIOA1_BASE:
            return INT_GPIOA1;
            break;

        case GPIOA2_BASE:
            return INT_GPIOA2;
            break;

        case GPIOA3_BASE:
            return INT_GPIOA3;
            break;

        default:
            ASSERT(false);
            return 0xFF;
            break;
    }
}

static void GpioOpen(GpioHandle_t* const handle,
              uint8_t pin,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              uint32_t value)
{
    /*TODO: map alternate function */
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);
    ASSERT(pin < PIN_GPIO_MAX);

    if (pin == PIN_NC)
    {
        return;
    }

    const GpioMap_t* map = &m_GPIO_MAP[pin];

    handle->gpio.cc3220.base = map->base;
    handle->gpio.cc3220.pinMask = map->mask;
    handle->irqHandler = NULL;

    GpioEnableClocks(map->base);

    uint32_t pinType = GpioMapPinType(pull, config);
    uint32_t pinStrength = GpioMapStrength(strength);

    PinConfigSet(map->pin, pinStrength, pinType);

    switch (mode)
    {
        case PIN_MODE_INPUT:
            PinModeSet(map->pin, PIN_MODE_0);
            GPIODirModeSet(map->base, map->mask, GPIO_DIR_MODE_IN);
            break;

        case PIN_MODE_OUTPUT:
        {
            PinModeSet(map->pin, PIN_MODE_0);
            GPIODirModeSet(map->base, map->mask, GPIO_DIR_MODE_OUT);

            uint8_t newValue = (value ? 1 : 0);
            GPIOPinWrite(map->base, map->mask, newValue);
        }
        break;

        case PIN_MODE_ALTERNATE:
        {
            uint32_t pinMode = value;
            PinModeSet(map->pin, pinMode);
        }
        break;

        case PIN_MODE_ANALOG:
            /* TODO: */
            break;

        default:
            ASSERT(false);
            break;
    }

    handle->initialized = true;
}

static void GpioClose(GpioHandle_t* const handle)
{
    /*TODO*/
    if ((handle == NULL) || (handle->ops != NULL))
    {
        return;
    }

    handle->initialized = false;

    GpioDisableClocks(handle->gpio.cc3220.base);

    /*TODO: disable interrupts */
}

static void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    uint32_t base = handle->gpio.cc3220.base;
    uint8_t pinMask = handle->gpio.cc3220.pinMask;

    uint8_t value = (state == PIN_STATE_HIGH) ? pinMask : 0U;

    GPIOPinWrite(base, pinMask, value);
}

static uint16_t GpioRead(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return 0xFF;
    }

    uint32_t base = handle->gpio.cc3220.base;
    uint8_t pinMask = handle->gpio.cc3220.pinMask;

    return (uint16_t)(GPIOPinRead(base, pinMask) ? 1U : 0U);
}

static void GpioToggle(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    uint32_t base = handle->gpio.cc3220.base;
    uint8_t pinMask = handle->gpio.cc3220.pinMask;

    /*TODO: how to make it atomic??? */
#if 0
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
#endif

    uint32_t value = GPIOPinRead(base, pinMask);
    uint8_t newValue = (value ? 0U : pinMask);

    GPIOPinWrite(base, pinMask, newValue);
#if 0
    __set_PRIMASK(primask);
#endif
}

static void GPIO_IRQ_Handler(void)
{
    /*TODO: probably split up to port specific handler */
    GpioHandle_t* handle = NULL;

    for (uint8_t i = 0; i < PIN_GPIO_MAX; i++)
    {
        handle = m_GpioIrq[i];
        if (handle == NULL)
        {
            continue;
        }

        uint32_t base = handle->gpio.cc3220.base;
        uint8_t mask = handle->gpio.cc3220.pinMask;

        uint32_t status = GPIOIntStatus(base, true);

        if (status & mask)
        {
            GPIOIntClear(base, mask);

            if (handle->irqHandler != NULL)
            {
                (*handle->irqHandler)();
            }
        }
    }
}

static void GpioSetInterrupt(GpioHandle_t* const handle, PIN_IRQ_MODES mode, uint8_t priority, GpioIrqHandler handler)
{
    ASSERT(handle != NULL);
    ASSERT(handler != NULL);
    ASSERT(handle->ops != NULL);

    if (mode == PIN_IRQ_NONE)
    {
        return;
    }

    if (!handle->initialized)
    {
        return;
    }

    uint32_t base = handle->gpio.cc3220.base;
    uint8_t pinMask = handle->gpio.cc3220.pinMask;
    uint8_t pinIndex = GpioGetPinIndex(base, pinMask);
    uint8_t intPriority = GpioMapInterruptPriority(priority);
    uint8_t intNum = GpioMapInterrupt(base);
    uint8_t edge = GpioMapEdge(mode);

    ASSERT(pinIndex != 0xFF);
    ASSERT(intPriority != 0xFF);
    ASSERT(intNum != 0xFF);

    for (uint8_t i = 0; i < PIN_GPIO_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->gpio.cc3220.pinIndex == pinIndex)
        {
            return;
        }
    }

    handle->irqHandler = handler;
    handle->gpio.cc3220.pinIndex = pinIndex;
    m_GpioIrq[pinIndex] = handle;

    GPIOIntDisable(base, pinMask);
    GPIOIntClear(base, pinMask);

    GPIOIntTypeSet(base, pinMask, edge);

    GPIOIntEnable(base, pinMask);

    IntPrioritySet(intNum, intPriority);

    IntRegister(intNum, GPIO_IRQ_Handler);

    IntEnable(intNum);
}

/* Gpio operations */
static const GpioOps_t m_GpioOps = {
    .open = &GpioOpen,
    .close = &GpioClose,
    .read = &GpioRead,
    .write = &GpioWrite,
    .toggle = &GpioToggle,
    .interrupt = &GpioSetInterrupt
};

const GpioOps_t* GpioGetOps(void)
{
    return &m_GpioOps;
}

