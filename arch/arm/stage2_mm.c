#include <stage2_mm.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <arch/arm/rtsm-config.h>
#include <mm.h>

#define MEM_ATTR_MASK   0x0F

vm_pgentry vm_l1_pgtable[NUM_GUESTS_STATIC][L1_ENTRY];
vm_pgentry vm_l2_pgtable[NUM_GUESTS_STATIC][L1_ENTRY][L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
vm_pgentry vm_l3_pgtable[NUM_GUESTS_STATIC][L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

void set_vm_table(vm_pgentry *entry)
{
    entry->table.valid = 1;
}

vm_pgentry set_entry(uint64_t pa, enum memattr mattr, pgsize_t size)
{
    vm_pgentry entry;

    switch(size) {
        case size_1gb:
            entry.raw = 0;
            entry.page.valid = 1;
            entry.page.type = 0;
            entry.page.base = pa >> L1_SHIFT;
            entry.page.mem_attr = mattr & MEM_ATTR_MASK;
            entry.page.ap = 3;
            entry.page.sh = 0;
            entry.page.af = 1;
            entry.page.ng = 0;
            entry.page.cb = 0;
            entry.page.pxn = 0;
            entry.page.reserved = 0;
            break;

        case size_2mb:
            entry.raw = 0;
            entry.page.valid = 1;
            entry.page.type = 0;
            entry.page.base = pa >> L2_SHIFT;
            entry.page.mem_attr = mattr & MEM_ATTR_MASK;
            entry.page.ap = 3;
            entry.page.sh = 0;
            entry.page.af = 1;
            entry.page.ng = 0;
            entry.page.cb = 0;
            entry.page.pxn = 0;
            entry.page.reserved = 0;
            break;

        case size_4kb:
            entry.raw = 0;
            entry.page.valid = 1;
            entry.page.type = 1;
            entry.page.base = pa >> L3_SHIFT;
            entry.page.mem_attr = mattr & MEM_ATTR_MASK;
            entry.page.ap = 3;
            entry.page.sh = 0;
            entry.page.af = 1;
            entry.page.ng = 0;
            entry.page.cb = 0;
            entry.page.pxn = 0;
            entry.page.reserved = 0;
            break;

        default:
            break;
    }

    return entry;
}

/* VTCR ATTRIBUTES */
#define VTCR_SL0_SECOND_LEVEL       0x0
#define VTCR_SL0_FIRST_LEVEL        0x1
#define VTCR_SL0_RESERVED           0x2
#define VTCR_SL0_UNPREDICTABLE      0x3
#define VTCR_SL0_BIT                6
#define VTCR_ORGN0_ONC              0X0
#define VTCR_ORGN0_OWBWAC           0x1
#define VTCR_ORGN0_OWTC             0x2
#define VTCR_ORGN0_OWBWAC           0x3
#define VTCR_ORGN0_BIT              10
#define VTCR_IRGN0_INC              0X0
#define VTCR_IRGN0_IWBWAC           0x1
#define VTCR_IRGN0_IWTC             0x2
#define VTCR_IRGN0_IWBWAC           0x3
#define VTCR_IRGN0_BIT              8

void guest_memory_init_mmu(void)
{
    uint32_t vtcr = 0;

    HVMM_TRACE_ENTER();

    /* Stage-2 Translation Pagetable start lookup configuration */
    vtcr |= (VTCR_SL0_FIRST_LEVEL << VTCR_SL0_BIT);
    /* Outer cacheability attribute */
    vtcr |= (VTCR_ORGN0_OWBWAC << VTCR_ORGN0_BIT);
    /* Inner cacheability attribute */
    vtcr |= (VTCR_IRGN0_IWBWAC << VTCR_IRGN0_BIT);

    write_vtcr(vtcr);
    vtcr = read_vtcr();
    printf("vtcr: 0x%08x\n", vtcr);

    HVMM_TRACE_EXIT();
}

vm_pgentry set_table(uint32_t paddr)
{
    vm_pgentry entry;

    entry.raw = 0;
    entry.table.base = paddr >> PAGE_SHIFT;
    entry.table.type = 1;

    return entry;
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

    vm_pgentry *l1_base_addr = (vm_pgentry *) _vmid_ttbl;
    vm_pgentry *l2_base_addr = l1_base_addr[l1_index].table.base << PAGE_SHIFT;
    vm_pgentry *l3_base_addr;

    set_vm_table(&l1_base_addr[l1_index]);
    size = guest_map->size;

    for (i = 0; size > 0; i++ ) {
        set_vm_table(&l2_base_addr[l2_index + i]);
        l3_base_addr = l2_base_addr[l2_index + i].table.base << PAGE_SHIFT;

        // TODO(casionwoo) : This should cover the case of serveral size such as 3KB, 1.5GB
        nr_pages = size >> PAGE_SHIFT;
        if(nr_pages > L3_ENTRY)
            nr_pages = L3_ENTRY;

        for (j = 0; j < nr_pages; j++) {
            l3_base_addr[l3_index + j] = set_entry(pa, guest_map->attr, size_4kb);
            pa += LPAE_PAGE_SIZE;
            size -= LPAE_PAGE_SIZE;
        }
    }
}

void init_pgtable()
{
    int i, j, k, l;

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        for(j = 0; j < L1_ENTRY; j++) {
            vm_l1_pgtable[i][j] = set_table(vm_l2_pgtable[i][j]);
        }
    }

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        for(j = 0; j < L1_ENTRY; j++) {
            for(k = 0; k < L2_ENTRY; k++) {
               vm_l2_pgtable[i][j][k] = set_table(vm_l3_pgtable[i][j][k]);
            }
        }
    }
}

void stage2_mm_setup()
{
    init_pgtable();
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

