#include <platform.h>
#include <lpae.h>

#include <mm.h>

#include <stdio.h>
#include <memory.h>
#include <armv7_p15.h>
#include <rtsm-config.h> // FOR SZ_4K

struct memmap_desc memdesc[] = {
    { "v2m_serial0", 0x1C090000, 0x1C090000, SZ_4K, MT_DEVICE },
    { "wdt", 0x1C0F0000, 0x1C0F0000, SZ_4K, MT_DEVICE },
    { "gicd", 0x2c001000, 0x2c001000, SZ_4K, MT_DEVICE },
    { "gicc", 0x2c002000, 0x2c002000, SZ_4K, MT_DEVICE },
    { "gicc", 0x2c003000, 0x2c003000, SZ_4K, MT_DEVICE },
    { "gich", 0x2c004000, 0x2c004000, SZ_4K, MT_DEVICE },
    { "gicv", 0x2c005000, 0x2c005000, SZ_4K, MT_DEVICE },
    { "gicvi", 0x2c006000, 0x2c006000, SZ_4K, MT_DEVICE },
    { "l1_2", 0x80000000, 0x80000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { "l1_3", 0xC0000000, 0xC0000000, SZ_1G, MT_WRITEBACK_RW_ALLOC},
    { 0, 0, 0, 0, 0 }
};

void init_platform()
{
    pl01x_init(115200, 24000000);
    __malloc_init();
    pgtable_init();
    set_pgtable(&memdesc);
    enable_mmu();
}
