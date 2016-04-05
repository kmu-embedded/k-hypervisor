#include <libc_init.h>
#include <serial.h>

// The variables for heap should be defined in Linker Script.
extern unsigned int __begin_heap;
extern unsigned int __end_heap;

void libc_init()
{
    __malloc_init(&__begin_heap, &__end_heap);
    __libc_putc = (__fputc_p) &serial_putc;
    __libc_getc = (__fgetc_p) &serial_getc;
}
