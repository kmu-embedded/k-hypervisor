#ifndef _SCHEDULER_SKELETON_H_
#define _SCHEDULER_SKELETON_H_

#include <stdint.h>

struct scheduler {
    unsigned int sched_id;

    int (*init)(void);
    int (*register_vcpu)(int);
    int (*unregister_vcpu)(void);
    int (*attach_vcpu)(int);
    int (*detach_vcpu)(void);
    int (*do_schedule)(uint32_t *);
};

extern const struct scheduler sched_rr;

#endif /* _SCHEDULER_SKELETON_H_ */
