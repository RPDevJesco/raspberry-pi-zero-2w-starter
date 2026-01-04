/*
 * framebuffer.c - HDMI Framebuffer Driver
 * 
 * Uses VideoCore mailbox to allocate framebuffer and provides
 * drawing primitives for text and graphics.
 */

#include "framebuffer.h"
#include "mailbox.h"
#include "font8x8.h"

/* Global framebuffer info */
static framebuffer_t fb_info;

/*
 * fb_init - Initialize framebuffer via mailbox
 * @width: Desired width in pixels
 * @height: Desired height in pixels  
 * @depth: Bits per pixel (typically 32)
 * Returns: true on success
 */
bool fb_init(uint32_t width, uint32_t height, uint32_t depth) {
    /* Build property message to set up framebuffer */
    uint32_t i = 0;
    
    mailbox_buffer[i++] = 0;                    /* Size (fill later) */
    mailbox_buffer[i++] = 0;                    /* Request code */
    
    /* Set physical display size */
    mailbox_buffer[i++] = TAG_FB_SET_PHYS_WH;
    mailbox_buffer[i++] = 8;                    /* Value buffer size */
    mailbox_buffer[i++] = 0;                    /* Request/response code */
    mailbox_buffer[i++] = width;                /* Width */
    mailbox_buffer[i++] = height;               /* Height */
    
    /* Set virtual display size (same as physical) */
    mailbox_buffer[i++] = TAG_FB_SET_VIRT_WH;
    mailbox_buffer[i++] = 8;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = width;
    mailbox_buffer[i++] = height;
    
    /* Set virtual offset to 0,0 */
    mailbox_buffer[i++] = TAG_FB_SET_VIRT_OFF;
    mailbox_buffer[i++] = 8;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = 0;                    /* X offset */
    mailbox_buffer[i++] = 0;                    /* Y offset */
    
    /* Set color depth */
    mailbox_buffer[i++] = TAG_FB_SET_DEPTH;
    mailbox_buffer[i++] = 4;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = depth;
    
    /* Allocate framebuffer */
    mailbox_buffer[i++] = TAG_FB_ALLOC;
    mailbox_buffer[i++] = 8;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = 16;                   /* Alignment (16 bytes) */
    mailbox_buffer[i++] = 0;                    /* Size (response) */
    
    /* Get pitch (bytes per row) */
    mailbox_buffer[i++] = TAG_FB_GET_PITCH;
    mailbox_buffer[i++] = 4;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = 0;                    /* Pitch (response) */
    
    /* End tag */
    mailbox_buffer[i++] = TAG_END;
    
    /* Set message size */
    mailbox_buffer[0] = i * 4;
    
    /* Send to GPU */
    if (!mailbox_call(MAILBOX_CH_PROP)) {
        return false;
    }
    
    /* Check if we got a framebuffer - address is at index 24 */
    if (mailbox_buffer[24] == 0) {
        return false;
    }
    
    /* Extract framebuffer info from response */
    fb_info.width = width;
    fb_info.height = height;
    fb_info.depth = depth;
    
    /* Framebuffer address - convert from bus address to ARM address */
    /* Bus address has 0xC0000000 bit set, ARM sees it at 0x00000000 base */
    fb_info.buffer = (uint8_t*)(uint64_t)(mailbox_buffer[24] & 0x3FFFFFFF);
    fb_info.size = mailbox_buffer[25];
    fb_info.pitch = mailbox_buffer[29];
    
    return true;
}

/*
 * fb_get_info - Get framebuffer info structure
 */
framebuffer_t *fb_get_info(void) {
    return &fb_info;
}

/*
 * fb_put_pixel - Draw a single pixel
 */
void fb_put_pixel(uint32_t x, uint32_t y, color_t color) {
    if (x >= fb_info.width || y >= fb_info.height) {
        return;
    }
    
    uint32_t offset = (y * fb_info.pitch) + (x * (fb_info.depth / 8));
    
    /* Assuming 32-bit BGRA format */
    fb_info.buffer[offset + 0] = color.b;
    fb_info.buffer[offset + 1] = color.g;
    fb_info.buffer[offset + 2] = color.r;
    fb_info.buffer[offset + 3] = color.a;
}

/*
 * fb_fill_rect - Fill a rectangle with color
 */
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color) {
    for (uint32_t py = y; py < y + h && py < fb_info.height; py++) {
        for (uint32_t px = x; px < x + w && px < fb_info.width; px++) {
            fb_put_pixel(px, py, color);
        }
    }
}

/*
 * fb_clear - Clear entire screen with color
 */
void fb_clear(color_t color) {
    fb_fill_rect(0, 0, fb_info.width, fb_info.height, color);
}

/*
 * fb_draw_char - Draw a single character
 * @x, @y: Top-left position
 * @c: Character to draw
 * @fg: Foreground color
 * @bg: Background color
 */
void fb_draw_char(uint32_t x, uint32_t y, char c, color_t fg, color_t bg) {
    /* Only handle printable ASCII */
    if (c < 32 || c > 126) {
        c = '?';
    }
    
    const uint8_t *glyph = font8x8[c - 32];
    
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            /* Check if pixel is set (MSB first) */
            if (bits & (0x80 >> col)) {
                fb_put_pixel(x + col, y + row, fg);
            } else {
                fb_put_pixel(x + col, y + row, bg);
            }
        }
    }
}

/*
 * fb_draw_string - Draw a null-terminated string
 */
void fb_draw_string(uint32_t x, uint32_t y, const char *str, color_t fg, color_t bg) {
    uint32_t orig_x = x;
    
    while (*str) {
        if (*str == '\n') {
            x = orig_x;
            y += FONT_HEIGHT + 2;  /* Line spacing */
        } else {
            fb_draw_char(x, y, *str, fg, bg);
            x += FONT_WIDTH;
        }
        str++;
    }
}
