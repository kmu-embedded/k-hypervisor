#ifndef __GUEST_MEMORY_HW__
#define __GUEST_MEMORY_HW__

#include <hvmm_types.h>
#include <mm.h>
#include <vmem.h>

/**
 * @brief Enum values of the stage2 memory attribute.
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

/**
 * @brief Memory map descriptor.
 *
 * Memory map information descriptor.
 * - label Name of the descriptor.
 * - va Intermediate physical address(IPA).
 * - pa Physical address.
 * - size Size of this memory area.
 * - attr Memory attribute value.
 */
hvmm_status_t memory_hw_create(struct vmem *vmem);
hvmm_status_t memory_hw_init(struct memmap_desc **memmap, char **_vmid_ttbl, vmid_t vmid);
hvmm_status_t memory_hw_save(void);
hvmm_status_t memory_hw_restore(vmid_t vmid, char **_vmid_ttbl);

#endif /*  __GUEST_MEMORY_HW__ */

