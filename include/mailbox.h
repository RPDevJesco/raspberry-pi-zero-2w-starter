/*
 * mailbox.h - VideoCore Mailbox Interface
 * 
 * The mailbox is the communication channel between ARM and VideoCore GPU.
 * We use it to configure framebuffer, query hardware info, etc.
 */

#ifndef MAILBOX_H
#define MAILBOX_H

#include "types.h"
#include "gpio.h"

/* Mailbox Registers */
#define MAILBOX_BASE        (PERIPHERAL_BASE + 0xB880)

#define MAILBOX_READ        ((volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MAILBOX_POLL        ((volatile uint32_t*)(MAILBOX_BASE + 0x10))
#define MAILBOX_SENDER      ((volatile uint32_t*)(MAILBOX_BASE + 0x14))
#define MAILBOX_STATUS      ((volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MAILBOX_CONFIG      ((volatile uint32_t*)(MAILBOX_BASE + 0x1C))
#define MAILBOX_WRITE       ((volatile uint32_t*)(MAILBOX_BASE + 0x20))

/* Mailbox Status Bits */
#define MAILBOX_FULL        0x80000000
#define MAILBOX_EMPTY       0x40000000

/* Mailbox Channels */
#define MAILBOX_CH_POWER    0
#define MAILBOX_CH_FB       1
#define MAILBOX_CH_VUART    2
#define MAILBOX_CH_VCHIQ    3
#define MAILBOX_CH_LED      4
#define MAILBOX_CH_BTN      5
#define MAILBOX_CH_TOUCH    6
#define MAILBOX_CH_COUNT    7
#define MAILBOX_CH_PROP     8   /* Property channel (ARM -> VC) */

/* Property Tags */
#define TAG_END             0x00000000

/* VideoCore Tags */
#define TAG_GET_FIRMWARE    0x00000001
#define TAG_GET_BOARD_MODEL 0x00010001
#define TAG_GET_BOARD_REV   0x00010002
#define TAG_GET_MAC_ADDR    0x00010003
#define TAG_GET_BOARD_SERIAL 0x00010004
#define TAG_GET_ARM_MEMORY  0x00010005
#define TAG_GET_VC_MEMORY   0x00010006
#define TAG_GET_CLOCKS      0x00010007

/* Framebuffer Tags */
#define TAG_FB_ALLOC        0x00040001
#define TAG_FB_RELEASE      0x00048001
#define TAG_FB_GET_PHYS_WH  0x00040003
#define TAG_FB_SET_PHYS_WH  0x00048003
#define TAG_FB_GET_VIRT_WH  0x00040004
#define TAG_FB_SET_VIRT_WH  0x00048004
#define TAG_FB_GET_DEPTH    0x00040005
#define TAG_FB_SET_DEPTH    0x00048005
#define TAG_FB_GET_PITCH    0x00040008
#define TAG_FB_SET_VIRT_OFF 0x00048009
#define TAG_FB_GET_PALETTE  0x0004000B
#define TAG_FB_SET_PALETTE  0x0004800B

/* Clock Tags */
#define TAG_GET_CLOCK_RATE  0x00030002
#define TAG_GET_MAX_CLOCK   0x00030004
#define TAG_GET_MIN_CLOCK   0x00030007
#define TAG_SET_CLOCK_RATE  0x00038002

/* Clock IDs */
#define CLOCK_ID_EMMC       1
#define CLOCK_ID_UART       2
#define CLOCK_ID_ARM        3
#define CLOCK_ID_CORE       4
#define CLOCK_ID_V3D        5
#define CLOCK_ID_H264       6
#define CLOCK_ID_ISP        7
#define CLOCK_ID_SDRAM      8
#define CLOCK_ID_PIXEL      9
#define CLOCK_ID_PWM        10

/* Mailbox message buffer - must be 16-byte aligned */
extern volatile uint32_t __attribute__((aligned(16))) mailbox_buffer[256];

/* Functions */
bool mailbox_call(uint8_t channel);
uint32_t mailbox_read(uint8_t channel);
void mailbox_write(uint8_t channel, uint32_t data);

#endif /* MAILBOX_H */
