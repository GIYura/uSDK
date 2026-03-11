#ifndef ESP8266_H
#define ESP8266_H

#include <stdint.h>

#include "uart.h"

typedef enum
{
    ESP_RESPONSE_OK = 0,
    ESP_RESPONSE_ERROR,
    ESP_RESPONSE_BUSY,
    ESP_RESPONSE_LED_ON,
    ESP_RESPONSE_LED_OFF,
    ESP_RESPONSE_UNKNOWN
} ESP_RESPONSE;

typedef enum
{
    ESP_STATE_IDLE = 0,
    ESP_STATE_AT_SENT,
    ESP_STATE_GMR_SENT,
    ESP_STATE_MODE_SENT,
    ESP_STATE_APCFG_SENT,
    ESP_STATE_ENABLE_CONN,
    ESP_STATE_READY,
} ESP_STATE;

typedef void (*EspResponseHandler_t)(ESP_RESPONSE result);

typedef struct
{
    ESP_STATE state;
    UartHandle_t* uart;
    uint8_t rxBuffer[128];
    EspResponseHandler_t handler;
} EspHandle_t;

/*Brief: ESP initialization
 * [in] - handle - pointer to ESP object
 * [in] - uart - pointer to UART object (transport layer)
 * [out] - none
 * */
void EspInit(EspHandle_t* const handle, UartHandle_t* uart);

/*Brief: ESP send commands
 * [in] - handle - pointer to ESP object
 * [in] - command - pointer to command
 * [out] - none
 * */
void EspSendCommand(EspHandle_t* const handle, const char* const command);

/*Brief: ESP register response handler
 * [in] - handle - pointer to ESP object
 * [in] - callback - callback
 * [out] - none
 * */
void EspRegisterResponseHandler(EspHandle_t* const handle, EspResponseHandler_t callback);

#endif /* ESP8266_H */

