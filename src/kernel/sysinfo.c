/*
 * sysinfo.c - System Information Implementation
 * 
 * Queries Raspberry Pi hardware specs using VideoCore mailbox interface.
 */

#include "sysinfo.h"
#include "mailbox.h"

/*
 * Helper to send a simple property request
 */
static bool query_property(uint32_t tag, uint32_t req_size, uint32_t *response, uint32_t resp_count) {
    uint32_t i = 0;
    
    mailbox_buffer[i++] = 0;                    /* Size (fill later) */
    mailbox_buffer[i++] = 0;                    /* Request code */
    mailbox_buffer[i++] = tag;
    mailbox_buffer[i++] = req_size;
    mailbox_buffer[i++] = 0;                    /* Request/response code */
    
    /* Clear response area */
    for (uint32_t j = 0; j < resp_count; j++) {
        mailbox_buffer[i++] = 0;
    }
    
    mailbox_buffer[i++] = TAG_END;
    mailbox_buffer[0] = i * 4;
    
    if (!mailbox_call(MAILBOX_CH_PROP)) {
        return false;
    }
    
    /* Copy response */
    for (uint32_t j = 0; j < resp_count; j++) {
        response[j] = mailbox_buffer[5 + j];
    }
    
    return true;
}

/*
 * Query clock rate for a specific clock
 */
static uint32_t query_clock_rate(uint32_t clock_id) {
    uint32_t i = 0;
    
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = TAG_GET_CLOCK_RATE;
    mailbox_buffer[i++] = 8;
    mailbox_buffer[i++] = 0;
    mailbox_buffer[i++] = clock_id;
    mailbox_buffer[i++] = 0;                    /* Rate (response) */
    mailbox_buffer[i++] = TAG_END;
    mailbox_buffer[0] = i * 4;
    
    if (!mailbox_call(MAILBOX_CH_PROP)) {
        return 0;
    }
    
    return mailbox_buffer[6];
}

/*
 * sysinfo_init - Populate system info structure
 */
bool sysinfo_init(sysinfo_t *info) {
    uint32_t resp[4];
    
    /* Clear structure */
    for (int i = 0; i < sizeof(sysinfo_t); i++) {
        ((uint8_t*)info)[i] = 0;
    }
    
    /* Get firmware version */
    if (query_property(TAG_GET_FIRMWARE, 4, resp, 1)) {
        info->firmware_version = resp[0];
    }
    
    /* Get board model */
    if (query_property(TAG_GET_BOARD_MODEL, 4, resp, 1)) {
        info->board_model = resp[0];
    }
    
    /* Get board revision */
    if (query_property(TAG_GET_BOARD_REV, 4, resp, 1)) {
        info->board_revision = resp[0];
    }
    
    /* Get serial number */
    if (query_property(TAG_GET_BOARD_SERIAL, 8, resp, 2)) {
        info->serial_number = ((uint64_t)resp[1] << 32) | resp[0];
    }
    
    /* Get ARM memory */
    if (query_property(TAG_GET_ARM_MEMORY, 8, resp, 2)) {
        info->arm_mem_base = resp[0];
        info->arm_mem_size = resp[1];
    }
    
    /* Get VideoCore memory */
    if (query_property(TAG_GET_VC_MEMORY, 8, resp, 2)) {
        info->vc_mem_base = resp[0];
        info->vc_mem_size = resp[1];
    }
    
    /* Get MAC address */
    if (query_property(TAG_GET_MAC_ADDR, 6, resp, 2)) {
        info->mac_address[0] = (resp[0] >> 0) & 0xFF;
        info->mac_address[1] = (resp[0] >> 8) & 0xFF;
        info->mac_address[2] = (resp[0] >> 16) & 0xFF;
        info->mac_address[3] = (resp[0] >> 24) & 0xFF;
        info->mac_address[4] = (resp[1] >> 0) & 0xFF;
        info->mac_address[5] = (resp[1] >> 8) & 0xFF;
    }
    
    /* Get clock rates */
    info->arm_clock = query_clock_rate(CLOCK_ID_ARM);
    info->core_clock = query_clock_rate(CLOCK_ID_CORE);
    info->sdram_clock = query_clock_rate(CLOCK_ID_SDRAM);
    
    return true;
}

