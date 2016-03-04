#include <stdio.h>
#include <debug_print.h>
#include <arch/armv7.h>
#include <hvmm_trace.h>
#include <rtsm-config.h>
#include <stdbool.h>

// TODO(wonseok): make it neat.
#include "../../arch/arm/mm.h"
#include <core/vm/vmem.h>
#include "../../arch/arm/lpae.h"

void guest_memory_init_mmu(void)
{
    HVMM_TRACE_ENTER();
    uint32_t vtcr = 0;
    /* Basically, we set_vm write policy to writeback for highest performance */
    /* Set pagetable lookup level at 1 for stage-2 address translation */
    vtcr |= (VTCR_SL0_FIRST_LEVEL << VTCR_SL0_BIT);
    vtcr |= (WRITEBACK_CACHEABLE << VTCR_ORGN0_BIT);
    vtcr |= (WRITEBACK_CACHEABLE << VTCR_IRGN0_BIT);
    write_vtcr(vtcr);

    vtcr = read_vtcr();
    debug_print("vtcr: 0x%08x\n", vtcr);

    HVMM_TRACE_EXIT();
}


void init_pgtable(uint32ptr_t *pgtable_base)
{
    int l1_index, l2_index;
    pgentry *vm_l1_pgtable, *vm_l2_pgtable, *vm_l3_pgtable;

    vm_l1_pgtable = (pgentry *) aligned_alloc ((sizeof(pgentry) * 4), 0x1000);

    for(l1_index = 0; l1_index < L1_ENTRY; l1_index++) {
        vm_l2_pgtable = (pgentry *) aligned_alloc ((sizeof(pgentry) * 512), 0x1000);
        vm_l1_pgtable[l1_index] = set_table(vm_l2_pgtable, 0);

        for(l2_index = 0; l2_index < L2_ENTRY; l2_index++) {
            vm_l3_pgtable = (pgentry *) aligned_alloc ((sizeof(pgentry) * 512), 0x1000);
            vm_l2_pgtable[l2_index] = set_table(vm_l3_pgtable, 0);
        }
    }
    *pgtable_base = vm_l1_pgtable;
}

void stage2_mm_create(uint32ptr_t *pgtable_base)
{
    init_pgtable(pgtable_base);
}

void stage2_mm_init(struct memdesc_t **mdlist, char **_vmid_ttbl, vmid_t vmid)
{
    int i, j;
    struct memdesc_t *memmap;

    for (i = 0; mdlist[i]; i++) {
        if (mdlist[i]->label == 0)
            continue;

        j = 0;
        memmap = mdlist[i];
        while (memmap[j].label != 0) {
            write_pgentry(*_vmid_ttbl, &memmap[j], true);
            j++;
        }
    }
}

