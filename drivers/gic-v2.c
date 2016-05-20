#include <stdio.h>
#include <asm/macro.h>

#include <arch/armv7/smp.h>
#include <arch/gic_regs.h>
#include <arch/armv7.h>
#include <arch/irq.h>

#include <core/vm/vcpu.h>
#include <core/scheduler.h>

#include <drivers/gic-v2.h>

#include "irq-chip.h"

#define EOI_ENABLE      1
#define EOI_DISABLE     0

#define HIGHEST_PRIORITY    0

#define VIRQ_MAX_ENTRIES                64
#define VGIC_MAINTENANCE_INTERRUPT_IRQ  25
#define VGIC_MAX_LISTREGISTERS          VGIC_NUM_MAX_SLOTS
#define VGIC_SLOT_NOTFOUND              (0xFFFFFFFF)

static uint32_t gic_find_free_slot(void)
{
    uint32_t slot;
    uint32_t shift = 0;

    slot = GICH_READ(GICH_ELSR(0));
    if (slot == 0 && GICv2.num_lr > 32) {
        /* first 32 slots are occupied, try the later */
        slot = GICH_READ(GICH_ELSR(1));
        shift = 32;
    }

    if (slot) {
        slot &= -(slot);
        slot = (31 - asm_clz(slot));
        slot += shift;
    } else {
        /* 64 slots are fully occupied */
        slot = VGIC_SLOT_NOTFOUND;
    }
    return slot;
}

static void gic_isr_maintenance_irq(int irq, void *pregs, void *pdata)
{
    if (GICH_READ(GICH_MISR) & GICH_MISR_EOI) {
        /* clean up invalid entries from List Registers */
        uint32_t eisr = GICH_READ(GICH_EISR(0));
        uint32_t slot;
        uint32_t pirq, virq;
        vcpuid_t vcpuid = get_current_vcpuid();
        struct virq_table *map;
        struct vcpu *vcpu;
        uint32_t lr;

        vcpu = vcpu_find(vcpuid);
        map = vcpu->map;

        while (eisr) {
            slot = (31 - asm_clz(eisr));
            eisr &= ~(1 << slot);
            lr = GICH_READ(GICH_LR(slot));
            virq = lr & 0x3ff;
            GICH_WRITE(GICH_LR(slot), 0);

            /* deactivate associated pirq at the slot */
            pirq = map[virq].pirq;
            if (pirq != PIRQ_INVALID) {
                gic_deactivate_irq(pirq);
            }
        }

        eisr = GICH_READ(GICH_EISR(1));
        while (eisr) {
            slot = (31 - asm_clz(eisr));
            eisr &= ~(1 << slot);
            lr = GICH_READ(GICH_LR(slot));
            virq = lr & 0x3ff;
            GICH_WRITE(GICH_LR(slot + 32), 0);

            /* deactivate associated pirq at the slot */
            pirq = map[virq].pirq;
            if (pirq != PIRQ_INVALID) {
                gic_deactivate_irq(pirq);
            }
        }
    }
}

static lr_entry_t set_lr_entry(uint8_t hw, enum virq_state state, uint32_t priority, uint32_t physicalid, uint32_t virtualid)
{
    lr_entry_t lr_entry;

    lr_entry.raw = 0;
    lr_entry.entry.hw = hw;
    lr_entry.entry.state = state;
    lr_entry.entry.priority = priority;
    lr_entry.entry.physicalid = physicalid;
    lr_entry.entry.virtualid = virtualid;

    return lr_entry;
}

static hvmm_status_t gic_maintenance_irq_enable()
{
    uint32_t irq = VGIC_MAINTENANCE_INTERRUPT_IRQ;

    register_irq_handler(irq, &gic_isr_maintenance_irq);
    gic_configure_irq(irq, IRQ_LEVEL_TRIGGERED);
    gic_enable_irq(irq);

    return HVMM_STATUS_SUCCESS;
}

