#include "vgic.h"
#include <stdio.h>

#include <hvmm_trace.h>
#include <hvmm_types.h>

#include <arch/armv7.h>
#include <arch/gic_regs.h>
#include <core/scheduler.h>
#include <debug_print.h>

#include <rtsm-config.h>
#include <core/vm/virq.h>
#include <io.h>

/* for test, surpress traces */
#define __VGIC_DISABLE_TRACE__

#ifndef __CONFIG_SMP__
#define VGIC_SIMULATE_HWVIRQ
#endif

/* Cortex-A15: 25 (PPI6) */
#define VGIC_MAINTENANCE_INTERRUPT_IRQ  25

#define VGIC_MAX_LISTREGISTERS          VGIC_NUM_MAX_SLOTS
#define VIRQ_MAX_ENTRIES    128
#define SLOT_INVALID        0xFFFFFFFF

#define GICH_READ(offset)           __readl(vGICv2.base + offset)
#define GICH_WRITE(offset, value)   __writel(value, vGICv2.base + offset)

struct vGICv2_HW {
    /** Base address of VGIC (Virtual Interface Control Registers) */
    uint32_t base;
    /** Number of List Registers */
    uint32_t num_lr;
    /** Mask of the Number of Valid List Register */
    uint64_t valid_lr_mask;
};

