#ifndef __VM_H__
#define __VM_H__

#include <vm_pool.h>
#include <vcpu.h>
#include <hvmm_type.h>

#define SIZE_OF_NAME    32
#define VCPU_MAX_SIZE   4

struct vmcb {
    unsigned int vmid;
    char name[SIZE_OF_NAME];

    /****************************************
    *
    *   vcpu : Virtualized CPU
    *       - VPIDR (Virtualization Processor ID Register : B4.1.158)

    *
    ****************************************/

    //TODO(casionwoo): pointer or statically
    unsigned int num_of_vcpu;
    struct vcpu *vcpu[VCPU_MAX_SIZE];

    /**************************************
    *
    *   vmem : It's a struct for memory vm has
    *       - Physical memory mapping
    *       - VTTBR
    *       - VTCR (Virtualization Translation Control Register : B4.1.159)
    *
    ***************************************/
    //struct vmem vmem;

    /***************************************
    *
    *   virq : It's a struct for interrupt vm has
    *       - vgic
    *
    ***************************************/
    //struct virq virq;

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



#endif //__VM_H__

