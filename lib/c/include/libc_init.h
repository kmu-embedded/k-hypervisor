#ifndef __LIBC_INIT_H__
#define __LIBC_INIT_H__

typedef int (*__fputc_p)(int c);
extern __fputc_p __libc_putc;

typedef void (*__fgetc_p)();
extern __fgetc_p __libc_getc;

void __malloc_init();

#endif
