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

typedef void (*interrupt_handler_t)(int irq, void *regs, void *pdata);

void interrupt_init();
void register_irq_handler(uint32_t irq, interrupt_handler_t handler);
void interrupt_service_routine(int irq, void *current_regs);

#endif
