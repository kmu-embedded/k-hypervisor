#ifndef __VCPU_H__
#define __VCPU_H__

#include <hvmm_type.h>
#include <vcpu_hw.h>

#define MAX_VCPU_SIZE 32

struct vcpu {
/*
    struct arch_regs regs;
    struct arch_context context;
    unsigned int vmpidr;
*/
    struct vcpu_regs vcpu_regs;
        
    unsigned char vcpuid;
    unsigned int vmid;

    unsigned int period;
    unsigned int deadline;

    unsigned long long running_time;
    unsigned long long actual_running_time;
};

struct vcpu* vcpu_create(unsigned int vmid);
hvmm_state_t vcpu_init(unsigned int vcpuid);
hvmm_state_t vcpu_start(unsigned int vcpuid);
hvmm_state_t vcpu_delete(unsigned int vcpuid);
hvmm_state_t vcpu_suspend(unsigned int vcpuid);

hvmm_state_t vcpu_save(unsigned int vcpuid, struct arch_regs *current_regs);
hvmm_state_t vcpu_restore(unsigned int vcpuid, struct arch_regs *current_regs);

bool vcpu_same_vmid(unsigned int first_vcpuid, unsigned int second_vcpuid);
bool vcpu_is_available_create(unsigned int num_of_vcpu);

#endif // __VCPU_H__
