##############################################
# STM32F4xx Platform Configuration
##############################################

##############################################
# CORE/MCU
##############################################
CPU = cortex-m4
MCU = STM32F411xE

##############################################
# SDK path
##############################################
STM32_SDK = $(HOME)/install/STM32Cube_FW_F4_V1.28.0

##############################################
# Linker
##############################################
LINKER_SCRIPT = $(SUBMODULES)/platforms/boot/stm32f4xx/stm32f4xx.ld

##############################################
# DEFINES
##############################################
DEFINES += -D$(MCU)

ifeq ($(PLATFORM),stm32f4xx)
DEFINES += -DPLATFORM_STM32F4
endif

##############################################
# Include directories
##############################################
PLATFORM_DIRS = \
    $(STM32_SDK)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
    $(STM32_SDK)/Drivers/CMSIS/Include \
    $(SUBMODULES)/platforms/stm32f4xx \
    $(SUBMODULES)/bsp/stm32f4xx

##############################################
# START-UP
##############################################
PLATFORM_BOOT = $(SUBMODULES)/platforms/boot/stm32f4xx
