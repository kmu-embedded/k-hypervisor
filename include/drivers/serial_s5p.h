#ifndef __S5P_H__
#define __S5P_H_

#define S5P_BASE 0x12c20000

#define S5P_ULCON   0x00
#define S5P_UCON    0x04
#define S5P_UFCON   0x08
#define S5P_UMCON   0x0C
#define S5P_UTRSTAT 0x10
#define S5P_UERSTAT 0x14
#define S5P_UFSTAT  0x18
#define S5P_UMSTAT  0x1C
#define S5P_UTXH    0x20
#define S5P_RES1_0  0x21
#define S5P_RES1_1  0x22
#define S5P_RES1_2  0x23
#define S5P_URXH    0x24
#define S5P_RES2_0  0x25
#define S5P_RES2_1  0x26
#define S5P_RES2_2  0x27
#define S5P_UBRDIV  0x28
#define S5P_REST    0x2C

#define ULCON_OVERRUN   1<<0
#define ULCON_PARITY    1<<1
#define ULCON_FRAME     1<<2
#define ULCON_BREAK     1<<3

int serial_init(void);
int serial_putc(const char c);
int serial_getc(void);
#endif