void gic_init(void)
{
    int i;
    uint32_t periphbase;
    uint8_t cpuid = smp_processor_id();

    // This code will be moved other parts, not here. */
    /* Get GICv2 base address */
    periphbase = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);

    periphbase = CFG_GIC_BASE_PA;
    GICv2.gicd_base = periphbase + GICD_OFFSET;
    GICv2.gicc_base = periphbase + GICC_OFFSET;
    GICv2.gich_base = periphbase + GICH_OFFSET;

    /*
     * We usually use the name of variables in lower case, but
     * here, using upper case is special case for readability.
     */
    if (cpuid == 0) {
        uint32_t gicd_typer = GICD_READ(GICD_TYPER);
        /* maximum number of irq lines: 32(N+1). */
        GICv2.ITLinesNumber = 32 * ((gicd_typer & GICD_NR_IT_LINES_MASK) + 1);
        GICv2.CPUNumber = 1 + ((gicd_typer & GICD_NR_CPUS_MASK) >> 5);
        printf("Number of IRQs: %d: CPUID[%d]\n", GICv2.ITLinesNumber, cpuid);
        printf("Number of CPU interfaces: %d\n", GICv2.CPUNumber);

        GICD_WRITE(GICD_CTLR, 0x0);
    }

    GICC_WRITE(GICC_CTLR, 0x0);
    /* No Priority Masking: the lowest value as the threshold : 255 */
    // We set 0xff but, real value is 0xf8
    GICC_WRITE(GICC_PMR, 0xff);
    GICC_WRITE(GICC_BPR, 0x0);

    // TODO(casionwoo) : This ISENABLER should be modified like upper
    GICD_WRITE(GICD_ISENABLER(0), 0xffffffff);

    if (cpuid == 0) {
        for (i = 32; i < GICv2.ITLinesNumber; i += 16) {
            GICD_WRITE(GICD_ICFGR(i >> 4), 0x0);
        }

        /* Disable all global interrupts. */
        for (i = 0; i < GICv2.ITLinesNumber; i += 32) {
            GICD_WRITE(GICD_ISENABLER(i >> 5), 0xffffffff);
            uint32_t valid = GICD_READ(GICD_ISENABLER(i >> 5));
            GICD_WRITE(GICD_ICENABLER(i >> 5), valid);
        }

    }

    // We set priority 0xa0 for each but real value is a 0xd0, Why?
    /* Set priority as default for all interrupts */
    for (i = 0; i < GICv2.ITLinesNumber; i += 4) {
        GICD_WRITE(GICD_IPRIORITYR(i >> 2), 0xa0a0a0a0);
    }

    // NOTE: GIC_ITRAGETSR0-7 is read-only on multiprocessor environment.
    for (i = 32; i < GICv2.ITLinesNumber; i += 4) {
#ifdef CONFIG_ARCH_EXYNOS
        GICD_WRITE(GICD_ITARGETSR(i >> 2), 0x10 << 0 | 0x10 << 8 | 0x10 << 16 | 0x10 << 24);
#else
        GICD_WRITE(GICD_ITARGETSR(i >> 2), 1 << 0 | 1 << 8 | 1 << 16 | 1 << 24);
#endif
    }

    GICC_WRITE(GICC_CTLR, GICC_CTL_ENABLE | GICC_CTL_EOI);
    if (cpuid == 0) {
        GICD_WRITE(GICD_CTLR, 0x1);
    }
}

void gich_init()
{
    uint8_t cpuid = smp_processor_id();

    // Initialization GICH
    if (cpuid == 0) {
        GICv2.num_lr = (GICH_READ(GICH_VTR) & GICH_VTR_LISTREGS_MASK) + 1;
        gic_maintenance_irq_enable();
    }

    gich_enable();
}

