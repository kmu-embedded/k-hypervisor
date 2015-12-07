#include <stdio.h>
#include <armv7_p15.h>
#include <stdint.h>
#include <hvmm_trace.h>
#include <lpae.h>
#include <memory.h>
#include <vcpu.h>
#include <smp.h>
#include <arch/arm/rtsm-config.h>

//#include <host_memory_hw.h>
/**
 * \defgroup Memory_Attribute_Indirection_Register
 *
 * It provide the memory attribute encodings corresponding to the possible
 * AttrIndx values in a Long descriptor format translation table entry for
 * stage-1 translations.
 *
 * - Only accessible from PL1 or higher.
 * AttrIndx[2] selects the appropriate MAIR
 * - AttrIndx[2] to 0 selects MAIR0
 *               to 1 selects MAIR1
 * - AttrIndx[1:0] gives the value of m in Attrm
 * - The MAIR0 and MAIR1 bit assignments.
 * <pre>
 * |     |31       24|23       16|15        8|7         0|
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR0|   Attr3   |   Attr2   |   Attr1   |   Attr0   |
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR1|   Attr7   |   Attr6   |   Attr5   |   Attr4   |
 * </pre>
 * - MAIRn Attrm[7:4] encoding
 * <pre>
 * |Attrm[7:4]     |Meaning                                                    |
 * |---------------|-----------------------------------------------------------|
 * |0000           |Strongly-ordered or Device memory, see encoding Attrm[3:0] |
 * |---------------|-----------------------------------------------------------|
 * |00RW, RW not 00| - Normal memory, Outer Write-Through transient.           |
 * |---------------|-----------------------------------------------------------|
 * |0100           |Normal memory, Outer non-cacheable.                        |
 * |---------------|-----------------------------------------------------------|
 * |01RW, RW not 00| - Normal memory, Outer Write-back transient.              |
 * |---------------|-----------------------------------------------------------|
 * |10RW           |Normal memory, Outer Write-Through Cacheable, Non-transient|
 * |---------------|-----------------------------------------------------------|
 * |11RW           | Normal memory, Ouet Write-Back Cacheable, Non-transient.  |
 * </pre>
 *
 * - MAIRn Attrm[3:0] encoding
 * <pre>
 * |Attrm[3:0]|When Attrm[7:4] is 0b000|When Attrm[7:4] is not 0b0000
 * |----------|------------------------|--------------------------------------|
 * |0000      |Strongly-ordered memory | UNPREDICTABLE                        |
 * |----------|------------------------|--------------------------------------|
 * |00RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Through |
 * |RW not 00 |                        |   transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |0100      | Device memory          | Normal memory, Inner Non-cacheable.  |
 * |----------|------------------------|--------------------------------------|
 * |01RW,     | UNPREDICTABLE          | - Normal memory, Inner Write-Back    |
 * |RW not 00 |                        |   Transient.                         |
 * |----------|------------------------|--------------------------------------|
 * |10RW      | UNPREDICTABLE          | Normal memory, Inner Write-Through   |
 * |          |                        | Cacheable, Non-transient.            |
 * |----------|------------------------|--------------------------------------|
 * |11RW      | UNPREDICTABLE          | Normal memory, Inner Write-back      |
 * |          |                        | Cacheable, Non-transient.            |
 * </pre>
 *
 * -R, W bit mean read-allocate and wrtie-allocate.
 *
 * - Used initial value of MAIR0, MAIR1.
 * <pre>
 * |     |31       24|23       16|15        8|7         0|
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR0| 1110 1110 | 1010 1010 | 0100 0100 | 0000 0000 |
 * |-----|-----------|-----------|-----------|-----------|
 * |MAIR1| 1111 1111 | 0000 0000 | 0000 0000 | 0000 0100 |
 * </pre>
 * @{
 */
/** @}*/




#define L2_ENTRY_MASK 0x1FF
#define L2_SHIFT 21

#define L3_ENTRY_MASK 0x1FF
#define L3_SHIFT 12


