#ifndef __PSCI_H__
#define __PSCI_H__

#include <stdint.h>
#include <arch/armv7.h>

#define PSCI_FN_BASE 0x95c1ba5e

// TODO(jigi.kim): Consider PSCI version >= 0.2.
typedef enum {
    PSCI_FN_CPU_SUSPEND = PSCI_FN_BASE,
    PSCI_FN_CPU_OFF,
    PSCI_FN_CPU_ON,
    PSCI_FN_MIGRATE
} psci_fn;

#define PSCI_RET_SUCCESS             0
#define PSCI_RET_NOT_SUPPORTED      -1
#define PSCI_RET_INVALID_PARAMS     -2
#define PSCI_RET_DENIED             -3
#define PSCI_RET_ALREADY_ON         -4
#define PSCI_RET_ON_PENDING         -5
#define PSCI_RET_INTERNAL_FAILURE   -6
#define PSCI_RET_NOT_PRESENT        -7
#define PSCI_RET_DISABLED           -8

void psci_cpu_suspend(uint32_t state, unsigned long entry_point);
void psci_cpu_off(uint32_t state);
void psci_cpu_on(unsigned long cpu_id, unsigned long entry_point);
void psci_migrate(unsigned long cpu_id);

int emulate_psci_cpu_on(struct core_regs *regs);

#endif
