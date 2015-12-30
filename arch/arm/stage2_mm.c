#include <stage2_mm.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_trace.h>
#include <arch/arm/rtsm-config.h>
#include <guest_mm.h>

#include <stage1_mm.h>  //PAGE_SHIFT

/* Stage 2 Level 1 */
#define VMM_L1_PTE_NUM          4
#define VMM_L1_PADDING_PTE_NUM   (512 - VMM_L1_PTE_NUM)

/* Stage 2 Level 2 */
#define VMM_L2_PTE_NUM          512
#define VMM_L3_PTE_NUM          512

#define VMM_L2L3_PTE_NUM_TOTAL  (VMM_L2_PTE_NUM \
        * VMM_L3_PTE_NUM + VMM_L2_PTE_NUM)

#define VMM_PTE_NUM_TOTAL  (VMM_L1_PTE_NUM                  \
        + VMM_L1_PADDING_PTE_NUM + VMM_L2L3_PTE_NUM_TOTAL   \
        * VMM_L1_PTE_NUM)

vm_pgentry guest_pgtable[NUM_GUESTS_STATIC][VMM_PTE_NUM_TOTAL]  __attribute((__aligned__(4096)));

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

// Added
void set_invalid_page(vm_pgentry *entry)
{
    entry->block.valid = 0;
    entry->block.type = 0;
}

