#ifndef _SCHEDULER_CONFIG_H_
#define _SCHEDULER_CONFIG_H_

#include <core/sched/scheduler_skeleton.h>

/* TODO:(igkang) merge this macro with NR_VCPUS in vdev_gicd.h */
#define TOTAL_VCPUS 8

extern const struct sched_policy *
        schedconf_g_policy[];

extern uint32_t
schedconf_g_vcpu_to_pcpu_map[TOTAL_VCPUS];

extern uint32_t
schedconf_rr_slice[TOTAL_VCPUS];

extern uint32_t
schedconf_rm_tick_interval_us[NR_CPUS];

extern uint32_t
schedconf_rm_period_budget[TOTAL_VCPUS][2];

extern uint32_t
schedconf_edf_tick_interval_us[NR_CPUS];

extern uint32_t
schedconf_edf_period_budget[TOTAL_VCPUS][2];

#endif /* _SCHEDULER_CONFIG_H_ */
