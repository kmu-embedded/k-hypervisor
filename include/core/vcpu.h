#ifndef __VCPU_H__
#define __VCPU_H__

#include <hvmm_types.h>
#include <vcpu_regs.h>
#include <list.h>

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

#define GUEST_VERBOSE_ALL       0xFF
#define GUEST_VERBOSE_LEVEL_0   0x01
#define GUEST_VERBOSE_LEVEL_1   0x02
#define GUEST_VERBOSE_LEVEL_2   0x04
#define GUEST_VERBOSE_LEVEL_3   0x08
#define GUEST_VERBOSE_LEVEL_4   0x10
#define GUEST_VERBOSE_LEVEL_5   0x20
#define GUEST_VERBOSE_LEVEL_6   0x40
#define GUEST_VERBOSE_LEVEL_7   0x80

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

// FIXME(casionwoo) : This function should be removed when trap.c is modified
void vcpu_dump_regs(struct core_regs *regs);

hvmm_status_t vcpu_save(struct vcpu *vcpu, struct core_regs *regs);
hvmm_status_t vcpu_restore(struct vcpu *vcpu, struct core_regs *regs);

struct vcpu *vcpu_find(vcpuid_t vcpuid);
void print_all_vcpu();

#endif /* __VCPU_H__ */
