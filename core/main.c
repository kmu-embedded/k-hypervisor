#include <stdio.h>
#include <stdlib.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>
#include <config.h>
#include <core/timer.h>
#include <vm_config.h>

extern struct vm_config vm_conf[];

static uint32_t smp_pen = 0;

void start_hypervisor()
{
    int i;

    uint32_t pcpu = smp_processor_id();

    if (pcpu == 0) {
        timemanager_init();
        sched_init();

        vm_setup();

        for (i = 0; i < CONFIG_NR_VMS; i++) {
            vmid_t vmid;

            if ((vmid = vm_create(vm_conf[i].nr_vcpus)) == VM_CREATE_FAILED) {
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
    }

#ifdef CONFIG_TIMER_ACCESS_TRAP
    /* FIXME:(igkang) WORKAROUND - enable timer access trap */
    uint32_t val;
    val = read_cp32(CNTHCTL);
    write_cp32(val & ~(ARM_CNTHCTL_PL1PCEN | ARM_CNTHCTL_PL1PCTEN), CNTHCTL);
#endif

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
