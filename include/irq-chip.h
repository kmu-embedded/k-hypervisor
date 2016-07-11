#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include <stdbool.h>
#include <core/vm/vcpu.h>

#include "types.h"

struct irq_hw {
    void (* init) (void);
    void (* enable) (uint32_t irq);
    void (* disable) (uint32_t irq);
    uint32_t (* ack) (void);
    void (* eoi) (uint32_t irq);
    void (* dir) (uint32_t irq);
    void (* set_irq_type) (uint32_t irq, uint8_t polarity);
};


struct virq_hw {
    void (* init) (void);
    void (* enable) (void);
    void (* disable) (void);
    bool (* forward_irq) (struct vcpu *vcpu, uint32_t virq, uint32_t pirq, uint8_t hw);
    hvmm_status_t (* forward_pending_irq) (vcpuid_t vcpuid);
};

struct irq_hw *irq_hw;
struct virq_hw *virq_hw;

void set_irqchip_type();

#endif //__IRQ_CHIP_H__
