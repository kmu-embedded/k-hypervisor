#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>
#include <core/vm/vcpu.h>

struct scheduler {
    unsigned int sched_id;

    int (*init)(void);
    int (*register_vcpu)(vcpuid_t);
    int (*unregister_vcpu)(vcpuid_t);
    int (*attach_vcpu)(vcpuid_t);
    int (*detach_vcpu)(vcpuid_t);
    int (*do_schedule)(uint32_t *);
};

extern const struct scheduler sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
