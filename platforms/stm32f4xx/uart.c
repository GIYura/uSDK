#include <stdbool.h>
#include <stddef.h>

#include "stm32f411xe.h"

#include "custom-assert.h"
#include "uart.h"
#include "gpio-name.h"

#define UART_PORT_MAX           (3)

#define UART_1_CLOCK_ENABLE     (RCC->APB2ENR |= (RCC_APB2ENR_USART1EN))
#define UART_2_CLOCK_ENABLE     (RCC->APB1ENR |= (RCC_APB1ENR_USART2EN))
#define UART_6_CLOCK_ENABLE     (RCC->APB2ENR |= (RCC_APB2ENR_USART6EN))

#if 0
/*
 * NOTE: not used so far
 * */
#define DMA_1_CLOCK_ENABLE      (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN))
#define DMA_2_CLOCK_ENABLE      (RCC->AHB1ENR |= (RCC_AHB1ENR_DMA2EN))
#endif

#define TIM_2_CLOCK_ENABLE      (RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN))
#define TIM_3_CLOCK_ENABLE      (RCC->APB1ENR |= (RCC_APB1ENR_TIM3EN))
#define TIM_4_CLOCK_ENABLE      (RCC->APB1ENR |= (RCC_APB1ENR_TIM4EN))

static UART_Handle_t* m_UartIrq[UART_PORT_MAX] = { NULL };

static void TxInterruptEnable(USART_TypeDef* const uart);
static void TxInterruptDisable(USART_TypeDef* const uart);

static void RxInterruptEnable(USART_TypeDef* const uart);
static void RxInterruptDisable(USART_TypeDef* const uart);

static void TcInterruptEnable(USART_TypeDef* const uart);
static void TcInterruptDisable(USART_TypeDef* const uart);

static IRQn_Type UartGetIrqType(uint8_t uart);

static void UartOnInterrupt(UART_Handle_t* const handle);

static void TransmitterEnable(USART_TypeDef* const uart);
static void ReceiverEnable(USART_TypeDef* const uart);

static void SetFormat(USART_TypeDef* const uart);

static void UartEnable(USART_TypeDef* const uart);

#if 0
/*
 * NOTE: for test only
 * */
#include "gpio.h"
#include "gpio-name.h"
static GpioHandle_t m_gpio2;
#endif

#if 0
static void DMA_Config(UART_Handle_t* const obj, UART_NAMES uartName);
#endif

/*Brief: Converts baud rate in to register value */
static uint16_t ComputeBaudRate(uint32_t pclk, BAUD_RATE baud)
{
    uint32_t baudrate = 0;

    switch (baud)
    {
        case BAUD_1200:
            baudrate = 1200;
            break;

        case BAUD_2400:
            baudrate = 2400;
            break;

        case BAUD_9600:
            baudrate = 9600;
            break;

        case BAUD_38400:
            baudrate = 38400;
            break;

        case BAUD_57600:
            baudrate = 57600;
            break;

        case BAUD_115200:
            baudrate = 115200;
            break;

        case BAUD_230400:
            baudrate = 230400;
            break;

        case BAUD_460800:
            baudrate = 460800;
            break;

        case BAUD_921600:
            baudrate = 921600;
            break;

        default:
            break;
    }

    float usartDiv = (float)pclk / (16.0f * baudrate);

    uint32_t mantissa = (uint32_t)usartDiv;
    uint32_t fraction = (uint32_t)((usartDiv - mantissa) * 16.0f + 0.5f);

    if (fraction > 15)
    {
        mantissa += 1;
        fraction = 0;
    }

    return (mantissa << 4) | (fraction & 0x0F);
}

static USART_TypeDef* UartGeBaseAddress(uint8_t uartNum)
{
    ASSERT(uartNum < UART_PORT_MAX);

    if (uartNum == 0) return USART1;
    if (uartNum == 1) return USART2;
    if (uartNum == 2) return USART6;

    /* should never reach here */
    return NULL;
}

static void OnRxTimeout(void* context)
{
    UART_Handle_t* handle = (UART_Handle_t*)context;

    if (handle != NULL && handle->onRxDone != NULL)
    {
        (*handle->onRxDone)(handle->context);
    }
}

