#ifndef GPIO_NAME_H
#define GPIO_NAME_H

typedef enum
{
    PA_0, PA_1, PA_2, PA_3, PA_4, PA_5, PA_6, PA_7, PA_8, PA_9, PA_10, PA_11, PA_12, PA_13, PA_14, PA_15,
    PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7, PB_8, PB_9, PB_10, PB_11, PB_12, PB_13, PB_14, PB_15,
    PC_0, PC_1, PC_2, PC_3, PC_4, PC_5, PC_6, PC_7, PC_8, PC_9, PC_10, PC_11, PC_12, PC_13, PC_14, PC_15,
    PD_0, PD_1, PD_2, PD_3, PD_4, PD_5, PD_6, PD_7, PD_8, PD_9, PD_10, PD_11, PD_12, PD_13, PD_14, PD_15,
    PE_0, PE_1, PE_2, PE_3, PE_4, PE_5, PE_6, PE_7, PE_8, PE_9, PE_10, PE_11, PE_12, PE_13, PE_14, PE_15,
    PH_0, PH_1, PH_2, PH_3, PH_4, PH_5, PH_6, PH_7, PH_8, PH_9, PH_10, PH_11, PH_12, PH_13, PH_14, PH_15,
} STM32FXX_PIN_NAMES;

/* Alternate gpio */
typedef enum
{
    PIN_AF_0 = 0,
    PIN_AF_1,
    PIN_AF_2,
    PIN_AF_3,
    PIN_AF_4,
    PIN_AF_5,
    PIN_AF_6,
    PIN_AF_7,
    PIN_AF_8,
    PIN_AF_9,
    PIN_AF_10,
    PIN_AF_11,
    PIN_AF_12,
    PIN_AF_13,
    PIN_AF_14,
    PIN_AF_15,
} STM32XX_PIN_AF;

#endif /* GPIO_NAME_H */