/*
 * Decode board type from new-style revision code
 */
static uint32_t decode_board_type(uint32_t revision) {
    if (revision & 0x800000) {
        /* New-style revision */
        return (revision >> 4) & 0xFF;
    }
    /* Old-style - map manually */
    return 0xFF;
}

/*
 * sysinfo_get_model_name - Get human-readable model name
 */
const char *sysinfo_get_model_name(uint32_t revision) {
    uint32_t type = decode_board_type(revision);
    
    switch (type) {
        case 0x00: return "Raspberry Pi Model A";
        case 0x01: return "Raspberry Pi Model B";
        case 0x02: return "Raspberry Pi Model A+";
        case 0x03: return "Raspberry Pi Model B+";
        case 0x04: return "Raspberry Pi 2 Model B";
        case 0x06: return "Raspberry Pi Compute Module 1";
        case 0x08: return "Raspberry Pi 3 Model B";
        case 0x09: return "Raspberry Pi Zero";
        case 0x0A: return "Raspberry Pi Compute Module 3";
        case 0x0C: return "Raspberry Pi Zero W";
        case 0x0D: return "Raspberry Pi 3 Model B+";
        case 0x0E: return "Raspberry Pi 3 Model A+";
        case 0x10: return "Raspberry Pi Compute Module 3+";
        case 0x11: return "Raspberry Pi 4 Model B";
        case 0x12: return "Raspberry Pi Zero 2 W";
        case 0x13: return "Raspberry Pi 400";
        case 0x14: return "Raspberry Pi Compute Module 4";
        case 0x17: return "Raspberry Pi 5";
        default:   return "Unknown Raspberry Pi";
    }
}

/*
 * Decode processor from revision
 */
static const char *decode_processor(uint32_t revision) {
    if (!(revision & 0x800000)) {
        return "BCM2835";
    }
    
    uint32_t proc = (revision >> 12) & 0xF;
    switch (proc) {
        case 0: return "BCM2835 (ARM1176JZF-S)";
        case 1: return "BCM2836 (Cortex-A7)";
        case 2: return "BCM2837 (Cortex-A53)";
        case 3: return "BCM2711 (Cortex-A72)";
        case 4: return "BCM2712 (Cortex-A76)";
        default: return "Unknown SoC";
    }
}

/*
 * Decode memory size from revision
 */
static const char *decode_memory(uint32_t revision) {
    if (!(revision & 0x800000)) {
        return "256MB/512MB";
    }
    
    uint32_t mem = (revision >> 20) & 0x7;
    switch (mem) {
        case 0: return "256 MB";
        case 1: return "512 MB";
        case 2: return "1 GB";
        case 3: return "2 GB";
        case 4: return "4 GB";
        case 5: return "8 GB";
        default: return "Unknown";
    }
}

/*
 * Decode manufacturer from revision
 */
static const char *decode_manufacturer(uint32_t revision) {
    if (!(revision & 0x800000)) {
        return "Unknown";
    }
    
    uint32_t mfr = (revision >> 16) & 0xF;
    switch (mfr) {
        case 0: return "Sony UK";
        case 1: return "Egoman";
        case 2: return "Embest";
        case 3: return "Sony Japan";
        case 4: return "Embest";
        case 5: return "Stadium";
        default: return "Unknown";
    }
}

/*
 * sysinfo_get_revision_info - Build detailed revision string
 */
void sysinfo_get_revision_info(uint32_t revision, char *buffer) {
    const char *proc = decode_processor(revision);
    const char *mem = decode_memory(revision);
    const char *mfr = decode_manufacturer(revision);
    
    /* Simple string concatenation */
    char *p = buffer;
    const char *s;
    
    s = "SoC: ";
    while (*s) *p++ = *s++;
    while (*proc) *p++ = *proc++;
    *p++ = '\n';
    
    s = "RAM: ";
    while (*s) *p++ = *s++;
    while (*mem) *p++ = *mem++;
    *p++ = '\n';
    
    s = "Mfr: ";
    while (*s) *p++ = *s++;
    while (*mfr) *p++ = *mfr++;
    
    *p = '\0';
}
