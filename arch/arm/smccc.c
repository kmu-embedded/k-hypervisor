#include <stdio.h>
#include <arch/armv7.h>
#include <arch/smccc.h>

#ifdef CONFIG_OPTEE
#include <arch/optee.h>
#endif

#ifdef CONFIG_ARM_PSCI
#include <arch/psci.h>
#endif

static inline void handle_misc_arm_smccc(struct core_regs *regs);

int handle_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch (SMCCC_SERVICE(function_id)) {
    case SMCCC_SERVICE_ARCH:
    case SMCCC_SERVICE_CPU:
    case SMCCC_SERVICE_SIP:
    case SMCCC_SERVICE_OEM:
    case SMCCC_SERVICE_STANDARD:
    case SMCCC_SERVICE_HYPERVISOR:
        break;
    case SMCCC_SERVICE_TRUSTED_APP ... SMCCC_SERVICE_TRUSTED_APP_END:
    case SMCCC_SERVICE_TRUSTED_OS ... SMCCC_SERVICE_TRUSTED_OS_END:
#ifdef CONFIG_OPTEE
        handle_optee_smc(regs);
#endif
        break;
    default:
        handle_misc_arm_smccc(regs);
    }

    return 0;
}

static inline void handle_misc_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch (function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    default:
        arm_smccc_smc(regs->gpr[0], regs->gpr[1], regs->gpr[2], regs->gpr[3],
                      regs->gpr[4], regs->gpr[5], regs->gpr[6], regs->gpr[7],
                      (struct arm_smccc_res *) regs->gpr);
    }
}
