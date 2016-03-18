#ifndef __VMEM_H__
#define __VMEM_H__

#include <stdint.h>
#include <vm_map.h>
#include <types.h>

struct vmem {
    struct memdesc_t *mmap;
    uint32_t base;
    uint32_t vtcr;
    uint64_t vttbr;
};

void vmem_setup();
void vmem_create(struct vmem *vmem, vmid_t vmid);
hvmm_status_t vmem_init(struct vmem *vmem);
hvmm_status_t vmem_save(void);
hvmm_status_t vmem_restore(struct vmem *vmem);

#endif /* __VMEM_H__ */
