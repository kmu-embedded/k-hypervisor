#include <stdio.h>
#include <debug_print.h>

#include <vm.h>
#include <core.h>
#include <scheduler.h>
#include <version.h>
#include <hvmm_trace.h>

#include <platform.h>

static vmid_t vm[NUM_GUESTS_STATIC];

static int cpu_init()
{
    hvmm_status_t status;
    unsigned char nr_vcpus = 1; // TODO: It will be read from configuration file.
    int i;

    // FIXME: Split khypervisor_init into per-core initialization and
    //        system-level initializatin to support SMP.
    status = khypervisor_init();
    if (status != HVMM_STATUS_SUCCESS) {
        goto error;
    }

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
    debug_print("%s", BANNER_STRING); // TODO: make a print_banner();
    guest_sched_start();

    /* The code flow must not reach here */
error:
    debug_print("-------- [%s] ERROR: K-Hypervisor must not reach here\n", __func__);
    hyp_abort_infinite();
    return 0;
}

// C Entry point
int main(void)
{
    init_platform();
    cpu_init();

    return 0;
}
