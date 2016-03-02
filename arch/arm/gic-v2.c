#include <stdio.h>
#include <debug_print.h>

#include <gic-v2.h>
#include <armv7_p15.h>
#include <a15_cp15_sysregs.h>
#include <smp.h>
#include <vcpu.h>
#include <hvmm_trace.h>

#include <gic_regs.h>
#include <asm_io.h>

#define gicd_read(offset)           getl(gic_hw.base + GICD_OFFSET + offset)
#define gicd_write(offset, value)   putl(value, gic_hw.base + GICD_OFFSET + offset)

#define gicc_read(offset)           getl(gic_hw.base + GICC_OFFSET + offset)
#define gicc_write(offset, value)   putl(value, gic_hw.base + GICC_OFFSET + offset)

#define gich_read(offset)           getl(gic_hw.base + GICH_OFFSET + offset)
#define gich_write(offset, value)   putl(value, gic_hw.base + GICH_OFFSET + offset)

uint64_t get_periphbase(void)
{
    uint32_t periphbase = 0x0;
    uint32_t cbar = (uint64_t) read_cbar();
    uint8_t upper_periphbase = cbar & 0xFF;

    if (upper_periphbase != 0) {
        periphbase |= (upper_periphbase << 32);
        cbar &= ~(0xFF);
    }
    periphbase |= cbar;

    return periphbase;
}

