#include <stddef.h>
#if 1

#include "stm32f411xe.h"

#include "custom-assert.h"
#include "delay.h"
#include "spi.h"
#include "ignore.h"

#include "gpio-name.h"

//#define WAIT_FLAG_TIMEOUT_MAX   100 /* us */

#define SPI_PORT_MAX           (5)

#define SPI_1_CLOCK_ENABLE      (RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN))
#define SPI_2_CLOCK_ENABLE      (RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN))
#define SPI_3_CLOCK_ENABLE      (RCC->APB1ENR |= (RCC_APB1ENR_SPI3EN))
#define SPI_4_CLOCK_ENABLE      (RCC->APB2ENR |= (RCC_APB2ENR_SPI4EN))
#define SPI_5_CLOCK_ENABLE      (RCC->APB2ENR |= (RCC_APB2ENR_SPI5EN))

static void SpiGpioInit(SpiHandle_t* const handle, uint8_t miso, uint8_t mosi, uint8_t sck);

static void SpiMode(SPI_TypeDef* const spi, SPI_POLARITY polarity, SPI_PHASE phase);
static void SpiFormat(SPI_TypeDef* const spi);

/*Brief: SPI speed
 * [in] - obj - pointer to SPI object
 * [in] - deriredFrequencyHz - desired SPI frequency in Hz
 * [out] - uint32_t value - actual SPI frequency in Hz
 * */
static uint32_t SpiSpeed(SpiHandle_t* const handle, uint32_t deriredFrequencyHz);
static void SpiClockEnable(SpiHandle_t* const handle);
static void SpiEnable(SPI_TypeDef* const spi);
static void SpiDisable(SPI_TypeDef* const spi);

static void SpiEnableTxInterrupt(SPI_TypeDef* const spi);
static void SpiDisableTxInterrupt(SPI_TypeDef* const spi);
static void SpiEnableRxInterrupt(SPI_TypeDef* const spi);
static void SpiDisableRxInterrupt(SPI_TypeDef* const spi);

//static bool WaitFlagTimeout(volatile uint32_t* reg, uint32_t flag, bool state, uint32_t timeoutUs);

static void SpiClearOverrun(SPI_TypeDef* const spi);

static void SpiIrqHandler(SpiHandle_t* const handle);

static IRQn_Type GetIrqType(const SpiHandle_t* const handle);

static SpiHandle_t* m_spiIrq[SPI_PORT_MAX];

static SPI_TypeDef* SpiGeBaseAddress(uint8_t spiNum)
{
    ASSERT(spiNum < SPI_PORT_MAX);

    if (spiNum == 0) return SPI1;
    if (spiNum == 1) return SPI2;
    if (spiNum == 2) return SPI3;
    if (spiNum == 3) return SPI4;
    if (spiNum == 4) return SPI5;

    /* should never reach here */
    return NULL;
}

static uint32_t SpiInit(SpiHandle_t* const handle, uint8_t spiNum, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz)
{
    ASSERT(handle != NULL);
    ASSERT(deriredFrequencyHz != 0);
    ASSERT(spiNum < SPI_PORT_MAX);

    uint32_t actualFreq = 0;

    SPI_TypeDef* spi = SpiGeBaseAddress(spiNum);

    handle->base = spi;
    handle->name = spiNum;
    handle->initialized = false;

    switch (spiNum)
    {
        case 0:
            SpiGpioInit(handle, PA_6, PA_7, PA_5);
            m_spiIrq[0] = handle;

            break;

        case 1:
            SpiGpioInit(handle, PB_14, PB_15, PB_13);
            m_spiIrq[1] = handle;

            break;

        case 2:
            SpiGpioInit(handle, PB_4, PB_5, PB_3);
            m_spiIrq[2] = handle;

            break;

        case 3:
            SpiGpioInit(handle, PE_5, PE_6, PE_4);
            m_spiIrq[3] = handle;

            break;

        case 4:
            SpiGpioInit(handle, PE_13, PE_14, PE_12);
            m_spiIrq[4] = handle;

            break;

        default:
            ASSERT(false);
            break;
    }

    SpiClockEnable(handle);

    SpiMode(spi, polarity, phase);

    SpiFormat(spi);

    actualFreq = SpiSpeed(handle, deriredFrequencyHz);

    SpiEnable(spi);

    SpiClearOverrun(spi);

    NVIC_EnableIRQ(GetIrqType(handle));

    BufferCreate(&handle->queue, handle->transactions, sizeof(handle->transactions), sizeof(SpiTransaction_t), false);

    handle->currentTransaction = NULL;

    handle->initialized = true;

    return actualFreq;
}
#if 0
void SpiDeinit(SPI_Handle_t* const obj)
{
    ASSERT(obj != NULL);

    obj->initialized = false;

    obj->currentTransaction = NULL;

    SpiDisable(obj);
}

