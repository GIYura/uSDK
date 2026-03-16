#include <stddef.h>
#include <string.h>

#include "custom-assert.h"
#include "adxl345.h"
#include "adxl345-regs.h"

#define ADXL345_SINGLE_REG_TRANSACTION_LEN  (2)
#define ADXL345_VECOTR_TRANSACTION_LEN      (7)

static void OnRegisterReadCompleted(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    if (handle->rxLength == ADXL345_SINGLE_REG_TRANSACTION_LEN && handle->txLength == ADXL345_SINGLE_REG_TRANSACTION_LEN)
    {
        if (handle->onReadRegister != NULL)
        {
            (*handle->onReadRegister)(&handle->rxBuffer[1], handle->context);
        }
    }
}

static void OnRegisterWriteCompleted(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    handle->initIndex++;

    if (handle->initIndex < handle->initCount)
    {
        AdxlWriteRegisterAsyncSpi(handle, handle->initSequence[handle->initIndex].reg, &handle->initSequence[handle->initIndex].value);
    }
    else
    {
        if (handle->onConfigDone != NULL)
        {
            (*handle->onConfigDone)(NULL, handle->context);
        }
    }
}

static void OnVectorReadCompleted(void* context)
{
    ASSERT(context != NULL);

    AdxlAcceleration_t acceleration;
    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    if (handle->rxLength == ADXL345_VECOTR_TRANSACTION_LEN && handle->txLength == ADXL345_VECOTR_TRANSACTION_LEN)
    {
        if (handle->onReadVector != NULL)
        {
            acceleration.x = (int16_t)(handle->rxBuffer[2] << 8 | handle->rxBuffer[1]);
            acceleration.y = (int16_t)(handle->rxBuffer[4] << 8 | handle->rxBuffer[3]);
            acceleration.z = (int16_t)(handle->rxBuffer[6] << 8 | handle->rxBuffer[5]);

            (*handle->onReadVector)(&acceleration, handle->context);
        }
    }
}

static void AdxlActivate(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    handle->nss->ops->write(handle->nss, 0);
}

static void AdxlDeactivate(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    handle->nss->ops->write(handle->nss, 1);
}

void AdxlInitSpi(AdxlHandle_t* const handle, SpiHandle_t* const spi, GpioHandle_t* const nss)
{
    ASSERT(handle != NULL);
    ASSERT(spi != NULL);
    ASSERT(nss != NULL);
    ASSERT(!handle->initialized);

    handle->onReadRegister = NULL;
    handle->onReadVector = NULL;
    handle->onWriteRegister = NULL;
    handle->spi = spi;
    handle->nss = nss;

    memset(handle->rxBuffer, 0, ADXL_TRANSACTION_LENGTH);
    memset(handle->txBuffer, 0, ADXL_TRANSACTION_LENGTH);

    handle->rxLength = 0;
    handle->txLength = 0;

    handle->initialized = true;
}

void AdxlReadRegisterAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* context)
{
    ASSERT(handle != NULL);
    ASSERT(context != NULL);

    if (!handle->initialized)
    {
        return;
    }

    handle->context = context;

    handle->txBuffer[0] = 0x80 | address;
    handle->txBuffer[1] = 0xFF;
    handle->txLength = ADXL345_SINGLE_REG_TRANSACTION_LEN;
    handle->rxLength = ADXL345_SINGLE_REG_TRANSACTION_LEN;

    SpiTransaction_t spiTransaction = {
        .txBuffer = handle->txBuffer,
        .rxBuffer = handle->rxBuffer,
        .txLen = handle->txLength,
        .rxLen = handle->rxLength,
        .preTransaction = &AdxlActivate,
        .postTransaction = &AdxlDeactivate,
        .onTransactionDone = &OnRegisterReadCompleted,
        .context = handle,
    };

    handle->spi->ops->transfer(handle->spi, &spiTransaction);
}

void AdxlReadVectorAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* context)
{
    ASSERT(handle != NULL);
    ASSERT(context != NULL);

    if (!handle->initialized)
    {
        return;
    }

    handle->context = context;

    memset(handle->txBuffer, 0xFF, ADXL345_VECOTR_TRANSACTION_LEN);
    handle->txBuffer[0] = 0x80 | 0x40 | address;
    handle->txLength = ADXL345_VECOTR_TRANSACTION_LEN;
    handle->rxLength = ADXL345_VECOTR_TRANSACTION_LEN;

    SpiTransaction_t spiTransaction = {
        .txBuffer = handle->txBuffer,
        .rxBuffer = handle->rxBuffer,
        .txLen = handle->txLength,
        .rxLen = handle->rxLength,
        .preTransaction = &AdxlActivate,
        .postTransaction = &AdxlDeactivate,
        .onTransactionDone = OnVectorReadCompleted,
        .context = handle,
    };

    handle->spi->ops->transfer(handle->spi, &spiTransaction);
}

void AdxlWriteRegisterAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* value)
{
    ASSERT(handle != NULL);
    ASSERT(value != NULL);

    if (!handle->initialized)
    {
        return;
    }

    handle->txBuffer[0] = 0x00 | address;
    handle->txBuffer[1] = *(uint8_t*)value;
    handle->txLength = ADXL345_SINGLE_REG_TRANSACTION_LEN;
    handle->rxLength = 0;

    SpiTransaction_t spiTransaction = {
        .txBuffer = handle->txBuffer,
        .rxBuffer = NULL,
        .txLen = handle->txLength,
        .rxLen = handle->rxLength,
        .preTransaction = &AdxlActivate,
        .postTransaction = &AdxlDeactivate,
        .onTransactionDone = &OnRegisterWriteCompleted,
        .context = handle,
    };

    handle->spi->ops->transfer(handle->spi, &spiTransaction);
}

void AdxlConfigureAsyncSpi(AdxlHandle_t* const handle, AdxlRegisters_t* const configSequence, uint8_t configSequenceSize)
{
    ASSERT(handle != NULL);
    ASSERT(configSequence != NULL);
    ASSERT(configSequenceSize > 0);

    handle->initSequence = configSequence;
    handle->initCount = configSequenceSize;
    handle->initIndex = 0;

    AdxlWriteRegisterAsyncSpi(handle, handle->initSequence[handle->initIndex].reg, &handle->initSequence[handle->initIndex].value);
}

void AdxlRegisterReadRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->onReadRegister = callback;
}

void AdxlRegisterWriteRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->onWriteRegister = callback;
}

void AdxlRegisterReadVectorHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->onReadVector = callback;
}

void AdxlRegisterConfigureHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->onConfigDone = callback;
}
