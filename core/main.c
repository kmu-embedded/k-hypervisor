#include <stdio.h>
#include <stdlib.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>
#include <config.h>
#include "../arch/arm/init.h"
#include <core/timer.h>
#include <drivers/pmu.h>

static uint32_t smp_pen = 0;

void start_hypervisor()
{
    int i;
    uint8_t nr_vcpus = 8; // TODO: It will be read from configuration file.

    uint32_t pcpu = read_mpidr() & 0x103;
    printf("(c%x) %s %d\n", pcpu, __func__, __LINE__);

    if (pcpu == BOOTABLE_CPUID) {
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
        printf("(c%x) %s is enabled\n", pcpu, __func__);
        smp_pen = 1;
    } else {
        while (!smp_pen) ;
        printf("(c%x) %s is enabled\n", pcpu, __func__);
    }

    /*
     * TODO: Add a function - return vmid or vcpu_id to execute for the first time.
     * TODO: Rename guest_sched_start to do_schedule or something others.
     *       do_schedule(vmid) or do_schedule(vcpu_id)
     */
//    while(1);
//    printf("(c%x) before cpu_start %s\n", pcpu, __func__);
//    if(pcpu != 0x100) {
//        while(1);
//    }
    printf("(c%x) sched_start!!!\n", pcpu);
    sched_start();

    /* The code flow must not reach here */
error:
    printf("-------- [%s] ERROR: K-Hypervisor must not reach here\n", __func__);
    abort();
}
