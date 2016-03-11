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

static uint32_t l1_pgtable;
static uint32_t l2_pgtable;
static uint32_t l3_pgtable;

static void write64(uint64_t value, uint32_t addr);
static uint64_t read64(uint32_t addr);

#define GET_OFFSET(i)        (i << 3)               /* size of pgentry */
#define GET_L2_INDEX(i)      GET_OFFSET((i << 9))   /* << 9 == 512 */
#define GET_L3_INDEX(i)      GET_OFFSET((i << 18))  /* << 18 == 512*512 */

#include <hvmm_trace.h>
void SECTION(".init") pgtable_init()
{
    int i, j;

    l1_pgtable = (uint32_t) &__pgtable_start;
    l2_pgtable = l1_pgtable + 0x1000;
    l3_pgtable = l2_pgtable + 0x4000;

    for (i = 0; i < L1_ENTRY; i++) {
        uint32_t l1_table_addr = l1_pgtable + GET_OFFSET(i);
        uint32_t l2_table_addr = l2_pgtable + GET_L2_INDEX(i);
        uint32_t l3_table_addr = l3_pgtable + GET_L3_INDEX(i);

        write64(set_table(l2_table_addr).raw, l1_table_addr);

        for (j = 0; j < L2_ENTRY; j++) {
            l3_table_addr += GET_L2_INDEX(j);
            write64(set_table(l3_table_addr).raw, l2_table_addr + GET_OFFSET(j));
        }
    }
}

void SECTION(".init")
write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size)
{
    uint32_t l1_index, l2_index, l3_index;
    uint32_t l1_table_addr, l2_table_addr, l3_table_addr;
    int i;

    for (i = 0; i < (size / SZ_4K); i++) {
        l1_index = (va & L1_BIT_MASK) >> L1_SHIFT;
        l2_index = (va & L2_BIT_MASK) >> L2_SHIFT;
        l3_index = (va & L3_BIT_MASK) >> L3_SHIFT;

        l1_table_addr = l1_pgtable + GET_OFFSET(l1_index);
        l2_table_addr = l2_pgtable + GET_L2_INDEX(l1_index) + GET_OFFSET(l2_index);
        l3_table_addr = l3_pgtable + GET_L3_INDEX(l1_index) + GET_L2_INDEX(l2_index) + GET_OFFSET(l3_index);

        write64(read64(l1_table_addr) | 0x1, l1_table_addr);
        write64(read64(l2_table_addr) | 0x1, l2_table_addr);
        write64(set_entry(pa, mem_attr, 0, size_4kb).raw, l3_table_addr);

        va += SZ_4K;
        pa += SZ_4K;
    }
}

void SECTION(".init") hyp_memtest(uint32_t base, uint32_t size)
{
    // TODO(casionwoo): implement pagetable walk by manually
}

void write_pgentry(void *pgtable_base, struct memdesc_t *mem_desc, bool is_guest)
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
    l1_index = (va & L1_BIT_MASK) >> L1_SHIFT;

    for (l1_offset = 0; size > 0; l1_offset++ ) {
        l1_pgtable_base[l1_index + l1_offset].table.valid = 1;

        l2_pgtable_base = (pgentry *) (l1_pgtable_base[l1_index + l1_offset].table.base << PAGE_SHIFT);
        l2_index = (va & L2_BIT_MASK) >> L2_SHIFT;

        for (l2_offset = 0; l2_index + l2_offset < L2_ENTRY && size > 0; l2_offset++ ) {
            l2_pgtable_base[l2_index + l2_offset].table.valid = 1;
            l3_pgtable_base = (pgentry *) (l2_pgtable_base[l2_index + l2_offset].table.base << PAGE_SHIFT);
            l3_index = (va & L3_BIT_MASK) >> L3_SHIFT;

            for (l3_offset = 0; l3_index + l3_offset < L3_ENTRY && size > 0; l3_offset++) {
                l3_pgtable_base[l3_index + l3_offset] = set_entry(pa, mem_desc->attr, 3, size_4kb);
                pa += LPAE_PAGE_SIZE;
                va += LPAE_PAGE_SIZE;
                size -= LPAE_PAGE_SIZE;
            }
        }
    }
}

void write_pgentry_4k(void *pgtable_base, struct memdesc_t *mem_desc, bool is_guest)
{
    uint32_t l1_index, l2_index, l3_index;
    uint32_t va, pa;
    uint32_t size;

    pgentry *l1_pgtable_base, *l2_pgtable_base, *l3_pgtable_base;

    size = mem_desc->size;
    if (size > SZ_4K) {
        debug_print("[%s] size is bigger than 4K\n");
        while (1) ;
    }
    va = (uint32_t) mem_desc->va;
    pa = (uint32_t) mem_desc->pa;

    l1_pgtable_base = (pgentry *) pgtable_base;

    l1_index = (va & L1_BIT_MASK) >> L1_SHIFT;
    l2_index = (va & L2_BIT_MASK) >> L2_SHIFT;
    l3_index = (va & L3_BIT_MASK) >> L3_SHIFT;

    l1_pgtable_base[l1_index].table.valid = 1;
    l2_pgtable_base = (pgentry *) (l1_pgtable_base[l1_index].table.base << PAGE_SHIFT);
    l2_pgtable_base[l2_index].table.valid = 1;
    l3_pgtable_base = (pgentry *) (l2_pgtable_base[l2_index].table.base << PAGE_SHIFT);
    l3_pgtable_base[l3_index] = set_entry(pa, mem_desc->attr, 3, size_4kb);
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
void SECTION(".init") set_httbr(void)
{
    write_httbr((uint64_t) l1_pgtable);
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
