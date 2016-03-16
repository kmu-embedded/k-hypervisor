#include "paging.h"
#include "lpae.h"

#include <assert.h>
#include <io.h>

static void set_memory_attribute(void);
static void setup_httbr(uint32_t base);

extern uint32_t __HYP_PGTABLE;

void paging_init()
{
    paging_create_mapping((addr_t) &__HYP_PGTABLE);
    setup_httbr((addr_t) &__HYP_PGTABLE);
    set_memory_attribute();
}

void paging_create_mapping(addr_t base)
{
    int i;

    uint32_t second_base = base + 0x1000;
    uint32_t third_base = second_base + 0x4000;

    for (i = 0; i < L1_ENTRY; i++) {
        write64(set_table(second_base + GET_L2_INDEX(i)).raw, base + GET_OFFSET(i));
    }

    for (i = 0; i < L1_ENTRY * L2_ENTRY; i++) {
        write64(set_table(third_base + GET_L2_INDEX(i)).raw, second_base + GET_OFFSET(i));
    }
}

void paging_add_mapping(addr_t va, addr_t pa, uint8_t mem_attr, uint32_t size)
{
    int i;
    uint64_t httbr = read_httbr();

    for (i = 0; i < (size >> PAGE_SHIFT); i++, va += 0x1000, pa += 0x1000) {
        write_pgentry((addr_t) httbr, va >> PAGE_SHIFT, pa, mem_attr, 0);
    }
}

void paging_add_ipa_mapping(addr_t base, addr_t ipa, addr_t pa, uint8_t mem_attr, uint32_t size)
{
    int i;

    for (i = 0; i < (size >> PAGE_SHIFT); i++, ipa += 0x1000, pa += 0x1000) {
        write_pgentry(base, ipa >> PAGE_SHIFT, pa, mem_attr, 3);
    }
}

void paging_enable_mmu(void)
{
    uint32_t hsctlr = 0;

    /* MMU, Alignment enable */
    hsctlr |= (HSCTLR_A | HSCTLR_M);
#ifndef __DISABLE_CACHE__
    hsctlr |= (HSCTLR_I | HSCTLR_C);
#else
    write_hsctlr(hsctlr);
#endif
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
static void set_memory_attribute(void)
{
    uint32_t htcr = 0;

    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    write_htcr(htcr);

    write_hmair0(HMAIR0_VALUE);
    write_hmair1(HMAIR1_VALUE);
}

/* Set Hyp Translation Table Base Register(HTTBR) */
static void setup_httbr(addr_t pgtable)
{
    write_httbr((uint64_t) pgtable);
    assert(read_httbr() == (uint32_t) &__HYP_PGTABLE);
}
