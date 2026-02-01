#include <stddef.h>
#include <stdbool.h>

#include "stm32f411xe.h"

#include "custom-assert.h"
#include "timer.h"

#define RTC_WRITE_PROTECTION_KEY_1  ((uint8_t)0xCAU)
#define RTC_WRITE_PROTECTION_KEY_2  ((uint8_t)0x53U)
#define RTC_WUT_MAX                 (0xFFFFU)

#if 1
/*
 * NOTE: for test only
 * */
#include "gpio.h"
#include "gpio-name.h"
static GpioHandle_t m_gpio2;
#endif

typedef enum
{
    RTC_CLOCK_NO = 0,
    RTC_CLOCK_LSE,
    RTC_CLOCK_LSI,
    RTC_CLOCK_HSE,
    RTC_CLOCK_SOURCE_COUNT
} RTC_CLOCK_SOURCE;

typedef enum
{
    RTC_PRESCALER_16 = 0,
    RTC_PRESCALER_8,
    RTC_PRESCALER_4,
    RTC_PRESCALER_2,
    RTC_PRESCALER_COUNT
} RTC_PRESCALER;

static const uint32_t RTC_WUCKSEL_BITS[RTC_PRESCALER_COUNT] =
{
    0b000, /* RTC/16 */
    0b001, /* RTC/8 */
    0b010, /* RTC/4 */
    0b011  /* RTC/2 */
};

static const uint32_t RTC_CLOCKSOURCE_BITS[RTC_CLOCK_SOURCE_COUNT] =
{
    0b00, /* no clock */
    0b01, /* LSE */
    0b10, /* LSI */
    0b11  /* HSE */
};

/* RTC frequencies in Hz */
static const uint32_t RTC_FREQUENCY[RTC_PRESCALER_COUNT] = { 2048, 4096, 8192, 16384 };

static TimerHandle_t* m_RtcIrq = NULL;

/* Power interface clock enable */
#define RTC_CLOCK_ENABLE    (RCC->APB1ENR |= (RCC_APB1ENR_PWREN))

/* Power interface clock disable */
#define RTC_CLOCK_DISABLE   (RCC->APB1ENR &= ~(RCC_APB1ENR_PWREN))

static void BackupReset(void)
{
    RCC->BDCR |= (RCC_BDCR_BDRST);
    RCC->BDCR &= ~(RCC_BDCR_BDRST);
}

static void BackupWriteProtectionEnable(void)
{
    PWR->CR |= (PWR_CR_DBP);
}

static void BackupWriteProtectionDisable(void)
{
    PWR->CR &= ~(PWR_CR_DBP);
}

static void RtcWriteProtectionDisable(RTC_TypeDef* const rtc)
{
    rtc->WPR = RTC_WRITE_PROTECTION_KEY_1;
    rtc->WPR = RTC_WRITE_PROTECTION_KEY_2;
}

static void RtcWriteProtectionEnable(RTC_TypeDef* const rtc)
{
    rtc->WPR = 0xFF;
}

static void RtcEnable(void)
{
    RCC->BDCR |= (RCC_BDCR_RTCEN);
}

static void RtcDisable(void)
{
    RCC->BDCR &= ~(RCC_BDCR_RTCEN);
}

#if 0
/*
 * NOTE: not used
 * */
static void LSI_Enable(void)
{
    RCC->CSR |= RCC_CSR_LSION;

    while (!(RCC->CSR & RCC_CSR_LSIRDY));
}

static void LSI_Disable(void)
{
    RCC->CSR &= ~RCC_CSR_LSION;
}
#endif

static void LSE_Enable(void)
{
    RCC->BDCR |= (RCC_BDCR_LSEON);

    while (!(RCC->BDCR & RCC_BDCR_LSERDY));
}

static void LSE_Disable(void)
{
    RCC->BDCR &= ~(RCC_BDCR_LSEON);
}

static void RtcWakeupTimerEnable(RTC_TypeDef* const rtc)
{
    rtc->CR |= (RTC_CR_WUTE);
}

static void RtcWakeupTimerDisable(RTC_TypeDef* const rtc)
{
    rtc->CR &= ~(RTC_CR_WUTE);
}

static void RtcWakeupTimerEnableInterrupt(RTC_TypeDef* const rtc)
{
    rtc->CR |= (RTC_CR_WUTIE);
}

static void RtcWakeupTimerDisableInterrupt(RTC_TypeDef* const rtc)
{
    rtc->CR &= ~(RTC_CR_WUTIE);
}

static void RtcSetupClockSource(RTC_CLOCK_SOURCE source)
{
    ASSERT(source < RTC_CLOCK_SOURCE_COUNT);

    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= (RTC_CLOCKSOURCE_BITS[source] << RCC_BDCR_RTCSEL_Pos);
}

static void RtcWakeupClockSelection(RTC_TypeDef* const rtc, RTC_PRESCALER prescaler)
{
    ASSERT(prescaler < RTC_PRESCALER_COUNT);

    rtc->CR &= ~RTC_CR_WUCKSEL;
    rtc->CR |= (RTC_WUCKSEL_BITS[prescaler] << RTC_CR_WUCKSEL_Pos);
}

static uint32_t RtcCalcWakeupTicks(uint32_t timeoutMs, uint32_t freqHz)
{
    uint32_t ticks;

    if (timeoutMs == 0)
    {
        timeoutMs = 1;
    }

    ticks = (timeoutMs * freqHz + 999U) / 1000U;

    if (ticks == 0)
    {
        ticks = 1;
    }

    if (ticks > (RTC_WUT_MAX + 1U))
    {
        ticks = RTC_WUT_MAX + 1U;
    }

    return (ticks - 1U);
}

