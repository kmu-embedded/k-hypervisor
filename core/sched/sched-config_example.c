#include <core/sched/sched-config.h>
#include <core/sched/scheduler_skeleton.h>

/* Description notation:
 *
 *      what array index meaning -> array element meaning
 *      in short,
 *      index -> element
 *
 *      use ZERO for UNUSED config datas
 */

/*****
 * Global configs for scheduler
 */

/* pcpu -> policy */
const struct sched_policy *schedconf_g_policy[NR_CPUS] = {
    &sched_rt_rm,
    &sched_rr,
    &sched_rr,
    &sched_rr
};

/* vcpu -> pcpu to be maped to */
uint32_t schedconf_g_vcpu_to_pcpu_map[TOTAL_VCPUS] = {
    0,
    0,
    1,
    0
};

/*****
 * Rate Monotonic policy configs
 */

/* pcpu(=policy) -> scheduler tick interval (mili second) */
uint32_t schedconf_rm_tick_interval_ms[NR_CPUS] = {
    1000,
    0,
    0,
    0
};

/* vcpu -> {period, budget} in tick count */
uint32_t schedconf_rm_period_budget[TOTAL_VCPUS][2] = {
    {5, 2},
    {3, 1},
    {0, },
    {90909, 90909} // {90909, 90909} /* magic number for idle task */
};

/*****
 * Round Robin policy configs
 */

/* vcpu -> {budget} in ms */
uint32_t schedconf_rr_slice[TOTAL_VCPUS] = {
    0,
    0,
    5,
    0
};

