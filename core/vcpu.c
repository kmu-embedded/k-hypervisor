#include <vcpu.h>
#include <timer.h>
#include <smp.h>
#include <stdio.h>
#include <scheduler.h>
#include <stdlib.h>

static struct list_head vcpu_list;
static int vcpu_count;

hvmm_status_t vcpu_setup()
{
	INIT_LIST_HEAD(&vcpu_list);
	vcpu_count = 0;

	return HVMM_STATUS_SUCCESS;
}

struct vcpu *vcpu_create()
{
	struct vcpu *vcpu = NULL;

	vcpu = malloc(sizeof(struct vcpu));
	if (vcpu == VCPU_CREATE_FAILED) {
		return vcpu;
	}

	vcpu->vcpuid = vcpu_count++;
	vcpu->state = VCPU_DEFINED;

	// TODO(casionwoo) : Initialize running_time and actual_running_time after time_module created
	// TODO(casionwoo) : Initialize period and deadline after menuconfig module created

	list_add_tail(&vcpu->head, &vcpu_list);

	return vcpu;
}

vcpu_state_t vcpu_init(struct vcpu *vcpu)
{
    vcpu_regs_init(&vcpu->vcpu_regs);
    vcpu->state = VCPU_REGISTERED;

	// TODO(casionwoo) : Register vcpu id to scheduler

    return vcpu->state;
}

vcpu_state_t vcpu_start(struct vcpu *vcpu)
{
	// TODO(casionwoo) : Signal scheduler to start the vcpu

	vcpu->state = VCPU_ACTIVATED;

	return vcpu->state;
}

vcpu_state_t vcpu_delete(struct vcpu *vcpu)
{
	// TODO(casionwoo) : Signal scheduler to stop the vcpu
	// TODO(casionwoo) : Unregister vcpu id from scheduler

	list_del(&vcpu->head);
	free(vcpu);

	return VCPU_UNDEFINED;
}

hvmm_status_t vcpu_save(struct vcpu *vcpu, struct core_regs *regs)
{
    return  vcpu_regs_save(&vcpu->vcpu_regs, regs);
}

hvmm_status_t vcpu_restore(struct vcpu *vcpu, struct core_regs *regs)
{
    return  vcpu_regs_restore(&vcpu->vcpu_regs, regs);
}

struct vcpu *vcpu_find(vcpuid_t vcpuid)
{
	struct vcpu *vcpu;

	list_for_each_entry(vcpu, &vcpu_list, head) {
		if (vcpu->vcpuid == vcpuid) {
			return vcpu;
		}
	}

	return VCPU_NOT_EXISTED;
}

void print_all_vcpu()
{
	struct vcpu *vcpu;

	list_for_each_entry(vcpu, &vcpu_list, head) {
		print_vcpu(vcpu);
	}
}

void print_vcpu(struct vcpu *vcpu)
{
	printf("ADDR  : 0x%p\n", vcpu);
	printf("VCPUID  : %d\n", vcpu->vcpuid);
	printf("STATE  : %d\n", vcpu->state);
}

