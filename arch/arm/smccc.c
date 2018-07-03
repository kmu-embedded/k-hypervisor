#include <stdio.h>
#include <arch/armv7.h>
#include <arch/optee.h>
#include <arch/psci.h>
#include <arch/smccc.h>

#define INVALID_SMCCC_FN -1

int handle_misc_arm_smccc(struct core_regs *regs);

int handle_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];
    int ret = 0;

    switch(SMCCC_FN_SERVICE(function_id)) {
    case SMCCC_SERVICE_ARCH:
    case SMCCC_SERVICE_CPU:
    case SMCCC_SERVICE_SIP:
    case SMCCC_SERVICE_OEM:
    case SMCCC_SERVICE_STANDARD:
    case SMCCC_SERVICE_HYPERVISOR:
        printf("No corresponding smc handler for fn 0x%08x\n", function_id);
        break;
    case SMCCC_SERVICE_TRUSTED_APP ... SMCCC_SERVICE_TRUSTED_APP_END:
    case SMCCC_SERVICE_TRUSTED_OS ... SMCCC_SERVICE_TRUSTED_OS_END:
        printf("fn: 0x%08x\n", function_id);
        handle_optee_smc(regs);
        break;
    default:
        ret = handle_misc_arm_smccc(regs);
    }

    return ret;
}

int handle_misc_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch(function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    default:
        printf("No corresponding smc handler for fn 0x%08x\n", function_id);
        return -1;
    }

    return 0;
}
