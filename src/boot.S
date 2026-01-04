/*
 * boot.S - Raspberry Pi Zero 2 W Boot Assembly
 * AArch64 entry point for BCM2710A1 (Cortex-A53)
 * 
 * On boot, all 4 cores start executing. We park cores 1-3 and
 * let core 0 continue to kernel_main.
 */

.section ".text.boot"

.global _start

_start:
    /* Read core ID from MPIDR_EL1 */
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    
    /* If not core 0, park it */
    cbz     x0, core0_boot
    
park_loop:
    wfe                         /* Wait for event (low power) */
    b       park_loop

core0_boot:
    /* Set up stack pointer (grows down from 0x80000) */
    ldr     x0, =_start
    mov     sp, x0
    
    /* Clear BSS section */
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
    cbz     x1, bss_done
    
bss_clear:
    str     xzr, [x0], #8
    subs    x1, x1, #1
    bne     bss_clear
    
bss_done:
    /* Jump to C kernel_main */
    bl      kernel_main
    
    /* If kernel_main returns, halt */
halt:
    wfe
    b       halt
