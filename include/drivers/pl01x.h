#ifndef __PL01X_H__
#define __PL01X_H__
#include <stdint.h>

#define PL01X_BASE       0x1C090000

/*
 * ARM PrimeCell UART's (PL01X)
 * ------------------------------------
 *
 */
/*  Data read or written from the interface. */
#define PL01X_UARTDR                   0x00
/*  Receive status register (Read). */
#define PL01X_UARTRSR                  0x04
/*  Error clear register (Write). */
#define PL01X_UARTECR                  0x04
/*  Flag register (Read only). */
#define PL01X_UARTFR                   0x18
#define PL01X_UARTRSR_OE               0x08
#define PL01X_UARTRSR_BE               0x04
#define PL01X_UARTRSR_PE               0x02
#define PL01X_UARTRSR_FE               0x01

#define PL01X_UARTFR_TXFE              0x80
#define PL01X_UARTFR_RXFF              0x40
#define PL01X_UARTFR_TXFF              0x20
#define PL01X_UARTFR_RXFE              0x10
#define PL01X_UARTFR_BUSY              0x08
#define PL01X_UARTFR_TMSK    (PL01X_UARTFR_TXFF + PL01X_UARTFR_BUSY)

#define PL01X_UARTIBRD                 0x24
#define PL01X_UARTFBRD                 0x28
#define PL01X_UARTLCR_H                0x2C
#define PL01X_UARTCR                   0x30
#define PL01X_UARTIMSC                 0x38
#define PL01X_UARTPERIPH_ID0           0xFE0

#define PL01X_UARTLCR_H_SPS             (1 << 7)
#define PL01X_UARTLCR_H_WLEN_8          (3 << 5)
#define PL01X_UARTLCR_H_WLEN_7          (2 << 5)
#define PL01X_UARTLCR_H_WLEN_6          (1 << 5)
#define PL01X_UARTLCR_H_WLEN_5          (0 << 5)
#define PL01X_UARTLCR_H_FEN             (1 << 4)
#define PL01X_UARTLCR_H_STP2            (1 << 3)
#define PL01X_UARTLCR_H_EPS             (1 << 2)
#define PL01X_UARTLCR_H_PEN             (1 << 1)
#define PL01X_UARTLCR_H_BRK             (1 << 0)

#define PL01X_UARTCR_CTSEN             (1 << 15)
#define PL01X_UARTCR_RTSEN             (1 << 14)
#define PL01X_UARTCR_OUT2              (1 << 13)
#define PL01X_UARTCR_OUT1              (1 << 12)
#define PL01X_UARTCR_RTS               (1 << 11)
#define PL01X_UARTCR_DTR               (1 << 10)
#define PL01X_UARTCR_RXE               (1 << 9)
#define PL01X_UARTCR_TXE               (1 << 8)
#define PL01X_UARTCR_LPE               (1 << 7)
#define PL01X_UARTCR_IIRLP             (1 << 2)
#define PL01X_UARTCR_SIREN             (1 << 1)
#define PL01X_UARTCR_UARTEN            (1 << 0)

#define PL01X_UARTIMSC_OEIM            (1 << 10)
#define PL01X_UARTIMSC_BEIM            (1 << 9)
#define PL01X_UARTIMSC_PEIM            (1 << 8)
#define PL01X_UARTIMSC_FEIM            (1 << 7)
#define PL01X_UARTIMSC_RTIM            (1 << 6)
#define PL01X_UARTIMSC_TXIM            (1 << 5)
#define PL01X_UARTIMSC_RXIM            (1 << 4)
#define PL01X_UARTIMSC_DSRMIM          (1 << 3)
#define PL01X_UARTIMSC_DCDMIM          (1 << 2)
#define PL01X_UARTIMSC_CTSMIM          (1 << 1)
#define PL01X_UARTIMSC_RIMIM           (1 << 0)

/** @brief Return status whether FIFO is empty or not
 *  @param base pl01x's base address.
 *  @return There is not a data in the FIFO then returns 0, otherwise 1.
 */
int pl01x_tst_fifo();

/** @brief Like putc
 *  @param c Character to be written.
 */
void pl01x_putc(const char c);

/** @brief Like getc
 *  @param base pl01x's base address.
 *  @return Read from pl01x's FIFO.
 */
char pl01x_getc();

/** @brief Initializes pl01x.
 *  @param base pl01x's base address.
 *  @param baudrate To set baud rate.
 *  @param input_clock pl01x's clock.
 */
void pl01x_init(uint32_t baudrate, uint32_t input_clock);
#endif
