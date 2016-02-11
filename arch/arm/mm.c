#include <mm.h>

pgentry set_table(uint32_t paddr, uint32_t valid)
{
    pgentry entry;

    entry.raw = 0;
    entry.table.valid = valid;
    entry.table.type = 1;
    entry.table.base = paddr >> PAGE_SHIFT;

    return entry;
}

// TODO(wonseok): configure AP(Access Permission) bit
// currently, we are going to set AP as a read/write only at PL2.
pgentry set_entry(uint32_t paddr, uint8_t mem_attr, uint8_t ap, pgsize_t size)
{
    pgentry entry;

    switch(size) {
        case size_1gb:
            entry.raw = 0;
            entry.block.valid = 1;
            entry.block.type = 0;
            entry.block.base = paddr >> L1_SHIFT;
            entry.block.mem_attr = mem_attr;
            entry.block.ap = ap; //
            entry.block.sh = 3; //
            entry.block.af = 1;
            entry.block.ng = 1; //
            entry.block.cb = 0;
            entry.block.pxn = 0;
            entry.block.xn = 0;
            break;

        case size_2mb:
            entry.raw = 0;
            entry.block.valid = 1;
            entry.block.type = 0;
            entry.block.base = paddr >> L2_SHIFT;
            entry.block.mem_attr = mem_attr;
            entry.block.ap = ap;
            entry.block.sh = 3;
            entry.block.af = 1;
            entry.block.ng = 1;
            entry.block.cb = 0;
            entry.block.pxn = 0;
            entry.block.xn = 0;
            break;

        case size_4kb:
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
            break;

        default:
            break;
    }

    return entry;
}
