/*
 * Raspberry Pi Zero 2 W Kernel Linker Script
 * BCM2710A1 (Cortex-A53) - AArch64
 * Kernel loads at 0x80000 in 64-bit mode
 */

ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    
    /* Code section */
    .text : {
        KEEP(*(.text.boot))    /* Boot code first */
        *(.text .text.*)
    }
    
    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
    }
    
    /* Initialized data */
    .data : {
        *(.data .data.*)
    }
    
    /* BSS - zero-initialized data */
    .bss ALIGN(16) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    __bss_size = (__bss_end - __bss_start) >> 3;
    
    /* Stack grows down from 0x80000 */
    . = ALIGN(16);
    __end = .;
}
