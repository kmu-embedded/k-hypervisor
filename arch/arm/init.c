#include <stdio.h>

#include <arch/armv7.h>
#include <platform.h>
#include <libc_init.h>
#include <arch/irq.h>
#include <vdev.h>
#include <core/timer.h>
#include <vm_map.h>
#include <arch/armv7/generic_timer.h>
#include <drivers/pmu.h>

#include "init.h"
#include "paging.h"

extern addr_t __hvc_vector;
extern addr_t __HYP_PGTABLE;
uint8_t secondary_smp_pen;

void init_cpu()
{
    uint32_t cpuid = read_mpidr() & 0x00000103;
    addr_t pgtable = (uint32_t) &__HYP_PGTABLE;

    // For console debugging.
	console_init();
	libc_init();

    printf("[K-HYP] BOOT START\n");

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
	printf("(c%x) wake up...other CPUs\n", cpuid);
	secondary_smp_pen = 1;
#endif
	printf("(c%x) %s[%d]: CPU[%x]\n", cpuid, __func__, __LINE__, cpuid);

    printf("(c%x) before enable mmu\n", cpuid);
    write_cp32(HSCTLR_VALUE, HSCTLR);
//    write_cp32((HSCTLR_VALUE & ~(1 << 12 | 1 << 2)), HSCTLR);
    printf("(c%x) enable mmu\n", cpuid);

// NOTE(casionwoo) : This section is for power up secondary cpus
#if 0
    int i;
    set_boot_addr();
    for(i = 1; i < 2; i++) {
        printf("%dth cpu start\n", i);
        boot_secondary(i);
        init_secondary(i);

        printf("%dth cpu end\n", i);
//        printf("(c%x) cpu[%d] is enabled on PCPU[%x]\n", cpuid, i, cpuid);
    }
    smp_rmb();
    dsb_sev();
#endif
	start_hypervisor();
}

void init_secondary_cpus()
{
    printf("%s[%d]\n", __func__, __LINE__);
    uint32_t cpuid = read_mpidr() & 0x00000103;

    addr_t pgtable = (uint32_t) &__HYP_PGTABLE;

	write_cp32((uint32_t) &__hvc_vector, HVBAR);
	assert(read_cp32(HVBAR) == (uint32_t) &__hvc_vector);

    write_cp64((uint64_t) pgtable, HTTBR);
    assert(read_cp64(HTTBR) == pgtable);

    write_cp32(HTCR_VALUE, HTCR);
    write_cp32(HMAIR0_VALUE, HMAIR0);
    write_cp32(HMAIR1_VALUE, HMAIR1);

    irq_init();
    //printf("(c%x) %s[%d]: CPU[%x]\n", cpuid, __func__, __LINE__, cpuid);
    //while(1);
    write_cp32(HSCTLR_VALUE, HSCTLR);
//    write_cp32((HSCTLR_VALUE & ~(1 << 12 | 1 << 2)), HSCTLR);
    printf("(c%x) enable mmu\n", cpuid);

    start_hypervisor();
}

/* Set Hyp System Trap Register(HSTR) */
void enable_traps(void)
{
    uint32_t hstr = 0;

    hstr = 1 << 0;
    write_cp32(hstr, HSTR);
}
