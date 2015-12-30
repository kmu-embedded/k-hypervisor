#include <stage2_mm.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <arch/arm/rtsm-config.h>

#include <stage1_mm.h>  //PAGE_SHIFT

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

void set_invalid_page(vm_pgentry *entry)
{
    entry->block.valid = 0;
    entry->block.type = 0;
}

void set_vm_table(vm_pgentry *entry, uint8_t valid)
{
    entry->table.valid = valid ? 1 : 0;
    entry->table.type = valid ? 1 : 0;
}

void guest_stage2_enable_l2_table(vm_pgentry *entry)
{
    entry->table.valid = 1;
    entry->table.type = 1;
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

/**
 * @brief Maps physical address of the guest to level 3 descriptors.
 *
 * Maps physical address to the target level 3 translation table descriptor.
 * Configure raw of the target descriptor whiche are initialized by initial
 * function. (guest_stage2_map_page)
 *
 * @param *third_level Level 3 translation table descriptor.
 * @param offset Offset from the level 3 table descriptor.
 *        - 0 ~ (2MB - pages * 4KB), start contiguous virtual address within
 *          level 2 block (2MB).
 *        - It is aligned L3 descriptor lock size(4KB).
 * @param pages Number of pages.
 *        - 0 ~ 512
 * @param pa Physical address.
 * @param mattr Memory Attribute.
 * @return void
 */
void guest_memory_third_level_map(vm_pgentry *third_level, uint64_t offset, uint32_t pages, uint64_t pa, enum memattr mattr)
{
    int index_l3 = 0;
    int index_l3_last = 0;

    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++) {
        set_vm_entry(&third_level[index_l3], (uint32_t)pa, mattr);
        pa += LPAE_PAGE_SIZE;
    }
}

/**
 * @brief Unmap level 3 descriptors.
 *
 * Unmap descriptors of third_level which is in between offset and offset + pages and
 * makes valid bit zero.
 *
 * @param *third_level Level 3 translation table descriptor.
 * @param offset Offset from the level 3 table descriptor.
 *        - 0 ~ (2MB - pages * 4KB), start contiguous virtual address within
 *          level 2 block (2MB).
 *        - It is aligned L3 descriptor lock size(4KB).
 * @param pages Number of pages.
 *        - 0 ~ 512
 * @return void
 */
void guest_memory_third_level_unmap(vm_pgentry *third_level, uint64_t offset, uint32_t pages)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset >> LPAE_PAGE_SHIFT;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++)
        third_level[index_l3].page.valid = 0;
}

/**
 * @brief Map second_level descriptors.
 *
 * Maps physical address to second_level and third_level descriptors and apply memory
 * attributes.
 *
 * - First, compute index of the target second_level descriptor and block offset.
 * - Second, maps physical address to third_level descriptors if head of block is
 *   not fits size of the second_level descriptor.
 * - Third, maps left physical address to second_level descriptors.
 * - Finally, if lefts the memory, maps it to third_level descriptors.
 *
 * @param *second_level Level 2 translation table descriptor.
 * @param va_offset
 *        - 0 ~ (1GB - size), start contiguous virtual address within level 1
 *          block (1GB).
 *        - It is aligned l2 descriptor lock size(2MB).
 * @param pa Physical address
 * @param size Size of target memory.
 *        - <= 1GB.
 *        - It is aligned page size.
 * @param Memory Attribute
 * @return void
 */
