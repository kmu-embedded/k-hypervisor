#include "lpae.h"

pgentry set_table(uint32_t paddr, uint32_t valid)
{
    pgentry entry;

    entry.raw = 0;
    entry.table.valid = valid;
    entry.table.type = 1;
    entry.table.base = paddr >> PAGE_SHIFT;

    return entry;
}

pgentry set_entry(uint32_t paddr, uint8_t mem_attr, uint8_t ap)
{
    pgentry entry;

    entry.raw = 0;
    entry.page.valid = 1;
    entry.page.type = 1;
    entry.page.base = paddr >> L3_SHIFT;
    entry.page.mem_attr = mem_attr;
    entry.page.ap = ap;
    entry.page.sh = 3;
    entry.page.af = 1;
    entry.page.ng = 1;
    entry.page.cb = 0;
    entry.page.pxn = 0;
    entry.page.xn = 0;

    return entry;
}

#include <io.h>
void write64(uint64_t value, uint32_t addr)
{
    uint32_t upper = 0, lower = 0;
    upper = (value >> 32) & 0xFFFFFFFF;
    lower = (value & 0x00000000FFFFFFFFUL);
    __writel(lower, addr);
    __writel(upper, addr + 0x4);
}

uint64_t read64(uint32_t addr)
{
    uint64_t result = 0x0UL;
    result |= __readl(addr);
    result |= (uint64_t) __readl(addr + 0x4) << 32;
    return result;
}

#include <stdio.h>

void write_pgentry(uint32_t base, uint32_t va, uint32_t pa, uint8_t mem_attr, uint8_t ap)
{
    uint32_t l1_index, l2_index, l3_index;
    uint32_t l1_entry_addr, l2_entry_addr, l3_entry_addr;

    l1_index = ((va & 0xC0000) >> ENTRY_SHIFT) >> ENTRY_SHIFT;
    l2_index = ((va & 0x3FE00) >> ENTRY_SHIFT);
    l3_index = (va & 0x001FF);

    l1_entry_addr = base + GET_OFFSET(l1_index);
    l2_entry_addr = (base + 0x1000) + GET_L2_INDEX(l1_index) + GET_OFFSET(l2_index);
    l3_entry_addr = (base + 0x5000) + GET_L3_INDEX(l1_index) + GET_L2_INDEX(l2_index)
                                    + GET_OFFSET(l3_index);

    write64(read64(l1_entry_addr) | set_valid, l1_entry_addr);
    write64(read64(l2_entry_addr) | set_valid, l2_entry_addr);
    write64(set_entry(pa, mem_attr, ap).raw, l3_entry_addr);
}
