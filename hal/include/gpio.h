#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

/* Gpio not connected */
#define PIN_NC                  (-1)

/* pin direction / mode */
typedef enum
{
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT,
    PIN_MODE_ALTERNATE,
    PIN_MODE_ANALOG
} PIN_MODES;

/* pull-up/pull-down */
typedef enum
{
    PIN_TYPE_NO_PULL = 0,
    PIN_TYPE_PULL_UP,
    PIN_TYPE_PULL_DOWN
} PIN_TYPES;

/* push-pull / open-drain */
typedef enum
{
    PIN_CONFIG_PUSH_PULL = 0,
    PIN_CONFIG_OPEN_DRAIN
} PIN_CONFIGS;

/* speed / drive strength */
typedef enum
{
    PIN_STRENGTH_LOW = 0,
    PIN_STRENGTH_MEDIUM,
    PIN_STRENGTH_HIGH
} PIN_STRENGTH;

/* pin states */
typedef enum
{
    PIN_STATE_LOW = 0,
    PIN_STATE_HIGH
} PIN_STATES;

/* IRQ mode */
typedef enum
{
    PIN_IRQ_NONE = 0,
    PIN_IRQ_RISING,
    PIN_IRQ_FALLING,
    PIN_IRQ_BOTH
} PIN_IRQ_MODES;

typedef void (*GpioIrqHandler)(void);

typedef struct GpioOps GpioOps_t;

typedef struct
{
    const GpioOps_t* ops;

    GpioIrqHandler irqHandler;

    struct
    {
        void* base;
        uint8_t pinMask;
        uint8_t pinIndex;
    } gpio;

    bool initialized;

} GpioHandle_t;

struct GpioOps
{
/*Brief: Gpio open
* [in] - handle - pointer to gpio object
* [in] - pin - name of the pin defined in platforms/gpio-name.h
* [in] - mode - gpio mode
* [in] - pull - gpio pull-up/pull-down
* [in] - strength - gpio speed
* [in] - config - gpio config
* [in] - value - gpio default value
* NOTE: this param can be used as alternate function
* [out] - none
* */
    void (*open)(GpioHandle_t* const handle, uint8_t pin, PIN_MODES mode, PIN_TYPES pull, PIN_STRENGTH strength, PIN_CONFIGS config, uint32_t value);

/*Brief: Gpio write
* [in] - handle - pointer to gpio object
* [in] - value - new gpio value
* [out] - none
* */
    void (*write)(const GpioHandle_t* const handle, PIN_STATES state);

/*Brief: Gpio read
* [in] - handle - pointer to gpio object
* [out] - value - gpio state
* */
    uint16_t (*read)(const GpioHandle_t* const handle);

/*Brief: Gpio toggle
* [in] - handle - pointer to gpio object
* [out] - none
* */
    void (*toggle)(const GpioHandle_t* const handle);

/*Brief: Gpio close
* [in] - handle - pointer to gpio object
* [out] - none
* */
    void (*close)(GpioHandle_t* const handle);

/*Brief: Gpio IRQ initialization
* [in] - handle - pointer to gpio object
* [in] - mode - IRQ mode
* [in] - priority - IRQ priority
* [in] - handler - callback function pointer
* [out] - none
* */
    void (*interrupt)(GpioHandle_t* const handle, PIN_IRQ_MODES mode, uint8_t priority, GpioIrqHandler handler);
};

/*Brief:
* [in] - none
* [out] - pointer to gpio operations
* */
const GpioOps_t* GpioGetOps(void);

#endif /* GPIO_H */
