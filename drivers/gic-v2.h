#ifndef __GIC_H__
#define __GIC_H__

#include <stdint.h>
#include <stdbool.h>
#include <io.h>
#include "../include/types.h"

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
        uint32_t virtualid:10;
        uint32_t physicalid:10;
        uint32_t reserved:3;
        uint32_t priority:5;
        uint32_t state:2;
        uint32_t grp1:1;
        uint32_t hw:1;
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

/* Banked Registers Size */
#define NR_BANKED_IPRIORITYR  8
#define NR_BANKED_ITARGETSR   8
#define NR_BANKED_CPENDSGIR   4
#define NR_BANKED_SPENDSGIR   4

/* We assume that ITLinesNumber has maximum number */
#define MAX_ITLinesNumber             31

#define NR_IGROUPR          (MAX_ITLinesNumber + 1)
#define NR_ISENABLER        (MAX_ITLinesNumber + 1)
#define NR_ICENABLER        (MAX_ITLinesNumber + 1)
#define NR_ISPENDR          (MAX_ITLinesNumber + 1)
#define NR_ICPENDR          (MAX_ITLinesNumber + 1)
#define NR_ISACTIVER        (MAX_ITLinesNumber + 1)
#define NR_ICACTIVER        (MAX_ITLinesNumber + 1)
#define NR_IPRIORITYR       (8 * (MAX_ITLinesNumber + 1))
#define NR_ITARGETSR        (8 * (MAX_ITLinesNumber + 1))
#define NR_ICFGR            (2 * (MAX_ITLinesNumber + 1))
#define NR_NSACR            32

#define NR_VCPUS			8

// Per VM
struct gicd {
    uint32_t ctlr;
    uint32_t typer;
    uint32_t iidr;

    uint32_t igroupr0[NR_VCPUS];
    uint32_t igroupr[NR_IGROUPR];

    uint32_t isenabler0[NR_VCPUS];
    uint32_t isenabler[NR_ISENABLER];

    uint32_t icenabler0[NR_VCPUS];
    uint32_t icenabler[NR_ICENABLER];

    uint32_t ispendr0[NR_VCPUS];
    uint32_t ispendr[NR_ISPENDR];

    uint32_t icpendr0[NR_VCPUS];
    uint32_t icpendr[NR_ICPENDR];

    uint32_t isactiver0[NR_VCPUS];
    uint32_t isactiver[NR_ISACTIVER];

    uint32_t icactiver0[NR_VCPUS];
    uint32_t icactiver[NR_ICACTIVER];

    uint32_t ipriorityr0[NR_VCPUS][NR_BANKED_IPRIORITYR];
    uint32_t ipriorityr[NR_IPRIORITYR];

    uint32_t itargetsr0[NR_VCPUS][NR_BANKED_ITARGETSR];
    uint32_t itargetsr[NR_ITARGETSR];

    uint32_t icfgr0[NR_VCPUS];
    uint32_t icfgr[NR_ICFGR];

    uint32_t nsacr[NR_NSACR];			// unused

    uint32_t sgir;

    uint32_t cpendsgir0[NR_VCPUS][NR_BANKED_CPENDSGIR];
    uint32_t spendsgir0[NR_VCPUS][NR_BANKED_SPENDSGIR];
};

#if 0
// Per Core
struct banked_gicd {
    uint32_t igroupr0[NR_VCPUS];
    uint32_t isenabler0[NR_VCPUS];
    uint32_t icenabler0[NR_VCPUS];
    uint32_t ispendr0[NR_VCPUS];


    uint32_t icpendr0[NR_VCPUS];
    uint32_t isactiver0[NR_VCPUS];
    uint32_t icactiver0[NR_VCPUS];
    uint32_t ipriorityr0[NR_VCPUS][NR_BANKED_IPRIORITYR];
    uint32_t itargetsr0[NR_VCPUS][NR_BANKED_ITARGETSR];
    uint32_t icfgr0[NR_VCPUS];
    uint32_t cpendsgir0[NR_VCPUS][NR_BANKED_CPENDSGIR];
    uint32_t spendsgir0[NR_VCPUS][NR_BANKED_SPENDSGIR];
};
#endif

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