/* Stage 2 Level 1 */
#define VMM_L1_PTE_NUM          4
#define VMM_L1_PADDING_PTE_NUM   (512 - VMM_L1_PTE_NUM)
/* Stage 2 Level 2 */
#define VMM_L2_PTE_NUM          512
#define VMM_L3_PTE_NUM          512
/**
 * @brief Gets total number of level 2 and level 3 page table entry.
 *
 * <pre>
 * VMM_L2_PTE_NUM * VMM_L3_PTE_NUM = /
 * Total Number Of Level 3 Page Table Entry
 * + VMM_L2_PTE_NUM = Total Number Of Level 2 Page Table Entry
 * </pre>
 *
 * @return Total number of l2, l3 table entry.
 */
#define VMM_L2L3_PTE_NUM_TOTAL  (VMM_L2_PTE_NUM \
        * VMM_L3_PTE_NUM + VMM_L2_PTE_NUM)
/**
 * @brief Gets total number of all page table entries.
 */
#define VMM_PTE_NUM_TOTAL  (VMM_L1_PTE_NUM                  \
        + VMM_L1_PADDING_PTE_NUM + VMM_L2L3_PTE_NUM_TOTAL   \
        * VMM_L1_PTE_NUM)
/**
 * \defgroup VTTBR
 *
 * Virtualization Translation Table Base Register(VTTBR) is almost the same as
 * \ref TTBRx "64bit TTBRx". It is just different from ASID to VMID.
 *
 * - VMID[55:48]
 *   - The VMID for the translation table.
 * @{
 */
#define VTTBR_INITVAL                                   0x0000000000000000ULL
#define VTTBR_VMID_MASK                                 0x00FF000000000000ULL
#define VTTBR_VMID_SHIFT                                48
#define VTTBR_BADDR_MASK                                0x000000FFFFFFF000ULL
#define VTTBR_BADDR_SHIFT                               12
/** @} */

/**
 * \defgroup VTCR
 *
 * Virtualization Translation Control Register(VTCR) controls the translation
 * table walks required for stage-2 translation of memory accesses from
 * non-secure modes other than hyp mode, and holds cacheability and
 * shareability information for the acceeses.
 *
 * - bit[31]
 *   - UNK/SBOP.
 * - SH0[13:12]
 *   - Shareability attribute.
 *   - \ref Shareability
 * - ORGN0[11:10]
 *   - Outer cacheability attribute for memory.
 *   - \ref Outer_cacheability
 * - IRGN0[9:8]
 *   - Inner cacheability attribute for memory.
 *   - \ref Inner_cacheability
 * - SL0[7:6]
 *   - Starting level for translation table walks using VTTBR.
 * <pre>
 * SL0 Meaning
 * - 00 : Start at second level
 * - 01 : Start at first level
 * </pre>
 * - S[4]
 *   - Sign extension bit. this bit must be programmed to the value of the
 *     T0SZ[3].
 * - T0SZ[3:0]
 *   - The size offset of the memory region addressed by VTTBR.
 * @{
 */
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
/** @} */

/*
 * Stage 2 Translation Table, look up begins at second level
 * VTTBR.BADDR[31:x]: x=14, VTCR.T0SZ = 0, 2^32 input address range,
 * VTCR.SL0 = 0(2nd), 16KB aligned base address
 * Statically allocated for now
 */

static union lpaed_t *_vmid_ttbl[NUM_GUESTS_STATIC];
/*
 * TODO: if you change the static variable, you will meet the system fault.
 * We don't konw about this issue, so we will checking this later time.
 */
union lpaed_t
_ttbl_guest[NUM_GUESTS_STATIC][VMM_PTE_NUM_TOTAL] \
                __attribute((__aligned__(4096)));

/**
 * @brief Obtains TTBL_L3 entry.
 * Returns the address of TTBL l3 at 'index_l2' entry of L2.
 *
 * - union lpaed_t *TTBL_L3(union lpaed_t *ttbl_l2, uint32_t index_l2);
 *
 */
#define TTBL_L3(ttbl_l2, index_l2) \
    (&ttbl_l2[VMM_L2_PTE_NUM + (VMM_L3_PTE_NUM * (index_l2))])
