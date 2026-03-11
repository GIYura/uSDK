#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

//#include "stm32f411xe.h"

#include "gpio.h"
#include "buffer.h"

#define SPI_TRANSACTION_QUEUE_SIZE      (31)

typedef enum
{
    SPI_OK = 0,
    SPI_BUSY,
    SPI_QUEUE_FULL,
    SPI_ERROR
} SPI_RESULT;

#if 0
typedef enum
{
    SPI_1 = 0,
    SPI_2,
    SPI_3,
    SPI_4,
    SPI_5,
    SPI_COUNT
} SPI_NAMES;
#endif

typedef enum
{
    CPOL_0 = 0,
    CPOL_1
} SPI_POLARITY;

typedef enum
{
    CPHA_0 = 0,
    CPHA_1
} SPI_PHASE;

typedef struct
{
    GpioHandle_t miso;
    GpioHandle_t mosi;
    GpioHandle_t sck;
    GpioHandle_t nss;
} SpiGpio_t;

typedef void (*SpiEventHandler_t)(void* context);
typedef void (*SpiCsCallback_t)(void* context);

typedef struct SpiOps SpiOps_t;

typedef struct
{
    uint8_t* txBuffer;
    uint8_t* rxBuffer;
    uint16_t txLen;
    uint16_t rxLen;
    SpiEventHandler_t onTransactionDone;
    SpiCsCallback_t preTransaction;
    SpiCsCallback_t postTransaction;
    void* context;
} SpiTransaction_t;

typedef struct
{
    //SPI_TypeDef* instance;

    const SpiOps_t* ops;
    void* context;

    void* base;
    uint8_t name;
    SpiGpio_t gpio;
    Buffer_t queue;
    SpiTransaction_t transactions[SPI_TRANSACTION_QUEUE_SIZE + 1];
    SpiTransaction_t* currentTransaction;
    bool initialized;
} SpiHandle_t;

struct SpiOps
{
    uint32_t (*open)(SpiHandle_t* const handle, uint8_t name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz);
    SPI_RESULT (*write)(SpiHandle_t* const handle, SpiTransaction_t* transaction);
};

/*Brief: Get SPI operations
* [in] - none
* [out] - pointer to SPI operations
* */
const SpiOps_t* SpiGetOps(void);

#if 0
/*Brief: SPI initialization
 * [in] - obj - pointer to SPI object
 * [in] - name - SPI name
 * [in] - polarity - SPI clock polarity
 * [in] - phase - SPI clock phase
 * [in] - deriredFrequencyHz - SPI frequency in Hz
 * [out] - actual SPI frequency in Hz
 * */
uint32_t SpiInit(SPI_Handle_t* const obj, SPI_NAMES name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t deriredFrequencyHz);

/*Brief: SPI de-initialization
 * [in] - obj - pointer to SPI object
 * [out] - none
 * */
void SpiDeinit(SPI_Handle_t* const obj);

/*Brief: SPI transmit/receive in blocking mode
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - buffer to transmit
 * [in] - rxBuffer - buffer to receive
 * [in] - size - buffer size
 * [out] - true - transfer successful; false - otherwise
 * */
bool SpiTransfer(SPI_Handle_t* const obj, const uint8_t* const txBuffer, uint8_t* const rxBuffer, uint8_t size);

/*Brief: SPI transmit/receive in non-blocking mode
 * [in] - obj - pointer to SPI object
 * [in] - txBuffer - buffer to transmit
 * [in] - rxBuffer - buffer to receive
 * [in] - size - buffer size
 * [in] - context - pointer to context (storage for response)
 * [out] - spi transaction state
 * */
SPI_RESULT SpiTransfer_IT(SPI_Handle_t* const obj, SPI_Transaction_t* transaction);
#endif

#endif /* SPI_H */
