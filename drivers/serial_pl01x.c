#include <drivers/serial_pl01x.h>

#include "../include/io.h"

int serial_putc(const char c)
{
    /* Wait until there is space in the FIFO */
    while (*((uint32_t *)(PL01X_BASE + 0x18)) & 0x20)
        ;
    /* Send the character */
    writel(c, (uint32_t *) PL01X_BASE + PL01X_UARTDR);

    return c;
}

int serial_tst_fifo()
{
    /* There is not a data in the FIFO */
    if (readl((void *)(PL01X_BASE + PL01X_UARTFR)) & PL01X_UARTFR_RXFE) {
        return 0;
    } else
        /* There is a data in the FIFO */
    {
        return 1;
    }
}

int serial_getc()
{
    char data;
    /* Wait until there is data in the FIFO */
    while (readl((void *)(PL01X_BASE + PL01X_UARTFR)) & PL01X_UARTFR_RXFE)
        ;
    data = *((uint32_t *)(PL01X_BASE + PL01X_UARTDR));
    /* Check for an error flag */
    if (data & 0xFFFFFF00) {
        /* Clear the error */
        writel(0xFFFFFFFF, (uint32_t *) PL01X_BASE + PL01X_UARTECR);
        return -1;
    }
    return data;
}

void serial_subinit(uint32_t base, uint32_t baudrate, uint32_t input_clock)
{
    uint32_t divider;
    uint32_t temp;
    uint32_t remainder;
    uint32_t fraction;

    /* First, disable everything */
    writel(0x0, (void *)(base + PL01X_UARTCR));

    /*
     * Set baud rate
     *
     * IBRD = UART_CLK / (16 * BAUD_RATE)
     * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE)))
     *    / (16 * BAUD_RATE))
     */
    temp = 16 * baudrate;
    divider = input_clock / temp;
    remainder = input_clock % temp;
    temp = (8 * remainder) / baudrate;
    fraction = (temp >> 1) + (temp & 1);

    writel(divider, (void *)(base + PL01X_UARTIBRD));
    writel(fraction, (void *)(base + PL01X_UARTFBRD));

    /* Set the UART to be 8 bits, 1 stop bit,
     * no parity, fifo enabled
     */
    writel((PL01X_UARTLCR_H_WLEN_8 | PL01X_UARTLCR_H_FEN),
           (void *)(base + PL01X_UARTLCR_H));

    /* Finally, enable the UART */
    writel((PL01X_UARTCR_UARTEN | PL01X_UARTCR_TXE | PL01X_UARTCR_RXE),
           (void *)(base + PL01X_UARTCR));
}

void serial_init(uint32_t baudrate, uint32_t input_clock)
{
    uint32_t divider;
    uint32_t temp;
    uint32_t remainder;
    uint32_t fraction;

    /* First, disable everything */
    writel(0x0, (void *)(PL01X_BASE + PL01X_UARTCR));

    /*
     * Set baud rate
     *
     * IBRD = UART_CLK / (16 * BAUD_RATE)
     * FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE)))
     *    / (16 * BAUD_RATE))
     */
    temp = 16 * baudrate;
    divider = input_clock / temp;
    remainder = input_clock % temp;
    temp = (8 * remainder) / baudrate;
    fraction = (temp >> 1) + (temp & 1);

    writel(divider, (void *)(PL01X_BASE + PL01X_UARTIBRD));
    writel(fraction, (void *)(PL01X_BASE + PL01X_UARTFBRD));

    /* Set the UART to be 8 bits, 1 stop bit,
     * no parity, fifo enabled
     */
    writel((PL01X_UARTLCR_H_WLEN_8 | PL01X_UARTLCR_H_FEN),
           (void *)(PL01X_BASE + PL01X_UARTLCR_H));

    /* Finally, enable the UART */
    writel((PL01X_UARTCR_UARTEN | PL01X_UARTCR_TXE | PL01X_UARTCR_RXE),
           (void *)(PL01X_BASE + PL01X_UARTCR));

    serial_subinit(0x1C0A0000, baudrate, input_clock);
    serial_subinit(0x1C0B0000, baudrate, input_clock);
    serial_subinit(0x1C0C0000, baudrate, input_clock);
}
