#ifndef __VM_H__
#define __VM_H__

#include <lib/list.h>
#include "../types.h"
#include "vm/vcpu.h"
#include "vm/vmem.h"
#include "vm/vgic.h"

#define SIZE_OF_NAME        32
#define NO_VM_FOUND         NULL
#define VM_NOT_EXISTED      (-1)
#define VM_CREATE_FAILED    (-2)

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
    uint8_t num_vcpus;

    struct vcpu **vcpu;
    struct vmem vmem;
    struct vgic vgic;

    vmcb_state_t state;
    struct list_head head;
};

void vm_setup();
vmid_t vm_create(uint8_t num_vcpu);
vmcb_state_t vm_init(vmid_t vmid);
vmcb_state_t vm_start(vmid_t vmid);
vmcb_state_t vm_delete(vmid_t vmid);

// TODO(casionwoo) : vm_suspend, resume, shutdown
void vm_save(vmid_t vmid);
void vm_restore(vmid_t vmid);

struct vmcb *vm_find(vmid_t vmid);
void print_all_vm();

#endif /* __VM_H__ */

