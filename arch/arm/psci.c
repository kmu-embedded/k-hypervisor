#include <arch/psci.h>

void psci_cpu_suspend(uint32_t state, unsigned long entry_point)
{
    arm_smccc_smc(ARM_PSCI_FN_CPU_SUSPEND, state, entry_point, 0);
}

void psci_cpu_off(uint32_t state)
{
    arm_smccc_smc(ARM_PSCI_FN_CPU_OFF, state, 0, 0);
}

void psci_cpu_on(unsigned long cpu_id, unsigned long entry_point)
{
    arm_smccc_smc(ARM_PSCI_FN_CPU_ON, cpu_id, entry_point, 0);
}

void psci_migrate(unsigned long cpu_id)
{
    arm_smccc_smc(ARM_PSCI_FN_MIGRATE, cpu_id, 0, 0);
}
