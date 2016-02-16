#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <rtsm-config.h>

#include <vtcr.h>
#include <mm.h>
#include <vmem.h>
#include <lpae.h>

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
    printf("vtcr: 0x%08x\n", vtcr);

    HVMM_TRACE_EXIT();
}

void write_pgentry(void *_vmid_ttbl, struct memmap_desc *guest_map)
{
    uint32_t l1_offset, l2_offset, l3_offset;
    uint32_t l1_index, l2_index, l3_index;
    uint32_t va, pa;
    uint32_t size;

    pgentry *l1_base_addr, *l2_base_addr, *l3_base_addr;

    size = guest_map->size;
    va = (uint32_t) guest_map->va;
    pa = (uint32_t) guest_map->pa;

    l1_base_addr = (pgentry *) _vmid_ttbl;
    l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
    for (l1_offset = 0; size > 0; l1_offset++ ) {
        l1_base_addr[l1_index + l1_offset].table.valid = 1;
        l2_base_addr = l1_base_addr[l1_index + l1_offset].table.base << PAGE_SHIFT;
        l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;

        for (l2_offset = 0; l2_index + l2_offset < L2_ENTRY && size > 0; l2_offset++ ) {
            l2_base_addr[l2_index + l2_offset].table.valid = 1;
            l3_base_addr = l2_base_addr[l2_index + l2_offset].table.base << PAGE_SHIFT;
            l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

            for (l3_offset = 0; l3_index + l3_offset < L3_ENTRY && size > 0; l3_offset++) {
                l3_base_addr[l3_index + l3_offset] = set_entry(pa, guest_map->attr, 3, size_4kb);
                pa += LPAE_PAGE_SIZE;
                va += LPAE_PAGE_SIZE;
                size -= LPAE_PAGE_SIZE;
            }
        }
    }
}
#if 0
void write_pgentry(void *_vmid_ttbl, struct memmap_desc *guest_map)
{
    uint32_t l1_offset, l2_offset, l3_offset;
    uint32_t l1_index, l2_index, l3_index;
    uint32_t va, pa;
    uint32_t size;

    pgentry *l1_base_addr, *l2_base_addr, *l3_base_addr;

    size = guest_map->size;
    va = (uint32_t) guest_map->va;
    pa = (uint32_t) guest_map->pa;


    switch(size) {
        case SZ_4K:
            l1_base_addr = (pgentry *) _vmid_ttbl; // replaced

            l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
            l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
            l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

            l2_base_addr = l1_base_addr[l1_index].table.base << PAGE_SHIFT;

            l3_base_addr = l2_base_addr[l2_index].table.base << PAGE_SHIFT;
            l3_base_addr[l3_index] = set_entry(pa, guest_map->attr, 3, size_4kb);

            l2_base_addr[l2_index].table.valid = 1;
            l1_base_addr[l1_index].table.valid = 1;

            break;
        default: /* size is bigger than 4KB */
            l1_base_addr = (pgentry *) _vmid_ttbl;
            l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
            for (l1_offset = 0; size > 0; l1_offset++ ) {
                l1_base_addr[l1_index + l1_offset].table.valid = 1;
                l2_base_addr = l1_base_addr[l1_index + l1_offset].table.base << PAGE_SHIFT;
                l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;

                for (l2_offset = 0; l2_index + l2_offset < L2_ENTRY && size > 0; l2_offset++ ) {
                    l2_base_addr[l2_index + l2_offset].table.valid = 1;
                    l3_base_addr = l2_base_addr[l2_index + l2_offset].table.base << PAGE_SHIFT;
                    l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

                    for (l3_offset = 0; l3_index + l3_offset < L3_ENTRY && size > 0; l3_offset++) {
                        l3_base_addr[l3_index + l3_offset] = set_entry(pa, guest_map->attr, 3, size_4kb);
                        pa += LPAE_PAGE_SIZE;
                        va += LPAE_PAGE_SIZE;
                        size -= LPAE_PAGE_SIZE;
                    }
                }
            }
            break;
    }

}
#endif

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

void stage2_mm_init(struct memmap_desc **mdlist, char **_vmid_ttbl, vmid_t vmid)
{
    int i, j;
    struct memmap_desc *memmap;

    for (i = 0; mdlist[i]; i++) {
        if (mdlist[i]->label == 0)
            continue;

        j = 0;
        memmap = mdlist[i];
        while (memmap[j].label != 0) {
            write_pgentry(*_vmid_ttbl, &memmap[j]);
            j++;
        }
    }
}

