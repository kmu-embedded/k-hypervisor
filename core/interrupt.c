#include <stdio.h>
#include <debug_print.h>

#include <hvmm_types.h>
#include <core/vm.h>
#include <core/vm/virq.h>
#include <core/interrupt.h>
#include <arch/armv7/smp.h>

#include <rtsm-config.h>
#include <core/vm/vcpu.h>


#define VIRQ_MIN_VALID_PIRQ 16
#define VIRQ_NUM_MAX_PIRQS  MAX_IRQS

#define VALID_PIRQ(pirq) \
    (pirq >= VIRQ_MIN_VALID_PIRQ && pirq < VIRQ_NUM_MAX_PIRQS)

/**< IRQ handler */
static interrupt_handler_t _host_ppi_handlers[NUM_CPUS][MAX_PPI_IRQS];
static interrupt_handler_t _host_spi_handlers[MAX_IRQS];

const int32_t interrupt_check_guest_irq(uint32_t pirq)
{
    int i;
    struct virqmap_entry *map;

    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        struct vmcb *vm = vm_find(i);
        map = vm->virq.guest_virqmap->map;

        if (map[pirq].virq != VIRQ_INVALID)
            return GUEST_IRQ;
    }

    return HOST_IRQ;
}

const uint32_t interrupt_virq_to_pirq(vmid_t vmid, uint32_t virq)
{
    struct vmcb *vm = vm_find(vmid);
    struct virqmap_entry *map = vm->virq.guest_virqmap->map;

    return map[virq].pirq;
}

const uint32_t interrupt_pirq_to_enabled_virq(vmid_t vmid, uint32_t pirq)
{
    uint32_t virq = VIRQ_INVALID;

    struct vmcb *vm = vm_find(vmid);
    struct virqmap_entry *map = vm->virq.guest_virqmap->map;

    if (map[pirq].enabled)
        virq = map[pirq].virq;

    return virq;
}

void interrupt_guest_inject(vmid_t vmid, uint32_t virq, uint32_t pirq, uint8_t hw)
{
    virq_inject(vmid, virq, pirq, hw);
}

void register_irq_handler(uint32_t irq, interrupt_handler_t handler)
{
    uint32_t cpu = smp_processor_id();

    if (irq < MAX_PPI_IRQS)
        _host_ppi_handlers[cpu][irq] = handler;

    else
        _host_spi_handlers[irq] = handler;
}

#if 0
void interrupt_host_disable(uint32_t irq)
{
    gic_disable_irq(irq);
}

void interrupt_host_configure(uint32_t irq)
{
    gic_configure_irq(irq, GIC_INT_POLARITY_LEVEL);
    gic_enable_irq(irq);
}
#endif

void interrupt_guest_enable(vmid_t vmid, uint32_t irq)
{
    struct vmcb *vm = vm_find(vmid);
    struct virqmap_entry *map = vm->virq.guest_virqmap->map;

    map[irq].enabled = GUEST_IRQ_ENABLE;
}

static void interrupt_inject_enabled_guest(int num_of_guests, uint32_t irq)
{
    int i;
    uint32_t virq;

    for (i = 0; i < num_of_guests; i++) {
        virq = interrupt_pirq_to_enabled_virq(i, irq);
        if (virq == VIRQ_INVALID)
            continue;
        interrupt_guest_inject(i, virq, irq, INJECT_HW);
    }
}

// The function as below will be moved into hvc_irq function.
void interrupt_service_routine(int irq, void *current_regs, void *pdata)
{
    struct arch_regs *regs = (struct arch_regs *)current_regs;
    uint32_t cpu = smp_processor_id();


    if (irq < MAX_IRQS) {
        if (interrupt_check_guest_irq(irq) == GUEST_IRQ) {

            /* IRQ INJECTION */
            /* priority drop only for hanlding irq in guest */
            gic_completion_irq(irq);
            interrupt_inject_enabled_guest(NUM_GUESTS_STATIC, irq);
        } else {
            /* host irq */
            if (irq < MAX_PPI_IRQS) {
                if (_host_ppi_handlers[cpu][irq])
                    _host_ppi_handlers[cpu][irq](irq, regs, 0);
            } else {
                if (_host_spi_handlers[irq])
                    _host_spi_handlers[irq](irq, regs, 0);
            }
            gic_completion_irq(irq);
            gic_deactivate_irq(irq);
        }
    } else
        debug_print("interrupt:no pending irq:%x\n", irq);
}

// TODO(casionwoo): Will be moved into vgic.c?
//static struct vgic_status _vgic_status[NUM_GUESTS_STATIC];

void interrupt_init()
{
    uint32_t cpu = smp_processor_id();

    gic_init();
    write_hcr(HCR_IMO | HCR_FMO);

    /* Virtual Interrupt: GIC Virtual Interface Control */
    vgic_init();
    vgic_enable(1);
    virq_table_init();

}
