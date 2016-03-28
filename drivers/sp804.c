#include "sp804.h"
#include <stdio.h>
#include <io.h>
#include <arch/irq.h>
#include <drivers/gic-v2.h>

#define SP804_BASE          0x1C110000
#define TIMER_LOAD          (SP804_BASE + 0x0)
#define TIMER_CONTROL       (SP804_BASE + 0x8)
#define TIMER_INTCLR        (SP804_BASE + 0x0C)

void sp804_handler(int irq, void *pregs, void *pdata)
{
    writel(0x1, TIMER_INTCLR);
    //printf("sp804!!\n");
}

void sp804_init()
{
    writel(0x10, TIMER_LOAD);
}

void sp804_enable()
{
    sp804_init();
    writel(0xE0, TIMER_CONTROL);
    register_irq_handler(34, &sp804_handler);
    gic_configure_irq(34, IRQ_LEVEL_TRIGGERED);
    gic_enable_irq(34);
}