hvmm_status_t gic_init(void)
{
    HVMM_TRACE_ENTER();
    int i;

    // This code will be moved other parts, not here. */
    /* Get GICv2 base address */
    gic_hw.base = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);
    if(gic_hw.base == 0x0) {
        printf("Warning: Curretn architecture has no value in CBAR\n    \
                The architecture do not follow design philosophy from ARM recommendation\n");
        // TODO(casionwoo): vaule of base address will be read from DTB or configuration file.
        // Currently, we set the base address of gic to 0x2C000000, it is for RTSM.
        gic_hw.base = 0x2C000000;
    }

    gic_hw.nr_irqs = 32 * ((gicd_read(GICD_TYPER) & GICD_TYPE_LINES_MASK) + 1);
    gic_hw.nr_cpus = 1 + ((gicd_read(GICD_TYPER) & GICD_TYPE_CPUS_MASK) >> GICD_TYPE_CPUS_SHIFT);

    gicc_write(GICC_CTLR, GICC_CTL_ENABLE | GICC_CTL_EOI);
    gicd_write(GICD_CTLR, 0x1);

    /* No Priority Masking: the lowest value as the threshold : 255 */
    // We set 0xff but, real value is 0xf8
    gicc_write(GICC_PMR, 0xff);

    debug_print("GIC: nr_irqs: 0x%08x\n", gic_hw.nr_irqs);
    debug_print(" nr_cpus: 0x%08x\n", gic_hw.nr_cpus);

    // Set interrupt configuration do not work.
    for (i = 32; i < gic_hw.nr_irqs; i += 16) {
        debug_print("BEFORE: GICD_ICFGR(%d): 0x%08x\n", i >> 4, gicd_read(GICD_ICFGR(i >> 4)));
        gicd_write(GICD_ICFGR(i >> 4), 0x0);
        debug_print("AFTER: GICD_ICFGR(%d): 0x%08x\n", i >> 4, gicd_read(GICD_ICFGR(i >> 4)));
    }

    /* Disable all global interrupts. */
    for (i = 0; i < gic_hw.nr_irqs; i += 32) {
        debug_print("BEFORE: GICD_ICENABLER(%d): 0x%08x\n", i >> 5, gicd_read(GICD_ICENABLER(i >> 5)));
        gicd_write(GICD_ISENABLER(i >> 5), 0xffffffff);
        uint32_t valid = gicd_read(GICD_ISENABLER(i >> 5));
        gicd_write(GICD_ICENABLER(i >> 5), valid);
        debug_print("AFTER: GICD_ICENABLER(%d): 0x%08x\n", i >> 5, gicd_read(GICD_ICENABLER(i >> 5)));
    }

    // We set priority 0xa0 for each but real value is a 0xd0, Why?
    /* Set priority as default for all interrupts */
    for (i = 0; i < gic_hw.nr_irqs; i += 4) {
        debug_print("BEFORE: GICD_IPRIORITYR(%d): 0x%08x\n", i >> 2, gicd_read(GICD_IPRIORITYR(i >> 2)));
        gicd_write(GICD_IPRIORITYR(i >> 2), 0xa0a0a0a0);
        debug_print("AFTER: GICD_IPRIORITYR(%d): 0x%08x\n", i >> 2, gicd_read(GICD_IPRIORITYR(i >> 2)));
    }

    /* Route all global IRQs to CPU0 */
    for (i = 32; i < gic_hw.nr_irqs; i += 4) {
        debug_print("BEFORE: GICD_ITARGETSR(%d): 0x%08x\n", i >> 2, gicd_read(GICD_ITARGETSR(i >> 2)));
        gicd_write(GICD_ITARGETSR(i >> 2), 1 << 0 | 1 << 8 | 1 << 16 | 1 << 24);
        debug_print("AFTER: GICD_ITARGETSR(%d): 0x%08x\n", i >> 2, gicd_read(GICD_ITARGETSR(i >> 2)));
    }

    // TODO(casionwoo): Set SGI and PPIs

    gic_hw.initialized = GIC_SIGNATURE_INITIALIZED;

    HVMM_TRACE_EXIT();
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_configure_irq(uint32_t irq,
                enum gic_irq_polarity polarity,  uint8_t cpumask,
                uint8_t priority)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    HVMM_TRACE_ENTER();
    if (irq < gic_hw.nr_irqs) {
        uint32_t icfg;
        volatile uint8_t *reg8;
        /* disable forwarding */
        result = gic_disable_irq(irq);
        if (result == HVMM_STATUS_SUCCESS) {
            /* polarity: level or edge */
            icfg = gicd_read(GICD_ICFGR(irq >> 4));

            if (polarity == GIC_INT_POLARITY_LEVEL)
                icfg &= ~(2u << (2 * (irq % 16)));
            else
                icfg |= (2u << (2 * (irq % 16)));

            gicd_write(GICD_ICFGR(irq >> 4), icfg);
            gicd_write(GICD_ITARGETSR(irq >> 2), cpumask);
            gicd_write(GICD_IPRIORITYR(irq >> 2), priority);

            /* enable forwarding */
            result = enable_irq(irq);
        }
    } else {
        debug_print("invalid irq: 0x%08x\n", irq);
        result = HVMM_STATUS_UNSUPPORTED_FEATURE;
    }
    HVMM_TRACE_EXIT();
    return result;
}

uint32_t gic_get_irq_number(void)
{
    return gicc_read(GICC_IAR) & GICC_IAR_MASK;
}

hvmm_status_t gic_set_sgi(const uint32_t target, uint32_t sgi)
{
    if(!(sgi < 16))
        return HVMM_STATUS_BAD_ACCESS;

    gicd_write(GICD_SGIR(0), GICD_SGIR_TARGET_LIST |
                            (target << GICD_SGIR_CPU_TARGET_LIST_OFFSET) |
                            (sgi & GICD_SGIR_SGI_INT_ID_MASK));

    return HVMM_STATUS_SUCCESS;
}

/* API functions */
hvmm_status_t enable_irq(uint32_t irq)
{
    gicd_write(GICD_ISENABLER(irq >> 5), 1UL << (irq & 0x1F));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_disable_irq(uint32_t irq)
{
    gicd_write(GICD_ICENABLER(irq >> 5), 1UL << (irq & 0x1F));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_completion_irq(uint32_t irq)
{
    gicc_write(GICC_EOIR, irq);
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_deactivate_irq(uint32_t irq)
{
    gicc_write(GICC_DIR, irq);
    return HVMM_STATUS_SUCCESS;
}

uint32_t *gic_vgic_baseaddr(void)
{
    return gic_hw.base + GICH_OFFSET;
}
