#include <platform.h>
#include <stdio.h>
#include <stdint.h>
#include <libc_init.h>
#include <pl01x.h>

void platform_init()
{
    // add_device_mapping()
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