bool SpiTransfer(SPI_Handle_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint8_t size)
{
    ASSERT(obj != NULL);

    if (!obj->initialized)
    {
        return false;
    }

    uint32_t dummy = 0;

    for (uint8_t i = 0; i < size; i++)
    {
        if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_TXE, 1, WAIT_FLAG_TIMEOUT_MAX))
        {
            goto timeout;
        }

        obj->instance->DR = (txBuffer != NULL) ? txBuffer[i] : 0xFF;

        if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_RXNE, 1, WAIT_FLAG_TIMEOUT_MAX))
        {
            goto timeout;
        }

        if (rxBuffer != NULL)
        {
            rxBuffer[i] = obj->instance->DR;
        }
        else
        {
            dummy = obj->instance->DR;
            IGNORE(dummy);
        }
    }

    if (!WaitFlagTimeout(&obj->instance->SR, SPI_SR_BSY, 0, WAIT_FLAG_TIMEOUT_MAX))
    {
        goto timeout;
    }

    return true;

timeout:
    SpiClearOverrun(obj);

    return false;
}
#endif

static SPI_RESULT SpiWriteNoneBlocking(SpiHandle_t* const handle, SpiTransaction_t* transaction)
{
    ASSERT(handle != NULL);
    ASSERT(transaction != NULL);

    SPI_TypeDef* spi = (SPI_TypeDef*)handle->base;

    if (!handle->initialized)
    {
        return SPI_ERROR;
    }

    if (!BufferPut(&handle->queue, transaction, sizeof(SpiTransaction_t)))
    {
        return SPI_QUEUE_FULL;
    }

    SpiEnableRxInterrupt(spi);

    SpiEnableTxInterrupt(spi);

    return SPI_OK;
}

