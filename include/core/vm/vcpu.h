#ifndef __VCPU_H__
#define __VCPU_H__

#include <lib/list.h>
#include <types.h>
#include <arch_regs.h>
#include "../../../drivers/gic-v2.h"
#include <core/vm/virq.h>

#define VCPU_CREATE_FAILED    NULL
#define VCPU_NOT_EXISTED      NULL
#define NUM_GUEST_CONTEXTS    NUM_GUESTS_CPU0_STATIC

typedef enum vcpu_state {
    VCPU_UNDEFINED,
    VCPU_DEFINED,
    VCPU_REGISTERED,
    VCPU_ACTIVATED,
} vcpu_state_t;

#define MAX_PENDING_VIRQS    64
#define MAX_NR_IRQ           1024
#define GUEST_IRQ_ENABLE 1
#define GUEST_IRQ_DISABLE 0

#define INJECT_SW 0
#define INJECT_HW 1

struct virq_table {
    uint32_t enabled;
    uint32_t virq;
    uint32_t pirq;
};


struct vcpu {
    vcpuid_t vcpuid;

    vmid_t vmid;

    struct arch_regs regs;

    struct banked_virq banked_virq;

    lr_entry_t pending_irqs[MAX_PENDING_VIRQS +1];

    uint32_t lr[64];

    uint32_t vmcr;

    unsigned int period;

    unsigned int deadline;

    uint64_t running_time;

    uint64_t actual_running_time;

    vcpu_state_t state;

    // TODO(casionwoo): it will be removed.
    struct virq_table map[MAX_NR_IRQ];

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

uint32_t virq_to_pirq(struct vcpu *v, uint32_t virq);
uint32_t pirq_to_enabled_virq(struct vcpu *v, uint32_t pirq);
uint32_t virq_to_enabled_pirq(struct vcpu *v, uint32_t virq);

void virq_enable(struct vcpu *v, uint32_t pirq, uint32_t virq);
void virq_disable(struct vcpu *v, uint32_t virq);
void pirq_enable(struct vcpu *v, uint32_t pirq, uint32_t virq);
void pirq_disable(struct vcpu *v, uint32_t pirq);
bool is_guest_irq(uint32_t irq);

#endif /* __VCPU_H__ */