/**
 * @brief Obtains TTBL_L2 Entry.
 * Returns the address of TTBL l2 at 'index_l1' entry of L1.
 *
 * - union lpaed_t *TTBL_L2(union lpaed_t *ttbl_l1, uint32_t index_l1);
 *
 */
#define TTBL_L2(ttbl_l1, index_l1) \
    (&ttbl_l1[(VMM_L1_PTE_NUM + VMM_L1_PADDING_PTE_NUM) \
              + (VMM_L2L3_PTE_NUM_TOTAL * (index_l1))])

/**
 * @brief Maps physical address of the guest to level 3 descriptors.
 *
 * Maps physical address to the target level 3 translation table descriptor.
 * Configure bits of the target descriptor whiche are initialized by initial
 * function. (lpaed_guest_stage2_map_page)
 *
 * @param *ttbl3 Level 3 translation table descriptor.
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
static void guest_memory_ttbl3_map(union lpaed_t *ttbl3, uint64_t offset,
                uint32_t pages, uint64_t pa, enum memattr mattr)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    printf("%s[%d]: ttbl3:%x offset:%x pte:%x pages:%d, pa:%x\n",
            __func__, __LINE__, (uint32_t) ttbl3, (uint32_t) offset,
            &ttbl3[offset], pages, (uint32_t) pa);
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++) {
        lpaed_guest_stage2_map_page(&ttbl3[index_l3], pa, mattr);
        pa += LPAE_PAGE_SIZE;
    }
}

/**
 * @brief Unmap level 3 descriptors.
 *
 * Unmap descriptors of ttbl3 which is in between offset and offset + pages and
 * makes valid bit zero.
 *
 * @param *ttbl3 Level 3 translation table descriptor.
 * @param offset Offset from the level 3 table descriptor.
 *        - 0 ~ (2MB - pages * 4KB), start contiguous virtual address within
 *          level 2 block (2MB).
 *        - It is aligned L3 descriptor lock size(4KB).
 * @param pages Number of pages.
 *        - 0 ~ 512
 * @return void
 */
static void guest_memory_ttbl3_unmap(union lpaed_t *ttbl3, uint64_t offset,
                uint32_t pages)
{
    int index_l3 = 0;
    int index_l3_last = 0;
    /* Initialize the address spaces with 'invalid' state */
    index_l3 = offset >> LPAE_PAGE_SHIFT;
    index_l3_last = index_l3 + pages;
    for (; index_l3 < index_l3_last; index_l3++)
        ttbl3[index_l3].stage1.valid = 0;
}

/**
 * @brief Unmap ttbl2 and ttbl3 descriptors which is in target virtual
 *        address area.
 *
 * Unmap descriptors of ttbl2 and ttbl3 by making valid bit to zero.
 *
 * - First, make level 2 descriptors invalidate.
 * - Second, if lefts space which can't be covered by level 2 descriptor
 *   (to small), make level 3 descriptors invalidate.
 *
 * @param *ttbl2 Level 2 translation table descriptor.
 * @param va_offset Offset of the virtual address.
 *        - 0 ~ (1GB - size), start contiguous virtual address within level 1
 *          block (1GB).
 *        - It is aligned L2 descriptor lock size(2MB).
 * @param size
 *        - <= 1GB.
 *        - It is aligned page size.
 * @return void
 */
static void guest_memory_ttbl2_unmap(union lpaed_t *ttbl2, uint64_t va_offset,
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
        ttbl2[index_l2].stage1.valid = 0;

    size &= LPAE_BLOCK_L2_MASK;
    if (size) {
        /* last partial block */
        union lpaed_t *ttbl3 = TTBL_L3(ttbl2, index_l2);
        guest_memory_ttbl3_unmap(ttbl3, 0x00000000, size >> LPAE_PAGE_SHIFT);
    }
}

