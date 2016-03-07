#include <stdbool.h>
#include <debug_print.h>
#include <arch/armv7.h>
#include <rtsm-config.h>
#include <core/vm/guest_memory_hw.h>

#include <mm.h>
#include <lpae.h>
#include <asm/asm.h>

static unsigned int l1_pgtable;
static unsigned int l2_pgtable;
static unsigned int l3_pgtable;

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

    // TODO(wonseok): How to configure T0SZ?
    write_htcr(htcr);
}

/* Set Hyp Translation Table Base Register(HTTBR) */
void SECTION(".init") set_httbr(void)
{
    write_httbr((uint32_t) l1_pgtable);
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

#include <asm/asm.h>
#include <io.h>
void inline write64(uint64_t value, uint32_t addr)
{
    uint32_t upper = 0, lower = 0;
    upper = (value >> 32) & 0xFFFFFFFF;
    lower = (value & 0x00000000FFFFFFFFUL);
    writel(lower, addr);
    writel(upper, addr + 0x4);
}

uint64_t inline read64(uint32_t addr)
{
    uint64_t result = 0x0UL;
    result |= readl(addr);
    result |= readl(addr + 0x4) << 32;
    return result;
}

extern unsigned int __pgtable_start;
extern unsigned int __pgtable_end;

void SECTION(".init") pgtable_init()
{
    int i, j;
    uint32_t l1_offset, l2_offset, l3_offset;

    l1_pgtable = &__pgtable_start;
    l2_pgtable = l1_pgtable + 0x1000;
    l3_pgtable = l2_pgtable + 0x4000;

    for(i = 0; i < 4; i++) {
        l1_offset = i << 3;
        l2_offset = (i << 9) << 3;
        write64(set_table(l2_pgtable + l2_offset).raw, l1_pgtable + l1_offset);

        for(j = 0; j < 512; j++) {
            l3_offset = ((i << 18) + j << 9) << 3;
            write64(set_table(l3_pgtable + l3_offset).raw, l2_pgtable + l2_offset);

        }
    }
}

void SECTION(".init")
write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size)
{
    uint32_t l1_index, l2_index, l3_index;
    int i;

    for (i = 0; i < (size/SZ_4K); i++) {
        l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
        l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
        l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

        // set l1 entry to valid
        write64(read64(l1_pgtable + (l1_index << 3)) | 0x1, l1_pgtable + (l1_index << 3));

        // set l2 entry to valid
        write64(read64(l2_pgtable + (((l1_index << 9) + l2_index) << 3 )) | 0x1,
                l2_pgtable + (((l1_index << 9) + l2_index) << 3));

        // write page table
        write64(set_entry(pa, mem_attr, 0, size_4kb).raw,
                l3_pgtable + (((l1_index << 18) + (l2_index << 9) + l3_index) << 3));

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
        while(1) ;
    }
    va = (uint32_t) mem_desc->va;
    pa = (uint32_t) mem_desc->pa;

    l1_pgtable_base = (pgentry *) pgtable_base;
    l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
    l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
    l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

    l1_pgtable_base[l1_index].table.valid = 1;
    l2_pgtable_base = l1_pgtable_base[l1_index].table.base << PAGE_SHIFT;
    l2_pgtable_base[l2_index].table.valid = 1;
    l3_pgtable_base = l2_pgtable_base[l2_index].table.base << PAGE_SHIFT;
    l3_pgtable_base[l3_index] = set_entry(pa, mem_desc->attr, 3, size_4kb);
}
