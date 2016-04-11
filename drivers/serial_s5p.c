#include <drivers/serial_s5p.h>
#include <io.h>

int serial_init(void)
{
    writel(0x117, S5P_BASE + S5P_UFCON);
    writel(0, S5P_BASE + S5P_UMCON);
    writel(0x3, S5P_BASE + S5P_ULCON);
    writel(0x3C5, S5P_BASE + S5P_UCON);

    return 0;
}

static int serial_err_check(void)
{
    unsigned int error_flag;

    error_flag = readl(S5P_BASE + S5P_UERSTAT);

    if (error_flag & ULCON_OVERRUN) {
        return 0;
    } else if (error_flag & ULCON_PARITY) {
        return 0;
    } else if (error_flag & ULCON_FRAME) {
        return 0;
    } else if (error_flag & ULCON_BREAK) {
        return 0;
    } else {
        return 1;
    }
}

int serial_getc(void)
{
    while (!(readl(S5P_BASE + S5P_UTRSTAT) & 0x1)) {
        if (serial_err_check()) {
            return 0;
        }
    }

    return readb(S5P_BASE + S5P_URXH) & 0xFF;
}

int serial_putc(const char c)
{
    while (!(readl(S5P_BASE + S5P_UTRSTAT) & 0x2)) {
        if (serial_err_check()) {
            return -1;
        }
    }
    writeb(c, S5P_BASE + S5P_UTXH);

    if (c == '\n') {
        serial_putc('\r');
    }
    return c;
}

