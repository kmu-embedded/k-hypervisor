#include <vcpu.h>

struct vcpu* vcpu_create(unsigned int vmid)
{
    struct vcpu *pvcpu = NULL;
    if((pvcpu = vcpup_create()) == NULL){
       //Exception Handling

       return NULL;
    }

    pvcpu->vmid = vmid;
        
    return pvcpu;
}

hvmm_state_t vcpu_init(unsigned int vcpuid)
{
    struct vcpu *pvcpu = NULL;
    struct vcpu_regs *pvcpu_regs = NULL;
    struct arch_regs *regs = NULL;

    if((pvcpu = vcpup_get(vcpuid)) == NULL){
        //Exception Handling
        return HVMM_STATE_FAIL;
    }

    pvcpu_regs = &(pvcpu->vcpu_regs);
    regs = &(pvcpu_regs->regs);
    if(vcpu_hw_init(pvcpu_regs, regs) != HVMM_STATE_SUCCESS){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    //register vcpu id to scheduler

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_start(unsigned int vcpuid)
{
    struct vcpu *pvcpu = NULL;
    if((pvcpu = vcpup_get(vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    //signal scheduler to start the vcpu

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_delete(unsigned int vcpuid)
{ 
    //signal scheduler to stop the vcpu
    //unregister vcpu id from scheduler

    if(vcpup_delete(vcpuid) == HVMM_STATE_FAIL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}


hvmm_state_t vcpu_suspend(unsigned int vcpuid)
{
    //signal scheduler to stop the vcpu

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_save(unsigned int vcpuid, struct arch_regs *current_regs)
{struct vcpu *pvcpu = NULL;
    if((pvcpu = vcpup_get(vcpuid)) == NULL){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    if(vcpu_hw_save(&(pvcpu->vcpu_regs), current_regs) != HVMM_STATE_SUCCESS){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_restore(unsigned int vcpuid, struct arch_regs *current_regs)
{
    struct vcpu *pvcpu = NULL;
    if((pvcpu = vcpup_get(vcpuid)) == NULL){
        //Exception Handling
        
        return HVMM_STATE_FAIL;
    }

    if(vcpu_hw_restore(&(pvcpu->vcpu_regs), current_regs) != HVMM_STATE_SUCCESS){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

bool vcpu_same_vmid(unsigned int first_vcpuid, unsigned int second_vcpuid)
{
    struct vcpu *first_pvcpu = NULL;
    struct vcpu *second_pvcpu = NULL;

    if((first_pvcpu = vcpup_get(first_vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    if((second_pvcpu = vcpup_get(second_vcpuid)) == NULL){
        //Exception Handling

        return HVMM_STATE_FAIL;
    }

    if(first_pvcpu->vmid == second_pvcpu->vmid)
        return TRUE;

    return FALSE;
}

bool vcpu_isAvailableCreate(unsigned int num_of_vcpu)
{
    unsigned int available_vcpus = 0;
   
    available_vcpus = vcpup_available_vcpus();
    if((available_vcpus >= num_of_vcpu) || (num_of_vcpu > 0))
        return TRUE;
    return FALSE;
}
