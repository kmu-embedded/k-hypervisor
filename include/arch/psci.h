#ifndef __PSCI_H__
#define __PSCI_H__

#include <stdint.h>
#include <arch/armv7.h>

#ifdef CONFIG_ARM_PSCI_VERSION_0_1

typedef enum {
    PSCI_FN_CPU_SUSPEND = CONFIG_ARM_PSCI_FN_BASE,
    PSCI_FN_CPU_OFF,
    PSCI_FN_CPU_ON,
    PSCI_FN_MIGRATE,

    PSCI_FN_LAST = PSCI_FN_MIGRATE
} psci_fn;

#else

typedef enum {
    PSCI_FN_PSCI_VERSION = CONFIG_ARM_PSCI_FN_BASE,
    PSCI_FN_CPU_SUSPEND,
    PSCI_FN_CPU_OFF,
    PSCI_FN_CPU_ON,
    PSCI_FN_AFFINITY_INFO,
    PSCI_FN_MIGRATE,
    PSCI_FN_MIGRATE_INFO_TYPE,
    PSCI_FN_INFO_UP_CPU,
    PSCI_FN_SYSTEM_OFF,
    PSCI_FN_SYSTEM_RESET,
    PSCI_FN_PSCI_FEATURES,
    PSCI_FN_PSCI_CPU_FREEZE,
    PSCI_FN_CPU_DEFAULT_SUSPEND,
    PSCI_FN_NODE_HW_STATE,
    PSCI_FN_SYSTEM_SUSPEND,
    PSCI_FN_PSCI_SET_SUSPEND_MODE,
    PSCI_FN_PSCI_STAT_RESIDENCY,
    PSCI_FN_PSCI_STAT_COUNT,

#ifdef CONFIG_ARM_PSCI_VERSION_1_0
    PSCI_FN_LAST = PSCI_FN_PSCI_STAT_COUNT
#else
    PSCI_FN_SYSTEM_RESET2,
    PSCI_FN_MEM_PROTECT,
    PSCI_FN_MEM_PROTECT_CHECK_RANGE,

    PSCI_FN_LAST = PSCI_FN_MEM_PROTECT_CHECK_RANGE
#endif
} psci_fn;

#endif

#define PSCI_RET_SUCCESS             0
#define PSCI_RET_NOT_SUPPORTED      -1
#define PSCI_RET_INVALID_PARAMS     -2
#define PSCI_RET_DENIED             -3
#define PSCI_RET_ALREADY_ON         -4
#define PSCI_RET_ON_PENDING         -5
#define PSCI_RET_INTERNAL_FAILURE   -6
#define PSCI_RET_NOT_PRESENT        -7
#define PSCI_RET_DISABLED           -8
#define PSCI_RET_INVALID_ADDRESS    -9

int psci_cpu_suspend(uint32_t state, unsigned long entry_point);
int psci_cpu_off(uint32_t state);
int psci_cpu_on(unsigned long cpu_id, unsigned long entry_point);
int psci_migrate(unsigned long cpu_id);

int emulate_psci_cpu_on(struct core_regs *regs);

#endif
