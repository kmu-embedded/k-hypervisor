#include <stdio.h>
#include <debug_print.h>
#include <interrupt.h>
#include <gic.h>
#include <vgic.h>

static hvmm_status_t host_interrupt_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;

    /* Route IRQ/IFQ to Hyp Exception Vector */
    {
        uint32_t hcr;
        hcr = read_hcr();
        debug_print("hcr: 0x%08x\n", hcr);
        hcr |= HCR_IMO | HCR_FMO;
        write_hcr(hcr);
        hcr = read_hcr();
        debug_print("hcr: 0x%08x\n", hcr);
    }

    /* Physical Interrupt: GIC Distributor & CPU Interface */
    result = gic_init();

    return result;
}

static hvmm_status_t host_interrupt_enable(uint32_t irq)
{
    return gic_enable_irq(irq);
}

static hvmm_status_t host_interrupt_disable(uint32_t irq)
{
    return gic_disable_irq(irq);
}

static hvmm_status_t host_interrupt_configure(uint32_t irq)
{
    return gic_configure_irq(irq, GIC_INT_POLARITY_LEVEL,
            gic_cpumask_current(), GIC_INT_PRIORITY_DEFAULT);
}

static hvmm_status_t host_interrupt_end(uint32_t irq)
{
    /* Completion & Deactivation */
    gic_completion_irq(irq);
    gic_deactivate_irq(irq);

    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t host_interrupt_dump(void)
{
    /* TODO : dumpping the interrupt status & count */
    return HVMM_STATUS_SUCCESS;
}

static hvmm_status_t host_sgi(uint32_t cpu, uint32_t sgi)
{
    return vgic_sgi(cpu, sgi);
}

struct interrupt_ops _host_interrupt_ops = {
    .init = host_interrupt_init,
    .enable = host_interrupt_enable,
    .disable = host_interrupt_disable,
    .configure = host_interrupt_configure,
    .end = host_interrupt_end,
    .dump = host_interrupt_dump,
    .sgi = host_sgi,
};
