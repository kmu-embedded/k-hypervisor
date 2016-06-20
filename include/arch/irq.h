#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "armv7.h"

// CBAR has base address of GIC interrupt controller.
static inline uint64_t get_periphbase(void)
{
    uint64_t periphbase = 0UL;
    unsigned long cbar = read_cp32(CBAR);
    uint64_t upper_periphbase = cbar & 0xFF;

    if (upper_periphbase != 0x0) {
        periphbase |= upper_periphbase << 32;
        cbar &= ~(0xFF);
    }
    periphbase |= cbar;

    return periphbase;
}

typedef void (*irq_handler_t)(int irq, void *regs, void *pdata);
void irq_init();
void register_irq_handler(uint32_t irq, irq_handler_t handler, uint8_t polarity);

#endif
