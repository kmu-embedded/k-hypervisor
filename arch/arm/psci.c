#include <stdio.h>
#include <arch/armv7.h>
#include <arch/psci.h>
#include <arch/smccc.h>
#include <core/scheduler.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>

int psci_cpu_suspend(uint32_t state, unsigned long entry_point)
{
    struct arm_smccc_res res;

    arm_smccc_smc(PSCI_FN_CPU_SUSPEND, state, entry_point, 0, 0, 0, 0, 0, &res);
    return res.a0;
}

int psci_cpu_off(uint32_t state)
{
    struct arm_smccc_res res;

    arm_smccc_smc(PSCI_FN_CPU_OFF, state, 0, 0, 0, 0, 0, 0, &res);
    return res.a0;
}

int psci_cpu_on(unsigned long cpu_id, unsigned long entry_point)
{
    struct arm_smccc_res res;

    arm_smccc_smc(PSCI_FN_CPU_ON, cpu_id, entry_point, 0, 0, 0, 0, 0, &res);
    return res.a0;
}

int psci_migrate(unsigned long cpu_id)
{
    struct arm_smccc_res res;

    arm_smccc_smc(PSCI_FN_MIGRATE, cpu_id, 0, 0, 0, 0, 0, 0, &res);
    return res.a0;
}

int emulate_psci_cpu_on(struct core_regs *regs)
{
    // TODO(jigi.kim): Separate general secondary vcpu init behavior as a function.
    uint32_t vcpuid = regs->gpr[1];
    uint32_t entry_point = regs->gpr[2];

    struct vmcb *vm = get_current_vm();
    struct vcpu *target_vcpu = NULL;

    if (vcpuid >= vm->num_vcpus) {
        return PSCI_RET_INVALID_PARAMS;
    }

    target_vcpu = vm->vcpu[vcpuid];
    target_vcpu->regs.core_regs.pc = entry_point;

    sched_vcpu_attach(target_vcpu->vcpuid, target_vcpu->pcpuid);
    target_vcpu->state = VCPU_ACTIVATED;

    printf("vcpu[%d] attatched to pcpu[%d] \n", target_vcpu->vcpuid, target_vcpu->pcpuid);

    return PSCI_RET_SUCCESS;
}
