#include <stdio.h>
#include <stdlib.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>

#include <config.h>

#include "../arch/arm/init.h"

#include <core/timer.h>

static uint32_t smp_pen = 0;
uint32_t linux_smp_pen = 0;

void start_hypervisor()
{
    int i;
    uint8_t nr_vcpus = 4; // TODO: It will be read from configuration file.

    uint32_t pcpu = smp_processor_id();

    if (pcpu == 0) {
        timemanager_init();
        sched_init();

        vm_setup();

        for (i = 0; i < NUM_GUESTS_STATIC; i++) {
            vmid_t vmid;

            if ((vmid = vm_create(nr_vcpus)) == VM_CREATE_FAILED) {
                printf("vm_create(vm[%d]) is failed\n", i);
                goto error;
            }
            if (vm_init(vmid) != HALTED) {
                printf("vm_init(vm[%d]) is failed\n", i);
                goto error;
            }
            if (vm_start(vmid) != RUNNING) {
                printf("vm_start(vm[%d]) is failed\n", i);
                goto error;
            }
        }

        smp_pen = 1;
    } else {
        while (!smp_pen) ;
        printf("cpu[%d] is enabled\n", pcpu);
        while (!linux_smp_pen) ;
    }

    /*
     * TODO: Add a function - return vmid or vcpu_id to execute for the first time.
     * TODO: Rename guest_sched_start to do_schedule or something others.
     *       do_schedule(vmid) or do_schedule(vcpu_id)
     */
    printf("sched_start!!!\n");
    sched_start();

    /* The code flow must not reach here */
error:
    printf("-------- [%s] ERROR: K-Hypervisor must not reach here\n", __func__);
    abort();
}
