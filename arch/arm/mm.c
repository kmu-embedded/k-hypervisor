#include <stdbool.h>
#include <debug_print.h>
#include <arch/armv7.h>
#include <rtsm-config.h>
#include <core/vm/guest_memory_hw.h>

#include <mm.h>
#include <lpae.h>
#include <asm/asm.h>
#include <io.h>

extern uint32_t __pgtable_start;
extern uint32_t __pgtable_end;

static void write64(uint64_t value, uint32_t addr);
static uint64_t read64(uint32_t addr);

#define GET_OFFSET(i)        (i << 3)               /* size of pgentry */
#define GET_L2_INDEX(i)      GET_OFFSET((i << 9))   /* << 9 == 512 */
#define GET_L3_INDEX(i)      GET_OFFSET((i << 18))  /* << 18 == 512*512 */

void SECTION(".init") pgtable_init(uint32_t base)
{
    int i;

    if (base == 0x0) {
        base = (uint32_t) &__pgtable_start;
    }

    uint32_t second_base = base + 0x1000;
    uint32_t third_base = second_base + 0x4000;

    for (i = 0; i < L1_ENTRY; i++) {
        write64(set_table(second_base + GET_L2_INDEX(i), invalid_page).raw, base + GET_OFFSET(i));
    }

    for (i = 0; i < L1_ENTRY * L2_ENTRY; i++) {
        write64(set_table(third_base + GET_L2_INDEX(i), invalid_page).raw, second_base + GET_OFFSET(i));
    }
}

void write_pgentry_4k(uint32_t base, uint32_t va, uint32_t pa, uint8_t mem_attr, uint8_t ap)
{
    uint32_t l1_mask, l2_mask, l3_mask;
    uint32_t l1_base, l2_base, l3_base;

    l1_mask = (va & 0xC0000) >> 18;
    l2_mask = (va & 0x3FE00) >> 9;
    l3_mask = (va & 0x001FF);

    if (base == 0) {
        base = (uint32_t) &__pgtable_start;
    }

    l1_base = base + GET_OFFSET(l1_mask);
    l2_base = (base + 0x1000) + GET_L2_INDEX(l1_mask) + GET_OFFSET(l2_mask);
    l3_base = (base + 0x5000) + GET_L3_INDEX(l1_mask) + GET_L2_INDEX(l2_mask) + GET_OFFSET(l3_mask);

    write64(read64(l1_base) | valid_page, l1_base);
    write64(read64(l2_base) | valid_page, l2_base);
    write64(set_entry(pa, mem_attr, ap).raw, l3_base);
}

void SECTION(".init")
write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size)
{
    int i;
    for (i = 0; i < (size >> 12); i++, va += 0x1000, pa += 0x1000) {
        write_pgentry_4k(0, va >> 12, pa, mem_attr, 0);
    }
}

void write_vm_pgentry(void *pgtable_base, uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size, bool is_guest)
{
    int i;
    for (i = 0; i < (size >> 12); i++, va += 0x1000, pa += 0x1000) {
        write_pgentry_4k((uint32_t) pgtable_base, va >> 12, pa, mem_attr, 3);
    }
}

void SECTION(".init") hyp_memtest(uint32_t base, uint32_t size)
{
    // TODO(casionwoo): implement pagetable walk by manually
}


static void write64(uint64_t value, uint32_t addr)
{
    uint32_t upper = 0, lower = 0;
    upper = (value >> 32) & 0xFFFFFFFF;
    lower = (value & 0x00000000FFFFFFFFUL);
    __writel(lower, addr);
    __writel(upper, addr + 0x4);
}

static uint64_t read64(uint32_t addr)
{
    uint64_t result = 0x0UL;
    result |= __readl(addr);
    result |= (uint64_t) __readl(addr + 0x4) << 32;
    return result;
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
void SECTION(".init") set_hmair(void)
{
    write_hmair0(HMAIR0_VALUE);
    write_hmair1(HMAIR1_VALUE);
}

/* Set Hyp Translation Control Register(HTCR)*/
void SECTION(".init") set_htcr(void)
{
    uint32_t htcr = 0;
    /* Shareability, Outer Cacheability, Inner Cacheability */
    htcr |= INNER_SHAREABLE << HTCR_SH0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_IRGN0_BIT;
    htcr |= WRITEBACK_CACHEABLE << HTCR_ORGN0_BIT;
    write_htcr(htcr);
}

/* Set Hyp Translation Table Base Register(HTTBR) */
#include <assert.h>
void SECTION(".init") set_httbr(void)
{
    uint64_t httbr = (uint32_t) (&__pgtable_start);
    write_httbr(httbr);
    assert(read_httbr() == (uint32_t) &__pgtable_start);
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
