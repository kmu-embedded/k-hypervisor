#ifndef __VCPU_H__
#define __VCPU_H__

#ifdef CONFIG_C99
#include <lib/c99/util_list.h>
#else
#include <lib/gnu/list.h>
#endif

#include <hvmm_types.h>

#include "vcpu_regs.h"

#define VCPU_CREATE_FAILED    NULL
#define VCPU_NOT_EXISTED      NULL
#define NUM_GUEST_CONTEXTS    NUM_GUESTS_CPU0_STATIC

enum hyp_hvc_result {
    HYP_RESULT_ERET = 0,
    HYP_RESULT_STAY = 1
};

typedef enum vcpu_state {
    VCPU_UNDEFINED,
    VCPU_DEFINED,
    VCPU_REGISTERED,
    VCPU_ACTIVATED,
} vcpu_state_t;

struct vcpu {
    vcpuid_t vcpuid;
    vmid_t vmid;

    struct vcpu_regs vcpu_regs;

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

hvmm_status_t vcpu_save(struct vcpu *vcpu, struct core_regs *regs);
hvmm_status_t vcpu_restore(struct vcpu *vcpu, struct core_regs *regs);

struct vcpu *vcpu_find(vcpuid_t vcpuid);
void print_all_vcpu();
void print_vcpu(struct vcpu *vcpu);

#endif /* __VCPU_H__ */
