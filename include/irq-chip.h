#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

struct irq_chip {
    void (* init) (void);                       // gic_init
    void (* enable) (uint32_t irq);         // gic_enable_irq
    void (* disable) (uint32_t irq);        // gic_disable_irq
    uint32_t (* ack) (void);                // gic_get_irq_number
    void (* eoi) (uint32_t irq);                    // gic_completion_irq
    void (* dir) (uint32_t irq);                    // gic_deactivate_irq
    void (* set_irq_type) (uint32_t irq, uint8_t polarity);                // gic_configure_irq
    //bool (* forward_irq) (vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw);
};

struct virq_chip {
    void (* init) (void);
    void (* enable) (void);
    void (* disable) (void);
    hvmm_status_t (* forward_pending_irq) (vcpuid_t vcpuid);
    bool (* forward_irq) (vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw);
};

struct irq_chip *irq_hw;
struct virq_chip *virq_hw;

void set_irqchip_type();

#endif //__IRQ_CHIP_H__