/**
 * @brief Map ttbl2 descriptors.
 *
 * Maps physical address to ttbl2 and ttbl3 descriptors and apply memory
 * attributes.
 *
 * - First, compute index of the target ttbl2 descriptor and block offset.
 * - Second, maps physical address to ttbl3 descriptors if head of block is
 *   not fits size of the ttbl2 descriptor.
 * - Third, maps left physical address to ttbl2 descriptors.
 * - Finally, if lefts the memory, maps it to ttbl3 descriptors.
 *
 * @param *ttbl2 Level 2 translation table descriptor.
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
static void guest_memory_ttbl2_map(union lpaed_t *ttbl2, uint64_t va_offset,
                uint64_t pa, uint32_t size, enum memattr mattr)
{
    uint64_t block_offset;
    uint32_t index_l2;
    uint32_t index_l2_last;
    uint32_t num_blocks;
    uint32_t pages;
    union lpaed_t *ttbl3;
    int i;
    HVMM_TRACE_ENTER();

    printf("ttbl2:%x va_offset:%x pa:%x size:%d\n",
            (uint32_t) ttbl2, (uint32_t) va_offset, (uint32_t) pa, size);
    index_l2 = va_offset >> LPAE_BLOCK_L2_SHIFT;
    block_offset = va_offset & LPAE_BLOCK_L2_MASK;
    printf("- index_l2:%d block_offset:%x\n",
            index_l2, (uint32_t) block_offset);
    /* head < BLOCK */
    if (block_offset) {
        uint64_t offset;
        offset = block_offset >> LPAE_PAGE_SHIFT;
        pages = size >> LPAE_PAGE_SHIFT;
        if (pages > VMM_L3_PTE_NUM)
            pages = VMM_L3_PTE_NUM;

        ttbl3 = TTBL_L3(ttbl2, index_l2);
        guest_memory_ttbl3_map(ttbl3, offset, pages, pa, mattr);
        lpaed_guest_stage2_enable_l2_table(&ttbl2[index_l2]);
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
            lpaed_guest_stage2_enable_l2_table(&ttbl2[i]);
            guest_memory_ttbl3_map(TTBL_L3(ttbl2, i), 0,
                    VMM_L3_PTE_NUM, pa, mattr);
            pa += LPAE_BLOCK_L2_SIZE;
            size -= LPAE_BLOCK_L2_SIZE;
        }
    }
    /* tail < BLOCK */
    if (size > 0) {
        pages = size >> LPAE_PAGE_SHIFT;
        printf("- pages:%d size:%d\n", pages, size);
        if (pages) {
            ttbl3 = TTBL_L3(ttbl2, index_l2_last);
            guest_memory_ttbl3_map(ttbl3, 0, pages, pa, mattr);
            lpaed_guest_stage2_enable_l2_table(&ttbl2[index_l2_last]);
        }
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize ttbl2 entries.
 *
 * Configures ttbl2 descriptor by mapping address of ttbl3 descriptor.
 * And configure all valid bit of ttbl3 descriptors to zero.
 *
 * @param *ttbl2 Level 2 translation table descriptor.
 * @return void
 */
static void guest_memory_ttbl2_init_entries(union lpaed_t *ttbl2)
{
    int i, j;
    HVMM_TRACE_ENTER();
    union lpaed_t *ttbl3;
    for (i = 0; i < VMM_L2_PTE_NUM; i++) {
        ttbl3 = TTBL_L3(ttbl2, i);
        printf("ttbl2[%d]:%x ttbl3[]:%x\n", i, &ttbl2[i], ttbl3);
        lpaed_guest_stage2_conf_l2_table(&ttbl2[i],
                (uint64_t)((uint32_t) ttbl3), 0);
        for (j = 0; j < VMM_L2_PTE_NUM; j++)
            ttbl3[j].stage1.valid = 0;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Initialize delivered ttbl2 descriptors.
 *
 * Initialize ttbl2 descriptors and unmap the descriptors.
 * Finally, map the ttbl2 descriptors by memory map descriptors.
 *
 * @param *ttbl2 Level 2 translation table descriptor.
 * @param *md Device memory map descriptor.
 * @return void
 */
static void guest_memory_init_ttbl2(union lpaed_t *ttbl2, struct memmap_desc *md)
{
    int i = 0;
    HVMM_TRACE_ENTER();
    printf(" - ttbl2:%x\n", (uint32_t) ttbl2);
    if (((uint64_t)((uint32_t) ttbl2)) & 0x0FFFULL)
        printf(" - error: invalid ttbl2 address alignment\n");

    /* construct l2-l3 table hirerachy with invalid pages */
    guest_memory_ttbl2_init_entries(ttbl2);
    guest_memory_ttbl2_unmap(ttbl2, 0x00000000, 0x40000000);
    while (md[i].label != 0) {
        guest_memory_ttbl2_map(ttbl2, md[i].va, md[i].pa,
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
 * @param *ttbl Target translation table descriptor.
 * @param *mdlist[] Memory map descriptor list.
 * @return void
 */
static void guest_memory_init_ttbl(union lpaed_t *ttbl,
            struct memmap_desc *mdlist[])
{
    int i = 0;
    HVMM_TRACE_ENTER();
    while (mdlist[i]) {
        struct memmap_desc *md = mdlist[i];
        if (md[0].label == 0)
            lpaed_guest_stage2_conf_l1_table(&ttbl[i], 0, 0);
        else {
            lpaed_guest_stage2_conf_l1_table(&ttbl[i],
                    (uint64_t)((uint32_t) TTBL_L2(ttbl, i)), 1);
            guest_memory_init_ttbl2(TTBL_L2(ttbl, i), md);
        }
        i++;
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Configures Virtualization Translation Control Register(VTCR).
 *
 * Configure translation control bits by modifing the VTCR.
 * \ref VTCR
 * .
 * - Configuration.
 *   - SL0 - Start at first level
 *   - ORGN0 - Normal memory, outer write-back no write allocate cacheable.
 *   - IRGN0 - Normal memory, inner write-back no write allocate cacheable.
 *
 * @return void
 */
static void guest_memory_init_mmu(void)
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

/**
 * @brief Enables or disables the stage-2 MMU.
 *
 * Configures Hyper Configuration Register(HCR) to enable or disable the
 * virtualization MMU.
 *
 * @param enable Enable or disable the MMU.
 *        - 1 : Enable the MMU.
 *        - 0 : Disable the MMU.
 * @return void
 */
static void guest_memory_stage2_enable(int enable)
{
    uint32_t hcr;
    /* HCR.VM[0] = enable */
    /* printf( "hcr:"); printf_hex32(hcr); printf("\n\r"); */
    hcr = read_hcr();
    if (enable)
        hcr |= (0x1);
    else
        hcr &= ~(0x1);

    write_hcr(hcr);
}

/**
 * @brief Changes the stage-2 translation table base address by configuring
 *        VTTBR.
 *
 * Configures Virtualization Translation Table Base Register(VTTBR) to change
 * the guest. Change vmid and base address from received vmid and ttbl
 * address.
 *
 * @param vmid Received vmid.
 * @param ttbl Level 1 translation table of the guest.
 * @return HVMM_STATUS_SUCCESS only.
 */
static hvmm_status_t guest_memory_set_vmid_ttbl(vmid_t vmid, union lpaed_t *ttbl)
{
    uint64_t vttbr;
    /*
     * VTTBR.VMID = vmid
     * VTTBR.BADDR = ttbl
     */
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    printf("current vttbr:");
    printf_hex64(vttbr);
    printf("\n\r");
#endif
    vttbr &= ~(VTTBR_VMID_MASK);
    vttbr |= ((uint64_t)vmid << VTTBR_VMID_SHIFT) & VTTBR_VMID_MASK;
    vttbr &= ~(VTTBR_BADDR_MASK);
    vttbr |= (uint32_t) ttbl & VTTBR_BADDR_MASK;
    write_vttbr(vttbr);
    vttbr = read_vttbr();
#if 0 /* ignore message due to flood log message */
    printf("changed vttbr:");
    printf_hex64(vttbr);
    printf("\n\r");
#endif
    return HVMM_STATUS_SUCCESS;
}



/**
 * @brief Initializes the virtual mode(guest mode) memory management
 * stage-2 translation.
 *
 * Configure translation tables of guests for stage-2 translation (IPA -> PA).
 *
 * - First, maps physical address of guest start address to descriptors.
 * - And configure the translation table descriptors based on the memory
 *   map descriptor lists.
 * - Last, initializes mmu.
 *
 * @return void
 */
static void guest_memory_init(struct memmap_desc **guest0_map,
        struct memmap_desc **guest1_map)
{
    /*
     * Initializes Translation Table for Stage2 Translation (IPA -> PA)
     */
    int i;
    uint32_t cpu = smp_processor_id();

    HVMM_TRACE_ENTER();

    if (!cpu) {
        for (i = 0; i < NUM_GUESTS_STATIC; i++)
            _vmid_ttbl[i] = &_ttbl_guest[i][0];
        guest_memory_init_ttbl(&_ttbl_guest[0][0], guest0_map);
        guest_memory_init_ttbl(&_ttbl_guest[1][0], guest1_map);
    } else {
        guest_memory_init_ttbl(&_ttbl_guest[2][0], guest0_map);
        guest_memory_init_ttbl(&_ttbl_guest[3][0], guest1_map);
    }

    HVMM_TRACE_EXIT();
}

/**
 * @brief Configures the memory management features.
 *
 * Configure all features of the memory management.
 *
 * - Generate and confgirue hyp mode & virtual mode translation tables.
 * - Configure MAIRx, HMAIRx register.
 *   - \ref Memory_Attribute_Indirection_Register.
 *   - \ref Attribute_Indexes.
 * - Configures Hyp Translation Control Register(HTCR).
 *   - Setting
 *     - Out shareable.
 *     - Normal memory, outer write-through, cacheable.
 *     - Normal memory, inner write-back write-allocate cacheable.
 *     - T0SZ is zero.
 *   - \ref HTCR
 * - Configures Hyper System Control Register(HSCTLR).
 *   - i-cache and alignment checking enable.
 *   - mmu, d-cache, write-implies-xn, low-latency, IRQs disable.
 *   - \ref SCTLR
 * - Configures Hyp Translation Table Base Register(HTTBR).
 *   - Writes the vmm_pgtable value to base address bits.
 *   - \ref HTTBR
 * - Enable MMU and D-cache in HSCTLR.
 * - Initialize heap area.
 *
 * @return HVMM_STATUS_SUCCESS, Always success.
 */
static int memory_hw_init(struct memmap_desc **guest0,
            struct memmap_desc **guest1)
{
    uint32_t cpu = smp_processor_id();
    printf("[memory] memory_init: enter\n\r");

    guest_memory_init(guest0, guest1);

    guest_memory_init_mmu();

    printf("[memory] memory_init: exit\n\r");

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Stops stage-2 translation by disabling mmu.
 *
 * We assume VTCR has been configured and initialized in the memory
 * management module.
 * - Disables stage-2 translation by HCR.vm = 0.
 */
static hvmm_status_t memory_hw_save(void)
{
    /*
     * We assume VTCR has been configured and initialized
     * in the memory management module
     */
    /* Disable Stage 2 Translation: HCR.VM = 0 */
    guest_memory_stage2_enable(0);

    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Restores translation table for the next guest and enable stage-2 mmu.
 *
 * - Chagne stage-2 translation table and vmid.
 * - Eanbles stage-2 MMU.
 *
 * @param guest Context of the next guest.
 */
static hvmm_status_t memory_hw_restore(vmid_t vmid)
{
    /*
     * Restore Translation Table for the next guest and
     * Enable Stage 2 Translation
     */
    guest_memory_set_vmid_ttbl(vmid, _vmid_ttbl[vmid]);

    guest_memory_stage2_enable(1);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t memory_hw_dump(void)
{
    return HVMM_STATUS_SUCCESS;
}

struct memory_ops _memory_ops = {
    .init = memory_hw_init,
    .save = memory_hw_save,
    .restore = memory_hw_restore,
    .dump = memory_hw_dump,
};

struct memory_module _memory_module = {
    .name = "K-Hypervisor Memory Module",
    .author = "Kookmin Univ.",
    .ops = &_memory_ops,
};

