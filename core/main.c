#include <stdio.h>
#include <stdlib.h>
#include <core/vm.h>
#include <core/scheduler.h>

#include <config.h>

#include "../arch/arm/init.h"

void start_hypervisor()
{
    int i;
    uint8_t nr_vcpus = 1; // TODO: It will be read from configuration file.

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

    /*
     * TODO: Add a function - return vmid or vcpu_id to execute for the first time.
     * TODO: Rename guest_sched_start to do_schedule or something others.
     *       do_schedule(vmid) or do_schedule(vcpu_id)
     */
    sched_start();

    /* The code flow must not reach here */
error:
    printf("-------- [%s] ERROR: K-Hypervisor must not reach here\n", __func__);
    abort();
}
