#ifndef __SH_H__
#define __SH_H__

#define SCIF_BASE   0xE6E60000

#define SCSCR_TE_ENABLE     0x0020
#define SCSCR_RE_ENABLE     0x0010
#define SCSCR_CKE1_ENABLE   0x0002

#define SCFSR_ER    0x0080
#define SCFSR_TEND  0x0040
#define SCFSR_BRK   0x0010
#define SCFSR_FER   0x0008
#define SCFSR_PER   0x0004
#define SCFSR_RDF   0x0002
#define SCFSR_DR    0x0001

#define SCLSR_ORER	0x0000

#define SCFSR_ER_ENABLE     0x0080
#define SCFSR_TEND_ENABLE   0x0040
#define SCFSR_TDFE_ENABLE   0x0020
#define SCFSR_BRK_ENABLE    0x0010

#define SCFCR_RFRST_ENABLE 0x0002
#define SCFCR_TFRST_ENABLE 0x0004

#define SCSCR   0x08
#define SCSMR   0x00
#define SCFCR   0x18
#define SCFSR   0x10
#define SCFTDR  0x0c
#define SCFRDR  0x14
#define SCLSR 	0x24

int serial_init(void);
int serial_putc(const char c);
int serial_getc(void);
#endif
