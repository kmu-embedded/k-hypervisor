#include <asm/asm.h>
#include <stdio.h>
#include <debug_print.h>

#include <core/vm.h>
#include <core/scheduler.h>
#include <hvmm_trace.h>

#include <assert.h>
#include <rtsm-config.h>

#include <platform.h>
#include <core/vdev.h>
#include <arch/armv7.h>

#include <core/scheduler.h>
#include <libc_init.h>
#include <core/timer.h>
#include <core/irq.h>

#include "../arch/arm/mm.h"
//#include "../platform/rtsm/arch_init.h"
#include "../arch/arm/cpu.h"

static vmid_t vm[NUM_GUESTS_STATIC];
uint8_t secondary_smp_pen;

#define PLATFORM_BASIC_TESTS 4
static void SECTION(".init") primary_core_init(void)
{
    int i;
    uint8_t nr_vcpus = 1; // TODO: It will be read from configuration file.

    irq_disable();

    // Setup some basic operations such as BSS init., cache invalidate, etc.
    cpu_init();

    console_init();

    libc_init();

    mm_init();

    platform_init();

    enable_mmu();

    irq_init();

#ifdef __CONFIG_SMP__
    printf("wake up...other CPUs\n");
    secondary_smp_pen = 1;
#endif

    // FIXME: Register entire devices initialization code into dev_init().
    dev_init(); /* we don't have */

    vdev_init(); /* Already we have */

    timer_init(26);

    debug_print("[%s : %d] Starting...Main CPU\n", __func__, __LINE__);

    // TODO: Rest parts of cpu_init() will be moved per-core initialization.
    sched_init();

    vm_setup();
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        if ((vm[i] = vm_create(nr_vcpus)) == VM_CREATE_FAILED) {
            debug_print("vm_create(vm[%d]) is failed\n", i);
            goto error;
        }
        if (vm_init(vm[i]) != HALTED) {
            debug_print("vm_init(vm[%d]) is failed\n", i);
            goto error;
        }
        if (vm_start(vm[i]) != RUNNING) {
            debug_print("vm_start(vm[%d]) is failed\n", i);
            goto error;
        }
    }

    /*
     * TODO: Add a function - return vmid or vcpu_id to execute for the first time.
     * TODO: Rename guest_sched_start to do_schedule or something others.
     *       do_schedule(vmid) or do_schedule(vcpu_id)
     */
    // TODO: make a print_banner().
    sched_start();

    /* The code flow must not reach here */
error:
    debug_print("-------- [%s] ERROR: K-Hypervisor must not reach here\n", __func__);
    hyp_abort_infinite();
}

#ifdef __CONFIG_SMP__
static void SECTION(".init") secondary_core_init()
{
    irq_disable();

    // Setup some basic operations such as BSS init., cache invalidate, etc.
    cpu_init();

    // enable mmu and configure malloc
    mm_init();
    printf("hcr 0x%08x h\n", read_hcr());

    printf("%s[%d]: cpuid is %d\n", __func__, __LINE__, smp_processor_id());
    write_hcr(HCR_IMO | HCR_FMO);

    // TODO(igxactly): add sched_init().
    // TODO(casionwoo): add vm_init().
    // TODO(igxactly): add start_sched().
    hyp_abort_infinite();
}
#endif

void SECTION(".init") main(void)
{
#ifdef __CONFIG_SMP__
    uint8_t cpuid = smp_processor_id();

    if (cpuid) {
        secondary_core_init();
    } else
#endif
        primary_core_init();
}
