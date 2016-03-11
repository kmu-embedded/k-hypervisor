#ifndef __GUEST_MEMORY_HW__
#define __GUEST_MEMORY_HW__

#include "vmem.h"

#include <hvmm_types.h>

// TODO(wonseok): make it neat.
#include "../../../arch/arm/mm.h"

/**
 * memory attribute for stage-2 address translation.
 *
 * \ref Memory_attribute "Memory attribute configuration"
 * We only support type of memory attributes as below:
 */
enum memattr {
    MEMATTR_STRONGLY_ORDERED        = 0x0,  // 00_00
    MEMATTR_DEVICE_MEMORY           = 0x1,  // 00_01
    MEMATTR_NORMAL_NON_CACHEABLE    = 0x5,  // 01_01
    MEMATTR_NORMAL_WT_CACHEABLE     = 0xA,  // 10_10
    MEMATTR_NORMAL_WB_CACHEABLE     = 0xF,  // 11_11
};

//For compiler warning:
// error: ‘struct vmem’ declared inside parameter list [-Werror]
struct vmem;
hvmm_status_t memory_hw_create(struct vmem *vmem);
hvmm_status_t memory_hw_init(struct memdesc_t **memmap, char **_vmid_ttbl, vmid_t vmid);
hvmm_status_t memory_hw_save(void);
hvmm_status_t memory_hw_restore(vmid_t vmid, char **_vmid_ttbl);

#endif /*  __GUEST_MEMORY_HW__ */