static void UartInit(UART_Handle_t* const handle, uint8_t uartNum, BAUD_RATE baud, SwTimer_t* const swTimer, uint32_t rxTimeoutMs)
{
    ASSERT(handle != NULL);
    ASSERT(swTimer != NULL);
    ASSERT(uartNum < UART_PORT_MAX);
    ASSERT(baud < BAUD_COUNT);

    USART_TypeDef* uart = UartGeBaseAddress(uartNum);

    handle->base = uart;
    handle->timer = swTimer;

    const GpioOps_t* ops = GpioGetOps();
    handle->txGpio.ops = ops;
    handle->rxGpio.ops = ops;

    handle->isTransmitting = false;
    handle->uartNum = uartNum;
    handle->isTransmitCompeted = true;
    handle->initialized = false;

    switch (uartNum)
    {
        case 0:
            handle->txGpio.ops->open(&handle->txGpio, PA_9, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            handle->rxGpio.ops->open(&handle->rxGpio, PA_10, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_1_CLOCK_ENABLE;

            break;

        case 1:
            handle->txGpio.ops->open(&handle->txGpio, PD_5, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            handle->rxGpio.ops->open(&handle->rxGpio, PD_6, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_2_CLOCK_ENABLE;

            break;

        case 2:
            handle->txGpio.ops->open(&handle->txGpio, PA_11, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);
            handle->rxGpio.ops->open(&handle->rxGpio, PA_12, PIN_MODE_ALTERNATE, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_AF_7);

            UART_6_CLOCK_ENABLE;

            break;

        default:
            ASSERT(false);
            break;
    }

    uart->BRR = ComputeBaudRate(SystemCoreClock, baud);

    TransmitterEnable(uart);

    ReceiverEnable(uart);
    RxInterruptEnable(uart);

    SetFormat(uart);

    UartEnable(uart);

    BufferCreate(&handle->txBuffer, &handle->txData, sizeof(handle->txData), sizeof(uint8_t), false);
    BufferCreate(&handle->rxBuffer, &handle->rxData, sizeof(handle->rxData), sizeof(uint8_t), false);

    NVIC_EnableIRQ(UartGetIrqType(uartNum));
    NVIC_SetPriority(UartGetIrqType(uartNum), 6);

    //m_UartIrq[handle->uartNum] = handle;

    SwTimerInit(swTimer, rxTimeoutMs, SW_TIMER_ONE_SHOT);
    SwTimerRegisterCallback(swTimer, &OnRxTimeout, handle);

    handle->initialized = true;

#if 0
    const GpioOps_t* gpioOps = GpioGetOps();
    m_gpio2.ops = gpioOps;
    m_gpio2.ops->open(&m_gpio2, PC_3, PIN_MODE_OUTPUT, PIN_TYPE_NO_PULL, PIN_STRENGTH_HIGH, PIN_CONFIG_PUSH_PULL, PIN_STATE_LOW);

    m_gpio2.ops->write(&m_gpio2, 0);
#endif
}

static void UartWriteNoneBlocking(UART_Handle_t* const handle, const uint8_t* const buffer, uint8_t size)
{
    ASSERT(handle != NULL);

    for (uint8_t i = 0; i < size; i++)
    {
        BufferPut(&handle->txBuffer, &buffer[i], sizeof(uint8_t));
    }

    if (!handle->isTransmitting)
    {
        handle->isTransmitting = true;
        handle->isTransmitCompeted = false;

        TxInterruptEnable(handle->base);
    }
}

static void UartSetIntrerrupt(UART_Handle_t* const handle, UART_EventHandler_t callback, void* context)
{
    ASSERT(handle != NULL);

    m_UartIrq[handle->uartNum] = handle;

    handle->onRxDone = callback;
    handle->context = context;
}

void USART1_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[0]);
}

void USART2_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[1]);
}

void USART6_IRQHandler(void)
{
    UartOnInterrupt(m_UartIrq[2]);
}

static void TxInterruptEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_TXEIE);
}

static void TxInterruptDisable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 &= ~(USART_CR1_TXEIE);
}

static void RxInterruptEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_RXNEIE);
}

static void RxInterruptDisable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 &= ~(USART_CR1_RXNEIE);
}

static void TcInterruptEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_TCIE);
}

static void TcInterruptDisable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 &= ~(USART_CR1_TCIE);
}

