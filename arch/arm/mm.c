#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_types.h>
#include <stdbool.h>

#include <mm.h>
#include <lpae.h>

#include <armv7/hsctlr.h>
#include <armv7/htcr.h>

//static pgentry hyp_l1_pgtable[L1_ENTRY];
static pgentry hyp_l2_pgtable[L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l3_pgtable[L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

pgentry *l1_pgtable;
pgentry **l2_pgtable;
pgentry ***l3_pgtable;

uint32_t set_cache(bool state)
{
    return ( state ? (HSCTLR_I | HSCTLR_C) : 0 );
}

hvmm_status_t stage1_mmu_init(void)
{
    uint32_t htcr = 0, hsctlr = 0;
    uint64_t httbr = 0;

    write_hmair0(MAIR0_VALUE);
    write_hmair1(MAIR1_VALUE);

    /* HTCR: Hyp Translation Control Register */
    /* Shareability, Outer Cacheability, Inner Cacheability */
    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    // TODO(wonseok): How to configure T0SZ?

    write_htcr(htcr);
    // FIXME(casionwoo) : Current printf can't support 64-bit, it should be fixed
    printf("htcr: 0x%x%x\n", htcr);

    /* HTTBR : Hyp Translation Table Base Register */
    httbr |= (uint32_t) l1_pgtable;
    write_httbr(httbr);
    printf("httbr: 0x%x%x\n", httbr);

    /* HSCTLR : Hyp System Control Register*/
    /* MMU, Alignment enable */
    hsctlr = (HSCTLR_A | HSCTLR_M);
    hsctlr |= set_cache(true); /* I-Cache, D-Cache init from hsctlr*/
    write_hsctlr(hsctlr);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t stage1_pgtable_init()
{
    uint64_t paddr;
    int i, j;

    l1_pgtable = (pgentry *) aligned_alloc ((sizeof(pgentry) * 4), 0x1000);
    printf("aligned_alloc: l1_pgtable[%p]\n", l1_pgtable);

    paddr = 0x00000000ULL;
    l1_pgtable[0] = set_entry(paddr, MT_DEVICE, 0, size_1gb);
    printf("l1_pgtable[0]: %p, %x\n", l1_pgtable[0], l1_pgtable[0]);
    printf("entry: %x\n", set_entry(paddr, MT_DEVICE, 0, size_1gb).raw);

    paddr += 0x40000000; // start with 0x4000_0000
    l1_pgtable[1] = set_entry(paddr, MT_NONCACHEABLE, 3, size_1gb);
    l1_pgtable[1].block.valid = 0;
    printf("l1_pgtable[1]: %p, %x\n", l1_pgtable[1], l1_pgtable[1]);
    printf("entry: %x\n", set_entry(paddr, MT_NONCACHEABLE, 3, size_1gb).raw);

    paddr += 0x40000000; // start with 0x8000_0000
    l1_pgtable[2] = set_entry(paddr, MT_WRITEBACK_RW_ALLOC, 0, size_1gb);
    printf("l1_pgtable[2]: %p, %x\n", l1_pgtable[2], l1_pgtable[2]);
    printf("entry: %x\n", set_entry(paddr, MT_WRITEBACK_RW_ALLOC, 0, size_1gb).raw);

    paddr += 0x40000000; // start with 0xC000_0000
    l1_pgtable[3] = set_table((uint32_t) &hyp_l2_pgtable[0], 1);
    for(i = 0; i < 512; i++) {
        hyp_l2_pgtable[i] = set_table((uint32_t) hyp_l3_pgtable[i], 1);
        for (j = 0; j < 512; paddr += 0x1000, j++) {
            hyp_l3_pgtable[i][j] = set_entry(paddr, MT_WRITEBACK_RW_ALLOC, 0, size_4kb);
        }
    }

    return HVMM_STATUS_SUCCESS;
}
