#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#include "buffer.h"
#include "gpio.h"
#include "sw-timer.h"

#define BUFFER_SIZE      1023

typedef enum
{
    BAUD_1200 = 0,
    BAUD_2400,
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200,
    BAUD_230400,
    BAUD_460800,
    BAUD_921600,
    BAUD_COUNT
} BAUD_RATE;

typedef void (*UartEventHandler_t)(void* context);

typedef struct UartOps UartOps_t;

typedef struct
{
    const UartOps_t* ops;
    void* context;

    void* base;
    uint8_t uartNum;
    GpioHandle_t txGpio;
    GpioHandle_t rxGpio;
    Buffer_t txBuffer;
    Buffer_t rxBuffer;
    UartEventHandler_t onRxDone;
    uint8_t txData[BUFFER_SIZE + 1];
    uint8_t rxData[BUFFER_SIZE + 1];
    volatile bool isTransmitting;
    volatile bool isTransmitCompleted;
    SwTimerHandle_t* timer;
    bool initialized;
} UartHandle_t;

struct UartOps
{
/*Brief: UART open
 * [in] - handle - pointer to UART handle
 * [in] - uartNum - platform-specific UART number
 * [in] - baud - baud rate
 * [in] - swTimer - pointer to software timer
 * [in] - rxTimeoutMs - receive timeout in ms
 * [out] - none
 * */
    void (*open)(UartHandle_t* const handle, uint8_t uartNum, BAUD_RATE baud, SwTimerHandle_t* const swTimer, uint32_t rxTimeoutMs);

/*Brief: UART write in non-blocking mode
 * [in] - handle - pointer to UART handle
 * [in] - buff - pointer to buffer
 * [in] - size - buffer size
 * [out] - none
 * */
    void (*write)(UartHandle_t* const handle, const uint8_t* const buffer, uint8_t size);

/*Brief: UART IRQ initialization
 * [in] - handle - pointer to UART handle
 * [in] - handler - callback function pointer
 * [in] - context - pointer to context
 * [out] - none
 * */
    void (*interrupt)(UartHandle_t* const handle, UartEventHandler_t handler, void* context);
};

/*Brief: Get UART operations
* [in] - none
* [out] - pointer to UART operations
* */
const UartOps_t* UartGetOps(void);

#endif /* UART_H */
