#include <stdio.h>
#include <armv7_p15.h>
#include <hvmm_types.h>
#include <stdbool.h>
#include <rtsm-config.h>
#include <guest_memory_hw.h>

#include <mm.h>
#include <lpae.h>

#include <armv7/hsctlr.h>
#include <armv7/htcr.h>

static pgentry hyp_l1_pgtable[L1_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

struct memmap_desc device_md[] = {
    { "l1_0", 0x00000000, 0x00000000, SZ_1G, MT_DEVICE},
    { "l1_1", 0x40000000, 0x40000000, SZ_1G, MT_NONCACHEABLE},
    { "l1_2", 0x80000000, 0x80000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { "l1_3", 0xC0000000, 0xC0000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { 0, 0, 0, 0, 0 }
};

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
    httbr |= (uint32_t) hyp_l1_pgtable;
    write_httbr(httbr);
    printf("httbr: 0x%x%x\n", httbr);

    /* HSCTLR : Hyp System Control Register*/
    /* MMU, Alignment enable */
    hsctlr = (HSCTLR_A | HSCTLR_M);
    hsctlr |= set_cache(true); /* I-Cache, D-Cache init from hsctlr*/
    write_hsctlr(hsctlr);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t stage1_pgtable_create()
{
    int i, j, k;

    for(i = 0; i < 4; i++) {
        hyp_l1_pgtable[i] = set_table((uint32_t) hyp_l2_pgtable[i], 1);
        for(j = 0; j < 512; j++) {
            hyp_l2_pgtable[i][j] = set_table((uint32_t) hyp_l3_pgtable[i][j], 1);
        }
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t stage1_pgtable_init()
{
    int i = 0;
    while(device_md[i].label != 0) {
        write_pgentry(hyp_l1_pgtable, &device_md[i], true);
        i++;
    }

    return HVMM_STATUS_SUCCESS;
}


