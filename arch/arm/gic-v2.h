#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>
#include <core/interrupt.h>

#define GIC_NUM_MAX_IRQS    1024
#define GIC_INT_PRIORITY_DEFAULT        0xa0

struct gic_hw_info {
    uint64_t base;        /**< GIC base address */
    uint32_t gich;              /**< Virtual interface control (common)*/
    uint32_t nr_irqs;           /**< The Maximum number of interrupts */
    uint32_t nr_cpus;           /**< The number of implemented CPU interfaces */
    uint32_t initialized;       /**< Check whether initializing GIC. */
};

#define GIC_SIGNATURE_INITIALIZED   0x5108EAD7

#define IRQ_LEVEL_TRIGGERED     0
#define IRQ_EDGE_TRIGGERED      1
#define IRQ_N_N_MODEL           0
#define IRQ_1_N_MODEL           1   /* every SPIs must be set 1:N model */

#define gic_cpumask_current()    (1u << smp_processor_id())

enum gic_sgi {
    GIC_SGI_SLOT_CHECK = 1,
};

void gic_enable_irq(uint32_t irq);
void gic_disable_irq(uint32_t irq);
void gic_init(void);
void gic_deactivate_irq(uint32_t irq);
void gic_completion_irq(uint32_t irq);

uint32_t *gic_vgic_baseaddr(void);

void gic_configure_irq(uint32_t irq, uint8_t polarity);

uint32_t gic_get_irq_number(void);

void gic_set_sgi(const uint32_t target, uint32_t sgi);

#endif
