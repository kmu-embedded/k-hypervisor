#include <lib/print.h>
#include <lib/format.h>
#include <drivers/pl01x.h>

void init_print()
{
    format_reg_putc(&uart_putc);
    format_reg_puts(&uart_print);
}

void printf(const char *format, ...)
{
    __builtin_va_list ap;
    va_start(ap, format);
    format_print(format, ap);
    va_end(ap);
}

