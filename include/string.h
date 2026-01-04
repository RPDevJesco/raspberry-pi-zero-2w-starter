/*
 * string.h - String Utilities
 */

#ifndef STRING_H
#define STRING_H

#include "types.h"

/* Memory operations */
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

/* String operations */
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);

/* Number to string conversion */
void itoa(int32_t value, char *buffer, int base);
void utoa(uint32_t value, char *buffer, int base);
void u64toa(uint64_t value, char *buffer, int base);

/* Formatted number to string (with padding) */
void format_hex32(uint32_t value, char *buffer);
void format_dec(uint32_t value, char *buffer);
void format_mhz(uint32_t hz, char *buffer);
void format_mb(uint32_t bytes, char *buffer);
void format_mac(uint8_t *mac, char *buffer);

#endif /* STRING_H */
