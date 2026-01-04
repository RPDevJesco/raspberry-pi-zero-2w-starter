/*
 * gpio.h - BCM2710 GPIO Definitions
 * 
 * Note: Pi Zero 2 W uses BCM2710 which has peripheral base at 0x3F000000
 * (same as BCM2835/2836 low peripheral mode, NOT the 0xFE000000 of BCM2711)
 */

#ifndef GPIO_H
#define GPIO_H

#include "types.h"

/* BCM2710 Peripheral Base Address */
#define PERIPHERAL_BASE     0x3F000000

/* GPIO Registers */
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000)

#define GPFSEL0             ((volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1             ((volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2             ((volatile uint32_t*)(GPIO_BASE + 0x08))
#define GPFSEL3             ((volatile uint32_t*)(GPIO_BASE + 0x0C))
#define GPFSEL4             ((volatile uint32_t*)(GPIO_BASE + 0x10))
#define GPFSEL5             ((volatile uint32_t*)(GPIO_BASE + 0x14))

#define GPSET0              ((volatile uint32_t*)(GPIO_BASE + 0x1C))
#define GPSET1              ((volatile uint32_t*)(GPIO_BASE + 0x20))

#define GPCLR0              ((volatile uint32_t*)(GPIO_BASE + 0x28))
#define GPCLR1              ((volatile uint32_t*)(GPIO_BASE + 0x2C))

#define GPLEV0              ((volatile uint32_t*)(GPIO_BASE + 0x34))
#define GPLEV1              ((volatile uint32_t*)(GPIO_BASE + 0x38))

#define GPPUD               ((volatile uint32_t*)(GPIO_BASE + 0x94))
#define GPPUDCLK0           ((volatile uint32_t*)(GPIO_BASE + 0x98))
#define GPPUDCLK1           ((volatile uint32_t*)(GPIO_BASE + 0x9C))

/* GPIO Function Select Values */
#define GPIO_FUNC_INPUT     0
#define GPIO_FUNC_OUTPUT    1
#define GPIO_FUNC_ALT0      4
#define GPIO_FUNC_ALT1      5
#define GPIO_FUNC_ALT2      6
#define GPIO_FUNC_ALT3      7
#define GPIO_FUNC_ALT4      3
#define GPIO_FUNC_ALT5      2

/* Delay function */
static inline void delay(uint32_t count) {
    while (count--) {
        asm volatile("nop");
    }
}

#endif /* GPIO_H */