void set_vm_table(vm_pgentry *entry, uint64_t baddr, uint8_t valid)
{
    entry->table.valid = valid ? 1 : 0;
    entry->table.type = valid ? 1 : 0;
    entry->table.base = baddr >>  PAGE_SHIFT;
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

#if 0
void guest_stage2_conf_l3_table(vm_pgentry *entry,
        uint64_t baddr, uint8_t valid)
{
    entry->stage2.valid = valid ? 1 : 0;
    entry->raw &= ~TTBL_L3_OUTADDR_MASK;
    entry->raw |= baddr & TTBL_L3_OUTADDR_MASK;
}
#endif

void guest_stage2_disable_l3_table(vm_pgentry *entry)
{
    entry->page.valid = 0;
}

#define TTBL_L3(first_level_l2, index_l2) \
    (&first_level_l2[VMM_L2_PTE_NUM + (VMM_L3_PTE_NUM * (index_l2))])
/**
 * @brief Obtains TTBL_L2 Entry.
 * Returns the address of TTBL l2 at 'index_l1' entry of L1.
 *
 * - vm_pgentry *TTBL_L2(vm_pgentry *first_level_l1, uint32_t index_l1);
 *
 */
#define TTBL_L2(first_level_l1, index_l1) \
    (&first_level_l1[(VMM_L1_PTE_NUM + VMM_L1_PADDING_PTE_NUM) \
              + (VMM_L2L3_PTE_NUM_TOTAL * (index_l1))])

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
static void guest_memory_third_level_map(vm_pgentry *third_level, uint64_t offset,
                uint32_t pages, uint64_t pa, enum memattr mattr)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    //printf("%s[%d]: third_level:%x offset:%x pte:%x pages:%d, pa:%x\n", __func__, __LINE__, (uint32_t) third_level, (uint32_t) offset, &third_level[offset], pages, (uint32_t) pa);
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++) {
        set_vm_entry(&third_level[index_l3], pa, mattr);
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
static void guest_memory_third_level_unmap(vm_pgentry *third_level, uint64_t offset,
                uint32_t pages)
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
 * @brief Unmap second_level and third_level descriptors which is in target virtual
 *        address area.
 *
 * Unmap descriptors of second_level and third_level by making valid bit to zero.
 *
 * - First, make level 2 descriptors invalidate.
 * - Second, if lefts space which can't be covered by level 2 descriptor
 *   (to small), make level 3 descriptors invalidate.
 *
 * @param *second_level Level 2 translation table descriptor.
 * @param va_offset Offset of the virtual address.
 *        - 0 ~ (1GB - size), start contiguous virtual address within level 1
 *          block (1GB).
 *        - It is aligned L2 descriptor lock size(2MB).
 * @param size
 *        - <= 1GB.
 *        - It is aligned page size.
 * @return void
 */
static void guest_memory_second_level_unmap(vm_pgentry *second_level, uint64_t va_offset,
                uint32_t size)
{
    int index_l2 = 0;
    int index_l2_last = 0;
    int num_blocks = 0;
    /* Initialize the address spaces with 'invalid' state */
    num_blocks = size >> LPAE_BLOCK_L2_SHIFT;
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    index_l2_last = index_l2 + num_blocks;

    for (; index_l2 < index_l2_last; index_l2++)
        second_level[index_l2].table.valid = 0;

    size &= LPAE_BLOCK_L2_MASK;
    if (size) {
        /* last partial block */
        vm_pgentry *third_level = TTBL_L3(second_level, index_l2);
        guest_memory_third_level_unmap(third_level, 0x00000000, size >> LPAE_PAGE_SHIFT);
    }
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
static void guest_memory_second_level_map(vm_pgentry *second_level, uint64_t va_offset,
                uint64_t pa, uint32_t size, enum memattr mattr)
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
        if (pages > VMM_L3_PTE_NUM)
            pages = VMM_L3_PTE_NUM;

        third_level = TTBL_L3(second_level, index_l2);
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
            guest_memory_third_level_map(TTBL_L3(second_level, i), 0, VMM_L3_PTE_NUM, pa, mattr);
            pa += LPAE_BLOCK_L2_SIZE;
            size -= LPAE_BLOCK_L2_SIZE;
        }
    }
    /* tail < BLOCK */
    if (size > 0) {
        pages = size >> LPAE_PAGE_SHIFT;
        printf("- pages:%d size:%d\n", pages, size);
        if (pages) {
            third_level = TTBL_L3(second_level, index_l2_last);
            guest_memory_third_level_map(third_level, 0, pages, pa, mattr);
            guest_stage2_enable_l2_table(&second_level[index_l2_last]);
        }
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize second_level entries.
 *
 * Configures second_level descriptor by mapping address of third_level descriptor.
 * And configure all valid bit of third_level descriptors to zero.
 *
 * @param *second_level Level 2 translation table descriptor.
 * @return void
 */
static void guest_memory_second_level_init_entries(vm_pgentry *second_level)
{
    int i, j;
    HVMM_TRACE_ENTER();
    vm_pgentry *third_level;
    for (i = 0; i < VMM_L2_PTE_NUM; i++) {
        third_level = TTBL_L3(second_level, i);
        //printf("second_level[%d]:%x third_level[]:%x\n", i, &second_level[i], third_level);
        set_vm_table(&second_level[i], (uint64_t)((uint32_t) third_level), 0);
        for (j = 0; j < VMM_L2_PTE_NUM; j++)
            third_level[j].page.valid = 0;
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
static void guest_memory_init_second_level(vm_pgentry *second_level, struct memmap_desc *md)
{
    int i = 0;
    HVMM_TRACE_ENTER();
    printf(" - second_level:%x\n", (uint32_t) second_level);
    if (((uint64_t)((uint32_t) second_level)) & 0x0FFFULL)
        printf(" - error: invalid second_level address alignment\n");

    /* construct l2-l3 table hirerachy with invalid pages */
    guest_memory_second_level_init_entries(second_level);

    guest_memory_second_level_unmap(second_level, 0x00000000, 0x40000000); // ??

    while (md[i].label != 0) {
        guest_memory_second_level_map(second_level, md[i].va, md[i].pa,
                md[i].size, md[i].attr);
        i++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Configure stage-2 translation table descriptors of guest.
 *
 * Configures the translation table based on the memory descriptor list.
 *
 * @param *first_level Target translation table descriptor.
 * @param *mdlist[] Memory map descriptor list.
 * @return void
 */

static pgentry set_guest_table(uint32_t paddr)
{
    pgentry entry;

    entry.raw = 0;
    entry.table.valid = 1;
    entry.table.type = 1;
    entry.table.base = paddr >> PAGE_SHIFT;

    return entry;
}

static void guest_memory_init_first_level(vm_pgentry *first_level, struct memmap_desc *mdlist[], vmid_t vmid)
{
    int i = 0;
    HVMM_TRACE_ENTER();

    while (mdlist[i]) {
        //struct memmap_desc *md = mdlist[i];
        if (mdlist[i]->label == 0)
            set_invalid_page(&first_level[i]);
        else {
            // Generate page table by size
            // if size == 4K need to generate three level translation table
            // else if size == 2M need to generate two level translation table
            // else if size == 1GB need to generate one level translation table
            set_vm_table(&first_level[i], (uint64_t)((uint32_t) TTBL_L2(first_level, i)), 1);
            guest_memory_init_second_level(TTBL_L2(first_level, i), mdlist[i]);
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
    printf("vttbr: %llu\n", vttbr);
    HVMM_TRACE_EXIT();
}

static void init_first_level(vm_pgentry *first_level, struct memmap_desc *mdlist[], vmid_t vmid)
{
    int i = 0;
    HVMM_TRACE_ENTER();

    while (mdlist[i]) {
        //struct memmap_desc *md = mdlist[i];
        if (mdlist[i]->label == 0)
            set_invalid_page(&first_level[i]);
        else {
            // Generate page table by size
            // if size == 4K need to generate three level translation table
            // else if size == 2M need to generate two level translation table
            // else if size == 1GB need to generate one level translation table
            set_vm_table(&first_level[i], (uint64_t)((uint32_t) TTBL_L2(first_level, i)), 1);
        }
        i++;
    }
    HVMM_TRACE_EXIT();
}

static vm_pgentry set_table(uint64_t baddr, uint8_t valid)
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

static vm_pgentry set_block(uint32_t paddr, enum memattr mem_attr)
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
    entry.page.ng = 0;
    entry.page.cb = 0;
    entry.page.pxn = 0;
    entry.page.xn = 0;

    return entry;
}

void stage2_mm_init(struct memmap_desc **guest_map, char **_vmid_ttbl, vmid_t vmid)
{
    uint64_t pa = 0x00000000ULL;
    *_vmid_ttbl = &guest_pgtable[vmid][0];
    guest_memory_init_first_level(&guest_pgtable[vmid][0], guest_map, 0);
}
