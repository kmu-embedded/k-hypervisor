#ifndef __IO_H__
#define __IO_H__

#define putw(v, a)   (*(volatile unsigned short *)(a) = (v))
#define putb(v, a)   (*(volatile unsigned char *)(a) = (v))
#define getw(a)      (*(volatile unsigned short *)(a))
#define getb(a)      (*(volatile unsigned char *)(a))
#define putl(v, a)   (*(volatile unsigned int *)(a) = (v))
#define getl(a)      (*(volatile unsigned int *)(a))
#define writel(v, a) ({ uint32_t vl = v; putl(vl, a); })
#define readl(a)     ({ uint32_t vl = getl(a); vl; })
#define writew(v, a) ({ uint16_t vl = v; putw(vl, a); })
#define readw(a)     ({ uint16_t vl = getw(a); vl; })
#define writeb(v, a) ({ uint8_t vl = v; putb(vl, a); })
#define readb(a)     ({ uint8_t vl = getb(a); vl; })

#if 0
void inline write64(uint64_t value, uint64_t addr)
{
    uint32_t upper, lower;
    upper = (value & 0xFFFFFFFF00000000UL) >> 32;
    lower = (value & 0x00000000FFFFFFFFUL);
}
#endif

#endif
