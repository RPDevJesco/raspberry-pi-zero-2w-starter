/*
 * sysinfo.h - System Information Query
 * 
 * Functions to query hardware specs via VideoCore mailbox.
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#include "types.h"

/* System info structure */
typedef struct {
    uint32_t board_model;
    uint32_t board_revision;
    uint64_t serial_number;
    uint32_t firmware_version;
    
    uint32_t arm_mem_base;
    uint32_t arm_mem_size;
    uint32_t vc_mem_base;
    uint32_t vc_mem_size;
    
    uint32_t arm_clock;         /* Hz */
    uint32_t core_clock;
    uint32_t sdram_clock;
    
    uint8_t mac_address[6];
} sysinfo_t;

/* Board models */
#define BOARD_MODEL_A       0
#define BOARD_MODEL_B       1
#define BOARD_MODEL_A_PLUS  2
#define BOARD_MODEL_B_PLUS  3
#define BOARD_MODEL_2B      4
#define BOARD_MODEL_ALPHA   5
#define BOARD_MODEL_CM1     6
#define BOARD_MODEL_3B      8
#define BOARD_MODEL_ZERO    9
#define BOARD_MODEL_CM3     10
#define BOARD_MODEL_ZERO_W  12
#define BOARD_MODEL_3B_PLUS 13
#define BOARD_MODEL_3A_PLUS 14
#define BOARD_MODEL_CM3_PLUS 16
#define BOARD_MODEL_4B      17
#define BOARD_MODEL_ZERO_2W 18
#define BOARD_MODEL_400     19
#define BOARD_MODEL_CM4     20

/* Functions */
bool sysinfo_init(sysinfo_t *info);
const char *sysinfo_get_model_name(uint32_t revision);
void sysinfo_get_revision_info(uint32_t revision, char *buffer);

#endif /* SYSINFO_H */
