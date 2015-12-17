#ifndef __MM_H__
#define __MM_H__

#include <hvmm_types.h>
#include <stdint.h>
#include <guest_memory_hw.h>

void memory_create();
hvmm_status_t memory_init(struct memmap_desc **guest0, struct memmap_desc **guest1);
hvmm_status_t memory_save(void);
hvmm_status_t memory_restore(vmid_t vmid);

#endif
