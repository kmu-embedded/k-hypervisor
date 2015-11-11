#include <vm_pool.h>

static struct vmcb vm_pool[MAX_VM_SIZE];
static bool vm_pool_state[MAX_VM_SIZE]={FALSE,};

int get_vmid();

struct vmcb* vmp_create()
{
    int index = 0;
    if((index = get_vmid()) == -1)
        return NULL;

    vm_pool[index].vmid = (unsigned int)index;
    vm_pool_state[index] = TRUE;

    return &vm_pool[index];
}

hvmm_state_t vmp_delete(unsigned int vmid)
{
    if(vm_pool_state[vmid] == FALSE)
        return HVMM_STATE_FAIL;

    vm_pool_state[vmid] = FALSE; 
    return HVMM_STATE_SUCCESS;
}

struct vmcb* vmp_get(unsigned int vmid)
{
    if(vm_pool_state[vmid] == FALSE)
        return NULL;

    return &vm_pool[vmid];
}

int get_vmid()
{
    int i = 0;
    for(i = 0 ; i < MAX_VM_SIZE ; i++)
    {
        if(vm_pool_state[i] == FALSE)
            return i;
    }
    return -1;
}

