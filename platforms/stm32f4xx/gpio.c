#include <stddef.h>
#include <stdbool.h>

#include "stm32f411xe.h"

#include "custom-assert.h"
#include "gpio.h"

#define GPIO_IRQ_MAX            (16U)
#define GPIO_PORT_MAX           (8U)

/* Port clock enable */
#define GPIO_CLOCK_ENABLE_PORTA (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN))
#define GPIO_CLOCK_ENABLE_PORTB (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN))
#define GPIO_CLOCK_ENABLE_PORTC (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN))
#define GPIO_CLOCK_ENABLE_PORTD (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN))
#define GPIO_CLOCK_ENABLE_PORTE (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN))
#define GPIO_CLOCK_ENABLE_PORTH (RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN))

/* Port clock disable */
#define GPIO_CLOCK_DISABLE_PORTA (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOAEN))
#define GPIO_CLOCK_DISABLE_PORTB (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOBEN))
#define GPIO_CLOCK_DISABLE_PORTC (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOCEN))
#define GPIO_CLOCK_DISABLE_PORTD (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIODEN))
#define GPIO_CLOCK_DISABLE_PORTE (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOEEN))
#define GPIO_CLOCK_DISABLE_PORTH (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOHEN))

/* System configuration controller clock enable */
#define SYS_CLOCK_ENABLE        (RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN))

/* System configuration controller clock disable */
#define SYS_CLOCK_DISABLE        (RCC->APB2ENR &= ~(RCC_APB2ENR_SYSCFGEN))

static GpioHandle_t* m_GpioIrq[GPIO_IRQ_MAX] = { NULL };

static const GPIO_TypeDef* m_GpioPorts[GPIO_PORT_MAX] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    NULL,
    NULL,
    GPIOH
};

static GPIO_TypeDef* GpioGetPort(uint8_t pin)
{
    ASSERT(pin != PIN_NC);

    uint8_t port = (uint8_t)((pin >> 4U) & 0x0F);

    if (port == 0U) return GPIOA;
    if (port == 1U) return GPIOB;
    if (port == 2U) return GPIOC;
    if (port == 3U) return GPIOD;
    if (port == 4U) return GPIOE;
    if (port == 5U) return GPIOH;

    /* should never reach here */
    return NULL;
}
static uint8_t GpioGetPinIndex(uint8_t pin)
{
    ASSERT(pin != PIN_NC);

    return (uint8_t)(pin & 0x0F);
}

static uint32_t GpioGetExtiLine(const GPIO_TypeDef* const port)
{
    ASSERT(port != NULL);

    for (uint32_t i = 0; i < GPIO_PORT_MAX; i++)
    {
        if (m_GpioPorts[i] == port)
        {
            return i;
        }
    }

    /* should never reach here */
    return 0xFF;
}

static void GpioDisableClocks(const GPIO_TypeDef* const port)
{
    ASSERT(port != NULL);

    if (port == GPIOA)
    {
        GPIO_CLOCK_DISABLE_PORTA;
        return;
    }

    if (port == GPIOB)
    {
        GPIO_CLOCK_DISABLE_PORTB;
        return;
    }

    if (port == GPIOC)
    {
        GPIO_CLOCK_DISABLE_PORTC;
        return;
    }

    if (port == GPIOD)
    {
        GPIO_CLOCK_DISABLE_PORTD;
        return;
    }

    if (port == GPIOE)
    {
        GPIO_CLOCK_DISABLE_PORTE;
        return;
    }

    if (port == GPIOH)
    {
        GPIO_CLOCK_DISABLE_PORTH;
        return;
    }
}

static void GpioEnableClocks(const GPIO_TypeDef* const port)
{
    ASSERT(port != NULL);

    if (port == GPIOA)
    {
        GPIO_CLOCK_ENABLE_PORTA;
        return;
    }

    if (port == GPIOB)
    {
        GPIO_CLOCK_ENABLE_PORTB;
        return;
    }

    if (port == GPIOC)
    {
        GPIO_CLOCK_ENABLE_PORTC;
        return;
    }

    if (port == GPIOD)
    {
        GPIO_CLOCK_ENABLE_PORTD;
        return;
    }

    if (port == GPIOE)
    {
        GPIO_CLOCK_ENABLE_PORTE;
        return;
    }

    if (port == GPIOH)
    {
        GPIO_CLOCK_ENABLE_PORTH;
        return;
    }
}

static void GpioSetSpeed(GPIO_TypeDef* port, uint32_t pinIndex, PIN_STRENGTH strength)
{
    ASSERT(port != NULL);

    port->OSPEEDR &= ~(0x03U << (pinIndex * 2U));
    port->OSPEEDR |= ((uint32_t)strength << (pinIndex * 2U));
}

