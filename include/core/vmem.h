#ifndef __VMEM_H__
#define __VMEM_H__

#include <hvmm_types.h>
#include <stdint.h>
#include <guest_memory_hw.h>
#include <rtsm-config.h>

// TODO(casionwoo) : This declaration will be moved to hvmm_types.h
typedef uint32_t uint32ptr_t;

struct vmem {
    struct memmap_desc **memmap;
    uint32ptr_t pgtable_base;
};

void vmem_setup();
void vmem_create(struct vmem *vmem, vmid_t vmid);
hvmm_status_t vmem_init(struct vmem *vmem, vmid_t vmid);
hvmm_status_t vmem_save(void);
hvmm_status_t vmem_restore(struct vmem *vmem, vmid_t vmid);

#endif /* __VMEM_H__ */
