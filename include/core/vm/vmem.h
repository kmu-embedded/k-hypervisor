#ifndef __VMEM_H__
#define __VMEM_H__

#include "guest_memory_hw.h"

#include <hvmm_types.h>
#include <stdint.h>
#include <size.h>

struct vmem {
    struct memdesc_t **memmap;
    uint32_t base;
    uint32_t vtcr;
    uint64_t vttbr;
};

void vmem_setup();
void vmem_create(struct vmem *vmem, vmid_t vmid);
hvmm_status_t vmem_init(struct vmem *vmem, vmid_t vmid);
hvmm_status_t vmem_save(void);
hvmm_status_t vmem_restore(struct vmem *vmem, vmid_t vmid);

#endif /* __VMEM_H__ */
