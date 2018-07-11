#include <stdio.h>
#include <arch/armv7.h>
#include <arch/optee.h>
#include <arch/psci.h>
#include <arch/smccc.h>

int handle_misc_arm_smccc(struct core_regs *regs);

int handle_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];
    int ret = INVALID_SMCCC_FN;

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
        ret = handle_optee_smc(regs);
#endif
        break;
    default:
        ret = handle_misc_arm_smccc(regs);
    }

    if (ret == INVALID_SMCCC_FN) {
        printf("No corresponding smc handler for fn 0x%08x\n", function_id);
        printf("type : %s\n", SMCCC_IS_FAST_CALL(function_id) ? "fast" : "std");
        printf("conv : %d\n", SMCCC_IS_64(function_id) ? 64 : 32);
        printf("owner: %d\n", SMCCC_SERVICE(function_id));
        printf("func : 0x%04x\n", SMCCC_FN_NUM(function_id));
    }

    return ret;
}

int handle_misc_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch (function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    default:
        return INVALID_SMCCC_FN;
    }

    return 0;
}