static void GpioSetPull(GPIO_TypeDef* port, uint32_t pinIndex, PIN_TYPES pull)
{
    ASSERT(port != NULL);

    port->PUPDR &= ~(0x03U << (pinIndex * 2U));
    port->PUPDR |= ((uint32_t)pull << (pinIndex * 2U));
}

static void GpioSetMode(GPIO_TypeDef* port, uint32_t pinIndex, PIN_MODES mode)
{
    ASSERT(port != NULL);

    port->MODER &= ~(0x03U << (pinIndex * 2U));
    port->MODER |= ((uint32_t)mode << (pinIndex * 2U));
}

static void GpioSetState(GPIO_TypeDef* port, uint32_t pinIndex, uint32_t value)
{
    ASSERT(port != NULL);
    ASSERT(value == PIN_STATE_LOW || value == PIN_STATE_HIGH);

    if (value == PIN_STATE_LOW)
    {
        port->BSRR = (1U << (pinIndex + 16U));
    }
    else if (value == PIN_STATE_HIGH)
    {
        port->BSRR = (1U << (pinIndex));
    }
}

static void GpioSetAlternateFunction(GPIO_TypeDef* port, uint32_t pinIndex, uint32_t af)
{
    ASSERT(port != NULL);
    ASSERT(af < 16U);

    uint32_t regIndex = (pinIndex < 8U) ? 0U : 1U;
    uint32_t shift = (pinIndex % 8U) * 4U;
    uint32_t mask = 0x0FU << shift;

    port->AFR[regIndex] &= ~mask;
    port->AFR[regIndex] |= ((af & 0x0FU) << shift);
}

static void GpioSetEdge(PIN_IRQ_MODES mode, uint32_t pinIndex)
{
    uint32_t mask = (1U << pinIndex);

    /*clear all */
    EXTI->RTSR &= ~mask;
    EXTI->FTSR &= ~mask;

    switch (mode)
    {
        case PIN_IRQ_RISING:
            EXTI->RTSR |= mask;
            break;

        case PIN_IRQ_FALLING:
            EXTI->FTSR |= mask;
            break;

        case PIN_IRQ_BOTH:
            EXTI->RTSR |= mask;
            EXTI->FTSR |= mask;
            break;

        default:
            /* should never reach here */
            ASSERT(false);
            break;
    }
}

static IRQn_Type GpioGetIrqNumber(uint32_t pinIndex)
{
    ASSERT(pinIndex < 16U);

    if (pinIndex <= 4)
    {
        return (IRQn_Type)(EXTI0_IRQn + pinIndex);
    }
    else if (pinIndex <= 9)
    {
        return EXTI9_5_IRQn;
    }
    else
    {
        return EXTI15_10_IRQn;
    }
}

static void GpioExtiHandler(uint8_t first, uint8_t last)
{
    for (uint8_t i = first; i <= last; i++)
    {
        uint32_t mask = (1U << i);

        if (EXTI->PR & mask)
        {
            /*
             * (rc_w1) Software can read as well as clear this bit by writing 1.
             * Writing ‘0’ has no effect on the bit value.
             * */
            EXTI->PR = mask;

            if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->irqHandler != NULL)
            {
                (*m_GpioIrq[i]->irqHandler)();
            }
        }
    }
}

static void GpioOpen(GpioHandle_t* handle,
              uint8_t pin,
              PIN_MODES mode,
              PIN_TYPES pull,
              PIN_STRENGTH strength,
              PIN_CONFIGS config,
              uint32_t value)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (pin == PIN_NC)
    {
        return;
    }

    GPIO_TypeDef* port = GpioGetPort(pin);
    uint8_t pinIndex = GpioGetPinIndex(pin);

    ASSERT(port != NULL);

    handle->gpio.base = port;
    handle->gpio.pinIndex = pinIndex;
    handle->irqHandler = NULL;

    GpioEnableClocks(port);
    GpioSetSpeed(port, pinIndex, strength);
    GpioSetPull(port, pinIndex, pull);
    GpioSetMode(port, pinIndex, mode);

    /* 0: Output push-pull (reset state) */
    port->OTYPER &= ~(1U << pinIndex);

    if (config == PIN_CONFIG_OPEN_DRAIN)
    {
        /* 1: Output open-drain */
        port->OTYPER |= (1U << pinIndex);
    }

    handle->initialized = true;

    if (mode == PIN_MODE_ALTERNATE)
    {
        GpioSetAlternateFunction(port, pinIndex, value);
        return;
    }

    if (mode == PIN_MODE_OUTPUT)
    {
        GpioSetState(port, pinIndex, value);
    }
}

