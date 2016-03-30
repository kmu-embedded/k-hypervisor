#ifndef __SP804_H__
#define __SP804_H__

#define TIMER1_IRQ					34
#define TIMER2_IRQ					35

#define TIMER1_BASE          		0x1C110000
#define TIMER2_BASE          		0x1C120000

#define TIMER1_LOAD(x)          	(x + 0x00)		// RW
#define TIMER1_VALUE(x)				(x + 0x04)		// RO
#define TIMER1_CONTROL(x)       	(x + 0x08)		// RW
#define TIMER1_INTCLR(x)			(x + 0x0C)		// WO
#define TIMER1_RIS(x)				(x + 0x10)		// RO
#define TIMER1_MIS(x)				(x + 0x14)		// RO
#define TIMER1_BGLOAD(x)			(x + 0x18)		// RW

#define TIMER2_LOAD(x)          	(x + 0x20)		// RW
#define TIMER2_VALUE(x)				(x + 0x24)		// RO
#define TIMER2_CONTROL(x)       	(x + 0x28)		// RW
#define TIMER2_INTCLR(x)			(x + 0x2C)		// WO
#define TIMER2_RIS(x)				(x + 0x30)		// RO
#define TIMER2_MIS(x)				(x + 0x34)		// RO
#define TIMER2_BGLOAD(x)			(x + 0x38)		// RW

#define TIMER_ITCR(x)				(x + 0xF00)		//RW
#define TIMER_ITOP(x)				(x + 0xF04)		//WO

#define TIMER_RERIPHID0(x)			(x + 0xFE0)		//RO
#define TIMER_RERIPHID1(x)			(x + 0xFE4)		//RO
#define TIMER_RERIPHID2(x)			(x + 0xFE8)		//RO
#define TIMER_RERIPHID3(x)			(x + 0xFEC)		//RO

#define TIMER_RPCELL0(x)			(x + 0xFF0)		//RO
#define TIMER_RPCELL1(x)			(x + 0xFF4)		//RO
#define TIMER_RPCELL2(x)			(x + 0xFF8)		//RO
#define TIMER_RPCELL3(x)			(x + 0xFFC)		//RO

void sp804_init();
void sp804_enable();

#endif
