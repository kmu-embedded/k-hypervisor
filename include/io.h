#ifndef __IO_H__
#define __IO_H__

#define __writew(v, a)  (*(volatile unsigned short *)(a) = (v))
#define __writeb(v, a)  (*(volatile uint8_t *)(a) = (v))
#define __readw(a)      (*(volatile unsigned short *)(a))
#define __readb(a)      (*(volatile uint8_t *)(a))
#define __writel(v, a)  (*(volatile uint32_t *)(a) = (v))
#define __readl(a)      (*(volatile uint32_t *)(a))

#define writel(v, a)    ({ uint32_t vl = v; __writel(vl, a); })
#define readl(a)        ({ uint32_t vl = __readl(a); vl; })
#define writew(v, a)    ({ uint16_t vl = v; __writew(vl, a); })
#define readw(a)        ({ uint16_t vl = __readw(a); vl; })
#define writeb(v, a)    ({ uint8_t vl = v; __writeb(vl, a); })
#define readb(a)        ({ uint8_t vl = __readb(a); vl; })

#endif
