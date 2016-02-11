#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <arch/arm/rtsm-config.h>
#include <mm.h>
#include "vtcr.h"

static pgentry vm_l1_pgtable[NUM_GUESTS_STATIC][L1_ENTRY];
static pgentry vm_l2_pgtable[NUM_GUESTS_STATIC][L1_ENTRY][L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry vm_l3_pgtable[NUM_GUESTS_STATIC][L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

void guest_memory_init_mmu(void)
{
    HVMM_TRACE_ENTER();
    uint32_t vtcr = 0;
    /* Basically, we set_vm write policy to writeback for highest performance */
    /* Set pagetable lookup level at 1 for stage-2 address translation */
    vtcr |= (VTCR_SL0_FIRST_LEVEL << VTCR_SL0_BIT);
    /* Set outer cacheability attribute */
    vtcr |= (VTCR_WRITEBACK_CACHEABLE << VTCR_ORGN0_BIT);
    /* Set inner cacheability attribute */
    vtcr |= (VTCR_WRITEBACK_CACHEABLE << VTCR_IRGN0_BIT);
    write_vtcr(vtcr);

    vtcr = read_vtcr();
    printf("vtcr: 0x%08x\n", vtcr);

    HVMM_TRACE_EXIT();
}

void write_pgentry(char *_vmid_ttbl, struct memmap_desc *guest_map)
{
    uint32_t i, j;
    uint32_t size, nr_pages;

    uint32_t va = (uint32_t) guest_map->va;
    uint64_t pa = guest_map->pa;

    uint32_t l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
    uint32_t l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
    uint32_t l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

    pgentry *l1_base_addr = (pgentry *) _vmid_ttbl;
    pgentry *l2_base_addr = l1_base_addr[l1_index].table.base << PAGE_SHIFT;
    pgentry *l3_base_addr;

    l1_base_addr[l1_index].table.valid = 1;;
    size = guest_map->size;

    for (i = 0; size > 0; i++ ) {
        l2_base_addr[l2_index + i].table.valid = 1;
        l3_base_addr = l2_base_addr[l2_index + i].table.base << PAGE_SHIFT;

        // TODO(casionwoo) : This should cover the case of serveral size such as 3KB, 1.5GB
        nr_pages = size >> PAGE_SHIFT;
        if(nr_pages > L3_ENTRY)
            nr_pages = L3_ENTRY;

        for (j = 0; j < nr_pages; j++) {
            l3_base_addr[l3_index + j] = set_entry(pa, guest_map->attr, 3, size_4kb);
            pa += LPAE_PAGE_SIZE;
            size -= LPAE_PAGE_SIZE;
        }
    }
}

void init_pgtable(vmid_t vmid)
{
    int l1_index, l2_index;

    for(l1_index = 0; l1_index < L1_ENTRY; l1_index++) {
        vm_l1_pgtable[vmid][l1_index] = set_table(vm_l2_pgtable[vmid][l1_index], 0);
    }

    for(l1_index = 0; l1_index < L1_ENTRY; l1_index++) {
        for(l2_index = 0; l2_index < L2_ENTRY; l2_index++) {
            vm_l2_pgtable[vmid][l1_index][l2_index] = set_table(vm_l3_pgtable[vmid][l1_index][l2_index], 0);
        }
    }
}

void stage2_mm_create(vmid_t vmid)
{
    init_pgtable(vmid);
}

void stage2_mm_init(struct memmap_desc **mdlist, char **_vmid_ttbl, vmid_t vmid)
{
    int i, j;
    struct memmap_desc *memmap;

    *_vmid_ttbl = vm_l1_pgtable[vmid];

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

