##############################################
# CC32XX Platform Configuration
##############################################

##############################################
# CORE/MCU
##############################################
CPU = cortex-m4
MCU = CC3220SF

##############################################
# SDK path
##############################################
TI_SDK = /opt/ti/simplelink_cc32xx_sdk_7_10_00_13

##############################################
# Linker
##############################################
LINKER_SCRIPT = $(SUBMODULES)/platforms/boot/cc32xx/cc32xx.ld

##############################################
# DEFINES
##############################################
DEFINES += -Dgcc -D$(MCU)

ifeq ($(PLATFORM),cc32xx)
DEFINES += -DPLATFORM_CC3220
endif

##############################################
# BOARD SUPPORT PACKAGE
##############################################
BSP = bsp

##############################################
# Include directories
##############################################
PLATFORM_DIRS = \
    $(TI_SDK)/source \
    $(TI_SDK)/source/ti/devices/cc32xx \
    $(TI_SDK)/source/ti/devices/cc32xx/inc \
    $(TI_SDK)/source/ti/devices/cc32xx/driverlib \
    $(SUBMODULES)/platforms/cc32xx \
    $(BSP)/cc32xx

##############################################
# START-UP
##############################################
PLATFORM_BOOT = $(SUBMODULES)/platforms/boot/cc32xx