static void SpiGpioInit(SpiHandle_t* const handle, uint8_t miso, uint8_t mosi, uint8_t sck)
{
    ASSERT(handle != NULL);

    const GpioOps_t* ops = GpioGetOps();

    handle->gpio.miso.ops = ops;
    handle->gpio.mosi.ops = ops;
    handle->gpio.sck.ops = ops;

    if (handle->name == 2 || handle->name == 4)
    {
        handle->gpio.miso.ops->open(&handle->gpio.miso, miso, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
        handle->gpio.miso.ops->open(&handle->gpio.mosi, mosi, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
        handle->gpio.miso.ops->open(&handle->gpio.sck, sck, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_6);
    }
    else
    {
        handle->gpio.miso.ops->open(&handle->gpio.miso, miso, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
        handle->gpio.miso.ops->open(&handle->gpio.mosi, mosi, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
        handle->gpio.miso.ops->open(&handle->gpio.sck, sck, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_5);
    }
}

static void SpiMode(SPI_TypeDef* const spi, SPI_POLARITY polarity, SPI_PHASE phase)
{
    ASSERT(spi != NULL);

    if (polarity == CPOL_0)
    {
        spi->CR1 &= ~(SPI_CR1_CPOL);
    }
    else
    {
        spi->CR1 |= (SPI_CR1_CPOL);
    }

    if (phase == CPHA_0)
    {
        spi->CR1 &= ~(SPI_CR1_CPHA);
    }
    else
    {
        spi->CR1 |= (SPI_CR1_CPHA);
    }
}

static void SpiFormat(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    /* master mode */
    spi->CR1 |= (SPI_CR1_MSTR);

    /* MSB transmitted first */
    spi->CR1 &= ~(SPI_CR1_LSBFIRST);

    /*Software slave management enabled  */
    spi->CR1 |= (SPI_CR1_SSM);
    spi->CR1 |= (SPI_CR1_SSI);

    /* Full duplex (Transmit and receive) */
    spi->CR1 &= ~(SPI_CR1_RXONLY);

    /* 8-bit data frame format is selected for transmission/reception */
    spi->CR1 &= ~(SPI_CR1_DFF);

    /* CRC calculation disabled */
    spi->CR1 &= ~(SPI_CR1_CRCEN);

    /* Data phase (no CRC phase) */
    spi->CR1 &= ~(SPI_CR1_CRCNEXT);

    /* 2-line unidirectional data mode selected */
    spi->CR1 &= ~(SPI_CR1_BIDIMODE);
}

static uint32_t SpiSpeed(SpiHandle_t* const handle, uint32_t deriredFrequencyHz)
{
    ASSERT(handle != NULL);

    uint32_t pclk = 0;
    uint32_t prescaler = 0;
    uint32_t brBits = 0;

    SPI_TypeDef* spi = (SPI_TypeDef*)handle->base;

    if (handle->name == 0 || handle->name == 3 || handle->name == 4)
    {
        /* APB2 */
        uint32_t hclk = SystemCoreClock;
        uint32_t apb2Prescaler = ((RCC->CFGR >> RCC_CFGR_PPRE2_Pos) & 0x7);
        if (apb2Prescaler < 4)
        {
            apb2Prescaler = 1;
        }
        else
        {
            apb2Prescaler = 1 << (apb2Prescaler - 3);
        }
        pclk = hclk / apb2Prescaler;
    }
    else
    {
        /* APB1 */
        uint32_t hclk = SystemCoreClock;
        uint32_t apb1Prescaler = ((RCC->CFGR >> RCC_CFGR_PPRE1_Pos) & 0x7);
        if (apb1Prescaler < 4)
        {
            apb1Prescaler = 1;
        }
        else
        {
            apb1Prescaler = 1 << (apb1Prescaler - 3);
        }
        pclk = hclk / apb1Prescaler;
    }

    prescaler = pclk / deriredFrequencyHz;

    while ((prescaler > 2) && (brBits < 7))
    {
        prescaler >>= 1;
        brBits++;
    }

    spi->CR1 &= ~(SPI_CR1_BR_Msk);
    spi->CR1 |= (brBits << SPI_CR1_BR_Pos);

    return pclk / (1U << (brBits + 1));
}

static void SpiClockEnable(SpiHandle_t* const handle)
{
    ASSERT(handle != NULL);

    switch (handle->name)
    {
        case 0:
            SPI_1_CLOCK_ENABLE;
            break;
        case 1:
            SPI_2_CLOCK_ENABLE;
            break;
        case 2:
            SPI_3_CLOCK_ENABLE;
            break;
        case 3:
            SPI_4_CLOCK_ENABLE;
            break;
        case 4:
            SPI_5_CLOCK_ENABLE;
            break;
        default:
            ASSERT(false);
            break;
    }
}

static void SpiEnable(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR1 |= (SPI_CR1_SPE);
}

static void SpiDisable(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR1 &= ~(SPI_CR1_SPE);
}

#if 0
static bool WaitFlagTimeout(volatile uint32_t* reg, uint32_t flag, bool state, uint32_t timeoutUs)
{
    while (((*reg & flag) ? 1 : 0) != (state ? 1 : 0))
    {
        if (timeoutUs == 0)
        {
            return false;
        }
        DelayUs(1);
        timeoutUs--;
    }
    return true;
}
#endif

static void SpiClearOverrun(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    volatile uint32_t dummy = 0;

    dummy = spi->DR;
    IGNORE(dummy);
    dummy = spi->SR;
    IGNORE(dummy);
}

static void SpiIrqHandler(SpiHandle_t* const handle)
{
    ASSERT(handle != NULL);

    SPI_TypeDef* spi = (SPI_TypeDef*)handle->base;

    if (handle->currentTransaction == NULL)
    {
        SpiTransaction_t next;

        if (!BufferGet(&handle->queue, &next, sizeof(SpiTransaction_t)))
        {
            SpiDisableTxInterrupt(spi);
            SpiDisableRxInterrupt(spi);
            return;
        }

        handle->currentTransaction = &next;

        if (handle->currentTransaction->preTransaction != NULL)
        {
            (*handle->currentTransaction->preTransaction)(handle->currentTransaction->context);
        }
    }

    SpiTransaction_t* t = handle->currentTransaction;

    if ((spi->SR & SPI_SR_RXNE) && (spi->CR2 & SPI_CR2_RXNEIE))
    {
        if (t->rxBuffer != NULL)
        {
            *(t->rxBuffer++) = (uint8_t)spi->DR;
            t->rxLen--;
        }
        else
        {
            IGNORE(spi->DR);
        }

        if (t->rxLen == 0 && t->txLen == 0)
        {
            SpiDisableTxInterrupt(spi);
            SpiDisableRxInterrupt(spi);

            if (t->postTransaction != NULL)
            {
                (*t->postTransaction)(NULL);
            }

            handle->currentTransaction = NULL;

            if (t->onTransactionDone != NULL)
            {
                (*t->onTransactionDone)(t->context);
            }
        }
    }

    if ((spi->SR & SPI_SR_TXE) && (spi->CR2 & SPI_CR2_TXEIE))
    {
        if (t->txLen > 0)
        {
            spi->DR = *(t->txBuffer++);
            t->txLen--;
        }
    }

    if ((spi->SR & SPI_SR_OVR) && (spi->CR2 & SPI_CR2_ERRIE))
    {
        IGNORE(spi->DR);
        IGNORE(spi->SR);
    }
}

static IRQn_Type GetIrqType(const SpiHandle_t* const handle)
{
    ASSERT(handle != NULL);

    IRQn_Type result;

    switch (handle->name)
    {
        case 0:
            result = SPI1_IRQn;
            break;

        case 1:
            result = SPI2_IRQn;
            break;

        case 2:
            result = SPI3_IRQn;
            break;

        case 3:
            result = SPI4_IRQn;
            break;

        case 4:
            result = SPI5_IRQn;
            break;

        default:
            ASSERT(false);
            break;
    }

    return result;
}

static void SpiEnableTxInterrupt(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR2 |= SPI_CR2_TXEIE;
}

static void SpiDisableTxInterrupt(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR2 &= ~SPI_CR2_TXEIE;
}

static void SpiEnableRxInterrupt(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR2 |= SPI_CR2_RXNEIE;
}

static void SpiDisableRxInterrupt(SPI_TypeDef* const spi)
{
    ASSERT(spi != NULL);

    spi->CR2 &= ~SPI_CR2_RXNEIE;
}

void SPI1_IRQHandler(void)
{
    SpiIrqHandler(m_spiIrq[0]);
}

void SPI2_IRQHandler(void)
{
    SpiIrqHandler(m_spiIrq[1]);
}

void SPI3_IRQHandler(void)
{
    SpiIrqHandler(m_spiIrq[2]);
}

void SPI4_IRQHandler(void)
{
    SpiIrqHandler(m_spiIrq[3]);
}

void SPI5_IRQHandler(void)
{
    SpiIrqHandler(m_spiIrq[4]);
}

/* SPI operations */
static const SpiOps_t m_SpiOps = {
    .open = &SpiInit,
    .write = &SpiWriteNoneBlocking,
};

const SpiOps_t* SpiGetOps(void)
{
    return &m_SpiOps;
}


#endif
