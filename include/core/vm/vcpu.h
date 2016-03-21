#ifndef __VCPU_H__
#define __VCPU_H__

#include <lib/list.h>
#include "../../types.h"
#include "arch_regs.h"
#include "virq.h"

#define VCPU_CREATE_FAILED    NULL
#define VCPU_NOT_EXISTED      NULL
#define NUM_GUEST_CONTEXTS    NUM_GUESTS_CPU0_STATIC

typedef enum vcpu_state {
    VCPU_UNDEFINED,
    VCPU_DEFINED,
    VCPU_REGISTERED,
    VCPU_ACTIVATED,
} vcpu_state_t;

struct vcpu {
    vcpuid_t vcpuid;
    vmid_t vmid;

    struct arch_regs arch_regs;

    struct virq virq;

    unsigned int period;
    unsigned int deadline;

    uint64_t running_time;
    uint64_t actual_running_time;

    vcpu_state_t state;
    struct list_head head;
};

void vcpu_setup();

struct vcpu *vcpu_create();
vcpu_state_t vcpu_init(struct vcpu *vcpu);
vcpu_state_t vcpu_start(struct vcpu *vcpu);
vcpu_state_t vcpu_delete(struct vcpu *vcpu);

void vcpu_save(struct vcpu *vcpu, struct core_regs *regs);
void vcpu_restore(struct vcpu *vcpu, struct core_regs *regs);

struct vcpu *vcpu_find(vcpuid_t vcpuid);
void print_all_vcpu();
void print_vcpu(struct vcpu *vcpu);

#endif /* __VCPU_H__ */
