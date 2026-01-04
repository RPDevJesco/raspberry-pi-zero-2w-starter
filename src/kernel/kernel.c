/*
 * kernel.c - Main Kernel Entry Point
 * Raspberry Pi Zero 2 W Custom Kernel
 * 
 * Displays system specifications on HDMI output
 * in classic green-on-black terminal style.
 */

#include "types.h"
#include "framebuffer.h"
#include "mailbox.h"
#include "sysinfo.h"
#include "string.h"
#include "led.h"

/* Display resolution */
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define COLOR_DEPTH     32

/* Terminal colors */
#define FG_COLOR        COLOR_TERM_GREEN
#define BG_COLOR        COLOR_BLACK

/* Text layout */
#define MARGIN_X        40
#define MARGIN_Y        40
#define LINE_HEIGHT     12      /* 8px font + 4px spacing */

/* Delay for visible LED blinks */
#define BLINK_DELAY     500000

/* Draw a horizontal line */
static void draw_hline(uint32_t y, uint32_t width) {
    framebuffer_t *fb = fb_get_info();
    for (uint32_t x = MARGIN_X; x < MARGIN_X + width && x < fb->width; x++) {
        fb_put_pixel(x, y, FG_COLOR);
    }
}

/* Draw a box border */
static void draw_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    /* Top and bottom lines */
    for (uint32_t i = 0; i < w; i++) {
        fb_put_pixel(x + i, y, FG_COLOR);
        fb_put_pixel(x + i, y + h - 1, FG_COLOR);
    }
    /* Left and right lines */
    for (uint32_t i = 0; i < h; i++) {
        fb_put_pixel(x, y + i, FG_COLOR);
        fb_put_pixel(x + w - 1, y + i, FG_COLOR);
    }
}

/* Print a labeled value */
static uint32_t print_info_line(uint32_t y, const char *label, const char *value) {
    fb_draw_string(MARGIN_X + 8, y, label, FG_COLOR, BG_COLOR);
    fb_draw_string(MARGIN_X + 200, y, value, FG_COLOR, BG_COLOR);
    return y + LINE_HEIGHT;
}

