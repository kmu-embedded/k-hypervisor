#ifndef __VCPU_H__
#define __VCPU_H__

#include <list.h>
#include <hvmm_types.h>

#define VCPU_CREATE_FAILED    NULL
#define VCPU_NOT_EXISTED      NULL

typedef unsigned char vcpuid_t;

typedef enum vcpu_state {
    VCPU_UNDEFINED,
    VCPU_DEFINED,
    VCPU_REGISTERED,
    VCPU_ACTIVATED,
} vcpu_state_t;

struct vcpu {
    vcpuid_t vcpuid;
    vmid_t vmid;

    // TODO(casionwoo) : Define vcpu registers

    unsigned int period;
    unsigned int deadline;

    unsigned long long running_time;
    unsigned long long actual_running_time;

    vcpu_state_t state;
    struct list_head head;
};

hvmm_status_t vcpu_setup();
struct vcpu *vcpu_create();
vcpu_state_t vcpu_init(struct vcpu *vcpu);
vcpu_state_t vcpu_start(struct vcpu *vcpu);
vcpu_state_t vcpu_delete(struct vcpu *vcpu);

// TODO(casionwoo) : vcpu_suspend, resume, shutdown
// TODO(casionwoo) : vcpu_save, vcpu_restore

struct vcpu *vcpu_find(vcpuid_t vcpuid);
void print_all_vcpu();

#endif /* __VCPU_H__ */