static void GpioClose(GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->gpio.base;
    uint8_t pinIndex = handle->gpio.pinIndex;
    uint32_t mask = (1U << pinIndex);
    uint8_t extiReg = (pinIndex / 4);
    uint8_t extiIndex = (pinIndex % 4) * 4;
    uint8_t extiValue = GpioGetExtiLine(port);
    IRQn_Type irqNum = GpioGetIrqNumber(pinIndex);

/*TODO:disable interrupts */

    for (uint8_t i = 0; i < GPIO_IRQ_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->gpio.pinIndex == pinIndex)
        {
            m_GpioIrq[i]->irqHandler = NULL;

            break;
        }
    }

    SYSCFG->EXTICR[extiReg] &= ~(0x0F << extiIndex);
    SYSCFG->EXTICR[extiReg] &= ~(extiValue << extiIndex);

    /* disable line on IMR */
    EXTI->IMR &= ~mask;

    /* clear pending on EXTI */
    if (EXTI->PR & mask)
    {
        EXTI->PR = mask;
    }

    NVIC_ClearPendingIRQ(irqNum);

    NVIC_DisableIRQ(irqNum);

    SYS_CLOCK_DISABLE;

    GpioDisableClocks(port);

    handle->initialized = false;
}

static void GpioWrite(const GpioHandle_t* const handle, PIN_STATES state)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->gpio.base;
    uint8_t pinIndex = handle->gpio.pinIndex;

    GpioSetState(port, pinIndex, state);
}

static uint16_t GpioRead(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return 0xFF;
    }

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->gpio.base;
    uint8_t pinIndex = handle->gpio.pinIndex;

    uint16_t value = (uint16_t)(port->IDR & (1U << pinIndex));

    value &= (1U << (pinIndex));

    return value ? 1U : 0U;
}

static void GpioToggle(const GpioHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->gpio.base;
    uint8_t pinIndex = handle->gpio.pinIndex;

    uint32_t odr = port->ODR;

    if (odr & (1 << pinIndex))
    {
        GpioSetState(port, pinIndex, PIN_STATE_LOW);
    }
    else
    {
        GpioSetState(port, pinIndex, PIN_STATE_HIGH);
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

    GPIO_TypeDef* port = (GPIO_TypeDef*)handle->gpio.base;
    uint8_t pinIndex = handle->gpio.pinIndex;
    uint32_t mask = (1U << pinIndex);

    for (uint8_t i = 0; i < GPIO_IRQ_MAX; i++)
    {
        if (m_GpioIrq[i] != NULL && m_GpioIrq[i]->gpio.pinIndex == pinIndex)
        {
            return;
        }
    }

    handle->irqHandler = handler;

    SYS_CLOCK_ENABLE;

    uint8_t extiReg = (pinIndex / 4);
    uint8_t extiIndex = (pinIndex % 4) * 4;
    uint8_t extiValue = GpioGetExtiLine(port);
    IRQn_Type irqNum = GpioGetIrqNumber(pinIndex);

    ASSERT(extiValue != 0xff);

    /* assign lines EXTIx to ports PA..PH */
    SYSCFG->EXTICR[extiReg] &= ~(0x0F << extiIndex);
    SYSCFG->EXTICR[extiReg] |= (extiValue << extiIndex);

    /* disable line on IMR */
    EXTI->IMR &= ~mask;

    GpioSetEdge(mode, pinIndex);

    /* clear pending on EXTI */
    if (EXTI->PR & mask)
    {
        EXTI->PR = mask;
    }

    NVIC_ClearPendingIRQ(irqNum);

    m_GpioIrq[pinIndex] = handle;

    NVIC_SetPriority(irqNum, priority);
    NVIC_EnableIRQ(irqNum);

    /* enable line on IMR */
    EXTI->IMR |= mask;
}

void EXTI0_IRQHandler(void)
{
    GpioExtiHandler(0, 0);
}

void EXTI1_IRQHandler(void)
{
    GpioExtiHandler(1, 1);
}

void EXTI2_IRQHandler(void)
{
    GpioExtiHandler(2, 2);
}

void EXTI3_IRQHandler(void)
{
    GpioExtiHandler(3, 3);
}

void EXTI4_IRQHandler(void)
{
    GpioExtiHandler(4, 4);
}

void EXTI9_5_IRQHandler(void)
{
    GpioExtiHandler(5, 9);
}

void EXTI15_10_IRQHandler(void)
{
    GpioExtiHandler(10, 15);
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
