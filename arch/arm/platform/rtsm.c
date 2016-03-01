#include <platform.h>
#include <stdio.h>
#include <stdint.h>
#include <libc_init.h>
#include <pl01x.h>

#include <mm.h>
#include <size.h>
#include <lpae.h>

struct memmap_desc kernel_memory[] = {
    { "kernel", 0xF0000000, 0xF0000000, SZ_256M, MT_WRITEBACK_RW_ALLOC},
    { 0, 0, 0, 0, 0 }
};
void platform_init()
{
    // add_device_mapping()
    // serial devices
    write_hyp_pgentry(0x1C090000, 0x1C090000, MT_DEVICE, SZ_4K);
    write_hyp_pgentry(0x1C0A0000, 0x1C0A0000, MT_DEVICE, SZ_4K);
    write_hyp_pgentry(0x1C0B0000, 0x1C0B0000, MT_DEVICE, SZ_4K);
    write_hyp_pgentry(0x1C0C0000, 0x1C0C0000, MT_DEVICE, SZ_4K);

    // TODO(wonseok): add GIC memory using cbar

    // hypervisor memory
    set_pgtable(&kernel_memory);
}

void console_init()
{
    // TODO(wonseok): add general initialization for console devices.
    pl01x_init(115200, 24000000);

    // Register putc and getc for printf and sscanf.
    __libc_putc = &pl01x_putc;
    __libc_getc = &pl01x_getc;
}

void dev_init()
{
}
