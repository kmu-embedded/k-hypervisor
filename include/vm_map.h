/*
 * vm_map.h
 *
 *  Created on: 18 Mar 2016
 *      Author: wonseok
 */

#ifndef __VM_MMAP_H__
#define __VM_MMAP_H__

#include <stdint.h>
#include <config.h>

struct memdesc_t {
    char *label;
    uint32_t ipa;
    uint32_t pa;
    uint32_t size;
    uint8_t attr;
};

// TODO(casionwoo) : These memory mapping variables should be removed after DTB implementation
enum memattr {
    MEMATTR_STRONGLY_ORDERED        = 0x0,  // 00_00
    MEMATTR_DEVICE_MEMORY           = 0x1,  // 00_01
    MEMATTR_NORMAL_NON_CACHEABLE    = 0x5,  // 01_01
    MEMATTR_NORMAL_WT_CACHEABLE     = 0xA,  // 10_10
    MEMATTR_NORMAL_WB_CACHEABLE     = 0xF,  // 11_11
};

extern struct memdesc_t *vm_mmap[NUM_GUESTS_STATIC];

void setup_vm_mmap(void);



#endif /* INCLUDE_VM_MAP_H_ */
