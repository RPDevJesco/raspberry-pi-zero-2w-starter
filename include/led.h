/*
 * led.h - Activity LED Control
 * 
 * Pi Zero 2 W ACT LED is on GPIO 29 (directly controlled, active LOW)
 */

#ifndef LED_H
#define LED_H

#include "types.h"
#include "gpio.h"

#define ACT_LED_PIN     29

/* Initialize ACT LED GPIO as output */
static inline void led_init(void) {
    /* GPIO 29 is in GPFSEL2 (pins 20-29), bits 27-29 */
    uint32_t sel = *GPFSEL2;
    sel &= ~(7 << 27);          /* Clear bits 27-29 */
    sel |= (1 << 27);           /* Set as output (001) */
    *GPFSEL2 = sel;
}

/* Turn LED on (active low) */
static inline void led_on(void) {
    *GPCLR0 = (1 << ACT_LED_PIN);
}

/* Turn LED off */
static inline void led_off(void) {
    *GPSET0 = (1 << ACT_LED_PIN);
}

/* Blink LED n times */
static inline void led_blink(int count, uint32_t delay_cycles) {
    for (int i = 0; i < count; i++) {
        led_on();
        delay(delay_cycles);
        led_off();
        delay(delay_cycles);
    }
}

#endif /* LED_H */
