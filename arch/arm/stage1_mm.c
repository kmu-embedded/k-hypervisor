#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_types.h>
#include <stage1_mm.h>
#include <mm.h>

static pgentry hyp_l1_pgtable[L1_ENTRY];
static pgentry hyp_l2_pgtable[L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l3_pgtable[L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

static pgentry set_table(uint32_t paddr);
static pgentry set_entry(uint32_t paddr, uint8_t attr_indx, pgsize_t size);

static pgentry set_table(uint32_t paddr)
{
    pgentry entry;

    entry.raw = 0;
    entry.table.valid = 1;
    entry.table.type = 1;
    entry.table.base = paddr >> PAGE_SHIFT;

    return entry;
}

// TODO(wonseok): configure AP(Access Permission) bit
// currently, we are going to set AP as a read/write only at PL2.
static pgentry set_entry(uint32_t paddr, uint8_t attr_indx, pgsize_t size)
{
    pgentry entry;

    switch(size) {
        case size_1gb:
            entry.raw = 0;
            entry.block.valid = 1;
            entry.block.type = 0;
            entry.block.base = paddr >> L1_SHIFT;
            entry.block.attr_indx = attr_indx;
            entry.block.ap = 1;
            entry.block.sh = 3;
            entry.block.af = 1;
            entry.block.ng = 1;
            entry.block.cb = 0;
            entry.block.pxn = 0;
            entry.block.xn = 0;
            break;

        case size_2mb:
            entry.raw = 0;
            entry.block.valid = 1;
            entry.block.type = 0;
            entry.block.base = paddr >> L2_SHIFT;
            entry.block.attr_indx = attr_indx;
            entry.block.ap = 1;
            entry.block.sh = 3;
            entry.block.af = 1;
            entry.block.ng = 1;
            entry.block.cb = 0;
            entry.block.pxn = 0;
            entry.block.xn = 0;
            break;

        case size_4kb:
            entry.raw = 0;
            entry.page.valid = 1;
            entry.page.type = 1;
            entry.page.base = paddr >> L3_SHIFT;
            entry.page.attr_indx = attr_indx;
            entry.page.ap = 1;
            entry.page.sh = 3;
            entry.page.af = 1;
            entry.page.ng = 1;
            entry.page.cb = 0;
            entry.page.pxn = 0;
            entry.page.xn = 0;
            break;

        default:
            break;
    }

    return entry;
}

#define HSCTLR_TE        (1 << 30) /**< Thumb Exception enable. */
#define HSCTLR_EE        (1 << 25) /**< Exception Endianness. */
#define HSCTLR_FI        (1 << 21) /**< Fast Interrupts configuration enable. */
#define HSCTLR_WXN       (1 << 19) /**< Write permission emplies XN. */
#define HSCTLR_I         (1 << 12) /**< Instruction cache enable.  */
#define HSCTLR_CP15BEN   (1 << 7)  /**< In ARMv7 this bit is RAZ/SBZP. */
#define HSCTLR_C         (1 << 2)  /**< Cache enable. */
#define HSCTLR_A         (1 << 1)  /**< Alignment check enable. */
#define HSCTLR_M         (1 << 0)  /**< MMU enable. */
#define HSCTLR_BASE       0x30c51878  /**< HSTCLR Base address */

#define OUTER_WRITETHROUGH_CACHEABLE (WRITETHROUGH_CACHEABLE << 10)
#define INNER_WRITETHROUGH_CACHEABLE (WRITETHROUGH_CACHEABLE << 12)

hvmm_status_t enable_stage1_mmu(void)
{
    uint32_t htcr = 0, hsctlr = 0;
    uint64_t httbr = 0;

    write_mair0(MAIR0_VALUE);
    write_mair1(MAIR1_VALUE);
    write_hmair0(MAIR0_VALUE);
    write_hmair1(MAIR1_VALUE);

    /* HTCR: Hyp Translation Control Register */
    /* Shareability, Outer Cacheability, Inner Cacheability */
    htcr |= (INNER_SHAREABLE << 12 | OUTER_WRITETHROUGH_CACHEABLE
           | INNER_WRITETHROUGH_CACHEABLE );
    // TODO(wonseok): How to configure T0SZ?
    write_htcr(htcr);
    // FIXME(casionwoo) : Current printf can't support 64-bit, it should be fixed
    printf("hsctlr: 0x%x%x\n", htcr);

    /* HTTBR : Hyp Translation Table Base Register */
    httbr |= (uint32_t) &hyp_l1_pgtable;
    write_httbr(httbr);
    printf("httbr: 0x%x%x\n", httbr);

    /* HSCTLR : Hyp System Control Register*/
    /* I-Cache, D-Cache, MMU, Alignment enable */
    hsctlr = (HSCTLR_I | HSCTLR_A | HSCTLR_M | HSCTLR_C);
    write_hsctlr(hsctlr);
    printf("hsctlr: 0x%x%x\n", hsctlr);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t stage1_mm_init()
{
    uint64_t paddr = 0x00000000ULL;
    int i, j;

    hyp_l1_pgtable[0] = set_entry(paddr, MT_DEVICE, size_1gb);
    paddr += 0x40000000;
    hyp_l1_pgtable[1] = set_entry(paddr, MT_NONCACHEABLE, size_1gb);
    paddr += 0x40000000;
    hyp_l1_pgtable[2] = set_entry(paddr, MT_WRITEBACK, size_1gb);

    paddr += 0x40000000;
    hyp_l1_pgtable[3] = set_table((uint32_t) &hyp_l2_pgtable[0]);

    for(i = 0; i < 512; i++) {
        hyp_l2_pgtable[i] = set_table((uint32_t) hyp_l3_pgtable[i]);

        for (j = 0; j < 512; paddr += 0x1000, j++) {
            hyp_l3_pgtable[i][j] = set_entry(paddr, MT_WRITETHROUGH_RW_ALLOC, size_4kb);
        }
    }

    return HVMM_STATUS_SUCCESS;
}
