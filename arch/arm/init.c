#include "init.h"
#include "cpu.h"

#include <platform.h>
#include <libc_init.h>
#include <arch/irq.h>
#include "paging.h"
#include <vdev.h>
#include <core/timer.h>
#include <vm_map.h>

extern uint32_t __HYP_PGTABLE;

void init_system()
{
    uint8_t cpuid = smp_processor_id();

    setup_vector();

    setup_httbr((uint32_t) &__HYP_PGTABLE);

    setup_mem_attr();

    if (cpuid == 0) {
        // TODO(wonseok) console init will be moved dev_init().
        console_init();

        libc_init();

        irq_init();

        paging_create((addr_t) &__HYP_PGTABLE);

        platform_init();

        dev_init(); /* we don't have */

        vdev_init(); /* Already we have */

        timer_init(26);

        setup_vm_mmap();

#ifdef __CONFIG_SMP__
        printf("wake up...other CPUs\n");
        secondary_smp_pen = 1;
#endif
    }

    enable_mmu();

    start_hypervisor();
}
