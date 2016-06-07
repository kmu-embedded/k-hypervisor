#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>
#include <core/vm/vcpu.h>
#include "../../types.h"
#include <lib/list.h>
#include <core/timer.h>

typedef uint32_t sched_id_t;

struct sched_policy {
    void * (*init)(uint32_t);
    int (*register_vcpu)(vcpuid_t, uint32_t, void *); /* void * policy_data */
    int (*unregister_vcpu)(vcpuid_t, uint32_t, void *);
    int (*attach_vcpu)(vcpuid_t, uint32_t, void *);
    int (*detach_vcpu)(vcpuid_t, uint32_t, void *);
    int (*do_schedule)(uint64_t *, void *);
};

struct sched_entry {
    struct list_head head;
    vcpuid_t vcpuid; /* how about using the pointer directly? */
    void *pdata; /* policy specific data */
};

struct scheduler {
    sched_id_t  id;
    uint32_t pcpuid;

    //registered vcpus list
    struct list_head registered_list;
    struct list_head attached_list;

    // current vcpu ptr
    vcpuid_t current_vcpuid;
    vcpuid_t next_vcpuid;
    struct vcpu *current_vcpu;
    struct vmcb *current_vm;

    // policy & data
    const struct sched_policy *policy;
    void *policy_data;

    // timers?
    struct timer timer;
};

struct running_vcpus_entry_t {
    struct list_head head;
    vcpuid_t vcpuid;
};

extern const struct sched_policy sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
