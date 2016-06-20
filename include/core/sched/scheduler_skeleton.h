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

    // struct vcpu *vcpu; /* TODO:(igkang) direct vcpu referencing */
    vcpuid_t vcpuid;
    sched_state state;

    /* policy-specific data follows (allocation) */
};

struct scheduler {
    sched_id_t  id;
    uint32_t pcpuid;

    struct list_head registered_list;
    struct list_head attached_list;

    vcpuid_t current_vcpuid;
    vcpuid_t next_vcpuid;
    struct vcpu *current_vcpu;
    struct vmcb *current_vm;

    struct timer timer;

    const struct sched_policy *policy;

    /* policy-specific data follows (allocation) */
};

extern const struct sched_policy sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
