#include "gic-v2.h"
#include <arch/gic_regs.h>  // Define OFFSETS
#include <debug_print.h>
#include <arch/armv7.h>     // get_periphbase()
#include <io.h>
#include <asm/asm.h>        // SECTION(x)
#include <core/vm/virq.h>
#include <hvmm_trace.h>
#include <core/vm/vcpu.h>
#include <core/scheduler.h>

#define VIRQ_MAX_ENTRIES                128
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

static uint32_t gic_inject_virq_hw(uint32_t virq, enum virq_state state,
        uint32_t priority, uint32_t pirq)
{
    uint32_t slot = VGIC_SLOT_NOTFOUND;
    HVMM_TRACE_ENTER();
    slot = gic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
        slot = gic_inject_virq(virq, slot, state, priority, 1, pirq, 0);
    }
    HVMM_TRACE_EXIT();
    return slot;
}

static uint32_t gic_inject_virq_sw(uint32_t virq, enum virq_state state,
        uint32_t priority, uint32_t cpuid, uint8_t maintenance)
{
    uint32_t slot = VGIC_SLOT_NOTFOUND;
    HVMM_TRACE_ENTER();
    slot = gic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
        slot = gic_inject_virq(virq, slot, state,
                priority, 0, cpuid, maintenance);
    }

    HVMM_TRACE_EXIT();
    return slot;
}

// Test mutex
static DEFINE_MUTEX(VIRQ_MUTEX);
hvmm_status_t virq_inject(vcpuid_t vcpuid, uint32_t virq, uint32_t pirq, uint8_t hw)
{
    int i;
    hvmm_status_t result = HVMM_STATUS_BUSY;
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct virq_entry *q = vcpu->virq.pending_irqs;

    if (vcpuid == get_current_vcpuid()) {
        uint32_t slot;
        lock_mutex(&VIRQ_MUTEX);
        if (hw) {
            slot = gic_inject_virq_hw(virq, VIRQ_STATE_PENDING, GIC_INT_PRIORITY_DEFAULT, pirq);
        } else {
            slot = gic_inject_virq_sw(virq, VIRQ_STATE_PENDING, 0, vcpuid, 1);
        }
        unlock_mutex(&VIRQ_MUTEX);

        if (slot == VGIC_SLOT_NOTFOUND) {
            return result;
        }

        result = HVMM_STATUS_SUCCESS;
    }
    else {
        for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
            if (q[i].valid == 0) {
                q[i].pirq = pirq;
                q[i].virq = virq;
                q[i].hw = hw;
                q[i].valid = 1;
                result = HVMM_STATUS_SUCCESS;
                break;
            }
        }
        debug_print("virq: queueing virq %d pirq %d to vcpuid %d %s\n",
                virq, pirq, vcpuid, result == HVMM_STATUS_SUCCESS ? "done" : "failed");
        if ((result == HVMM_STATUS_SUCCESS) && (virq < 16) ) {
            gic_set_sgi(1 << vcpuid, GIC_SGI_SLOT_CHECK);
        }
    }
    return result;
}

hvmm_status_t gic_inject_pending_irqs(vcpuid_t vcpuid)
{
    /* Actual injection of queued VIRQs takes place here */
    int i;
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct virq_entry *entries = vcpu->virq.pending_irqs;

    for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
        if (entries[i].valid) {
            uint32_t slot;
            if (entries[i].hw) {
                slot = gic_inject_virq_hw(entries[i].virq, VIRQ_STATE_PENDING,
                        GIC_INT_PRIORITY_DEFAULT, entries[i].pirq);
            } else {
                slot = gic_inject_virq_sw(entries[i].virq, VIRQ_STATE_PENDING,
                        GIC_INT_PRIORITY_DEFAULT, smp_processor_id(), 1);
            }
            if (slot == VGIC_SLOT_NOTFOUND) {
                break;
            }

            /* Forget */
            entries[i].valid = 0;
        }
    }

    return HVMM_STATUS_SUCCESS;
}

static uint32_t gic_is_free_slot(uint32_t slot)
{
    uint32_t free_slot = VGIC_SLOT_NOTFOUND;
    if (slot < 32) {
        if (GICH_READ(GICH_ELSR(0)) & (1 << slot)) {
            free_slot = slot;
        }
    } else {
        if (GICH_READ(GICH_ELSR(1)) & (1 << (slot - 32))) {
            free_slot = slot;
        }
    }
    if (free_slot != slot) {
        free_slot = gic_find_free_slot();
    }
    return free_slot;
}

