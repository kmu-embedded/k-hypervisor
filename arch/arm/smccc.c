#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/smccc.h>
#include <core/scheduler.h>

void handle_optee_smc(uint32_t *a);

int handle_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch(function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    //TODO: Add trusted OS related case, here.
    default:
        handle_optee_smc(regs->gpr);
    }

    return 0;
}

//TODO: Move below to separated c file.

#define FN_GET_SHM_CONFIG   0xb2000007

// how to initialize optee_thread?
void handle_optee_smc(uint32_t *a)
{
    uint32_t function_id = a[0];

    arm_smccc_smc(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7],
            (struct arm_smccc_res *) a);

    // TODO: make it with switch statement.
    if (function_id == FN_GET_SHM_CONFIG) {
        vcpuid_t vcpuid = get_current_vcpuid();

        uint32_t base_addr = a[1];
        uint32_t offset = a[2];

        a[2] = offset/CONFIG_NR_VMS;
        a[1] = base_addr + (a[2] * vcpuid);
    }
}
