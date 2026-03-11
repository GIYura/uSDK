#ifndef I2C_H
#define I2C_H

#include <stdbool.h>

#include "stm32f411xe.h"

#include "gpio.h"
#include "buffer.h"

#define I2C_TRANSACTION_QUEUE_SIZE 31

typedef enum
{
    I2C_OK = 0,
    I2C_BUSY,
    I2C_QUEUE_FULL,
    I2C_ERROR
} I2C_RESULT;

typedef enum
{
    I2C_1 = 0,
    I2C_2,
    I2C_3,
    I2C_COUNT
} I2C_NAMES;

typedef enum
{
    I2C_SPEED_STANDARD_MODE = 100000,   /* Hz */
    I2C_SPEED_FAST_MODE = 400000,       /* Hz */
} I2C_SPEED;

typedef enum
{
    I2C_IDLE = 0,
    I2C_BUSY_TX,
    I2C_BUSY_RX
} I2C_STATE;

typedef void (*I2C_EventHandler_t)(void* context);

typedef struct
{
    uint32_t speed;
    uint8_t ackControl;
    uint8_t dutyCycle;
} I2C_Config_t;

typedef struct
{
    uint8_t devAddress;
    uint8_t* txBuffer;
    uint8_t* rxBuffer;
    uint32_t txLen;
    uint32_t rxLen;
    uint8_t TxRxState;
    I2C_EventHandler_t onTxDone;
    I2C_EventHandler_t onRxDone;
    void* context;
} I2C_Transaction_t;

typedef struct
{
    I2C_TypeDef* instance;
    I2C_NAMES name;
    Gpio_t sda;
    Gpio_t scl;
    Buffer_t queue;
    I2C_Transaction_t transactions[I2C_TRANSACTION_QUEUE_SIZE + 1];
    I2C_Transaction_t* currentTransaction;
    I2C_Config_t config;
    bool initialized;
} I2C_Handle_t;

/*Brief: I2C initialization
 * [in] - obj - pointer to I2C object
 * [in] - name - I2C name
 * [out] - none
 * */
void I2C_Init(I2C_Handle_t* const obj, I2C_NAMES name);

/*Brief: I2C de-initialization
 * [in] - obj - pointer to I2C object
 * [out] - none
 * */
void I2C_Deinit(I2C_Handle_t* const obj);

/*Brief: I2C transmit in blocking mode
 * [in] - obj - pointer to I2C object
 * [in] - txBuffer - buffer to transmit
 * [in] - size - buffer size
 * [in] - slaveAddr - address of the slave device
 * [out] - none
 * */
void I2C_MasterTransmit(I2C_Handle_t* const obj, const uint8_t* txBuffer, uint8_t size, uint8_t slaveAddr);

/*Brief: I2C receive in blocking mode
 * [in] - obj - pointer to I2C object
 * [in] - rxBuffer - buffer to receive
 * [in] - size - buffer size
 * [in] - slaveAddr - address of the slave device
 * [out] - none
 * */
void I2C_MasterReceive(I2C_Handle_t* const obj, uint8_t* rxBuffer, uint8_t size, uint8_t slaveAddr);

/*Brief: I2C bus recover
 * [in] - obj - pointer to I2C object
 * [out] - none
 * */
void I2C_Recovery(I2C_Handle_t* const obj);

/*Brief: I2C transmit in non-blocking mode
 * [in] - obj - pointer to I2C object
 * [in] - transaction - pointer to transaction
 * [out] - I2C state
 * */
I2C_RESULT I2C_MasterTransmit_IT(I2C_Handle_t* const obj, I2C_Transaction_t* transaction);

/*Brief: I2C receive in non-blocking mode
 * [in] - obj - pointer to I2C object
 * [in] - transaction - pointer to transaction
 * [out] - I2C state
 * */
I2C_RESULT I2C_MasterReceive_IT(I2C_Handle_t* const obj, I2C_Transaction_t* transaction);

#endif /* I2C_H */
