#include <libc_init.h>
#include <drivers/pl01x.h>

// The variables for heap should be defined in Linker Script.
extern unsigned int __begin_heap;
extern unsigned int __end_heap;

void libc_init()
{
    __malloc_init(&__begin_heap, &__end_heap);
    __libc_putc = &pl01x_putc;
    __libc_getc = &pl01x_getc;
}
