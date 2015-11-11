#include <vm.h>



unsigned int vm_create(unsigned int num_of_vcpu)
{
    int i = 0;
    struct vmcb *pvmcb = NULL;
    struct vcpu *vcpu = NULL;

    //Check if # of available vcpu is less than request
    if(vcpu_isAvailableCreate(num_of_vcpu) == FALSE)
        return HVMM_STATE_FAIL;

    pvmcb = vmp_create();
    if(pvmcb == NULL)
        return HVMM_STATE_FAIL;

    pvmcb->vm_state = VMCB_DEFINED ;
    pvmcb->num_of_vcpu = num_of_vcpu;

    //vCPU_CREATE
    for(i = 0 ; i < pvmcb->num_of_vcpu ; i++)
    {
        vcpu = vcpu_create(pvmcb->vmid);
        if(vcpu == NULL)
        {
            //Exception Handling.    
            return -1;
        }
        pvmcb->vcpu[i] = vcpu;
    }

    //vMEM_CREATE
    //vIRQ_CREATE

    return pvmcb->vmid;
}

hvmm_state_t vm_init(unsigned int vmid)
{
    int i = 0;
    struct vmcb *pvmcb = NULL;
    struct vcpu *vcpu = NULL;

    pvmcb = vmp_get(vmid);
    if(pvmcb == NULL)
        return HVMM_STATE_FAIL;

    pvmcb->vm_state = VMCB_HALTED;

    //vCPU_INIT
    for(i = 0 ; i < pvmcb->num_of_vcpu ; i++)
    {
        vcpu = pvmcb->vcpu[i];
        if(vcpu_init(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            //Exception Handling.    
            return HVMM_STATE_FAIL;
        }

    }

    //vMEM_INIT
    //vIRQ_INIT

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_start(unsigned int vmid)
{
    int i = 0;
    struct vmcb *pvmcb = NULL;    
    struct vcpu *vcpu = NULL;

    pvmcb = vmp_get(vmid);
    if(pvmcb == NULL)
        return HVMM_STATE_FAIL;

    pvmcb->vm_state = VMCB_RUNNING;

    for(i = 0 ; i < pvmcb->num_of_vcpu ; i++)
    {
        vcpu = pvmcb->vcpu[i];
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
    struct vmcb *pvmcb = NULL;
    struct vcpu *vcpu = NULL;

    pvmcb = vmp_get(vmid);
    if(pvmcb == NULL)
        return HVMM_STATE_FAIL;

    pvmcb->vm_state = VMCB_UNDEFINED;

    for(i = 0 ; i < pvmcb->num_of_vcpu ; i++)
    {
        vcpu = pvmcb->vcpu[i];
        if(vcpu_delete(vcpu->vcpuid) == HVMM_STATE_FAIL)
        {
            return HVMM_STATE_FAIL;
        }
    }
    
    if(vmp_delete(vmid) == HVMM_STATE_FAIL)
    {
        //Exception Handling
        return HVMM_STATE_FAIL;
    }

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_suspend(unsigned int vmid)
{
    int i = 0;
    struct vmcb *pvmcb = NULL;
    struct vcpu *vcpu = NULL;

    pvmcb = vmp_get(vmid);
    if(pvmcb == NULL)
        return HVMM_STATE_FAIL;

    pvmcb->vm_state = VMCB_SUSPENDED;

    for(i = 0 ; i < pvmcb->num_of_vcpu ; i++)
    {
        vcpu = pvmcb->vcpu[i];
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
    //vmem save
    //virq save
    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vm_restore(unsigned int vmid)
{
    //vmem restore
    //virq restore
    return HVMM_STATE_SUCCESS;
}

