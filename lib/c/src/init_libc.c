#include <libc_init.h>
#include <drivers/pl01x.h>

void libc_init()
{
    __malloc_init();
    __libc_putc = &pl01x_putc;
    __libc_getc = &pl01x_getc;
}
