#include <core/vm/vm.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <atags.h>

void print_vm(struct vmcb *vm);

static struct list_head vm_list;
static vmid_t vm_count;

void vm_setup()
{
    vm_count = 0;
    LIST_INITHEAD(&vm_list);

    vcpu_setup();
#ifdef CONFIG_ATAGS
    atags_setup();
#endif
}

vmid_t vm_create(uint8_t num_vcpus)
{
    int i;
    struct vmcb *vm = NULL;

    vm = malloc(sizeof(struct vmcb));
    if (vm == NULL) {
        return VM_CREATE_FAILED;
    }
    memset(vm, 0, sizeof(struct vmcb));

    vm->vmid = vm_count++;
    vm->state = DEFINED;
    vm->num_vcpus = num_vcpus;

    vm->vcpu = malloc(sizeof(struct vcpu *) * vm->num_vcpus);
    if (vm->vcpu == NULL) {
        return VM_CREATE_FAILED;
    }

    for (i = 0; i < vm->num_vcpus; i++) {
        if ((vm->vcpu[i] = vcpu_create()) == VCPU_CREATE_FAILED) {
            free(vm);
            return VM_CREATE_FAILED;
        }
        vm->vcpu[i]->vmid = vm->vmid;
        vm->vcpu[i]->id = i;
    }

    vmem_create(&vm->vmem, vm->vmid);
    vdev_create(&vm->vdevs, vm->vmid);

    LIST_ADDTAIL(&vm->head, &vm_list);

    return vm->vmid;
}

#include <drivers/gic-v2.h>
#include <arch/gic_regs.h>

vmcb_state_t vm_init(vmid_t vmid)
{
    struct vmcb *vm = vm_find(vmid);
    if (vm == NO_VM_FOUND) {
        return VM_NOT_EXISTED;
    }

    int i;
    for (i = 0; i < vm->num_vcpus; i++) {
        if (vcpu_init(vm->vcpu[i]) != VCPU_REGISTERED) {
            return vm->state;
        }
    }

    vm->state = HALTED;

    vmem_init(&vm->vmem);

    return vm->state;
}

vmcb_state_t vm_start(vmid_t vmid)
{
    struct vmcb *vm = vm_find(vmid);
    if (vm == NO_VM_FOUND) {
        return VM_NOT_EXISTED;
    }

    int i;
    for (i = 0; i < vm->num_vcpus; i++) {
        if (vcpu_start(vm->vcpu[i]) != VCPU_ACTIVATED) {
            return vm->state;
        }
    }

    vm->state = RUNNING;

    return vm->state;
}

vmcb_state_t vm_delete(vmid_t vmid)
{
    int i;
    struct vmcb *vm = vm_find(vmid);

    if (vm == NO_VM_FOUND) {
        return VM_NOT_EXISTED;
    }

    for (i = 0; i < vm->num_vcpus; i++) {
        if (vcpu_delete(vm->vcpu[i]) != VCPU_UNDEFINED) {
            return vm->state;
        }
    }

    LIST_DEL(&vm->head);
    free(vm);

    return UNDEFINED;
}

void vm_save(vmid_t save_vmid)
{
    struct vmcb *vm = vm_find(save_vmid);
    if (vm == NO_VM_FOUND) {
        debug_print("[%s]: NO VM FOUND %d\n", __func__, save_vmid);
    }

    vmem_save();
}

void vm_restore(vmid_t restore_vmid)
{
    struct vmcb *vm = vm_find(restore_vmid);
    if (vm == NO_VM_FOUND) {
        debug_print("[%s]: NO VM FOUND %d\n", __func__, restore_vmid);
    }

    vmem_restore(&vm->vmem);
}

struct vmcb *vm_find(vmid_t vmid)
{
    struct vmcb *vm = NULL;
    list_for_each_entry(struct vmcb, vm, &vm_list, head) {
        if (vm->vmid == vmid) {
            return vm;
        }
    }
    return NO_VM_FOUND;
}

void print_all_vm()
{
    struct vmcb *vm = NULL;
    list_for_each_entry(struct vmcb, vm, &vm_list, head) {
        print_vm(vm);
    }
}

void print_vm(struct vmcb *vm)
{
    debug_print("ADDR  : 0x%p\n", vm);
    debug_print("VMID  : %d\n", vm->vmid);
    debug_print("STATE : %d\n", vm->state);
}

