#include "lpae.h"
#include <io.h>

pgentry set_table(addr_t pa)
{
    pgentry entry;

    entry.raw = 0;
    entry.table.valid = 0;
    entry.table.type = 1;
    entry.table.base = pa >> PAGE_SHIFT;

    return entry;
}

static pgentry set_entry(addr_t pa, uint8_t mem_attr, uint8_t ap, uint8_t af)
{
    pgentry entry;

    entry.raw = 0;
    entry.page.valid = 1;
    entry.page.type = 1;
    entry.page.base = pa >> L3_SHIFT;
    entry.page.mem_attr = mem_attr;
    entry.page.ap = ap;
    entry.page.sh = 3;
    entry.page.af = af;
//    entry.page.ng = 1;
    entry.page.ng = 0;
    entry.page.cb = 0;
    entry.page.pxn = 0;
    entry.page.xn = 0;

    return entry;
}

void write_pgentry(addr_t base, addr_t va, addr_t pa, uint8_t mem_attr, uint8_t ap, uint8_t af)
{
    uint32_t l1_index, l2_index, l3_index;
    addr_t l1_entry_addr, l2_entry_addr, l3_entry_addr;

    l1_index = ((va & 0xC0000) >> ENTRY_SHIFT) >> ENTRY_SHIFT;
    l2_index = ((va & 0x3FE00) >> ENTRY_SHIFT);
    l3_index = (va & 0x001FF);

    l1_entry_addr = base + GET_OFFSET(l1_index);
    l2_entry_addr = (base + 0x1000) + GET_L2_INDEX(l1_index) + GET_OFFSET(l2_index);
    l3_entry_addr = (base + 0x5000) + GET_L3_INDEX(l1_index) + GET_L2_INDEX(l2_index)
                    + GET_OFFSET(l3_index);

    write64(read64(l1_entry_addr) | set_valid, l1_entry_addr);
    write64(read64(l2_entry_addr) | set_valid, l2_entry_addr);
    write64(set_entry(pa, mem_attr, ap, af).raw, l3_entry_addr);
}
