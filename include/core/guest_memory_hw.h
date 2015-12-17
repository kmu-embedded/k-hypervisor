#ifndef __GUEST_MEMORY_HW__
#define __GUEST_MEMORY_HW__

#include <hvmm_types.h>

/**
 * @brief Enum values of the stage2 memory attribute.
 *
 * \ref Memory_attribute "Memory attribute configuration"
 */
enum memattr {
    MEMATTR_SO = 0x0,               /* Strongly Ordered */
    MEMATTR_DM = 0x1,               /* Device memory */
    MEMATTR_NORMAL_ONC = 0x4,       /* Outer Non-cacheable */
    MEMATTR_NORMAL_OWT = 0x8,
    MEMATTR_NORMAL_OWB = 0xC,
    MEMATTR_NORMAL_INC = 0x1,
    MEMATTR_NORMAL_IWT = 0x2,
    MEMATTR_NORMAL_IWB = 0x3,
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
struct memmap_desc {
    char *label;
    uint64_t va;
    uint64_t pa;
    uint32_t size;
    enum memattr attr;
};

hvmm_status_t memory_hw_init(struct memmap_desc **guest0, struct memmap_desc **guest1);
hvmm_status_t memory_hw_save(void);
hvmm_status_t memory_hw_restore(vmid_t vmid);

#endif /*  __GUEST_MEMORY_HW__ */

