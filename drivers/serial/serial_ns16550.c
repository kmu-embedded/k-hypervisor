#include <drivers/serial_ns16550.h>
#include <io.h>

void serial_init()
{
    while (!(readb(NS16550_BASE + NS16550_LSR) & UART_LSR_TEMT))
        ;

    writeb(0x0, NS16550_BASE + NS16550_IER);
    writeb(0x3, NS16550_BASE + NS16550_MCR);
    writeb(0x7, NS16550_BASE + NS16550_FCR);
}

int serial_putc(const char c)
{
    while (!(readb(NS16550_BASE + NS16550_LSR) & UART_LSR_THRE))
        ;

    writeb(c, NS16550_BASE + NS16550_THR);

    return 0;
}

int serial_getc()
{
    while (!(readb(NS16550_BASE + NS16550_LSR) & UART_LSR_DR))
        ;

    return readb(NS16550_BASE + NS16550_RBR);
}
