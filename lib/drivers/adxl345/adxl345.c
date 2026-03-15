#include <stddef.h>
#include <string.h>

#include "custom-assert.h"
#include "adxl345.h"
#include "adxl345-regs.h"

#define ADXL345_SINGLE_REG_TRANSACTION_LEN  (2)
#define ADXL345_VECOTR_TRANSACTION_LEN      (7)

static void AdxlResgistersInit(AdxlHandle_t* const handle);

static void OnRegisterReadCompleted(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    if (handle->rxLength == ADXL345_SINGLE_REG_TRANSACTION_LEN && handle->txLength == ADXL345_SINGLE_REG_TRANSACTION_LEN)
    {
        if (handle->readRegister != NULL)
        {
            (*handle->readRegister)(&handle->rxBuffer[1], handle->context);
        }
    }
}

static void OnRegisterWriteCompleted(void* context)
{
    ASSERT(context != NULL);

    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    if (handle->rxLength == 0)
    {
        if (handle->writeRegister != NULL)
        {
            (*handle->writeRegister)(NULL, NULL);
        }
    }
}

static void OnVectorReadCompleted(void* context)
{
    ASSERT(context != NULL);

    Acceleration_t acceleration;
    AdxlHandle_t* handle = (AdxlHandle_t*)context;

    if (handle->rxLength == ADXL345_VECOTR_TRANSACTION_LEN && handle->txLength == ADXL345_VECOTR_TRANSACTION_LEN)
    {
        if (handle->readVector != NULL)
        {
            acceleration.x = (int16_t)(handle->rxBuffer[2] << 8 | handle->rxBuffer[1]);
            acceleration.y = (int16_t)(handle->rxBuffer[4] << 8 | handle->rxBuffer[3]);
            acceleration.z = (int16_t)(handle->rxBuffer[6] << 8 | handle->rxBuffer[5]);

            (*handle->readVector)(&acceleration, handle->context);
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

    handle->readRegister = NULL;
    handle->readVector = NULL;
    handle->writeRegister = NULL;
    handle->spi = spi;
    handle->nss = nss;

    memset(handle->rxBuffer, 0, ADXL_TRANSACTION_LENGTH);
    memset(handle->txBuffer, 0, ADXL_TRANSACTION_LENGTH);

    handle->rxLength = 0;
    handle->txLength = 0;

    handle->initialized = true;

    AdxlResgistersInit(handle);
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

void AdxlRegisterReadRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->readRegister = callback;
}

void AdxlRegisterWriteRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->writeRegister = callback;
}

void AdxlRegisterReadVectorHandler(AdxlHandle_t* const handle, AdxlHandler_t callback)
{
    ASSERT(handle != NULL);

    handle->readVector = callback;
}

static void AdxlResgistersInit(AdxlHandle_t* const handle)
{
    ASSERT(handle != NULL);

    if (!handle->initialized)
    {
        return;
    }

    handle->registers[0].reg = ADXL345_DEVID;
    handle->registers[0].value = ADXL345_ID;

    handle->registers[1].reg = ADXL345_THRESH_TAP;
    handle->registers[2].reg = ADXL345_OFSX;
    handle->registers[3].reg = ADXL345_OFSY;
    handle->registers[4].reg = ADXL345_OFSZ;
    handle->registers[5].reg = ADXL345_DUR;
    handle->registers[6].reg = ADXL345_LATENT;
    handle->registers[7].reg = ADXL345_WINDOW;
    handle->registers[8].reg = ADXL345_THRESH_ACT;
    handle->registers[9].reg = ADXL345_THRESH_INACT;
    handle->registers[10].reg = ADXL345_TIME_INACT;
    handle->registers[11].reg = ADXL345_ACT_INACT_CTL;
    handle->registers[12].reg = ADXL345_THRESH_FF;
    handle->registers[13].reg = ADXL345_TIME_FF;
    handle->registers[14].reg = ADXL345_TAP_AXES;
    handle->registers[15].reg = ADXL345_ACT_TAP_STATUS;
    handle->registers[16].reg = ADXL345_BW_RATE;
    handle->registers[16].value = 0x0A;
    handle->registers[17].reg = ADXL345_POWER_CTL;
    handle->registers[18].reg = ADXL345_INT_ENABLE;
    handle->registers[19].reg = ADXL345_INT_MAP;
    handle->registers[20].reg = ADXL345_INT_SOURCE;
    handle->registers[20].value = 0x02;
    handle->registers[21].reg = ADXL345_DATA_FORMAT;
    handle->registers[22].reg = ADXL345_DATAX0;
    handle->registers[23].reg = ADXL345_DATAX1;
    handle->registers[24].reg = ADXL345_DATAY0;
    handle->registers[25].reg = ADXL345_DATAY1;
    handle->registers[26].reg = ADXL345_DATAZ0;
    handle->registers[27].reg = ADXL345_DATAZ1;
    handle->registers[28].reg = ADXL345_FIFO_CTL;
    handle->registers[29].reg = ADXL345_FIFO_STATUS;
}

AdxlRegisters_t* AdxlResgistersGet(AdxlHandle_t* const handle)
{
    ASSERT(handle != NULL);

    return handle->registers;
}

bool AdxlCheckRegisters(const AdxlRegisters_t* const src, const AdxlRegisters_t* const dst)
{
    ASSERT(src != NULL);
    ASSERT(dst != NULL);

    for (uint8_t i = 0; i < ADXL_REGISTERS_COUNT; i++)
    {
        if (src[i].value == dst[i].value)
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}
