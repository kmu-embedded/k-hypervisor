#include <core/vm/vcpu.h>
#include <debug.h>
#include <core/timer.h>
#include <arch/armv7/smp.h>
#include <stdio.h>
#include <core/scheduler.h>
#include <string.h>
#include <stdlib.h>

static struct list_head vcpu_list;
static int nr_vcpus = 0;

void vcpu_setup()
{
    LIST_INITHEAD(&vcpu_list);
    if (nr_vcpus != 0) {
        /* Never this happend */
        nr_vcpus = 0;
    }
}

struct vcpu *vcpu_create()
{
    struct vcpu *vcpu = NULL;

    vcpu = malloc(sizeof(struct vcpu));
    if (vcpu == VCPU_CREATE_FAILED) {
        return vcpu;
    }
    memset(vcpu, 0, sizeof(struct vcpu));

    vcpu->vcpuid = nr_vcpus++;
    vcpu->state = VCPU_DEFINED;

    // TODO(casionwoo) : Initialize running_time and actual_running_time after time_module created
    // TODO(casionwoo) : Initialize period and deadline after menuconfig module created

    virq_create(&vcpu->virq);

    LIST_ADDTAIL(&vcpu->head, &vcpu_list);

    return vcpu;
}

vcpu_state_t vcpu_init(struct vcpu *vcpu)
{
    arch_regs_init(&vcpu->arch_regs);

    // TODO(casionwoo): make it neat.
    virq_init(&vcpu->virq, vcpu->vmid);

    // TODO(casionwoo) : Check the return value after scheduler status value defined
    sched_vcpu_register(vcpu->vcpuid);

    vcpu->state = VCPU_REGISTERED;

    return vcpu->state;
}

vcpu_state_t vcpu_start(struct vcpu *vcpu)
{
    // TODO(casionwoo) : Check the return value after scheduler status value defined
    sched_vcpu_attach(vcpu->vcpuid);

    vcpu->state = VCPU_ACTIVATED;

    return vcpu->state;
}

vcpu_state_t vcpu_delete(struct vcpu *vcpu)
{
    // TODO(casionwoo) : Signal scheduler to stop the vcpu
    // TODO(casionwoo) : Detach vcpu id from scheduler

    LIST_DEL(&vcpu->head);
    free(vcpu);

    return VCPU_UNDEFINED;
}

void vcpu_save(struct vcpu *vcpu, struct core_regs *regs)
{
    arch_regs_save(&vcpu->arch_regs, regs);
    virq_save(&vcpu->virq);
}

void vcpu_restore(struct vcpu *vcpu, struct core_regs *regs)
{
    arch_regs_restore(&vcpu->arch_regs, regs);
    virq_restore(&vcpu->virq, vcpu->vmid);
}

struct vcpu *vcpu_find(vcpuid_t vcpuid)
{
    struct vcpu *vcpu = NULL;
    list_for_each_entry(struct vcpu, vcpu, &vcpu_list, head) {
        if (vcpu->vcpuid == vcpuid) {
            return vcpu;
        }
    }
    return vcpu;
}

void print_all_vcpu()
{
    struct vcpu *vcpu = NULL;
    list_for_each_entry(struct vcpu, vcpu, &vcpu_list, head) {
        print_vcpu(vcpu);
    }
}

void print_vcpu(struct vcpu *vcpu)
{
    debug_print("ADDR  : 0x%p\n", vcpu);
    debug_print("VCPUID  : %d\n", vcpu->vcpuid);
    debug_print("VMID  : %d\n", vcpu->vmid);
    debug_print("STATE  : %d\n", vcpu->state);
}

