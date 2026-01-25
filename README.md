### Brief

This repository contains a set of reusable software modules designed for different embedded platforms and applications. 
The modules are organized in a way that allows them to be easily adapted and used across multiple MCUs and project types.

### Structure

## 1. HAL (Hardware Abstraction Layer)

The `hal/include/` directory defines clean, MCU-agnostic APIs.

## 2. Platforms (MCU-Specific Implementations)

Each MCU has its own folder under `platforms/`:

`MCU/` — MCU specific implementation

`boot/MCU` — platform-specific startup and linker script files

These files contain the actual register-level implementation of the HAL API.

## 3. Lib — Device Drivers and Services

These modules are fully portable and do not depend on MCU specifics.

### drivers/
Device drivers on top of HAL.

### services/
Services on top of HAL.

## 4. Core Components

System-level functionality:

- **freertos/** — FreeRTOS kernel sources  
- **mpu/** — memory protection utilities  
- **cmsis/** — CMSIS files
- **assert/** — project-wide custom assert system  

## 5. Utility Modules

Standalone helpers:

- **buffer/** — circular buffer  
- **delay/** — blocking delay functions  
- **event/** — event queue system  
- **utils/** — macros, helpers, small utilities 
