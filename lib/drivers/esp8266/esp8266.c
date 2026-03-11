#include <string.h>

#include "custom-assert.h"
#include "buffer.h"
#include "esp8266.h"

#define ESP_RESPONSE_MAX    128 /* bytes */

static ESP_RESPONSE EspParseResponse(const char* resp);

static void OnUartReceiveCompleted(void* context)
{
    EspHandle_t* handle = (EspHandle_t*)context;
    uint16_t count = BufferCount(&handle->uart->rxBuffer);

    if (count < ESP_RESPONSE_MAX)
    {
        for (uint16_t i = 0; i < count; i++)
        {
            BufferGet(&handle->uart->rxBuffer, &handle->rxBuffer[i], sizeof(uint8_t));
        }

        handle->rxBuffer[count] = '\0';

        ESP_RESPONSE result = EspParseResponse((char*)handle->rxBuffer);

        if (handle->handler != NULL)
        {
            (*handle->handler)(result);
        }
    }
    else
    {
        /* TODO: */
    }
}

static ESP_RESPONSE EspParseResponse(const char* resp)
{
    if (strstr(resp, "OK"))
    {
        return ESP_RESPONSE_OK;
    }
    else if (strstr(resp, "ERROR"))
    {
        return ESP_RESPONSE_ERROR;
    }
    else if (strstr(resp, "BUSY"))
    {
        return ESP_RESPONSE_BUSY;
    }
    else if (strstr(resp, "LED_ON"))
    {
        return ESP_RESPONSE_LED_ON;
    }
    else if (strstr(resp, "LED_OFF"))
    {
        return ESP_RESPONSE_LED_OFF;
    }
    else
    {
        return ESP_RESPONSE_UNKNOWN;
    }
}

void EspInit(EspHandle_t* const handle, UartHandle_t* uart)
{
    ASSERT(handle);
    ASSERT(uart);

    handle->state = ESP_STATE_IDLE;
    handle->handler = NULL;
    handle->uart = uart;

    handle->uart->ops->interrupt(handle->uart, &OnUartReceiveCompleted, handle);
}

void EspSendCommand(EspHandle_t* const handle, const char* const command)
{
    ASSERT(handle);
    ASSERT(command);

    uint8_t commandLen = strlen(command);

    handle->uart->ops->write(handle->uart, (uint8_t*)command, commandLen);
    handle->uart->ops->write(handle->uart, (uint8_t*)"\r\n", 2);
}

void EspRegisterResponseHandler(EspHandle_t* const handle, EspResponseHandler_t callback)
{
    ASSERT(handle);

    handle->handler = callback;
}

