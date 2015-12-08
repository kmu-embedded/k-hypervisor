#include <vcpu.h>
#include <timer.h>
#include <smp.h>
#include <stdio.h>
#include <scheduler.h>
#include <stdlib.h>

#define _valid_vmid(vmid) \
    (guest_first_vmid() <= vmid && guest_last_vmid() >= vmid)

void print_vcpu(struct vcpu *vcpu);

static struct list_head vcpu_list;
static vcpuid_t vcpu_count;

hvmm_status_t vcpu_save(struct vcpu *vcpu, struct core_regs *regs)
{
    /* vcpu_regs_save : save the current guest's context*/
    return  vcpu_regs_save(&vcpu->vcpu_regs, regs);
}

hvmm_status_t vcpu_restore(struct vcpu *vcpu, struct core_regs *regs)
{
    /* vcpu_regs_restore : The next becomes the current */
     return  vcpu_regs_restore(&vcpu->vcpu_regs, regs);
}

void vcpu_dump_regs(struct core_regs *core_regs)
{
    /* vcpu_regs_dump */
    vcpu_regs_dump(GUEST_VERBOSE_ALL, core_regs);
}

hvmm_status_t vcpu_init()
{
    struct timer_val timer;
    hvmm_status_t result = HVMM_STATUS_SUCCESS;
    struct vcpu *vcpu;
    int i;
    int vcpu_count;
    int start_vmid = 0;
    uint32_t cpu = smp_processor_id();
    printf("[hyp] init_vcpus: enter\n");
    /* Initializes vcpus */
    vcpu_count = num_of_vcpu(cpu);

    if (cpu)
        start_vmid = num_of_vcpu(cpu - 1);
    else
        start_vmid = 0;

    vcpu_count += start_vmid;

    for (i = start_vmid; i < vcpu_count; i++) {
        /* Guest i @vcpu_bin_start */
        vcpu = &running_vcpu[i];
        vcpu->vcpuid = i;
        /* vcpu_hw_init */
        vcpu_regs_init(&vcpu->vcpu_regs);
    }

    printf("[hyp] init_vcpus: return\n");

    /* 100Mhz -> 1 count == 10ns at RTSM_VE_CA15, fast model*/
    timer.interval_us = GUEST_SCHED_TICK;
    timer.callback = &guest_schedule;

    result = timer_set(&timer, HOST_TIMER);

    if (result != HVMM_STATUS_SUCCESS)
        printf("[%s] timer startup failed...\n", __func__);

    return result;
}

void vcpu_copy(struct vcpu *dst, vmid_t vmid_src)
{
    vcpu_regs_move(&dst->vcpu_regs, &(running_vcpu[vmid_src]).vcpu_regs);
}

void reboot_vcpu(vmid_t vmid, uint32_t pc, struct core_regs **core_regs)
{
    struct vcpu_regs *vcpu_regs = &running_vcpu[vmid].vcpu_regs;
    vcpu_regs_init(vcpu_regs);
    vcpu_regs->core_regs.pc = pc;
    vcpu_regs->core_regs.gpr[10] = 1;
    if (core_regs != 0)
        vcpu_regs_restore(vcpu_regs, *core_regs);
}

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

//vcpu_state_t vcpu_init(struct vcpu *vcpu)
//{
//	// TODO(casionwoo) : Initialize vcpu hardware
//	// TODO(casionwoo) : Register vcpu id to scheduler
//
//	vcpu->state = VCPU_REGISTERED;
//
//	return vcpu->state;
//}

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
