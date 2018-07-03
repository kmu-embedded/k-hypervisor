#include <arch/armv7.h>
#include <arch/optee.h>
#include <arch/psci.h>
#include <arch/smccc.h>
#include <core/scheduler.h>

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
