#include <vcpu_pool.h>

int get_vcpuid();

static struct vcpu vcpu_pool[MAX_VCPU_SIZE];
static bool vcpu_pool_state[MAX_VCPU_SIZE] = {FALSE, };

struct vcpu* vcpup_create()
{
    int index = 0;
    if((index = get_vcpuid()) == -1)
        return NULL;

    vcpu_pool[index].vcpuid = (unsigned int)index;
    vcpu_pool_state[index] = TRUE;

    return &vcpu_pool[index];
}

hvmm_state_t vcpup_delete(unsigned int vcpuid)
{
    if(vcpu_pool_state[vcpuid] == FALSE)
        return HVMM_STATE_FAIL;

    vcpu_pool_state[vcpuid] = FALSE;

    return HVMM_STATE_SUCCESS;
}

struct vcpu* vcpup_get(unsigned int vcpuid)
{
    if(vcpu_pool_state[vcpuid] == FALSE)
        return NULL;

    return &vcpu_pool[vcpuid];
}

int get_vcpuid()
{
    int i = 0;
    for(i = 0 ; i < MAX_VCPU_SIZE ; i++)
    {
        if(vcpu_pool_state[i] == FALSE)
            return i;
    }

    return -1;
}
unsigned int vcpup_available_vcpus()
{
    unsigned int available_vcpus = 0;
    int i = 0;
    for(i = 0 ; i < MAX_VCPU_SIZE ; i++)
    {
        if(vcpu_pool_state[i] == FALSE)
            available_vcpus++;
    }
    return available_vcpus;
}
