#include <stdio.h>
#include <debug_print.h>
#include <asm_io.h>

#include <gic.h>
#include <armv7_p15.h>
#include <a15_cp15_sysregs.h>
#include <smp.h>
#include <vcpu.h>
#include <hvmm_trace.h>
#include <gic_regs.h>
#include <hvmm_types.h>

#include <board/rtsm-config.h>

#define CBAR_PERIPHBASE_MSB_MASK    0x000000FF

#define ARM_CPUID_CORTEXA15   0x412fc0f1

#define MIDR_MASK_PPN        (0x0FFF << 4)
#define MIDR_PPN_CORTEXA15    (0xC0F << 4)

#define GIC_INT_PRIORITY_DEFAULT_WORD    ((GIC_INT_PRIORITY_DEFAULT << 24) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 16) \
                                         |(GIC_INT_PRIORITY_DEFAULT << 8) \
                                         |(GIC_INT_PRIORITY_DEFAULT))

#define GIC_SIGNATURE_INITIALIZED   0x5108EAD7
/**
 * @brief Registers for Generic Interrupt Controller(GIC)
 */
struct gic_hw_info {
    uint32_t base;          /**< GIC base address */
    uint32_t gicd; /**< Distributor */
    uint32_t gicc; /**< CPU interface */
    uint32_t gich; /**< Virtual interface control (common)*/
    volatile uint32_t *gicv;/**< Virtual CPU interface */
    uint32_t nr_irqs;             /**< The Maximum number of interrupts */
    uint32_t nr_cpus;              /**< The number of implemented CPU interfaces */
    uint32_t initialized;       /**< Check whether initializing GIC. */
};

static struct gic_hw_info _gic;

static void dump_gic_regs(void)
{
    uint32_t midr;
    HVMM_TRACE_ENTER();
    midr = read_midr();
    debug_print("midr: 0x%08x\n", midr);
    if ((midr & MIDR_MASK_PPN) == MIDR_PPN_CORTEXA15) {
        uint32_t value;
        debug_print("cbar: 0x%08x\n", _gic.base);
        debug_print("gicd: 0x%08x\n", (uint32_t)_gic.gicd);
        debug_print("gicc: 0x%08x\n", (uint32_t)_gic.gicc);
        debug_print("gich: 0x%08x\n", (uint32_t)_gic.gich);
        debug_print("gicv: 0x%08x\n", (uint32_t)_gic.gicv);

#if 0
        value = _gic.gicd[GICD_CTLR];
        debug_print("GICD_CTLR: 0x%08x\n", value);
        value = _gic.gicd[GICD_TYPER];
        debug_print("GICD_TYPER: 0x%08x\n", value);
        value = _gic.gicd[GICD_IIDR];
        debug_print("GICD_IIDR: 0x%08x\n", value);
#endif
    }
    HVMM_TRACE_EXIT();
}

/**
 * @brief Return base address of GIC.
 *
 * When 40 bit address supports, This function wil use.
 */
static uint64_t get_periphbase(void)
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

/**
 * @brief   Return address of GIC memory map to _gic.base.
 * @param   va_base Base address(Physical) of GIC.
 * @return  If target architecture is Cortex-A15 then return success,
 *          otherwise return failed.
 */
static hvmm_status_t init_gic_base(uint32_t va_base)
{
    /* MIDR[15:4], CRn:c0, Op1:0, CRm:c0, Op2:0  == 0xC0F (Cortex-A15) */
    /* Cortex-A15 C15 System Control, C15 Registers */
    /* Name: Op1, CRm, Op2 */
    uint32_t midr;
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    HVMM_TRACE_ENTER();
    midr = read_midr();
    debug_print("midr: 0x%08x\n", midr);
    /*
     * Note:
     * We currently support GICv2 with Cortex-A15 only.
     * Other architectures with GICv2 support will be further
     * listed and added for support later
     */
    /* fall-back to periphbase addr from cbar */
    if (va_base == 0) {
        va_base = (uint32_t)(get_periphbase() & \
                0x00000000FFFFFFFFULL);
    }
    _gic.gicd = va_base + GIC_OFFSET_GICD;
    _gic.gicc = va_base + GIC_OFFSET_GICC;
    _gic.gich = (uint32_t *)(_gic.base + GIC_OFFSET_GICH);
    _gic.gicv = va_base + GIC_OFFSET_GICV;
    result = HVMM_STATUS_SUCCESS;
    HVMM_TRACE_EXIT();
    return result;
}
/**
 * @brief Initializes and enables GIC Distributor
 * <pre>
 * Initialization sequence
 * 1. Set Default SPI's polarity.
 * 2. Set Default priority.
 * 3. Diable all interrupts.
 * 4. Route all IRQs to all target processors.
 * 5. Enable Distributor.
 * </pre>
 * @return Always return success.
 */
