#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>
#include <stdbool.h>
#include <io.h>
#include "../include/types.h"
//#include <core/interrupt.h>

#define GIC_NUM_MAX_IRQS                1024
#define GIC_INT_PRIORITY_DEFAULT        0xa0
#define VGIC_NUM_MAX_SLOTS              64

enum virq_state {
    VIRQ_STATE_INACTIVE = 0x00,
    VIRQ_STATE_PENDING = 0x01,
    VIRQ_STATE_ACTIVE = 0x02,
    VIRQ_STATE_PENDING_ACTIVE = 0x03,
};

struct GICv2_HW {
    uint32_t gicd;
    uint32_t gicc;
    uint32_t gich;
    uint32_t ITLinesNumber;
    uint32_t CPUNumber;
    uint32_t num_lr;
    uint32_t valid_lr_mask;
} GICv2;

#define IRQ_LEVEL_TRIGGERED     0
#define IRQ_EDGE_TRIGGERED      1

enum gic_sgi {
    GIC_SGI_SLOT_CHECK = 1,
};

void gic_init(void);
void gic_enable_irq(uint32_t irq);
void gic_disable_irq(uint32_t irq);
uint32_t gic_get_irq_number(void);
void gic_completion_irq(uint32_t irq);
void gic_deactivate_irq(uint32_t irq);
void gic_configure_irq(uint32_t irq, uint8_t polarity);

void gic_set_sgi(const uint32_t target, uint32_t sgi);

/* GICH related functions */
void gich_init(void);
void gich_enable(void);
void gich_disable(void);
hvmm_status_t gic_inject_pending_irqs(vcpuid_t vcpuid);
bool virq_inject(vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw);

void update_lr(uint32_t offset, uint32_t value);
uint32_t gic_inject_virq(uint32_t hw, enum virq_state state, uint32_t priority, uint32_t physicalid, uint32_t virtualid, uint32_t slot);

#include <io.h>
#define GICD_READ(offset)           __readl(GICv2.gicd + offset)
#define GICD_WRITE(offset, value)   __writel(value, GICv2.gicd + offset)

#define GICC_READ(offset)           __readl(GICv2.gicc + offset)
#define GICC_WRITE(offset, value)   __writel(value, GICv2.gicc + offset)

#define GICH_READ(offset)           __readl(GICv2.gich + offset)
#define GICH_WRITE(offset, value)   __writel(value, GICv2.gich + offset)

#endif

