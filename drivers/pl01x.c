#include <arch/arm/rtsm-config.h>


#define UART0_BASE       0x1C090000
void uart_print(const char *str)
{
    volatile char *pUART = (char *) UART0_BASE;
    while (*str)
        *pUART = *str++;
}

#if 0
void putchar(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((unsigned int *)(UART0_BASE + 0x18)) & 0x20)
        ;
    /* Send the character */
    volatile char *pUART = (char *) UART0_BASE;
    *pUART = c;
}
#endif

void uart_putc(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((unsigned int *)(UART0_BASE + 0x18)) & 0x20)
        ;
    /* Send the character */
    volatile char *pUART = (char *) UART0_BASE;
    *pUART = c;
}

void uart_print_hex32(unsigned int v)
{
    unsigned int mask8 = 0xF;
    unsigned int c;
    int i;
    uart_print("0x");
    for (i = 7; i >= 0; i--) {
        c = ((v >> (i * 4)) & mask8);
        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        uart_putc((char) c);
    }
}

// warning: right shift count >= width of type [enabled by default]
// void uart_print_hex64(unsigned long long v)
void uart_print_hex64(unsigned long v)
{
    uart_print_hex32(v);
    uart_print_hex32((unsigned int)(v & 0xFFFFFFFF));
}
