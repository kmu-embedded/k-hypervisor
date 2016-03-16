#include "init.h"
#include <assert.h>

extern uint32_t __hvc_vector;
void SECTION(".init.arch") setup_vector()
{
    uint32_t vector_base = (uint32_t) &__hvc_vector;
    write_hvbar(vector_base);
    assert(read_hvbar() == vector_base);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}

#include <platform.h>
#include <libc_init.h>
#include <core/irq.h>
#include "paging.h"
#include <core/vdev.h>
#include <core/timer.h>

void init_system()
{
    uint8_t cpuid = smp_processor_id();

    if (cpuid == 0) {
        console_init();

        libc_init();

        irq_init();

        paging_init();

        platform_init();

        dev_init(); /* we don't have */

        vdev_init(); /* Already we have */

        timer_init(26);

#ifdef __CONFIG_SMP__
        printf("wake up...other CPUs\n");
        secondary_smp_pen = 1;
#endif
    }

    // Per CPU
    setup_vector();
    paging_enable_mmu();
    start_hypervisor();
}
