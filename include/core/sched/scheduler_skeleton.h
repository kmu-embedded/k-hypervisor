#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>

#include "../../types.h"

typedef uint32_t sched_id_t;
struct scheduler {
    sched_id_t  id;

    int (*init)(uint32_t);
    int (*register_vcpu)(vcpuid_t, uint32_t);
    int (*unregister_vcpu)(vcpuid_t, uint32_t);
    int (*attach_vcpu)(vcpuid_t, uint32_t);
    int (*detach_vcpu)(vcpuid_t, uint32_t);
    int (*do_schedule)(uint64_t *);
};

extern const struct scheduler sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
