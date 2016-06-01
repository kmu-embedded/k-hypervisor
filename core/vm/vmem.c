#include <core/vm/vmem.h>
#include <config.h>
#include <arch/armv7.h>
#include <size.h>
#include "../../arch/arm/paging.h"
#include <vm_map.h>
#include <stdio.h>

extern uint32_t __VM_PGTABLE;

#define PGTABLE_SIZE    0x805000

#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFFFFFULL

void vmem_create(struct vmem *vmem, vmid_t vmid)
{

    vmem->base = (uint32_t) &__VM_PGTABLE + (PGTABLE_SIZE << (vmid - 1));
    paging_create(vmem->base);

    vmem->vttbr = ((uint64_t) vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
    vmem->vttbr &= ~(VTTBR_BADDR_MASK);
    vmem->vttbr |= (uint32_t) vmem->base & VTTBR_BADDR_MASK;

    vmem->mmap = vm_mmap[vmid];
}

hvmm_status_t vmem_init(struct vmem *vmem)
{
    int j = 0;

    do {
        paging_add_ipa_mapping(vmem->base, vmem->mmap[j].ipa, vmem->mmap[j].pa, vmem->mmap[j].attr, vmem->mmap[j].af,
                               vmem->mmap[j].size);
        j++;
    } while (vmem->mmap[j].label != 0);

    vmem->vtcr = (VTCR_SL0_FIRST_LEVEL << VTCR_SL0_BIT);
    vmem->vtcr |= (WRITEBACK_CACHEABLE << VTCR_ORGN0_BIT);
    vmem->vtcr |= (WRITEBACK_CACHEABLE << VTCR_IRGN0_BIT);
    write_cp32(vmem->vtcr, VTCR);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vmem_save(void)
{
    uint32_t hcr = 0;
    hcr = read_cp32(HCR);
    write_cp32((hcr & ~(0x1)), HCR);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vmem_restore(struct vmem *vmem)
{
    uint32_t hcr = 0;
#ifdef CONFIG_SMP
    /* Set SMP bit in ACTLR */
    write_cp32(vmem->vtcr, VTCR);
    write_cp32(1 << 6, ACTLR);
#endif

    write_cp64(vmem->vttbr, VTTBR);

    hcr = read_cp32(HCR);
    write_cp32((hcr | (0x1)), HCR);

    return HVMM_STATUS_SUCCESS;
}

