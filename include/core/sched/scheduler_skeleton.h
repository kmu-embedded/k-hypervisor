#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>
#include <core/vm/vcpu.h>
#include "../../types.h"
#include <lib/list.h>
#include <core/timer.h>

typedef uint32_t sched_id_t;

struct scheduler;
struct sched_entry;

typedef enum {
    SCHED_DETACHED,
    SCHED_WAITING,
    SCHED_RUNNING
} sched_state;

struct sched_policy {
    size_t size_sched_extra;
    size_t size_entry_extra;

    void (*init)(struct scheduler *);
    int (*register_vcpu)(struct scheduler *, struct sched_entry *);
    int (*unregister_vcpu)(struct scheduler *, struct sched_entry *);
    int (*attach_vcpu)(struct scheduler *, struct sched_entry *);
    int (*detach_vcpu)(struct scheduler *, struct sched_entry *);
    int (*do_schedule)(struct scheduler *, uint64_t *);
};

struct sched_entry {
    struct list_head head_registered;
    struct list_head head_attached;
    vcpuid_t vcpuid; /* how about using the pointer directly? */
    // struct vcpu *vcpu;
    sched_state state;
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

    // timers?
    struct timer timer;

    // policy & data
    const struct sched_policy *policy;
};

struct running_vcpus_entry_t {
    struct list_head head;
    vcpuid_t vcpuid;
};

extern const struct sched_policy sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