static void gic_isr_maintenance_irq(int irq, void *pregs, void *pdata)
{
    HVMM_TRACE_ENTER();
    if (GICH_READ(GICH_MISR) & GICH_MISR_EOI) {
        /* clean up invalid entries from List Registers */
        uint32_t eisr = GICH_READ(GICH_EISR(0));
        uint32_t slot;
        uint32_t pirq, virq;
        vcpuid_t vcpuid = get_current_vcpuid();
        struct virqmap_entry *map;
        struct vcpu *vcpu;
        uint32_t lr;

        vcpu = vcpu_find(vcpuid);
        map = vcpu->virq.guest_virqmap->map;

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

    HVMM_TRACE_EXIT();
}

static hvmm_status_t gic_maintenance_irq_enable(uint8_t enable)
{
    uint32_t irq = VGIC_MAINTENANCE_INTERRUPT_IRQ;
    HVMM_TRACE_ENTER();
    if (enable) {
        register_irq_handler(irq, &gic_isr_maintenance_irq);
        gic_configure_irq(irq, IRQ_LEVEL_TRIGGERED);
        gic_enable_irq(irq);
    } else {
        register_irq_handler(irq, 0);
        gic_disable_irq(irq);
    }
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}


static uint64_t gic_valid_lr_mask(uint32_t num_lr)
{
    uint64_t mask_valid_lr = 0xFFFFFFFFFFFFFFFFULL;
    if (num_lr < VGIC_MAX_LISTREGISTERS) {
        mask_valid_lr >>= num_lr;
        mask_valid_lr <<= num_lr;
        mask_valid_lr = ~mask_valid_lr;
    }
    return mask_valid_lr;
}

void SECTION(".init") gic_init(void)
{
    int i;
    uint32_t periphbase;

    // This code will be moved other parts, not here. */
    /* Get GICv2 base address */
    periphbase = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);
    if (periphbase == 0x0) {
        printf("Warning: Curretn architecture has no value in CBAR\n    \
                The architecture do not follow design philosophy from ARM recommendation\n");
        // TODO(casionwoo): vaule of base address will be read from DTB or configuration file.
        // Currently, we set the base address of gic to 0x2C000000, it is for RTSM.
        periphbase = 0x2C000000;
    }
    GICv2.gicd = periphbase + GICD_OFFSET;
    GICv2.gicc = periphbase + GICC_OFFSET;
    GICv2.gich = periphbase + GICH_OFFSET;

    /*
     * We usually use the name of variables in lower case, but
     * here, using upper case is special case for readability.
     */
    uint32_t gicd_typer = GICD_READ(GICD_TYPER_OFFSET);
    /* maximum number of irq lines: 32(N+1). */
    GICv2.ITLinesNumber = 32 * ((gicd_typer & GICD_NR_IT_LINES_MASK) + 1);
    GICv2.CPUNumber = 1 + (gicd_typer & GICD_NR_CPUS_MASK);
    printf("Number of IRQs: %d\n", GICv2.ITLinesNumber);
    printf("Number of CPU interfaces: %d\n", GICv2.CPUNumber);

    GICC_WRITE(GICC_CTLR_OFFSET, GICC_CTL_ENABLE | GICC_CTL_EOI);
    GICD_WRITE(GICD_CTLR_OFFSET, 0x1);

    /* No Priority Masking: the lowest value as the threshold : 255 */
    // We set 0xff but, real value is 0xf8
    GICC_WRITE(GICC_PMR_OFFSET, 0xff);

    // Set interrupt configuration do not work.
    for (i = 32; i < GICv2.ITLinesNumber; i += 16) {
        GICD_WRITE(GICD_ICFGR(i >> 4), 0x0);
    }

    /* Disable all global interrupts. */
    for (i = 0; i < GICv2.ITLinesNumber; i += 32) {
        GICD_WRITE(GICD_ISENABLER(i >> 5), 0xffffffff);
        uint32_t valid = GICD_READ(GICD_ISENABLER(i >> 5));
        GICD_WRITE(GICD_ICENABLER(i >> 5), valid);
    }

    // We set priority 0xa0 for each but real value is a 0xd0, Why?
    /* Set priority as default for all interrupts */
    for (i = 0; i < GICv2.ITLinesNumber; i += 4) {
        GICD_WRITE(GICD_IPRIORITYR(i >> 2), 0xa0a0a0a0);
    }

#ifndef __CONFIG_SMP__
    // NOTE: GIC_ITRAGETSR is read-only on multiprocessor environment.
    for (i = 32; i < GICv2.ITLinesNumber; i += 4) {
        GICD_WRITE(GICD_ITARGETSR(i >> 2), 1 << 0 | 1 << 8 | 1 << 16 | 1 << 24);
    }
#endif

    // Initialization GICH
    GICv2.num_lr = (GICH_READ(GICH_VTR) & GICH_VTR_LISTREGS_MASK) + 1;
    GICv2.valid_lr_mask = gic_valid_lr_mask(GICv2.num_lr);
    gic_maintenance_irq_enable(1);
    gich_enable(1);

    // TODO(casionwoo): Set SGI and PPIs

}

