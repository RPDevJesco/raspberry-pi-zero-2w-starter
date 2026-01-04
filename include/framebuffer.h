/*
 * framebuffer.h - HDMI Framebuffer Interface
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "types.h"

/* Framebuffer info structure */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;         /* Bytes per row */
    uint32_t depth;         /* Bits per pixel */
    uint8_t *buffer;        /* Framebuffer address */
    uint32_t size;          /* Buffer size in bytes */
} framebuffer_t;

/* Color in RGB format (for 32-bit depth) */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} color_t;

/* Predefined colors */
#define COLOR_BLACK     (color_t){0x00, 0x00, 0x00, 0xFF}
#define COLOR_WHITE     (color_t){0xFF, 0xFF, 0xFF, 0xFF}
#define COLOR_RED       (color_t){0xFF, 0x00, 0x00, 0xFF}
#define COLOR_GREEN     (color_t){0x00, 0xFF, 0x00, 0xFF}
#define COLOR_BLUE      (color_t){0x00, 0x00, 0xFF, 0xFF}
#define COLOR_YELLOW    (color_t){0xFF, 0xFF, 0x00, 0xFF}
#define COLOR_CYAN      (color_t){0x00, 0xFF, 0xFF, 0xFF}
#define COLOR_MAGENTA   (color_t){0xFF, 0x00, 0xFF, 0xFF}

/* Terminal green (classic CRT look) */
#define COLOR_TERM_GREEN (color_t){0x33, 0xFF, 0x33, 0xFF}

/* Functions */
bool fb_init(uint32_t width, uint32_t height, uint32_t depth);
framebuffer_t *fb_get_info(void);
void fb_put_pixel(uint32_t x, uint32_t y, color_t color);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
void fb_clear(color_t color);
void fb_draw_char(uint32_t x, uint32_t y, char c, color_t fg, color_t bg);
void fb_draw_string(uint32_t x, uint32_t y, const char *str, color_t fg, color_t bg);

#endif /* FRAMEBUFFER_H */