static struct vGICv2_HW vGICv2;

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
            slot = vgic_inject_virq_hw(virq, VIRQ_STATE_PENDING, GIC_INT_PRIORITY_DEFAULT, pirq);
        } else {
            slot = vgic_inject_virq_sw(virq, VIRQ_STATE_PENDING, 0, vcpuid, 1);
        }

        if (slot == VGIC_SLOT_NOTFOUND) {
            return result;
        }

        result = HVMM_STATUS_SUCCESS;
    }
    else {
        /* Inject only the same virq is not present in a slot */
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
hvmm_status_t vgic_flush_virqs(vcpuid_t vcpuid)
{
    /* Actual injection of queued VIRQs takes place here */
    int i;
    struct vcpu *vcpu = vcpu_find(vcpuid);
    struct virq_entry *entries = vcpu->virq.pending_irqs;

    for (i = 0; i < VIRQ_MAX_ENTRIES; i++) {
        if (entries[i].valid) {
            uint32_t slot;
            if (entries[i].hw) {
                slot = vgic_inject_virq_hw(entries[i].virq, VIRQ_STATE_PENDING,
                        GIC_INT_PRIORITY_DEFAULT, entries[i].pirq);
            } else {
                slot = vgic_inject_virq_sw(entries[i].virq, VIRQ_STATE_PENDING,
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

/**
 * @brief Find one empty List Register index, from ELRSR0/1's LSB.
 * @return Empty List Register.
 */
static uint32_t vgic_find_free_slot(void)
{
    uint32_t slot;
    uint32_t shift = 0;
    slot = GICH_READ(GICH_ELSR(0));
    if (slot == 0 && vGICv2.num_lr > 32) {
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

/*
 * Test if the List Registers 'slot' is free
 * Return
 *  Free - 'slot' is returned
 *  Occupied but another free slot found - new free slot
 *  Fully occupied - VGIC_SLOT_NOTFOUND
 */

static uint32_t vgic_is_free_slot(uint32_t slot)
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
        free_slot = vgic_find_free_slot();
    }
    return free_slot;
}
/**
 * @brief   Shows VGIC status
 * <pre>
 * Initialized  : Whether the initialization VGIC.
 * Num ListRegs : The number of List registers.
 * LR_MASK      : Mask of the Number of Valid List Register.
 * </pre>
 */
static void _vgic_dump_status(void)
{
    /*
     * === VGIC Status Summary ===
     * Initialized: Yes
     * Num ListRegs: n
     * Hypervisor Control
     *  - Enabled: Yes
     *  - EOICount:
     *  - Underflow:
     *  - LRENPIE:
     *  - NPIE:
     *  - VGrp0EIE:
     *  - VGrp0DIE:
     *  - VGrp1EIE:
     *  - VGrp1DIE:
     * VGIC Type
     *  - ListRegs:
     *  - PREbits:
     *  - PRIbits:
     * Virtual Machine Control
     *  -
     */
    debug_print("=== VGIC Status ===\n");
    debug_print(" Initialized:");
    debug_print(" %s\n", (VGIC_READY() ? "Yes" : "No"));
    debug_print(" Num ListRegs: 0x%08x\n", vGICv2.num_lr);
    debug_print(" LR_MASK: 0x%llu\n", vGICv2.valid_lr_mask);
}

/**
 * @brief   Shows virtual interface control registers.
 * <pre>
 * HCR     : Hypervisor Control Register
 * VTR     : VGIC Type Register
 * VMCR    : Virtual Machine Control Register
 * MISR    : Maintenance Interrupt Status Register
 * EISR0~1 : End of Interrupt Status Registers
 * ELSR0~1 : Empty List Register Status Registers
 * APR     : Active Priorities Register
 * LR0~n   : List Registers
 * </pre>
 */
static void _vgic_dump_regs(void)
{
#ifndef __VGIC_DISABLE_TRACE__
    int i;
    HVMM_TRACE_ENTER();
    debug_print("  hcr: 0x%08x\n", GICH_READ(GICH_HCR));
    debug_print("  vtr: 0x%08x\n", GICH_READ(GICH_VTR));
    debug_print(" vmcr: 0x%08x\n", GICH_READ(GICH_VMCR));
    debug_print(" misr: 0x%08x\n", GICH_READ(GICH_MISR));
    debug_print("eisr0: 0x%08x\n", GICH_READ(GICH_EISR(0)));
    debug_print("eisr1: 0x%08x\n", GICH_READ(GICH_EISR(1)));
    debug_print("elsr0: 0x%08x\n", GICH_READ(GICH_ELSR(0)));
    debug_print("elsr1: 0x%08x\n", GICH_READ(GICH_ELSR(1)));
    debug_print("  apr: 0x%08x\n", GICH_READ(GICH_APR));
    debug_print("   LR:\n");
    for (i = 0; i < vGICv2.num_lr; i++) {
        if (vgic_is_free_slot(i) != i) {
            debug_print("0x%08x - %d\n", GICH_READ(GICH_LR(i)), i);
        }
    }
    HVMM_TRACE_EXIT();
#endif
}



static void _vgic_isr_maintenance_irq(int irq, void *pregs, void *pdata)
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
                debug_print("vgic: deactivated pirq %d at slot %d\n", pirq, slot);
            } else {
                debug_print("vgic: deactivated virq at slot %d\n", slot);
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
                debug_print("vgic: deactivated pirq %d at slot %d\n", pirq, slot);
            } else {
                debug_print("vgic: deactivated virq at slot %d\n", slot);
            }
        }
    }

    HVMM_TRACE_EXIT();
}

hvmm_status_t vgic_enable(uint8_t enable)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    if (enable) {
        uint32_t hcr = GICH_READ(GICH_HCR);
        hcr |= GICH_HCR_EN;
        GICH_WRITE(GICH_HCR, hcr);
    } else {
        uint32_t hcr = GICH_READ(GICH_HCR);
        hcr &= ~(GICH_HCR_EN);
        GICH_WRITE(GICH_HCR, hcr);
    }
    result = HVMM_STATUS_SUCCESS;
    return result;
}

hvmm_status_t vgic_injection_enable(uint8_t enable)
{
    uint32_t hcr;
    hcr = read_hcr();
    if (enable) {
        if ((hcr & HCR_VI) == 0) {
            hcr |= HCR_VI;
            write_hcr(hcr);
        }
    } else {
        if (hcr & HCR_VI) {
            hcr &= ~(HCR_VI);
            write_hcr(hcr);
        }
    }
    hcr = read_hcr();
    debug_print(" updated hcr: %x\n", hcr);
    return HVMM_STATUS_SUCCESS;
}

/*
 * Params
 * @virq            virtual id (seen to the guest as an IRQ)
 * @slot            index to GICH_LR, slot < vGICv2.num_lr
 * @state           INACTIVE, PENDING, ACTIVE, or PENDING_ACTIVE
 * @priority        5bit priority
 * @hw              1 - physical interrupt, 0 - otherwise
 * @physrc          hw:1 - Physical ID, hw:0 - CPUID
 * @maintenance     hw:0, requires EOI asserts Virtual Maintenance Interrupt
 *
 * @return          slot index, or VGIC_SLOT_NOTFOUND
 */
uint32_t vgic_inject_virq(
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
    slot = vgic_is_free_slot(slot);
    HVMM_TRACE_HEX32("lr_desc:", lr_desc);
    HVMM_TRACE_HEX32("free slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
        GICH_WRITE(GICH_LR(slot), lr_desc);
    }

    _vgic_dump_regs();
    HVMM_TRACE_EXIT();
    return slot;
}

/*
 * Return: slot index if successful, VGIC_SLOT_NOTFOUND otherwise
 */
uint32_t vgic_inject_virq_hw(uint32_t virq, enum virq_state state,
        uint32_t priority, uint32_t pirq)
{
    uint32_t slot = VGIC_SLOT_NOTFOUND;
    HVMM_TRACE_ENTER();
    slot = vgic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
#ifdef VGIC_SIMULATE_HWVIRQ
        slot = vgic_inject_virq(virq, slot, state, priority, 0, smp_processor_id(), 1);
#else
        slot = vgic_inject_virq(virq, slot, state, priority, 1, pirq, 0);
#endif
    }
    HVMM_TRACE_EXIT();
    return slot;
}

