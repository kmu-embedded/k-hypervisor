#include <vm.h>
#include <core.h>
#include <scheduler.h>
#include <arch/arm/rtsm-config.h>
#include <version.h>
#include <hvmm_trace.h>

static vmid_t vm[NUM_GUESTS_STATIC];

int main_cpu_init()
{
    int i = 0;

    printf("[%s : %d] Starting...Main CPU\n", __func__, __LINE__);

    /* HYP initialization */
    khypervisor_init();

    /* Print Banner */
    printf("%s", BANNER_STRING);

    /* Scheduler initialization */
    sched_init();

    /* Virtual Machines Initialization */
    vm_setup();
    for (i = 0; i < NUM_GUESTS_STATIC; i++) {
        if ((vm[i] = vm_create(1)) == VM_CREATE_FAILED) {
            printf("vm_create(vm[%d]) is failed\n", i);
        }
        if (vm_init(vm[i]) != HALTED) {
            printf("vm_init(vm[%d]) is failed\n", i);
        }
        if (vm_start(vm[i]) != RUNNING) {
            printf("vm_start(vm[%d]) is failed\n", i);
        }
    }

    /* Switch to the first vCPU */
    guest_sched_start();

    /* The code flow must not reach here */
    printf("[hyp_main] ERROR: CODE MUST NOT REACH HERE\n");
    hyp_abort_infinite();
}

int main(void)
{
    main_cpu_init();

    return 0;
}