void guest_memory_second_level_map(vm_pgentry *second_level, uint64_t va_offset, uint64_t pa, uint32_t size, enum memattr mattr)
{
    uint64_t block_offset;
    uint32_t index_l2;
    uint32_t index_l2_last;
    uint32_t num_blocks;
    uint32_t pages;
    vm_pgentry *third_level;
    int i;
    HVMM_TRACE_ENTER();

    printf("second_level:%x va_offset:%x pa:%x size:%d\n",
            (uint32_t) second_level, (uint32_t) va_offset, (uint32_t) pa, size);
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    block_offset = va_offset & LPAE_BLOCK_L2_MASK;
    printf("- index_l2:%d block_offset:%x\n", index_l2, (uint32_t) block_offset);

    /* head < BLOCK */
    if (block_offset) {
        uint64_t offset;
        offset = block_offset >> LPAE_PAGE_SHIFT;
        pages = size >> LPAE_PAGE_SHIFT;
        if (pages > L3_ENTRY)
            pages = L3_ENTRY;

        third_level = second_level[index_l2].table.base << PAGE_SHIFT;
        guest_memory_third_level_map(third_level, offset, pages, pa, mattr);
        guest_stage2_enable_l2_table(&second_level[index_l2]);
        va_offset |= ~LPAE_BLOCK_L2_MASK;
        size -= pages * LPAE_PAGE_SIZE;
        pa += pages * LPAE_PAGE_SIZE;
        index_l2++;
    }
    /* body : n BLOCKS */
    if (size > 0) {
        num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
        index_l2_last = index_l2 + num_blocks;
        printf("- index_l2_last:%d num_blocks:%d size:%d\n",
                index_l2_last, (uint32_t) num_blocks, size);
        for (i = index_l2; i < index_l2_last; i++) {
            guest_stage2_enable_l2_table(&second_level[i]);
            third_level = second_level[i].table.base << PAGE_SHIFT;
            guest_memory_third_level_map(third_level, 0, L3_ENTRY, pa, mattr);
            pa += LPAE_BLOCK_L2_SIZE;
            size -= LPAE_BLOCK_L2_SIZE;
        }
    }
    /* tail < BLOCK */
    if (size > 0) {
        pages = size >> LPAE_PAGE_SHIFT;
        printf("- pages:%d size:%d\n", pages, size);
        if (pages) {
            third_level = second_level[index_l2_last].table.base << PAGE_SHIFT;
            guest_memory_third_level_map(third_level, 0, pages, pa, mattr);
            guest_stage2_enable_l2_table(&second_level[index_l2_last]);
        }
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize delivered second_level descriptors.
 *
 * Initialize second_level descriptors and unmap the descriptors.
 * Finally, map the second_level descriptors by memory map descriptors.
 *
 * @param *second_level Level 2 translation table descriptor.
 * @param *md Device memory map descriptor.
 * @return void
 */
void guest_memory_init_second_level(vm_pgentry *second_level, struct memmap_desc *md)
{
    int i = 0;

    HVMM_TRACE_ENTER();

    if (((uint64_t)((uint32_t) second_level)) & 0x0FFFULL)
        printf(" - error: invalid second_level address alignment\n");

    while (md[i].label != 0) {
        guest_memory_second_level_map(second_level, md[i].va, md[i].pa, md[i].size, md[i].attr);
        i++;
    }

    HVMM_TRACE_EXIT();
}

void guest_memory_init_first_level(vm_pgentry *first_level, struct memmap_desc *mdlist[], vmid_t vmid)
{
    int i = 0;
    vm_pgentry *l2_base_addr;

    HVMM_TRACE_ENTER();

    while (mdlist[i]) {
        if (mdlist[i]->label == 0)
            set_invalid_page(&first_level[i]);
        else {
            l2_base_addr = first_level[i].table.base << PAGE_SHIFT;
            set_vm_table(&first_level[i], 1);
            guest_memory_init_second_level(l2_base_addr, mdlist[i]);
        }
        i++;
    }

    HVMM_TRACE_EXIT();
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

void init_first_level(vm_pgentry *first_level, struct memmap_desc *mdlist[], vmid_t vmid)
{
    int i = 0;
    HVMM_TRACE_ENTER();

    while (mdlist[i]) {
        if (mdlist[i]->label == 0)
            set_invalid_page(&first_level[i]);
        else {
            set_vm_table(&first_level[i], 1);
        }
        i++;
    }
    HVMM_TRACE_EXIT();
}

vm_pgentry set_table(uint64_t baddr, uint8_t valid)
{
    vm_pgentry entry;

    entry.raw = 0x0;
    if (valid == 0)
        return entry;

    entry.table.valid = 1;
    entry.table.type = 1;
    entry.table.base = baddr >>  PAGE_SHIFT;

    return entry;
}

vm_pgentry set_block(uint32_t paddr, enum memattr mem_attr)
{
    vm_pgentry entry;

    entry.raw = 0;
    entry.page.valid = 1;
    entry.page.type = 1;
    entry.page.base = paddr >> L1_SHIFT;
    entry.page.mem_attr = mem_attr;
    entry.page.ap = 3;
    entry.page.sh = 0;
    entry.page.af = 1;
    entry.page.cb = 0;
    entry.page.xn = 0;

    return entry;
}

void set_next_table(vm_pgentry *entry, uint32_t paddr)
{
    entry->raw = 0;
    entry->table.base = paddr >> PAGE_SHIFT;

    return entry;
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

void stage2_mm_init(struct memmap_desc **guest_map, char **_vmid_ttbl, vmid_t vmid)
{
    if (vmid == 0)
        *_vmid_ttbl = vm0_l1_pgtable;
    else
        *_vmid_ttbl = vm1_l1_pgtable;

    guest_memory_init_first_level(*_vmid_ttbl, guest_map, 0);
}
