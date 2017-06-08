#ifndef __PSCI_H__
#define __PSCI_H__

#include <stdint.h>
#include "v7/smccc.h"

#define ARM_PSCI_FN_BASE 0x95c1ba5e

// TODO(jigi.kim): Consider PSCI version >= 0.2.
typedef enum {
    ARM_PSCI_FN_CPU_SUSPEND = ARM_PSCI_FN_BASE,
    ARM_PSCI_FN_CPU_OFF,
    ARM_PSCI_FN_CPU_ON,
    ARM_PSCI_FN_MIGRATE
} arm_psci_fn;

void psci_cpu_suspend(uint32_t state, unsigned long entry_point);
void psci_cpu_off(uint32_t state);
void psci_cpu_on(unsigned long cpu_id, unsigned long entry_point);
void psci_migrate(unsigned long cpu_id);

#endif