void gic_configure_irq(uint32_t irq, uint8_t polarity)
{
    if (irq < GICv2.ITLinesNumber) {
        uint32_t reg;
        uint32_t mask;

        printf("IRQ: %d: GICD_ICFGR(%d) %x\n", irq, irq >> 4, GICD_READ(GICD_ICFGR(irq >> 4)));
        printf("%x\n", GICv2.gicd);
        reg = GICD_READ(GICD_ICFGR(irq >> 4));
        printf("IRQ: %d: GICD_ICFGR(%d) %x\n", irq, irq >> 4, reg);

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
        printf("IRQ: %d: GICD_ICFGR(%d) %x\n", irq, irq >> 4, GICD_READ(GICD_ICFGR(irq >> 4)));
    } else {
        printf("invalid or spurious irq: %d\n", irq);
    }
}

void gic_set_sgi(const uint32_t target, uint32_t sgi)
{
    if (!(sgi < 16)) {
        return ;
    }
    GICD_WRITE(GICD_SGIR(0), GICD_SGIR_TARGET_LIST |
               (target << GICD_SGIR_CPU_TARGET_LIST_OFFSET) |
               (sgi & GICD_SGIR_SGI_INT_ID_MASK));

}

/* API functions */
uint32_t gic_get_irq_number(void)
{
    return (GICC_READ(GICC_IAR_OFFSET) & GICC_IAR_MASK);
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
    GICC_WRITE(GICC_EOIR_OFFSET, irq);
}

void gic_deactivate_irq(uint32_t irq)
{
    GICC_WRITE(GICC_DIR_OFFSET, irq);
}

void gich_enable(uint8_t enable)
{
    if (enable) {
        uint32_t hcr = GICH_READ(GICH_HCR);
        hcr |= GICH_HCR_EN;
        GICH_WRITE(GICH_HCR, hcr);
    } else {
        uint32_t hcr = GICH_READ(GICH_HCR);
        hcr &= ~(GICH_HCR_EN);
        GICH_WRITE(GICH_HCR, hcr);
    }
}

uint32_t gic_inject_virq(
        uint32_t virq, uint32_t slot, enum virq_state state, uint32_t priority,
        uint8_t hw, uint32_t physrc, uint8_t maintenance)
{
    uint32_t physicalid;
    uint32_t lr_desc;
    HVMM_TRACE_ENTER();
    physicalid = (hw ? physrc : (maintenance << 9) | \
            (physrc & 0x7)) << GICH_LR_PHYSICALID_SHIFT;
    physicalid &= GICH_LR_PHYSICALID_MASK;
    lr_desc = (GICH_LR_HW_MASK & (hw << GICH_LR_HW_SHIFT)) |
        /* (GICH_LR_GRP1_MASK & (1 << GICH_LR_GRP1_SHIFT) )| */
        (GICH_LR_STATE_MASK & (state << GICH_LR_STATE_SHIFT)) |
        (GICH_LR_PRIORITY_MASK & \
         ((priority >> 3)  << GICH_LR_PRIORITY_SHIFT)) |
        physicalid |
        (GICH_LR_VIRTUALID_MASK & virq);
    slot = gic_is_free_slot(slot);
    HVMM_TRACE_HEX32("lr_desc:", lr_desc);
    HVMM_TRACE_HEX32("free slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
        GICH_WRITE(GICH_LR(slot), lr_desc);
    }

    HVMM_TRACE_EXIT();
    return slot;
}

