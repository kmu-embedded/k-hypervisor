#include <stdio.h>
#include <stdint.h>
#include <libc_init.h>

extern int __fgetc();
/* Put character for khypervisor */
__fgetc_p __libc_getc;

int
__fgetc()
{
    return (int) __libc_getc;
}

