#ifndef _SCHEDULER_CONFIG_H_
#define _SCHEDULER_CONFIG_H_

#include <core/sched/scheduler_skeleton.h>

#define TOTAL_VCPUS CONFIG_NR_VMS

extern const struct sched_policy *
    schedconf_g_policy[NR_CPUS];

extern uint32_t
    schedconf_g_vcpu_to_pcpu_map[TOTAL_VCPUS];

extern uint32_t
    schedconf_rm_tick_interval_ms[NR_CPUS];

extern uint32_t
    schedconf_rm_period_budget[TOTAL_VCPUS][2];

extern uint32_t
    schedconf_rr_slice[TOTAL_VCPUS];

#endif /* _SCHEDULER_CONFIG_H_ */
