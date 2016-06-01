#include <drivers/serial_sh.h>
#include <io.h>
#include <arch/armv7/arm_inline.h>

int serial_init(void)
{
    writew((SCSCR_TE_ENABLE | SCSCR_RE_ENABLE | SCSCR_CKE1_ENABLE), SCIF_BASE + SCSCR);
    writew(0, SCIF_BASE + SCSMR);
    writew(SCFCR_RFRST_ENABLE | SCFCR_TFRST_ENABLE, SCIF_BASE + SCFCR);

    readw(SCIF_BASE + SCFCR);

    writew(0, SCIF_BASE + SCFCR);

    return 0;
}

static void handle_error(void)
{
    writew((SCFSR_ER_ENABLE | SCFSR_TEND_ENABLE | SCFSR_TDFE_ENABLE | SCFSR_BRK_ENABLE), SCIF_BASE + SCFSR);
    writew(0x00, SCIF_BASE + SCLSR);
}

static void serial_raw_putc(const char c)
{
    while (1) {
        if (readw(SCIF_BASE + SCFSR) & SCFSR_TEND) {
            break;
        }
    }

    writeb(c, SCIF_BASE + SCFTDR);
    writew(readw(SCIF_BASE + SCFSR) & ~SCFSR_TEND, SCIF_BASE + SCFSR);
}

int serial_putc(const char c)
{
    if (c == '\n') {
        serial_raw_putc('\r');
    }
    serial_raw_putc(c);
    return c;
}


static int serial_getc_check(void)
{
    unsigned short status;

    status = readw(SCIF_BASE + SCFSR);

    if (status & (SCFSR_PER | SCFSR_FER | SCFSR_ER | SCFSR_BRK)) {
        handle_error();
    }
    if (readw(SCIF_BASE + SCLSR) & SCLSR_ORER) {
        handle_error();
    }
    return status & (SCFSR_DR | SCFSR_RDF);
}

int serial_getc(void)
{
    unsigned short status;
    char ch;

    while (!serial_getc_check()) {
        ;
    }

    ch = readb(SCIF_BASE + SCFRDR);
    status = readw(SCIF_BASE + SCFSR);

    writew((SCFSR_ER_ENABLE | SCFSR_TEND_ENABLE | SCFSR_TDFE_ENABLE | SCFSR_BRK_ENABLE), SCIF_BASE + SCFSR);

    if (status & (SCFSR_PER | SCFSR_FER | SCFSR_ER | SCFSR_BRK)) {
        handle_error();
    }
    if (readw(SCIF_BASE + SCLSR) & SCLSR_ORER) {
        handle_error();
    }
    return ch;
}

