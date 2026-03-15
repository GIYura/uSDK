#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>
#include <stdbool.h>

#include "spi.h"

#define ADXL345_ID                      (0xE5)

#define ADXL_TRANSACTION_LENGTH         (64)

#define ADXL_REGISTERS_COUNT            (30)

typedef void (*AdxlHandler_t)(void* value, void* context);

typedef struct
{
    uint8_t reg;
    uint8_t value;
} AdxlRegisters_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} Acceleration_t;

typedef struct
{
    SpiHandle_t* spi;
    GpioHandle_t* nss;
    uint8_t txBuffer[ADXL_TRANSACTION_LENGTH];
    uint8_t rxBuffer[ADXL_TRANSACTION_LENGTH];
    uint8_t txLength;
    uint8_t rxLength;
    AdxlHandler_t readRegister;
    AdxlHandler_t writeRegister;
    AdxlHandler_t readVector;
    void* context;
    AdxlRegisters_t registers[ADXL_REGISTERS_COUNT];
    bool initialized;
} AdxlHandle_t;

/*Brief: ADXL345 initialization over SPI
 * [in] - handle - pointer to ADXL345 handle
 * [in] - spi - pointer to SPI handle
 * [in] - nss - pointer to GPIO handle
 * [out] - none
 * */
void AdxlInitSpi(AdxlHandle_t* const handle, SpiHandle_t* const spi, GpioHandle_t* const nss);

/*Brief: ADXL345 read single register async over SPI
 * [in] - handle - pointer to ADXL345 handle
 * [in] - address - ADXL345 register address
 * [in] - context - response storage address
 * [out] - none
 * */
void AdxlReadRegisterAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* context);

/*Brief: ADXL345 read vector async over SPI
 * [in] - handle - pointer to ADXL345 handle
 * [in] - address - register address
 * [in] - context - response storage address
 * [out] - none
 * */
void AdxlReadVectorAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* context);

/*Brief: ADXL345 write single register async over SPI
 * [in] - handle - pointer to ADXL345 handle
 * [in] - address - register address
 * [in] - value - pointer to new value
 * [out] - none
 * */
void AdxlWriteRegisterAsyncSpi(AdxlHandle_t* const handle, uint8_t address, void* value);

/*Brief: ADXL345 register read single register handler
 * [in] - handle - pointer to ADXL345 handle
 * [in] - callback - callback function on receive done
 * [out] - none
 * */
void AdxlRegisterReadRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback);

/*Brief: ADXL345 register write single register handler
 * [in] - handle - pointer to ADXL345 handle
 * [in] - callback - callback function on write done
 * [out] - none
 * */
void AdxlRegisterWriteRegHandler(AdxlHandle_t* const handle, AdxlHandler_t callback);

/*Brief: ADXL345 register read vector handler
 * [in] - handle - pointer to ADXL345 handle
 * [in] - callback - callback function on vector receive done
 * [out] - none
 * */
void AdxlRegisterReadVectorHandler(AdxlHandle_t* const handle, AdxlHandler_t callback);

#if 0
/*
 * NOTE: for test only
 * Brief: Read all ADXL345 registers over SPI
 * [in] - src - pointer to default registers
 * [in] - dst - pointer to read registers
 * [out] - true - test ok; otherwise - test failed
 * */
bool AdxlCheckRegisters(const AdxlRegisters_t* const src, const AdxlRegisters_t* const dst);
AdxlRegisters_t* AdxlResgistersGet(AdxlHandle_t* const handle);
#endif

#endif /* ADXL345_H */
