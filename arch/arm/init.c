#include <stdio.h>

#include <arch/armv7.h>
#include <platform.h>
#include <libc_init.h>
#include <arch/irq.h>
#include <vdev.h>
#include <core/timer.h>
#include <vm_map.h>
#include <arch/armv7/generic_timer.h>
#include "paging.h"

extern void start_hypervisor(void);

extern addr_t __hvc_vector;
extern addr_t __HYP_PGTABLE;
uint8_t secondary_smp_pen;

void init_cpu()
{
    uint8_t cpuid = smp_processor_id();
    addr_t pgtable = (uint32_t) &__HYP_PGTABLE;

    // For console debugging.
    console_init();
    libc_init();

    // Set HYP vector table.
    write_cp32((uint32_t) &__hvc_vector, HVBAR);
    assert(read_cp32(HVBAR) == (uint32_t) &__hvc_vector);

    // Set pgtable for HYP mode.
    write_cp64((uint64_t) pgtable, HTTBR);
    assert(read_cp64(HTTBR) == pgtable);

    // Set memory attributes.
    write_cp32(HTCR_VALUE, HTCR);
    write_cp32(HMAIR0_VALUE, HMAIR0);
    write_cp32(HMAIR1_VALUE, HMAIR1);

    // Two lines as below will be removed.
    paging_create((addr_t) &__HYP_PGTABLE);
    platform_init();

    irq_init();

    dev_init(); /* we don't have */

    vdev_init(); /* Already we have */

    setup_vm_mmap();

#ifdef CONFIG_SMP
    printf("wake up...other CPUs\n");
    secondary_smp_pen = 1;
#endif

    printf("%s[%d]: CPU[%d]\n", __func__, __LINE__, cpuid);

    write_cp32(HSCTLR_VALUE, HSCTLR);

    start_hypervisor();
}

void init_secondary_cpus()
{
    uint8_t cpuid = smp_processor_id();

    addr_t pgtable = (uint32_t) &__HYP_PGTABLE;

    write_cp32((uint32_t) &__hvc_vector, HVBAR);
    assert(read_cp32(HVBAR) == (uint32_t) &__hvc_vector);

    write_cp64((uint64_t) pgtable, HTTBR);
    assert(read_cp64(HTTBR) == pgtable);

    write_cp32(HTCR_VALUE, HTCR);
    write_cp32(HMAIR0_VALUE, HMAIR0);
    write_cp32(HMAIR1_VALUE, HMAIR1);

    irq_init();

    printf("%s[%d]: CPU[%d]\n", __func__, __LINE__, cpuid);

    write_cp32(HSCTLR_VALUE, HSCTLR);

    start_hypervisor();
}

/* Set Hyp System Trap Register(HSTR) */
void enable_traps(void)
{
    uint32_t hstr = 0;

    hstr = 1 << 0;
    write_cp32(hstr, HSTR);
}
