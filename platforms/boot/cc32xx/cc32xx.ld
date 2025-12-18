/*-----------------------------------------------------------------------------------------------------------------------
* Linker script for CC32xx
*
* Board: CC3220-LAUNCHXL
-------------------------------------------------------------------------------------------------------------------------*/

/* Entry point of the program */
ENTRY(Reset_Handler)

/* Memory regions
    SWRU465 – February 2017
    Table 2-4. Memory Map
    0x2000.0000 - 0x2003.FFFF Bit-banded on-chip SRAM
    Total: 256KB
    */
MEMORY
{
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

/* stack size */
STACK_SIZE = 4K;

SECTIONS
{
/* --- VECTOR TABLE --- */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > SRAM /* in this case VMA and LMA are the same */

/* --- CODE --- */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        . = ALIGN(4);
    } > SRAM
    
/* --- CONST --- */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);
    } > SRAM

/* --- INIT-DATA (NO ROM -> RAM copy) --- */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

/* --- BSS (zero-fill) --- */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
    
/* --- STACK --- */
    .stack (NOLOAD) :
    {
        . = ORIGIN(SRAM) + LENGTH(SRAM) - STACK_SIZE;
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > SRAM
}

/*-----------------------------------------------------------------------------------------------------------------------
* EOF
-------------------------------------------------------------------------------------------------------------------------*/