void gic_configure_irq(uint32_t irq, uint8_t polarity)
{
    if (irq < GICv2.ITLinesNumber) {
        uint32_t reg;
        uint32_t mask;

        reg = GICD_READ(GICD_ICFGR(irq >> 4));

        mask = (reg >> 2 * (irq % 16)) & 0x3;
        if (polarity == IRQ_LEVEL_TRIGGERED) {
            mask &= 0;
            mask |= IRQ_LEVEL_TRIGGERED << 0;
        } else { /* IRQ_EDGE_TRIGGERED */
            mask &= 0;
            mask |= IRQ_EDGE_TRIGGERED << 0;
        }
        reg = reg & ~(0x3 << 2 * (irq % 16));
        reg = reg | (mask << 2 * (irq % 16));

        GICD_WRITE(GICD_ICFGR(irq >> 4), reg);
    } else {
        printf("invalid or spurious irq: %d\n", irq);
    }
}

void gic_set_sgi(const uint32_t target, uint32_t sgi)
{
    if (!(sgi < 16)) {
        return ;
    }

    GICD_WRITE(GICD_SGIR, GICD_SGIR_TARGET_LIST |
               (target << GICD_SGIR_CPU_TARGET_LIST_OFFSET) |
               (sgi & GICD_SGIR_SGI_INT_ID_MASK));
}

uint32_t gic_get_irq_number(void)
{
    return (GICC_READ(GICC_IAR) & GICC_IAR_MASK);
}

void gic_enable_irq(uint32_t irq)
{
    GICD_WRITE(GICD_ISENABLER(irq >> 5), 1UL << (irq & 0x1F));
}

void gic_disable_irq(uint32_t irq)
{
    GICD_WRITE(GICD_ICENABLER(irq >> 5), 1UL << (irq & 0x1F));
}

void gic_completion_irq(uint32_t irq)
{
    GICC_WRITE(GICC_EOIR, irq);
}

void gic_deactivate_irq(uint32_t irq)
{
    GICC_WRITE(GICC_DIR, irq);
}

void gich_disable(void)
{
    uint32_t hcr = GICH_READ(GICH_HCR);
    hcr &= ~(GICH_HCR_EN);
    GICH_WRITE(GICH_HCR, hcr);
}

void gich_enable(void)
{
    uint32_t hcr = GICH_READ(GICH_HCR);
    hcr |= GICH_HCR_EN;
    GICH_WRITE(GICH_HCR, hcr);
}

hvmm_status_t gic_inject_pending_irqs(vcpuid_t vcpuid)
{
    int i;
    struct vcpu *vcpu = vcpu_find(vcpuid);
    lr_entry_t *entries = vcpu->pending_irqs;

    for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
        if (entries[i].raw) {
            uint32_t slot = gic_find_free_slot();

            if (slot == VGIC_SLOT_NOTFOUND) {
                break;
            }

            gic_inject_virq(entries[i], slot);
            entries[i].raw = 0;
        }
    }

    return HVMM_STATUS_SUCCESS;
}

void gic_inject_virq(lr_entry_t lr_entry, uint32_t slot)
{
    GICH_WRITE(GICH_LR(slot), (uint32_t) lr_entry.raw);
}

#include <arch/armv7.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>
bool virq_inject(struct vcpu *vcpu, uint32_t virq, uint32_t pirq, uint8_t hw)
{
    int i;

    if (!hw) {
        pirq = 0;
        pirq |= vcpu->pcpuid;
        pirq |= (EOI_ENABLE << 9);
    }

    lr_entry_t lr_entry = set_lr_entry(hw, VIRQ_STATE_PENDING, GIC_INT_PRIORITY_DEFAULT,
                                       pirq, virq);

    if (vcpu->vcpuid == get_current_vcpuid()) {
        uint32_t slot = gic_find_free_slot();

        if (slot == VGIC_SLOT_NOTFOUND) {
            lr_entry_t *q = vcpu->pending_irqs;

            for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
                if (!q[i].raw) {
                    q[i] = lr_entry;
                    return true;
                }
            }
            return false;
        } else {
            gic_inject_virq(lr_entry, slot);
            return true;
        }

    } else {
        lr_entry_t *q = vcpu->pending_irqs;
        for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
            if (!q[i].raw) {
                q[i] = lr_entry;
                return true;
            }
        }
    }

    return false;
}
