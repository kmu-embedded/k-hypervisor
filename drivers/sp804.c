#include <stdio.h>
#include <io.h>
#include <arch/irq.h>
#include <drivers/gic-v2.h>
#include <drivers/sp804.h>

static uint64_t tickcount = 0;

void sp804_handler(int irq, void *pregs, void *pdata)
{
#if 0
    printf("SP804 IRQ[%d]: T1 %x, T2: %x\n", irq, readl(TIMER1_VALUE(TIMER0_BASE)),
           readl(TIMER2_VALUE(TIMER0_BASE)));
#endif
    tickcount++;
    //printf("tickcount: %d\n", tickcount);

    writel(0x1, TIMER1_INTCLR(TIMER0_BASE));
}

void sp804_init()
{
    writel(0x10, TIMER1_LOAD(TIMER0_BASE));
}

void sp804_enable()
{
    sp804_init();
    writel(0xE2, TIMER1_CONTROL(TIMER0_BASE));
    register_irq_handler(34, &sp804_handler);
    gic_configure_irq(34, IRQ_LEVEL_TRIGGERED);
    gic_enable_irq(34);
}
