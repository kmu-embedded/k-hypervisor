#include <vm.h>

int __get_vmid();
struct vmcb* __vm_alloc();
struct vmcb* __vm_create();
hvmm_state_t __vm_delete(unsigned int vmid);
struct vmcb* __vm_get(unsigned int vmid);

static struct vmcb vm[MAX_VM_SIZE];
static bool vm_state[MAX_VM_SIZE]={FALSE,};
static int remain_vms = MAX_VM_SIZE;


// TODO(casionwoo) : add parameter for memory and irq
int vm_create(unsigned int num_vcpu)
{
    struct vmcb *vmcb = NULL;
    struct vcpu *vcpu = NULL;
    int i;

    if(vcpu_is_available(num_vcpu) == FALSE) {
        return VMCB_UNDEFINED;
    }

    vmcb = __vm_alloc();
    if(vmcb == NULL) {
        return VMCB_UNDEFINED;
    }

    for (i = 0; i < num_vcpu; i++) {
        vcpu = vcpu_create();
        vcpu->vmid = vmcb->vmid;
        vmcb->vcpu[i] = vcpu;
        vmcb->num_of_vcpu++;
    }

    //TODO(casionwoo) : vmem_creaet()
    //TODO(casionwoo) : virq_create()

    vmcb->state = VMCB_DEFINED;

    return vmcb->vmid;
}

hvmm_state_t vm_init(unsigned int vmid)
{
    int i = 0;
    struct vmcb *vm = NULL;
    struct vcpu *vcpu = NULL;

    vm = __vm_get(vmid);
    if(vm == NULL)
    {
        return HVMM_STATE_FAIL;
    }

    vm->state = VMCB_HALTED;

    // vCPU_INIT
    for(i = 0; i < vm->num_of_vcpu; i++)
    {
        vcpu = vm->vcpu[i];
        if(vcpu_init(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            //Exception Handling.
            return HVMM_STATE_FAIL;
        }
    }

    // TODO(casionwoo) : vMEM_INIT, vIRQ_INIT

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_start(unsigned int vmid)
{
    int i = 0;
    struct vmcb *vm = NULL;
    struct vcpu *vcpu = NULL;

    if((vm = __vm_get(vmid)) == NULL)
    {
        return HVMM_STATE_FAIL;
    }

    vm->state = VMCB_RUNNING;

    for(i = 0; i < vm->num_of_vcpu; i++)
    {
        vcpu = vm->vcpu[i];
        if(vcpu_start(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            return HVMM_STATE_FAIL;
        }
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_delete(unsigned int vmid)
{
    int i = 0;
    struct vmcb *vm = NULL;
    struct vcpu *vcpu = NULL;

    if((vm = __vm_get(vmid)) == NULL)
    {
        return HVMM_STATE_FAIL;
    }

    vm->state = VMCB_UNDEFINED;

    for(i = 0; i < vm->num_of_vcpu; i++)
    {
        vcpu = vm->vcpu[i];
        if(vcpu_delete(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            return HVMM_STATE_FAIL;
        }
    }

    if(__vm_delete(vmid) == HVMM_STATE_FAIL)
    {
        //Exception Handling
        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_suspend(unsigned int vmid)
{
    int i = 0;
    struct vmcb *vm = NULL;
    struct vcpu *vcpu = NULL;

    if((vm = __vm_get(vmid)) == NULL)
    {
        return HVMM_STATE_FAIL;
    }

    vm->state = VMCB_SUSPENDED;

    for(i = 0; i < vm->num_of_vcpu; i++)
    {
        vcpu = vm->vcpu[i];
        if(vcpu_suspend(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            return HVMM_STATE_FAIL;
        }
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_resume(unsigned int vmid)
{
    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_shutdown(unsigned int vmid)
{
    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_save(unsigned int vmid)
{
    // TODO(casionwoo) : vmem save
    // TODO(casionwoo) : virq save
    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_restore(unsigned int vmid)
{
    // TODO(casionwoo) : vmem restore
    // TODO(casionwoo) : virq restore
    return HVMM_STATE_SUCCESS;
}

struct vmcb* __vm_alloc()
{
    int index = 0;
    if((index = __get_vmid()) == HVMM_STATE_FAIL)
        return NULL;

    vm[index].vmid = (unsigned int)index;
    vm_state[index] = TRUE;
    remain_vms--;

    return &vm[index];
}

struct vmcb* __vm_create()
{
    int index = 0;
    if((index = __get_vmid()) == HVMM_STATE_FAIL)
        return NULL;

    vm[index].vmid = (unsigned int)index;
    vm_state[index] = TRUE;

    return &vm[index];
}

hvmm_state_t __vm_delete(unsigned int vmid)
{
    if(vm_state[vmid] == FALSE)
        return HVMM_STATE_FAIL;

    vm_state[vmid] = FALSE;
    return HVMM_STATE_SUCCESS;
}

struct vmcb* __vm_get(unsigned int vmid)
{
    if(vm_state[vmid] == FALSE)
        return NULL;

    return &vm[vmid];
}

int __get_vmid()
{
    int i = 0;
    for(i = 0 ; i < MAX_VM_SIZE ; i++)
    {
        if(vm_state[i] == FALSE)
            return i;
    }
    return HVMM_STATE_FAIL;
}

