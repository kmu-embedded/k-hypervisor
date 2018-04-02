#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/smccc.h>

int emulate_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch(function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    default:
        // Forward SMCCC as intended.
        arm_smccc_smc(regs->gpr[0], regs->gpr[1], regs->gpr[2], regs->gpr[3],
                      regs->gpr[4], regs->gpr[5], regs->gpr[6], regs->gpr[7],
                      (struct arm_smccc_res *) regs->gpr);
    }

    return 0;
}
