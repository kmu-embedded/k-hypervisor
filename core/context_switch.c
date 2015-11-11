#include <context_switch.h>

hvmm_state_t context_switch(unsigned int src_vcpuid, unsigned int dst_vcpuid, struct arch_regs *current_regs)
{
    if(vcpu_save(src_vcpuid, current_regs) != HVMM_STATE_SUCCESS)
       //Exception Handling 

    if(vcpu_restore(dst_vcpuid, current_regs) != HVMM_STATE_SUCCESS)
       //Exception Handling 

    if(vcpu_same_vmid(src_vcpuid, dst_vcpuid) == FALSE)
        return HVMM_STATE_SUCCESS;

    //vm_save & restore
    if(vm_save(src_vcpuid) != HVMM_STATE_SUCCESS)
       //Exception Handling 

    if(vm_restore(dst_vcpuid) != HVMM_STATE_SUCCESS)
       //Exception Handling 


    return HVMM_STATE_SUCCESS;
}

