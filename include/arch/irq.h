#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>

typedef void (*irq_handler_t)(int irq, void *regs, void *pdata);
void irq_init();
void register_irq_handler(uint32_t irq, irq_handler_t handler);

#endif
