#include <vcpu.h>

int __get_vcpuid();
struct vcpu* __vcpu_create();                                                                 
hvmm_state_t __vcpu_delete(unsigned int vcpuid);
struct vcpu* __vcpu_get(unsigned int vcpuid);
unsigned int __vcpu_available_vcpus();

static struct vcpu vcpu_pool[MAX_VCPU_SIZE];
static bool vcpu_pool_state[MAX_VCPU_SIZE] = {FALSE, };

struct vcpu *vcpu_create(unsigned int vmid)
{
    struct vcpu *vcpu = NULL;

    if((vcpu = __vcpu_create()) == NULL){
       //Exception Handling

       return NULL;
    }

    vcpu->vmid = vmid;
        
    return vcpu;
}

hvmm_state_t vcpu_init(unsigned int vcpuid)
{
    struct vcpu *vcpu = NULL;
    struct vcpu_regs *vcpu_regs = NULL;
    struct arch_regs *regs = NULL;

    if((vcpu = __vcpu_get(vcpuid)) == NULL){
        //Exception Handling
        return HVMM_STATE_FAIL;
    }

    vcpu_regs = &(vcpu->vcpu_regs);
    regs = &(vcpu_regs->regs);
    if(vcpu_hw_init(vcpu_regs, regs) != HVMM_STATE_SUCCESS){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }
    // TODO(casionwoo) : Register vcpu id to scheduler

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_start(unsigned int vcpuid)
{
    struct vcpu *vcpu = NULL;

    if((vcpu = __vcpu_get(vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }
    // TODO(casionwoo) : Signal scheduler to start the vcpu

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_delete(unsigned int vcpuid)
{ 
    //TODO(casionwoo) : Signal scheduler to stop the vcpu, Unregister vcpu id from scheduler

    if(__vcpu_delete(vcpuid) == HVMM_STATE_FAIL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}


hvmm_state_t vcpu_suspend(unsigned int vcpuid)
{
    //TODO(casionwoo) : Signal scheduler to stop the vcpu

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_save(unsigned int vcpuid, struct arch_regs *current_regs)
{
    struct vcpu *vcpu = NULL;
    
    if((vcpu = __vcpu_get(vcpuid)) == NULL){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    if(vcpu_hw_save(&(vcpu->vcpu_regs), current_regs) != HVMM_STATE_SUCCESS){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_restore(unsigned int vcpuid, struct arch_regs *current_regs)
{
    struct vcpu *vcpu = NULL;

    if((vcpu = __vcpu_get(vcpuid)) == NULL){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    if(vcpu_hw_restore(&(vcpu->vcpu_regs), current_regs) != HVMM_STATE_SUCCESS){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

bool vcpu_same_vmid(unsigned int first_vcpuid, unsigned int second_vcpuid)
{
    struct vcpu *first_vcpu = NULL;
    struct vcpu *second_vcpu = NULL;

    if((first_vcpu = __vcpu_get(first_vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    if((second_vcpu = __vcpu_get(second_vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    if(first_vcpu->vmid == second_vcpu->vmid)
        return TRUE;

    return FALSE;
}

bool vcpu_is_available_create(unsigned int num_vcpu)
{
    unsigned int available_vcpus = 0;
   
    available_vcpus = __vcpu_available_vcpus();
    if((available_vcpus >= num_vcpu) || (num_vcpu > 0))
        return TRUE;
    return FALSE;
}

struct vcpu* __vcpu_create()
{
    int index = 0;
    if((index = __get_vcpuid()) == HVMM_STATE_FAIL)
        return NULL;

    vcpu_pool[index].vcpuid = (unsigned int)index;
    vcpu_pool_state[index] = TRUE;

    return &vcpu_pool[index];
}

hvmm_state_t __vcpu_delete(unsigned int vcpuid)
{
    if(vcpu_pool_state[vcpuid] == FALSE)
        return HVMM_STATE_FAIL;

    vcpu_pool_state[vcpuid] = FALSE;

    return HVMM_STATE_SUCCESS;
}

struct vcpu* __vcpu_get(unsigned int vcpuid)
{
    if(vcpu_pool_state[vcpuid] == FALSE)
        return NULL;

    return &vcpu_pool[vcpuid];
}

int __get_vcpuid()
{
    int i = 0;
    for(i = 0 ; i < MAX_VCPU_SIZE ; i++)
    {
        if(vcpu_pool_state[i] == FALSE)
            return i;
    }

    return HVMM_STATE_FAIL;;
}
unsigned int __vcpu_available_vcpus()
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