static hvmm_status_t gicd_init(void)
{
#if 1
    uint32_t type;
    int i;
    uint32_t cpumask;
    HVMM_TRACE_ENTER();
    /* Disable Distributor */
    //_gic.gicd[GICD_CTLR] = 0;
    writel(0x0, _gic.gicd + GICD_CTLR);
    // writel (value, addr);
    //type = _gic.gicd[GICD_TYPER];
    type = readl(_gic.gicd + GICD_TYPER);

    _gic.nr_irqs = 32 * ((type & GICD_TYPE_LINES_MASK) + 1);
    _gic.nr_cpus = 1 + ((type & GICD_TYPE_CPUS_MASK) >> GICD_TYPE_CPUS_SHIFT);

    debug_print("GIC: nr_irqs: 0x%08x\n", _gic.nr_irqs);
    debug_print(" nr_cpus: 0x%08x\n", _gic.nr_cpus);
    debug_print(" IID: 0x%08x\n", readl(_gic.gicd + GICD_IIDR));

    /* Interrupt polarity for SPIs (Global Interrupts) active-low */
    for (i = 32; i < _gic.nr_irqs; i += 16)
        writel(0x0, _gic.gicd + GICD_ICFGR(i >> 4));

    /* Disable all global interrupts.
     * Private/Banked interrupts will be configured separately
     */
    for (i = 32; i < _gic.nr_irqs; i += 32)
        writel(0xffffffff, _gic.gicd + GICD_ICENABLER(i >> 5));

    /* Default Priority for all Interrupts
     * Private/Banked interrupts will be configured separately
     */
    for (i = 32; i < _gic.nr_irqs; i += 4)
        writel(0xa0a0a0a0, _gic.gicd + GICD_IPRIORITYR(i >> 2));

    /* Route all global IRQs to this CPU */
    cpumask = 1 << smp_processor_id();
    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;
    for (i = 32; i < _gic.nr_irqs; i += 4)
        writel(cpumask, _gic.gicd + GICD_ITARGETSR(i >> 2));

    /* Enable Distributor */
    writel(0x1, _gic.gicd + GICD_CTLR);
    HVMM_TRACE_EXIT();
#endif
    return HVMM_STATUS_SUCCESS;
}

/**
 * @brief Initializes GICv2 CPU Interface
 * <pre>
 * Initialization sequence
 * 1. Diable all PPI interrupts, ensure all SGI interrupts are enabled.
 * 2. Set priority on PPI and SGI interrupts.
 * 3. Set priority threshold(Priority Masking),
 *    Finest granularity of priority
 * 4. Enable signaling of interrupts.
 * </pre>
 * @return Always return success.
 */
static hvmm_status_t gicc_init(void)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
#if 1
    int i;
    /* Disable forwarding PPIs(ID16~31) */
    writel(0xffff0000, _gic.gicd + GICD_ICENABLER(0 >> 5));

    /* Enable forwarding SGIs(ID0~15) */
    writel(0x0000ffff, _gic.gicd + GICD_ISENABLER(0 >> 5));

    /* Default priority for SGIs and PPIs */
    for (i = 0; i < 32; i += 4)
        writel(0xa0a0a0a0, _gic.gicd + GICD_IPRIORITYR(i >> 2));

    /* No Priority Masking: the lowest value as the threshold : 255 */
    writel(0xff, _gic.gicc + GICC_PMR);
    writel(0x0, _gic.gicc + GICC_BPR);

    /* Enable signaling of interrupts and GICC_EOIR only drops priority */
    writel(GICC_CTL_ENABLE | GICC_CTL_EOI, _gic.gicc + GICC_CTLR);