static void RtcOpen(TimerHandle_t* const handle, uint32_t timeout)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    handle->timer.instance = RTC;
    handle->timer.timeoutMs = timeout;
    handle->initialized = false;

    RTC_TypeDef* rtc = handle->timer.instance;
    uint8_t rtcPrescaler = RTC_PRESCALER_2;

    /* enable clock */
    RTC_CLOCK_ENABLE;

    /* enable access to RTC and RTC Backup registers */
    BackupWriteProtectionEnable();

    /* Force backup domain reset */
    BackupReset();

    /* LSE RC oscillator ON */
    LSE_Enable();

    /* Set RTC clock source to LSE */
    RtcSetupClockSource(RTC_CLOCK_LSE);

    /* Enable the RTC */
    RtcEnable();

    /* Disable RTC registers write protection */
    RtcWriteProtectionDisable(rtc);

    /* Wakeup timer disabled */
    RtcWakeupTimerDisable(rtc);

    while (!(rtc->ISR & RTC_ISR_WUTWF)) {}

    RtcWakeupClockSelection(rtc, rtcPrescaler);

    rtc->WUTR = RtcCalcWakeupTicks(timeout, RTC_FREQUENCY[rtcPrescaler]);

    rtc->ISR &= ~RTC_ISR_WUTF;      /* clear wakeup flag */
    EXTI->PR = EXTI_PR_PR22;        /* clear EXTI pending */

    RtcWriteProtectionEnable(rtc);

    handle->initialized = true;

#if 1
    const GpioOps_t* gpioOps = GpioGetOps();
    m_gpio2.ops = gpioOps;
    m_gpio2.ops->open(&m_gpio2, PC_3, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_LOW);
#endif
}

static void RtcClose(TimerHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    RTC_TypeDef* rtc = (RTC_TypeDef*)handle->timer.instance;
    IRQn_Type irqNum = RTC_WKUP_IRQn;

    /* Disable write protection */
    RtcWriteProtectionDisable(rtc);

    /* Disable interrupt + wakeup timer */
    RtcWakeupTimerDisableInterrupt(rtc);
    RtcWakeupTimerDisable(rtc);

    /* Clear flags */
    rtc->ISR &= ~RTC_ISR_WUTF;
    EXTI->PR = EXTI_PR_PR22;

    RtcWriteProtectionEnable(rtc);

    /* Disable NVIC */
    NVIC_DisableIRQ(irqNum);
    NVIC_ClearPendingIRQ(irqNum);

    BackupWriteProtectionDisable();

    LSE_Disable();

    /* Disable RTC */
    RtcDisable();

    RTC_CLOCK_DISABLE;

    handle->initialized = false;
}

static void RtcStart(const TimerHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    RTC_TypeDef* rtc = handle->timer.instance;

    RtcWriteProtectionDisable(rtc);

    /* Clear pending before start */
    rtc->ISR &= ~RTC_ISR_WUTF;
    EXTI->PR = EXTI_PR_PR22;

    RtcWakeupTimerEnable(rtc);

    RtcWriteProtectionEnable(rtc);
}

static void RtcStop(const TimerHandle_t* const handle)
{
    ASSERT(handle != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    RTC_TypeDef* rtc = handle->timer.instance;

    RtcWriteProtectionDisable(rtc);

    RtcWakeupTimerDisable(rtc);

    RtcWriteProtectionEnable(rtc);
}

void RTC_WKUP_IRQHandler(void)
{
#if 1
    m_gpio2.ops->toggle(&m_gpio2);
#endif

    if (RTC->ISR & RTC_ISR_WUTF)
    {
        RTC->ISR &= ~(RTC_ISR_WUTF);
        EXTI->PR = EXTI_PR_PR22;

        if (m_RtcIrq && m_RtcIrq->irqHandler != NULL)
        {
            (*m_RtcIrq->irqHandler)();
        }
    }
}

static void RtcSetInterrupt(TimerHandle_t* const handle, TimerIrqHandler handler)
{
    ASSERT(handle != NULL);
    ASSERT(handler != NULL);
    ASSERT(handle->ops != NULL);

    if (!handle->initialized)
    {
        return;
    }

    handle->irqHandler = handler;

    RTC_TypeDef* rtc = handle->timer.instance;
    m_RtcIrq = handle;
    IRQn_Type irqNum = RTC_WKUP_IRQn;

    EXTI->IMR |= EXTI_IMR_IM22;
    EXTI->EMR |= EXTI_EMR_EM22;
    EXTI->PR = EXTI_PR_PR22;

    /* 17.5 RTC interrupts
    RM0383: Configure and enable the EXTI Line 22 in interrupt mode and select the rising edge sensitivity
    */
    EXTI->RTSR |= EXTI_RTSR_TR22;

    RtcWriteProtectionDisable(rtc);

    RtcWakeupTimerEnableInterrupt(rtc);

    RtcWriteProtectionEnable(rtc);

    NVIC_ClearPendingIRQ(irqNum);
    NVIC_SetPriority(irqNum, 0);
    NVIC_EnableIRQ(irqNum);
}

/* Timer operations */
static const TimerOps_t m_TimerOps = {
    .open = &RtcOpen,
    .close = &RtcClose,
    .start = &RtcStart,
    .stop = &RtcStop,
    .interrupt = &RtcSetInterrupt
};

const TimerOps_t* TimerGetOps(void)
{
    return &m_TimerOps;
}

