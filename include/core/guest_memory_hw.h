#ifndef __GUEST_MEMORY_HW__
#define __GUEST_MEMORY_HW__

#include <hvmm_types.h>

hvmm_status_t memory_hw_init(struct memmap_desc **guest0, struct memmap_desc **guest1);
hvmm_status_t memory_hw_save(void);
hvmm_status_t memory_hw_restore(vmid_t vmid);

#endif /*  __GUEST_MEMORY_HW__ */

