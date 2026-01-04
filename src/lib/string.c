/*
 * string.c - String Utilities Implementation
 */

#include "string.h"

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

static const char hex_chars[] = "0123456789ABCDEF";

void utoa(uint32_t value, char *buffer, int base) {
    char tmp[33];
    char *p = tmp;
    
    if (value == 0) {
        *buffer++ = '0';
        *buffer = '\0';
        return;
    }
    
    while (value) {
        *p++ = hex_chars[value % base];
        value /= base;
    }
    
    while (p > tmp) {
        *buffer++ = *--p;
    }
    *buffer = '\0';
}

void itoa(int32_t value, char *buffer, int base) {
    if (value < 0 && base == 10) {
        *buffer++ = '-';
        value = -value;
    }
    utoa((uint32_t)value, buffer, base);
}

void u64toa(uint64_t value, char *buffer, int base) {
    char tmp[65];
    char *p = tmp;
    
    if (value == 0) {
        *buffer++ = '0';
        *buffer = '\0';
        return;
    }
    
    while (value) {
        *p++ = hex_chars[value % base];
        value /= base;
    }
    
    while (p > tmp) {
        *buffer++ = *--p;
    }
    *buffer = '\0';
}

void format_hex32(uint32_t value, char *buffer) {
    *buffer++ = '0';
    *buffer++ = 'x';
    
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    buffer[8] = '\0';
}

void format_dec(uint32_t value, char *buffer) {
    utoa(value, buffer, 10);
}

void format_mhz(uint32_t hz, char *buffer) {
    uint32_t mhz = hz / 1000000;
    utoa(mhz, buffer, 10);
    
    /* Find end */
    while (*buffer) buffer++;
    
    *buffer++ = ' ';
    *buffer++ = 'M';
    *buffer++ = 'H';
    *buffer++ = 'z';
    *buffer = '\0';
}

void format_mb(uint32_t bytes, char *buffer) {
    uint32_t mb = bytes / (1024 * 1024);
    utoa(mb, buffer, 10);
    
    /* Find end */
    while (*buffer) buffer++;
    
    *buffer++ = ' ';
    *buffer++ = 'M';
    *buffer++ = 'B';
    *buffer = '\0';
}

void format_mac(uint8_t *mac, char *buffer) {
    for (int i = 0; i < 6; i++) {
        *buffer++ = hex_chars[(mac[i] >> 4) & 0xF];
        *buffer++ = hex_chars[mac[i] & 0xF];
        if (i < 5) {
            *buffer++ = ':';
        }
    }
    *buffer = '\0';
}
