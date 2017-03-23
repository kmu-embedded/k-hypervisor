#ifndef __NS16550_H__
#define __NS16550_H__

#define NS16550_BASE    0x01C28000

#define NS16550_RBR     0x00
#define NS16550_THR     0x00
#define NS16550_DLL     0x00
#define NS16550_DLH     0x04
#define NS16550_IER     0x04
#define NS16550_IIR     0x08
#define NS16550_FCR     0x08
#define NS16550_LCR     0x0C
#define NS16550_MCR     0x10
#define NS16550_LSR     0x14
#define NS16550_MSR     0x18
#define NS16550_SCH     0x1C
#define NS16550_USR     0x7C
#define NS16550_TFL     0x80
#define NS16550_RFL     0x84
#define NS16550_HALT    0xA4

#define UART_LSR_DR     0x01
#define UART_LSR_THRE   0x20
#define UART_LSR_TEMT   0x40

void serial_init();
int serial_putc(const char c);
int serial_getc();

#endif
