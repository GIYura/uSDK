### Brief

This repository contains a set of reusable software modules designed for different embedded platforms and applications. 
The modules are organized in a way that allows them to be easily adapted and used across multiple MCUs and project types.

### Structure

## 1. HAL (Hardware Abstraction Layer)

The `hal/include/` directory defines clean, MCU-agnostic APIs:

- `uart.h` — UART interface  
- `gpio.h` — GPIO interface
- `i2c.h` — I2C interface
- `spi.h` — SPI interface
- `timer.h` — timer interface

## 2. Platforms (MCU-Specific Implementations)

Each MCU has its own folder under `platforms/`:

`MCU/` — MCU specific implementation

`boot/MCU` — platform-specific startup and linker script files

These files contain the actual register-level implementation of the HAL API.

## 3. Lib — Device Drivers and Services

These modules are fully portable and do not depend on MCU specifics.

### drivers/
Device drivers on top of HAL:

| Directory     | Description                    |
|---------------|--------------------------------|
| `adxl345/`    | Accelerometer driver (I2C/SPI) |
| `esp8266/`    | Wi-Fi module via AT commands   |
| `sx126x/`     | LoRa transceiver driver        |
| `radio/`      | Abstract radio interface       |
| `pwm/`        | PWM built on HAL timers        |
| `led/`        | LED driver                     |
| `sw-timer/`   | SW timers                      |


### services/

| Directory | Description        |
|-----------|--------------------|
| `log/`    | UART-based logger  |
| `cli/`    | Command line shell |

- uart-service.h — UART service interface
- uart-service.c — UART service implementation

## 4. Board support package

Each MCU has its own folder under `bsp/`:

| Directory       |  Description                                    |
|-----------------|-------------------------------------------------|
| `stm32f4xx/`    | stm32f411 nucleo board specific implementation  |
| `cc32xx/`       | cc3220 launchxl board specific implementation   |

## 5. Core Components

System-level functionality:

- **freertos/** — FreeRTOS kernel sources  
- **mpu/** — memory protection utilities  
- **custom-assert/** — project-wide assert system  

## 6. Utility Modules

Standalone helpers:

- **buffer/** — circular buffer  
- **delay/** — blocking delay functions  
- **event/** — event queue system  
- **utils/** — macros, helpers, small utilities 


 
