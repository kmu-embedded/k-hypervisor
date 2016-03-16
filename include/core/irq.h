#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "hvmm_types.h"

#define HOST_IRQ 0
#define GUEST_IRQ 1
#define GUEST_IRQ_ENABLE 1
#define GUEST_IRQ_DISABLE 0

#define INJECT_SW 0
#define INJECT_HW 1

typedef void (*irq_handler_t)(int irq, void *regs, void *pdata);

void irq_init();
void register_irq_handler(uint32_t irq, irq_handler_t handler);

#endif
