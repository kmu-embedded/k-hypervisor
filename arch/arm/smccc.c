#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/smccc.h>
#include <core/scheduler.h>

int emulate_arm_smccc(struct core_regs *regs)
{
    uint32_t function_id = regs->gpr[0];

    switch(function_id) {
#ifdef CONFIG_ARM_PSCI
    case PSCI_FN_CPU_ON:
        regs->gpr[0] = emulate_psci_cpu_on(regs);
        break;
#endif
    //TODO(jigi.kim): Add trusted OS related case, here.
    default:
        // Forward SMCCC as intended.
        arm_smccc_smc(regs->gpr[0], regs->gpr[1], regs->gpr[2], regs->gpr[3],
                      regs->gpr[4], regs->gpr[5], regs->gpr[6], regs->gpr[7],
                      (struct arm_smccc_res *) regs->gpr);

        // Partition OP-TEE static shared memory space for each VM.
        if ((function_id & 0x0000ffff) == 0x7) {
            vcpuid_t vcpuid = get_current_vcpuid();

            uint32_t base_addr = regs->gpr[1];
            uint32_t offset = regs->gpr[2];

            regs->gpr[1] = base_addr + (offset/CONFIG_NR_VMS) * vcpuid;
            regs->gpr[2] = offset/CONFIG_NR_VMS;
        }

    }

    return 0;
}
