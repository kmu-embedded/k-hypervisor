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
#include <gic_regs.h>

static pgentry hyp_l1_pgtable[L1_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

struct memmap_desc vmm_memdesc[] = {
    { "sysreg", 0x1C010000, 0x1C010000, SZ_4K, MT_DEVICE },
    { "sysctl", 0x1C020000, 0x1C020000, SZ_4K, MT_DEVICE },
    { "aaci", 0x1C040000, 0x1C040000, SZ_4K, MT_DEVICE },
    { "mmci", 0x1C050000, 0x1C050000, SZ_4K, MT_DEVICE },
    { "kmi", 0x1C060000, 0x1C060000,  SZ_64K, MT_DEVICE },
    { "kmi2", 0x1C070000, 0x1C070000, SZ_64K, MT_DEVICE },
    { "v2m_serial0", 0x1C090000, 0x1C090000, SZ_4K, MT_DEVICE },
    { "v2m_serial1", 0x1C0A0000, 0x1C0A0000, SZ_4K, MT_DEVICE },
    { "v2m_serial2", 0x1C0B0000, 0x1C0B0000, SZ_4K, MT_DEVICE },
    { "v2m_serial3", 0x1C0C0000, 0x1C0C0000, SZ_4K, MT_DEVICE },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MT_DEVICE },
    { "v2m_timer01(sp804)", 0x1C110000, 0x1C110000, SZ_4K, MT_DEVICE },
    { "v2m_timer23", 0x1C120000, 0x1C120000, SZ_4K, MT_DEVICE },
    { "rtc", 0x1C170000, 0x1C170000, SZ_4K, MT_DEVICE },
    { "clcd", 0x1C1F0000, 0x1C1F0000, SZ_4K, MT_DEVICE },
    { "gicd", 0x2c001000, 0x2c001000, SZ_4K, MT_DEVICE },
    { "gicc", 0x2c002000, 0x2c002000, SZ_4K, MT_DEVICE },
    { "gicc", 0x2c003000, 0x2c003000, SZ_4K, MT_DEVICE },
    { "gich", 0x2c004000, 0x2c004000, SZ_4K, MT_DEVICE },
    { "gicv", 0x2c005000, 0x2c005000, SZ_4K, MT_DEVICE },
    { "gicvi", 0x2c006000, 0x2c006000, SZ_4K, MT_DEVICE },
    { "SMSC91c111i", 0x1A000000, 0x1A000000, SZ_16M, MT_DEVICE },
    { "simplebus2", 0x18000000, 0x18000000, SZ_64M, MT_DEVICE },
    { "l1_2", 0x80000000, 0x80000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { "l1_3", 0xC0000000, 0xC0000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { 0, 0, 0, 0, 0 }
};

uint32_t set_cache(bool state)
{
    return ( state ? (HSCTLR_I | HSCTLR_C) : 0 );
}

hvmm_status_t enable_mmu(void)
{
    uint32_t htcr = 0, hsctlr = 0;
    uint64_t httbr = 0;

    /* Configure Memory Attributes */
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

hvmm_status_t pgtable_init()
{
    int i, j;

    for(i = 0; i < 4; i++) {
        hyp_l1_pgtable[i] = set_table((uint32_t) hyp_l2_pgtable[i], 0);
        for(j = 0; j < 512; j++) {
            hyp_l2_pgtable[i][j] = set_table((uint32_t) hyp_l3_pgtable[i][j], 0);
        }
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t set_pgtable()
{
    int i = 0;

    while(vmm_memdesc[i].label != 0) {
        write_pgentry(hyp_l1_pgtable, &vmm_memdesc[i], false);
        i++;
    }

    return HVMM_STATUS_SUCCESS;
}

void write_pgentry(void *pgtable_base, struct memmap_desc *mem_desc, bool is_guest)
{
    uint32_t l1_offset, l2_offset, l3_offset;
    uint32_t l1_index, l2_index, l3_index;
    uint32_t va, pa;
    uint32_t size;

    pgentry *l1_pgtable_base, *l2_pgtable_base, *l3_pgtable_base;

    size = mem_desc->size;
    va = (uint32_t) mem_desc->va;
    pa = (uint32_t) mem_desc->pa;

    l1_pgtable_base = (pgentry *) pgtable_base;
    l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
    for (l1_offset = 0; size > 0; l1_offset++ ) {
        l1_pgtable_base[l1_index + l1_offset].table.valid = 1;
        l2_pgtable_base = l1_pgtable_base[l1_index + l1_offset].table.base << PAGE_SHIFT;
        l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;

        for (l2_offset = 0; l2_index + l2_offset < L2_ENTRY && size > 0; l2_offset++ ) {
            l2_pgtable_base[l2_index + l2_offset].table.valid = 1;
            l3_pgtable_base = l2_pgtable_base[l2_index + l2_offset].table.base << PAGE_SHIFT;
            l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

            for (l3_offset = 0; l3_index + l3_offset < L3_ENTRY && size > 0; l3_offset++) {
                l3_pgtable_base[l3_index + l3_offset] = set_entry(pa, mem_desc->attr, (is_guest ? 3:0), size_4kb);
                pa += LPAE_PAGE_SIZE;
                va += LPAE_PAGE_SIZE;
                size -= LPAE_PAGE_SIZE;
            }
        }
    }
}