uint32_t vgic_inject_virq_sw(uint32_t virq, enum virq_state state,
        uint32_t priority, uint32_t cpuid, uint8_t maintenance)
{
    uint32_t slot = VGIC_SLOT_NOTFOUND;
    HVMM_TRACE_ENTER();
    slot = vgic_find_free_slot();
    HVMM_TRACE_HEX32("slot:", slot);
    if (slot != VGIC_SLOT_NOTFOUND) {
        slot = vgic_inject_virq(virq, slot, state,
                priority, 0, cpuid, maintenance);
    }

    HVMM_TRACE_EXIT();
    return slot;
}

/**
 * @brief   Enables Virtual Maintenance Interrupt.
 * @return  Result status. Always return success.
 */
static hvmm_status_t _vgic_maintenance_irq_enable(uint8_t enable)
{
    uint32_t irq = VGIC_MAINTENANCE_INTERRUPT_IRQ;
    HVMM_TRACE_ENTER();
    if (enable) {
        register_irq_handler(irq, &_vgic_isr_maintenance_irq);
        gic_configure_irq(irq, IRQ_LEVEL_TRIGGERED);
        gic_enable_irq(irq);
    } else {
        register_irq_handler(irq, 0);
        gic_disable_irq(irq);
    }
    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}
/**
 * @brief Returns Mask of the Number of Valid List Register.
 * @return Mask bit of the Number of valid List Register.
 */
static uint64_t _vgic_valid_lr_mask(uint32_t num_lr)
{
    uint64_t mask_valid_lr = 0xFFFFFFFFFFFFFFFFULL;
    if (num_lr < VGIC_MAX_LISTREGISTERS) {
        mask_valid_lr >>= num_lr;
        mask_valid_lr <<= num_lr;
        mask_valid_lr = ~mask_valid_lr;
    }
    return mask_valid_lr;
}

hvmm_status_t vgic_init(void)
{
    uint32_t cpu = smp_processor_id();

    if (!cpu) {
        vGICv2.base = gic_vgic_baseaddr();
        vGICv2.num_lr = (GICH_READ(GICH_VTR) & GICH_VTR_LISTREGS_MASK) + 1;
        vGICv2.valid_lr_mask = _vgic_valid_lr_mask(vGICv2.num_lr);
    }

    _vgic_maintenance_irq_enable(1);
    if (!cpu) {
        _vgic_dump_status();
        _vgic_dump_regs();
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_init_status(struct vgic_status *status)
{
    int i;
    status->hcr = 0;
    status->apr = 0;
    status->vmcr = 0;
    status->saved_once = 0;
    for (i = 0; i < vGICv2.num_lr; i++) {
        status->lr[i] = 0;
    }

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_save_status(struct vgic_status *status)
{
    int i;

    for (i = 0; i < vGICv2.num_lr; i++) {
        status->lr[i] = GICH_READ(GICH_LR(i));
    }
    status->hcr = GICH_READ(GICH_HCR);
    status->apr = GICH_READ(GICH_APR);
    status->vmcr = GICH_READ(GICH_VMCR);
    vgic_enable(0);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_restore_status(struct vgic_status *status, vcpuid_t vcpuid)
{
    int i;

    for (i = 0; i < vGICv2.num_lr; i++) {
        GICH_WRITE(GICH_LR(i), status->lr[i]);
    }

    GICH_WRITE(GICH_APR, status->apr);
    GICH_WRITE(GICH_VMCR, status->vmcr);
    GICH_WRITE(GICH_HCR, status->hcr);
    /* Inject queued virqs to the next guest */
    /*
     * Staying at the currently active guest.
     * Flush out queued virqs since we didn't have a chance
     * to switch the context, where virq flush takes place,
     * this time
     */
    vgic_flush_virqs(vcpuid);
    vgic_enable(1);
    _vgic_dump_regs();
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vgic_sgi(uint32_t cpu, enum gic_sgi sgi)
{
    hvmm_status_t result = HVMM_STATUS_BAD_ACCESS;
    vcpuid_t vcpuid = get_current_vcpuid();

    if (cpu != vcpuid) {
        return result;
    }

    switch (sgi) {
        case GIC_SGI_SLOT_CHECK:
            result = vgic_flush_virqs(vcpuid);
            break;
        default:
            debug_print("sgi: wrong sgi %d\n", sgi);
            break;
    }

    return result;
}
