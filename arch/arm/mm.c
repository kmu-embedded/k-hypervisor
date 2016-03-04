#include <stdbool.h>
#include <debug_print.h>
#include <arch/armv7.h>
#include <rtsm-config.h>
#include <core/vm/guest_memory_hw.h>

#include <mm.h>
#include <lpae.h>

static pgentry hyp_l1_pgtable[L1_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l2_pgtable[L1_ENTRY][L2_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));
static pgentry hyp_l3_pgtable[L1_ENTRY][L2_ENTRY][L3_ENTRY] __attribute((__aligned__(LPAE_PAGE_SIZE)));

uint32_t set_cache(bool state)
{
    return ( state ? (HSCTLR_I | HSCTLR_C) : 0 );
}

/* Set Hyp Memory Attribute Indirection Registers 0 and 1 */
void set_hmair(void)
{
    write_hmair0(HMAIR0_VALUE);
    write_hmair1(HMAIR1_VALUE);
}

/* Set Hyp Translation Control Register(HTCR)*/
void set_htcr(void)
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
void set_httbr()
{
    write_httbr((uint32_t) hyp_l1_pgtable);
}

/* Set Hyp System Control Register(HSCTLR) */
void enable_mmu(void)
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
void SECTION(".init.arch") pgtable_init()
{
    int i, j;
    for(i = 0; i < 4; i++) {
        hyp_l1_pgtable[i] = set_table((uint32_t) hyp_l2_pgtable[i], 0);
        for(j = 0; j < 512; j++) {
            hyp_l2_pgtable[i][j] = set_table((uint32_t) hyp_l3_pgtable[i][j], 0);
        }
    }
}

void set_pgtable(struct memmap_desc *desc)
{
    int i = 0;

    while(desc[i].label != 0) {
        if(desc[i].size == 0x1000) {
            write_pgentry_4k(hyp_l1_pgtable, &desc[i], false);
        } else {
            write_pgentry(hyp_l1_pgtable, &desc[i], false);
        }
        i++;
    }
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

void write_hyp_pgentry(uint32_t va, uint32_t pa, uint8_t mem_attr, uint32_t size)
{
    bool is_guest = false;
    uint32_t l1_index, l2_index, l3_index;
    uint32_t i, nr_loop;
    nr_loop = size / SZ_4K;

    while(1) {
        if (nr_loop == 0) {
            break;
        }
        l1_index = (va & L1_INDEX_MASK) >> L1_SHIFT;
        l2_index = (va & L2_INDEX_MASK) >> L2_SHIFT;
        l3_index = (va & L3_INDEX_MASK) >> L3_SHIFT;

        hyp_l1_pgtable[l1_index].table.valid = 1;
        hyp_l2_pgtable[l1_index][l2_index].table.valid = 1;
        hyp_l3_pgtable[l1_index][l2_index][l3_index] = set_entry(pa, mem_attr, (is_guest ? 3:0), size_4kb);
        va += SZ_4K;
        pa += SZ_4K;
        nr_loop--;
    }
}

void write_pgentry_4k(void *pgtable_base, struct memmap_desc *mem_desc, bool is_guest)
{
    uint32_t l1_index, l2_index, l3_index;
    uint32_t va, pa;
    uint32_t size;

    pgentry *l1_pgtable_base, *l2_pgtable_base, *l3_pgtable_base;

    size = mem_desc->size;
    if (size > 0x1000) {
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
    l3_pgtable_base[l3_index] = set_entry(pa, mem_desc->attr, (is_guest ? 3:0), size_4kb);
}
