#include "lpae.h"
#include "mm.h"
#include <assert.h>

static void set_hmair(void);
static void set_htcr(void);
static void set_httbr(uint32_t pgtable);

extern uint32_t __HYP_PGTABLE;
void SECTION(".init") mm_init()
{
    set_hmair();
    set_htcr();
    set_httbr((uint32_t) &__HYP_PGTABLE);
    pgtable_init((uint32_t) &__HYP_PGTABLE);
}

void SECTION(".init")
add_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size)
{
    int i;
    uint64_t httbr = read_httbr();

    for (i = 0; i < (size >> PAGE_SHIFT); i++, va += 0x1000, pa += 0x1000) {
        write_pgentry((uint32_t) httbr, va >> PAGE_SHIFT, pa, mem_attr, 0);
    }
}

/* Set Hyp System Control Register(HSCTLR) */
void SECTION(".init") enable_mmu(void)
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
static void SECTION(".init") set_hmair(void)
{
    write_hmair0(HMAIR0_VALUE);
    write_hmair1(HMAIR1_VALUE);
}

/* Set Hyp Translation Control Register(HTCR)*/
static void SECTION(".init") set_htcr(void)
{
    uint32_t htcr = 0;
    /* Shareability, Outer Cacheability, Inner Cacheability */
    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    write_htcr(htcr);
}

/* Set Hyp Translation Table Base Register(HTTBR) */
static void SECTION(".init") set_httbr(uint32_t pgtable)
{
    write_httbr((uint64_t) pgtable);
    assert(read_httbr() == (uint32_t) &__HYP_PGTABLE);
}

void SECTION(".init") pgtable_init(uint32_t pgtable_base)
{
    int i;

    uint32_t second_base = pgtable_base + 0x1000;
    uint32_t third_base = second_base + 0x4000;

    for (i = 0; i < L1_ENTRY; i++) {
        write64(set_table(second_base + GET_L2_INDEX(i), set_invalid).raw,
                pgtable_base + GET_OFFSET(i));
    }

    for (i = 0; i < L1_ENTRY * L2_ENTRY; i++) {
        write64(set_table(third_base + GET_L2_INDEX(i), set_invalid).raw,
                second_base + GET_OFFSET(i));
    }
}

