/*
 * mailbox.c - VideoCore Mailbox Implementation
 * 
 * Handles communication between ARM and VideoCore GPU.
 */

#include "mailbox.h"

/* Shared mailbox buffer - 16-byte aligned for DMA */
volatile uint32_t __attribute__((aligned(16))) mailbox_buffer[256];

/*
 * mailbox_write - Write to mailbox
 * @channel: Channel number (0-15)
 * @data: Data to write (must be 16-byte aligned address)
 */
void mailbox_write(uint8_t channel, uint32_t data) {
    /* Wait until mailbox is not full */
    while (*MAILBOX_STATUS & MAILBOX_FULL) {
        asm volatile("nop");
    }
    
    /* Write address + channel (lower 4 bits) */
    *MAILBOX_WRITE = (data & 0xFFFFFFF0) | (channel & 0xF);
}

/*
 * mailbox_read - Read from mailbox
 * @channel: Channel number to read from
 * Returns: Data from mailbox
 */
uint32_t mailbox_read(uint8_t channel) {
    uint32_t data;
    
    while (1) {
        /* Wait for data */
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
            asm volatile("nop");
        }
        
        data = *MAILBOX_READ;
        
        /* Check if it's our channel */
        if ((data & 0xF) == channel) {
            return data & 0xFFFFFFF0;
        }
    }
}

/*
 * mailbox_call - Send message and wait for response
 * @channel: Channel to use (typically MAILBOX_CH_PROP)
 * Returns: true on success
 * 
 * Uses the global mailbox_buffer for the message.
 */
bool mailbox_call(uint8_t channel) {
    /* Get physical address of buffer (in lower 1GB, identity mapped) */
    uint32_t addr = (uint32_t)(uint64_t)&mailbox_buffer;
    
    /* Write buffer address to mailbox */
    mailbox_write(channel, addr);
    
    /* Wait for response */
    while (1) {
        /* Wait for data */
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
            asm volatile("nop");
        }
        
        uint32_t data = *MAILBOX_READ;
        
        /* Check if response is for us */
        if ((data & 0xF) == channel) {
            /* Check response code in buffer */
            return mailbox_buffer[1] == 0x80000000;
        }
    }
}
