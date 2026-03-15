#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

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
    const SpiOps_t* ops;
    void* context;

    void* base;
    uint8_t name;
    SpiGpio_t gpio;
    Buffer_t queue;
    SpiTransaction_t transactions[SPI_TRANSACTION_QUEUE_SIZE + 1];
    SpiTransaction_t* currentTransaction;
    SpiTransaction_t currentTransactionStorage;
    bool initialized;
} SpiHandle_t;

struct SpiOps
{
    uint32_t (*open)(SpiHandle_t* const handle, uint8_t name, SPI_POLARITY polarity, SPI_PHASE phase, uint32_t desiredFrequencyHz);
    SPI_RESULT (*transfer)(SpiHandle_t* const handle, const SpiTransaction_t* const transaction);
};

/*Brief: Get SPI operations
* [in] - none
* [out] - pointer to SPI operations
* */
const SpiOps_t* SpiGetOps(void);

#endif /* SPI_H */