static IRQn_Type UartGetIrqType(uint8_t uart)
{
    ASSERT(uart < UART_PORT_MAX);

    IRQn_Type result;

    switch (uart)
    {
        case 0:
            result = USART1_IRQn;
            break;

        case 1:
            result = USART2_IRQn;
            break;

        case 2:
            result = USART6_IRQn;
            break;

        default:
            ASSERT(false);
            break;
    }

    return result;
}

static void UartOnInterrupt(UART_Handle_t* const handle)
{
    ASSERT(handle != NULL);

    USART_TypeDef* uart = (USART_TypeDef*)handle->base;

    uint8_t item = 0;

    /* TX handle */
    if ((uart->SR & (USART_SR_TXE)) && (uart->CR1 & (USART_CR1_TXEIE)))
    {
        if (BufferGet(&handle->txBuffer, &item, sizeof(item)))
        {
            uart->DR = item;
        }
        else
        {
            TxInterruptDisable(uart);
            TcInterruptEnable(uart);
        }
    }
    /* RX handle */
    if ((uart->SR & (USART_SR_RXNE)) && (uart->CR1 & (USART_CR1_RXNEIE)))
    {
        item = uart->DR;

        if (BufferPut(&handle->rxBuffer, &item, sizeof(item)))
        {
            SwTimerStart(handle->timer);

#if 0
            if (BufferCount(&handle->rxBuffer) >= 10)
            {
                m_gpio2.ops->write(&m_gpio2, 1);
            }
#endif
        }
        else
        {
            RxInterruptDisable(uart);
        }
    }

    /* TX complete handle */
    if ((uart->SR & (USART_SR_TC)) && (uart->CR1 & (USART_CR1_TCIE)))
    {
        uart->SR &= ~(USART_SR_TC);

        TcInterruptDisable(uart);

        handle->isTransmitting = false;
        handle->isTransmitCompeted = true;
    }
}

static void TransmitterEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_TE);
}

static void ReceiverEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_RE);
}

static void SetFormat(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    /* format: 1 Start bit, 8 Data bits, n Stop bit */
    uart->CR1 &= ~(USART_CR1_M);

    /* Parity control disabled */
    uart->CR1 &= ~(USART_CR1_PCE);

    /* 1 Stop bit */
    uart->CR2 &= ~(USART_CR2_STOP);

    /* oversampling by 16 */
    uart->CR1 &= ~(USART_CR1_OVER8);
}

static void UartEnable(USART_TypeDef* const uart)
{
    ASSERT(uart != NULL);

    uart->CR1 |= (USART_CR1_UE);
}

/* UART operations */
static const UartOps_t m_UartOps = {
    .open = &UartInit,
    .write = &UartWriteNoneBlocking,
    .interrupt = &UartSetIntrerrupt
};

const UartOps_t* UartGetOps(void)
{
    return &m_UartOps;
}

#if 0
static void DMA_Config(UART_Handle_t* const obj, UART_NAMES uartName)
{
    ASSERT(obj != NULL);

    switch (uartName)
    {
        case UART_1:
            DMA_2_CLOCK_ENABLE;

            DMA2_Stream7->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream7->CR & DMA_SxCR_EN);

            DMA2_Stream7->PAR = obj->instance->DR;
            DMA2_Stream7->CR = (4U << DMA_SxCR_CHSEL_Pos)  /* Channel 4 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream7->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        case UART_2:
            DMA_1_CLOCK_ENABLE;

            DMA2_Stream6->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream6->CR & DMA_SxCR_EN);

            DMA2_Stream6->PAR = obj->instance->DR;
            DMA2_Stream6->CR = (4U << DMA_SxCR_CHSEL_Pos)  /* Channel 4 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream6->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        case UART_6:
            DMA_2_CLOCK_ENABLE;

            DMA2_Stream7->CR &= ~DMA_SxCR_EN;

            while (DMA2_Stream7->CR & DMA_SxCR_EN);

            DMA2_Stream7->PAR = obj->instance->DR;
            DMA2_Stream7->CR = (5U << DMA_SxCR_CHSEL_Pos)  /* Channel 5 */
                               | DMA_SxCR_PL_1             /* High priority */
                               | DMA_SxCR_DIR_0            /* Memory -> Peripheral */
                               | DMA_SxCR_MINC;            /* Increment memory */

            DMA2_Stream7->FCR = 0;  /* Direct mode (no FIFO) */

            break;

        default:
            ASSERT(false);
            break;
    }
}
#endif
