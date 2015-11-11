#ifndef __VM_H__
#define __VM_H__

#include <vcpu.h>
#include <hvmm_type.h>

#define SIZE_OF_NAME    32
#define MAX_VM_SIZE     8
#define VCPU_MAX_SIZE   4

struct vmcb {
    unsigned int vmid;
    char name[SIZE_OF_NAME];

    unsigned int num_of_vcpu;
    struct vcpu *vcpu[VCPU_MAX_SIZE];

    // TODO(casionwoo) : define vmem

    // TODO(casionwoo) : define virq

    vmcb_state_t vm_state;
};

unsigned int vm_create(unsigned int num_of_vcpu);
hvmm_state_t vm_init(unsigned int vmid);
hvmm_state_t vm_start(unsigned int vmid);
hvmm_state_t vm_delete(unsigned int vmid);
hvmm_state_t vm_suspend(unsigned int vmid);

//TODO(casionwoo): Implement later.
hvmm_state_t vm_resume(unsigned int vmid);
hvmm_state_t vm_shutdown(unsigned int vmid);

hvmm_state_t vm_save(unsigned int vmid);
hvmm_state_t vm_restore(unsigned int vmid);

struct vmcb* __vm_create();
hvmm_state_t __vm_delete(unsigned int vmid);
struct vmcb* __vm_get(unsigned int vmid);


#endif //__VM_H__

