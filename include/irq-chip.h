#ifndef __IRQ_CHIP_H__
#define __IRQ_CHIP_H__

#include <stdint.h>
#include <hvmm_types.h>

struct irq_chip_t {
    void (* init) (void);                       // gic_init
    void (* enable) (uint32_t irq);         // gic_enable_irq
    void (* disable) (uint32_t irq);        // gic_disable_irq
    uint32_t (* ack) (void);                // gic_get_irq_number
    void (* eoi) (uint32_t irq);                    // gic_completion_irq
    void (* dir) (uint32_t irq);                    // gic_deactivate_irq
    void (* set_irq_type) (uint32_t irq, uint8_t polarity);                // gic_configure_irq
    //void (* irq_forward) (vcpuid_t id, uint32_t virq, uint32_t pirq, bool type);              // gic_deactivate_irq
    hvmm_status_t (* forward) (vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw);
};

struct irq_chip_t *irq_chip;
void register_irq_chip();
#endif //__IRQ_CHIP_H__