/* Main kernel entry point (called from boot.S) */
void kernel_main(void) {
    sysinfo_t sysinfo;
    char buffer[128];
    uint32_t y;
    
    /* Initialize LED for debugging */
    led_init();
    
    /* Blink 1: Kernel started */
    led_blink(1, BLINK_DELAY);
    delay(BLINK_DELAY * 2);
    
    /* Initialize framebuffer */
    if (!fb_init(SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH)) {
        /* FB failed - blink rapidly forever */
        while (1) {
            led_blink(5, BLINK_DELAY / 5);
            delay(BLINK_DELAY * 2);
        }
    }
    
    /* Blink 2: Framebuffer initialized */
    led_blink(2, BLINK_DELAY);
    delay(BLINK_DELAY * 2);
    
    /* Clear screen to black */
    fb_clear(BG_COLOR);
    
    /* Blink 3: Screen cleared */
    led_blink(3, BLINK_DELAY);
    
    /* Query system information */
    sysinfo_init(&sysinfo);
    
    /* === Draw Header === */
    y = MARGIN_Y;
    
    /* Title banner */
    draw_box(MARGIN_X, y, 600, 50);
    fb_draw_string(MARGIN_X + 16, y + 12, "RASPBERRY PI ZERO 2 W", FG_COLOR, BG_COLOR);
    fb_draw_string(MARGIN_X + 16, y + 28, "Custom Bare-Metal Kernel v1.0", FG_COLOR, BG_COLOR);
    
    y += 70;
    
    /* === Board Information === */
    fb_draw_string(MARGIN_X, y, "=== BOARD INFORMATION ===", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT + 8;
    
    /* Model name */
    const char *model_name = sysinfo_get_model_name(sysinfo.board_revision);
    y = print_info_line(y, "Model:", model_name);
    
    /* Board revision */
    format_hex32(sysinfo.board_revision, buffer);
    y = print_info_line(y, "Revision:", buffer);
    
    /* Serial number */
    u64toa(sysinfo.serial_number, buffer, 16);
    y = print_info_line(y, "Serial:", buffer);
    
    /* Firmware version */
    format_dec(sysinfo.firmware_version, buffer);
    y = print_info_line(y, "Firmware:", buffer);
    
    y += 16;
    
    /* === Processor Information === */
    fb_draw_string(MARGIN_X, y, "=== PROCESSOR ===", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT + 8;
    
    y = print_info_line(y, "SoC:", "BCM2710A1 (Broadcom)");
    y = print_info_line(y, "CPU:", "Quad-core ARM Cortex-A53");
    y = print_info_line(y, "Architecture:", "ARMv8-A (64-bit)");
    
    /* ARM clock speed */
    format_mhz(sysinfo.arm_clock, buffer);
    y = print_info_line(y, "ARM Clock:", buffer);
    
    /* Core clock speed */
    format_mhz(sysinfo.core_clock, buffer);
    y = print_info_line(y, "Core Clock:", buffer);
    
    y += 16;
    
    /* === Memory Information === */
    fb_draw_string(MARGIN_X, y, "=== MEMORY ===", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT + 8;
    
    /* ARM memory */
    format_mb(sysinfo.arm_mem_size, buffer);
    strcat(buffer, " @ ");
    char addr[16];
    format_hex32(sysinfo.arm_mem_base, addr);
    strcat(buffer, addr);
    y = print_info_line(y, "ARM Memory:", buffer);
    
    /* VideoCore memory */
    format_mb(sysinfo.vc_mem_size, buffer);
    strcat(buffer, " @ ");
    format_hex32(sysinfo.vc_mem_base, addr);
    strcat(buffer, addr);
    y = print_info_line(y, "GPU Memory:", buffer);
    
    /* SDRAM clock */
    format_mhz(sysinfo.sdram_clock, buffer);
    y = print_info_line(y, "SDRAM Clock:", buffer);
    
    y += 16;
    
    /* === Network Information === */
    fb_draw_string(MARGIN_X, y, "=== NETWORK ===", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT + 8;
    
    y = print_info_line(y, "WiFi:", "802.11 b/g/n (2.4 GHz)");
    y = print_info_line(y, "Bluetooth:", "Bluetooth 4.2, BLE");
    
    /* MAC address */
    format_mac(sysinfo.mac_address, buffer);
    y = print_info_line(y, "MAC Address:", buffer);
    
    y += 16;
    
    /* === Display Information === */
    fb_draw_string(MARGIN_X, y, "=== DISPLAY ===", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT + 8;
    
    framebuffer_t *fb = fb_get_info();
    
    /* Resolution */
    utoa(fb->width, buffer, 10);
    strcat(buffer, " x ");
    char tmp[16];
    utoa(fb->height, tmp, 10);
    strcat(buffer, tmp);
    strcat(buffer, " @ ");
    utoa(fb->depth, tmp, 10);
    strcat(buffer, tmp);
    strcat(buffer, "bpp");
    y = print_info_line(y, "Resolution:", buffer);
    
    /* Pitch */
    utoa(fb->pitch, buffer, 10);
    strcat(buffer, " bytes/row");
    y = print_info_line(y, "Pitch:", buffer);
    
    /* Framebuffer address */
    format_hex32((uint32_t)(uint64_t)fb->buffer, buffer);
    y = print_info_line(y, "FB Address:", buffer);
    
    /* Framebuffer size */
    format_mb(fb->size, buffer);
    y = print_info_line(y, "FB Size:", buffer);
    
    y += 24;
    
    /* === Footer === */
    draw_hline(y, 600);
    y += 8;
    fb_draw_string(MARGIN_X, y, "Kernel loaded at 0x80000 | Running on Core 0", FG_COLOR, BG_COLOR);
    y += LINE_HEIGHT;
    fb_draw_string(MARGIN_X, y, "Cores 1-3 parked in WFE loop", FG_COLOR, BG_COLOR);
    
    /* Draw decorative element - blinking cursor simulation */
    y += 24;
    fb_draw_string(MARGIN_X, y, "> System ready _", FG_COLOR, BG_COLOR);
    
    /* Success - slow heartbeat blink */
    while (1) {
        led_on();
        delay(BLINK_DELAY / 2);
        led_off();
        delay(BLINK_DELAY * 4);
    }
}
