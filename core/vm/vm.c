#include <vm.h>
#include <stdio.h>
#include <stdlib.h>

void print_vm(struct vmcb *vm);

static struct list_head vm_list;
static vmid_t vm_count;

hvmm_status_t vm_setup()
{
    INIT_LIST_HEAD(&vm_list);
    vm_count = 0;

    vcpu_setup();

    return HVMM_STATUS_SUCCESS;
}

vmid_t vm_create(unsigned char num_vcpus)
{
    int i;
    struct vmcb *vm = NULL;

    vm = malloc(sizeof(struct vmcb));
    if (vm == NULL) {
        return VM_CREATE_FAILED;
    }
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
    }

    // TODO(casionwoo) : vMEM create
    // TODO(casionwoo) : vIRQ create

    list_add_tail(&vm->head, &vm_list);

    return vm->vmid;
}

vmcb_state_t vm_init(vmid_t vmid)
{
    int i;
    struct vmcb *vm = vm_find(vmid);

    if (vm == NO_VM_FOUND) {
        return VM_NOT_EXISTED;
    }

    for (i = 0; i < vm->num_vcpus; i++) {
        if (vcpu_init(vm->vcpu[i]) != VCPU_REGISTERED) {
           return vm->state;
        }
    }

    vm->state = HALTED;

    // TODO(casionwoo) : vMEM init
    // TODO(casionwoo) : vIRQ init

    return vm->state;
}

vmcb_state_t vm_start(vmid_t vmid)
{
    int i;
    struct vmcb *vm = vm_find(vmid);

    if (vm == NO_VM_FOUND) {
        return VM_NOT_EXISTED;
    }

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

    list_del(&vm->head);
    free(vm);

    return UNDEFINED;
}

struct vmcb *vm_find(vmid_t vmid)
{
    struct vmcb *vm = NULL;

    list_for_each_entry(vm, &vm_list, head) {
        if (vm->vmid == vmid) {
            return vm;
        }
    }
    return NO_VM_FOUND;
}

void print_all_vm()
{
    struct vmcb *vm = NULL;

    list_for_each_entry(vm, &vm_list, head) {
        print_vm(vm);
    }
}

void print_vm(struct vmcb *vm)
{
    printf("ADDR  : 0x%p\n", vm);
    printf("VMID  : %d\n", vm->vmid);
    printf("STATE : %d\n", vm->state);
}

