#ifndef __VM_H__
#define __VM_H__

#include <list.h>
#include <vcpu.h>
#include <vmem.h>
#include <virq.h>
#include <hvmm_types.h>

#define SIZE_OF_NAME        32
#define NO_VM_FOUND         NULL
#define VM_NOT_EXISTED      -1
#define VM_CREATE_FAILED    -2

typedef enum vmcb_state {
    UNDEFINED,
    DEFINED,
    HALTED,
    RUNNING,
    SUSPENDED
} vmcb_state_t;

struct vmcb {
    vmid_t vmid;
    char name[SIZE_OF_NAME];
    unsigned char num_vcpus;

    struct vcpu **vcpu;
    struct vmem vmem;
    // TODO(casionwoo) : Define vIRQ

    struct virq virq;

    vmcb_state_t state;
    struct list_head head;
};

hvmm_status_t vm_setup();
vmid_t vm_create(unsigned char num_vcpu);
vmcb_state_t vm_init(vmid_t vmid);
vmcb_state_t vm_start(vmid_t vmid);
vmcb_state_t vm_delete(vmid_t vmid);

// TODO(casionwoo) : vm_suspend, resume, shutdown
hvmm_status_t vm_save(vmid_t vmid);
hvmm_status_t vm_restore(vmid_t vmid);

struct vmcb *vm_find(vmid_t vmid);
void print_all_vm();

#endif /* __VM_H__ */

