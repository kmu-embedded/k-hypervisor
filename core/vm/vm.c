#include <vm.h>

static void print_vm(struct vmcb *vm);

static struct list_head list;
static vmid_t vmid_count;

int vm_setup()
{
    INIT_LIST_HEAD(&list);
    vmid_count = 0;

    return 0;
}

vmid_t vm_create(unsigned char num_vcpus)
{
    struct vmcb *vm = NULL;

    vm = malloc(sizeof(struct vmcb));
    if (vm == NULL) {
        return VM_CREATE_FAILED;
    }
    vm->vmid = vmid_count++;
    vm->state = DEFINED;
    vm->num_vcpus = num_vcpus;

    // TODO(casionwoo) : vCPU create
    // TODO(casionwoo) : vMEM create
    // TODO(casionwoo) : vIRQ create

    list_add_tail(&vm->head, &list);

    return vm->vmid;
}

vmcb_state_t vm_delete(vmid_t vmid)
{
    struct vmcb *vm = vm_find(vmid);
    if (vm == NULL) {
        return VM_NOT_EXISTED;
    }

    // TODO(casionwoo) : vCPU delete

    list_del(&vm->head);
    free(vm);

    return UNDEFINED;
}

vmcb_state_t vm_init(vmid_t vmid)
{
    struct vmcb *vm = vm_find(vmid);

    if (vm == NULL) {
        return VM_NOT_EXISTED;
    }

    vm->state = HALTED;

    // TODO(casionwoo) : vCPU init
    // TODO(casionwoo) : vMEM init
    // TODO(casionwoo) : vIRQ init

    return vm->state;
}

vmcb_state_t vm_start(vmid_t vmid)
{
    struct vmcb *vm = vm_find(vmid);

    if (vm == NULL) {
        return VM_NOT_EXISTED;
    }
    vm->state = RUNNING;

    // TODO(casionwoo) : vCPU start

    return vm->state;
}

struct vmcb *vm_find(vmid_t vmid)
{
    struct vmcb *vm = NULL;

    list_for_each_entry(vm, &list, head) {
        if (vm->vmid == vmid) {
            return vm;
        }
    }
    return NULL;
}

void print_all_vm()
{
    struct vmcb *vm = NULL;

    list_for_each_entry(vm, &list, head) {
        print_vm(vm);
    }
}

void print_vm(struct vmcb *vm)
{
    printf("ADDR  : 0x%p\n", vm);
    printf("VMID  : %d\n", vm->vmid);
    printf("STATE : %d\n", vm->state);
}

