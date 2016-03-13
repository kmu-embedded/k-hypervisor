#include <stdbool.h>
#include <lpae.h>
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
