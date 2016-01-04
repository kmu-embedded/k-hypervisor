#include <stage2_mm.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <arch/arm/rtsm-config.h>
#include <mm.h>

vm_pgentry vm0_l1_pgtable[L1_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm0_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm0_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(4096)));

vm_pgentry vm1_l1_pgtable[L1_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm1_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(4096)));
vm_pgentry vm1_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(4096)));

/**
 * \defgroup LPAE_address_mask
 *
 * The address mask of each level descriptors.
 * - TTBL_L1_OUTADDR_MASK[39:30] Level1 Block address mask.
 * - TTBL_L2_OUTADDR_MASK[39:21] Level2 Block Address mask.
 * - TTBL_L3_OUTADDR_MASK[39:12] Page address mask.
 *
 * - TTBL_L1_TABADDR_MASK[39:12] Level2 table descriptor address mask.
 * - TTBL_L2_TABADDR_MASK]30:12] Level3 table descriptor address mask.
 * @{
 */

#define TTBL_L1_OUTADDR_MASK    0x000000FFC0000000ULL
#define TTBL_L2_OUTADDR_MASK    0x000000FFFFE00000ULL
#define TTBL_L3_OUTADDR_MASK    0x000000FFFFFFF000ULL

#define TTBL_L1_TABADDR_MASK    0x000000FFFFFFF000ULL
#define TTBL_L2_TABADDR_MASK    0x000000FFFFFFF000ULL

void set_vm_table(vm_pgentry *entry, uint8_t valid)
{
    entry->table.valid = valid ? 1 : 0;
    entry->table.type = valid ? 1 : 0;
}

void set_vm_entry(vm_pgentry *entry, uint64_t pa, enum memattr mattr)
{
    entry->raw = 0;

    entry->page.valid = 1;
    entry->page.type = 1;
    entry->page.base = pa >> L3_SHIFT;

    entry->page.mem_attr = mattr & 0x0F;
    entry->page.ap = 3;
    entry->page.sh = 0;
    entry->page.af = 1;
    entry->page.ng = 0;

    entry->page.cb = 0;
    entry->page.pxn = 0;
    entry->page.reserved = 0;
}

#define VTCR_INITVAL                                    0x80000000
#define VTCR_SH0_MASK                                   0x00003000
#define VTCR_SH0_SHIFT                                  12
#define VTCR_ORGN0_MASK                                 0x00000C00
#define VTCR_ORGN0_SHIFT                                10
#define VTCR_IRGN0_MASK                                 0x00000300
#define VTCR_IRGN0_SHIFT                                8
#define VTCR_SL0_MASK                                   0x000000C0
#define VTCR_SL0_SHIFT                                  6
#define VTCR_S_MASK                                     0x00000010
#define VTCR_S_SHIFT                                    4
#define VTCR_T0SZ_MASK                                  0x00000003
#define VTCR_T0SZ_SHIFT                                 0

void guest_memory_init_mmu(void)
{
    uint32_t vtcr, vttbr;
    HVMM_TRACE_ENTER();
    vtcr = read_vtcr();
    printf("vtcr: 0x%08x\n", vtcr);
    printf("\n\r");
    /* start lookup at level 1 table */
    vtcr &= ~VTCR_SL0_MASK;
    vtcr |= (0x01 << VTCR_SL0_SHIFT) & VTCR_SL0_MASK;
    vtcr &= ~VTCR_ORGN0_MASK;
    vtcr |= (0x3 << VTCR_ORGN0_SHIFT) & VTCR_ORGN0_MASK;
    vtcr &= ~VTCR_IRGN0_MASK;
    vtcr |= (0x3 << VTCR_IRGN0_SHIFT) & VTCR_IRGN0_MASK;
    write_vtcr(vtcr);
    vtcr = read_vtcr();
    printf("vtcr: 0x%08x\n", vtcr);
    {
        uint32_t sl0 = (vtcr & VTCR_SL0_MASK) >> VTCR_SL0_SHIFT;
        uint32_t t0sz = vtcr & 0xF;
        uint32_t baddr_x = (sl0 == 0 ? 14 - t0sz : 5 - t0sz);
        printf("vttbr.baddr.x: 0x%08x\n", baddr_x);
    }
    /* VTTBR */
    vttbr = read_vttbr();
    printf("vttbr: %x%x\n", vttbr);
    HVMM_TRACE_EXIT();
}

void set_next_table(vm_pgentry *entry, uint32_t paddr)
{
    entry->raw = 0;
    entry->table.base = paddr >> PAGE_SHIFT;

    return entry;
}

void write_pgentry(char *_vmid_ttbl, struct memmap_desc *guest_map)
{
    int i, j;

    uint32_t size, pages;

    uint32_t va = (uint32_t) guest_map->va;
    uint64_t pa = guest_map->pa;

    uint32_t l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
    uint32_t l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
    uint32_t l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

    vm_pgentry *l1_base_addr = (vm_pgentry *) _vmid_ttbl;
    vm_pgentry *l2_base_addr = l1_base_addr[l1_index].table.base << PAGE_SHIFT;
    vm_pgentry *l3_base_addr;

    set_vm_table(&l1_base_addr[l1_index], 1);
    size = guest_map->size;

    i = 0;
    do {
        set_vm_table(&l2_base_addr[l2_index + i], 1);
        l3_base_addr = l2_base_addr[l2_index + i].table.base << PAGE_SHIFT;

        pages = size >> PAGE_SHIFT;
        if(pages > L3_ENTRY)
            pages = L3_ENTRY;

        for (j = 0; j < pages; j++) {
            set_vm_entry(&l3_base_addr[l3_index + j], pa, guest_map->attr);
            pa += LPAE_PAGE_SIZE;
            size -= LPAE_PAGE_SIZE;
        }

        i++;
    } while(size > 0);
}

void init_pgtable()
{
    int i, j, k;

    for(i = 0; i < L1_ENTRY; i++) {
        set_next_table(&vm0_l1_pgtable[i], vm0_l2_pgtable[i]);
        set_next_table(&vm1_l1_pgtable[i], vm1_l2_pgtable[i]);
    }

    for(i = 0; i < L1_ENTRY; i++) {
        for(j = 0; j < L2_ENTRY; j++) {
            set_next_table(&vm0_l2_pgtable[i][j], vm0_l3_pgtable[i][j]);
            set_next_table(&vm1_l2_pgtable[i][j], vm1_l3_pgtable[i][j]);
        }
    }

    for(i = 0; i < L1_ENTRY; i++) {
        for(j = 0; j < L2_ENTRY; j++) {
            for(k = 0; k < L3_ENTRY; k++) {
                vm0_l3_pgtable[i][j][k].page.valid = 0;
                vm1_l3_pgtable[i][j][k].page.valid = 0;
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

    *_vmid_ttbl = (!vmid) ? vm0_l1_pgtable : vm1_l1_pgtable;

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