#endif
    result = HVMM_STATUS_SUCCESS;
    return result;
}

/* API functions */
hvmm_status_t gic_enable_irq(uint32_t irq)
{
    writel(1UL << (irq & 0x1F), _gic.gicd + GICD_ISENABLER(irq >> 5));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_disable_irq(uint32_t irq)
{
    writel(1UL << (irq & 0x1F), _gic.gicd + GICD_ICENABLER(irq >> 5));
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_completion_irq(uint32_t irq)
{
    writel(irq, _gic.gicc + GICC_EOIR);
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t gic_deactivate_irq(uint32_t irq)
{
    writel(irq, _gic.gicc + GICC_DIR);
    return HVMM_STATUS_SUCCESS;
}

volatile uint32_t *gic_vgic_baseaddr(void)
{
    if (_gic.initialized != GIC_SIGNATURE_INITIALIZED) {
        HVMM_TRACE_ENTER();
        debug_print("gic: ERROR - not initialized\n\r");
        HVMM_TRACE_EXIT();
    }
    return _gic.gich;
}

hvmm_status_t gic_init(void)
{
    HVMM_TRACE_ENTER();

    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    uint32_t cpu = smp_processor_id();
    uint32_t va_base = 0x0;

    va_base = (uint32_t)(get_periphbase() & 0x000000FFFFFFFFFFULL);

    if(va_base == 0x0) {
        va_base = 0x2C000000;
    }

    _gic.gicd = va_base + GIC_OFFSET_GICD;
    _gic.gicc = va_base + GIC_OFFSET_GICC;
    _gic.gich = va_base + GIC_OFFSET_GICH;
    _gic.gicv = va_base + GIC_OFFSET_GICV;
    printf("gicd: 0x%08x\n", _gic.gicd);
    printf("gicc: 0x%08x\n", _gic.gicc);
    printf("gich: 0x%08x\n", _gic.gich);
    printf("gicv: 0x%08x\n", _gic.gicv);
    gicd_init();
    gicc_init();
    _gic.initialized = GIC_SIGNATURE_INITIALIZED;

    HVMM_TRACE_EXIT();
    return result;
}

hvmm_status_t gic_configure_irq(uint32_t irq,
                enum gic_int_polarity polarity,  uint8_t cpumask,
                uint8_t priority)
{
    hvmm_status_t result = HVMM_STATUS_UNKNOWN_ERROR;
    HVMM_TRACE_ENTER();
    if (irq < _gic.nr_irqs) {
        uint32_t icfg;
        volatile uint8_t *reg8;
        /* disable forwarding */
        result = gic_disable_irq(irq);
        if (result == HVMM_STATUS_SUCCESS) {
            /* polarity: level or edge */
            icfg = readl(_gic.gicd + GICD_ICFGR(irq >> 4));

            if (polarity == GIC_INT_POLARITY_LEVEL)
                icfg &= ~(2u << (2 * (irq % 16)));
            else
                icfg |= (2u << (2 * (irq % 16)));

            writel(icfg, _gic.gicd + GICD_ICFGR(irq >> 4));
            writel(cpumask, _gic.gicd + GICD_ITARGETSR(irq >> 2));
            writel(priority, _gic.gicd + GICD_IPRIORITYR(irq >> 2));

            /* enable forwarding */
            result = gic_enable_irq(irq);
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
    /*
     * 1. ACK - CPU Interface - GICC_IAR read
     * 2. Completion - CPU Interface - GICC_EOIR
     * 2.1 Deactivation - CPU Interface - GICC_DIR
     */
    uint32_t iar;
    uint32_t irq;
    /* ACK */
    iar = readl(_gic.gicc + GICC_IAR);
    irq = iar & GICC_IAR_INTID_MASK;

    return irq;
}

hvmm_status_t gic_set_sgi(const uint32_t target, uint32_t sgi)
{
    if(!(sgi < 16))
        return HVMM_STATUS_BAD_ACCESS;

    writel (GICD_SGIR_TARGET_LIST | (target << GICD_SGIR_CPU_TARGET_LIST_OFFSET) | (sgi & GICD_SGIR_SGI_INT_ID_MASK), _gic.gicd + GICD_SGIR(0));
    return HVMM_STATUS_SUCCESS;
}
