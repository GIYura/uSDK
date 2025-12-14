#ifndef BOARD_H
#define BOARD_H

#include "led.h"

typedef enum
{
    BOARD_LED_GREEN,
    BOARD_LED_GREEN_EXT,
    BOARD_LED_YELLOW,
    BOARD_LED_RED,
    BOARD_LED_COUNT
} BOARD_LED_ID;

/*Brief: Board initialization
* [in] - none
* [out] - none
* */
void Board_Init(void);

/*Brief: Get board LED
* [in] - id  - LED id
* [out] - pointer to LED object
* */
Led_t* Board_GetLed(BOARD_LED_ID id);

#endif /* BOARD_H */
