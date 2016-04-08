#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>
#include <stdbool.h>
#include <io.h>
#include "../include/types.h"

#ifdef SERIAL_PL01X
#include <../../platform/rtsm/config.h>
#elif SERIAL_SH
#include <../../platform/lager/config.h>
#elif SERIAL_S5P
#include <../../platform/odroidxu/config.h>
#endif

#define HW_IRQ      1
#define SW_IRQ      0

#define GIC_NUM_MAX_IRQS                1024
#define GIC_INT_PRIORITY_DEFAULT        0xa0
#define VGIC_NUM_MAX_SLOTS              64

enum virq_state {
    VIRQ_STATE_INACTIVE = 0x00,
    VIRQ_STATE_PENDING = 0x01,
    VIRQ_STATE_ACTIVE = 0x02,
    VIRQ_STATE_PENDING_ACTIVE = 0x03,
};

union LR {
    uint32_t raw;
    struct {
        uint32_t virtualid: 10;
        uint32_t physicalid: 10;
        uint32_t reserved: 3;
        uint32_t priority: 5;
        uint32_t state: 2;
        uint32_t grp1: 1;
        uint32_t hw: 1;
    } entry __attribute__((__packed__));
};

typedef union LR lr_entry_t;

struct GICv2_HW {
    uint32_t gicd_base;
    uint32_t gicc_base;
    uint32_t gich_base;
    uint32_t ITLinesNumber;
    uint32_t CPUNumber;
    uint32_t num_lr;
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
void gic_inject_virq(lr_entry_t lr_entry, uint32_t slot);

#include <io.h>
#define GICD_READ(offset)           __readl(GICv2.gicd_base + offset)
#define GICD_WRITE(offset, value)   __writel(value, GICv2.gicd_base + offset)

#define GICC_READ(offset)           __readl(GICv2.gicc_base + offset)
#define GICC_WRITE(offset, value)   __writel(value, GICv2.gicc_base + offset)

#define GICH_READ(offset)           __readl(GICv2.gich_base + offset)
#define GICH_WRITE(offset, value)   __writel(value, GICv2.gich_base + offset)

#endif

