#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>

#include "../../types.h"

typedef uint32_t sched_id_t;
struct scheduler {
    sched_id_t  id;

    int (*init)(void);
    int (*register_vcpu)(vcpuid_t);
    int (*unregister_vcpu)(vcpuid_t);
    int (*attach_vcpu)(vcpuid_t);
    int (*detach_vcpu)(vcpuid_t);
    int (*do_schedule)(uint32_t *);
};

extern const struct scheduler sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
