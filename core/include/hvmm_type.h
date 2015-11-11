#ifndef __HVMM_TYPE_H__
#define __HVMM_TYPE_H__

#define NULL (void *) 0

typedef enum {
    TRUE  = 1,
    FALSE = 0
}bool;

typedef enum vmcb_state{
    VMCB_UNDEFINED   = 0,
    VMCB_DEFINED     = 1,
    VMCB_HALTED      = 2,
    VMCB_RUNNING     = 3,
    VMCB_SUSPENDED   = 4
}vmcb_state_t;

//Scheduler will control vcpu_state
/*
typedef enum vcpu_state{
    VCPU_TERMINATED  = 0,
    VCPU_REGISTERED  = 1,
    VCPU_PENDING     = 2,
    VCPU_RUNNING     = 3
}vcpu_state_t;
*/

typedef enum hvmm_state{
    HVMM_STATE_SUCCESS              =  0,
    HVMM_STATE_UNKNOWN_EEOR         = -1,
    HVMM_STATE_UNSUPPRTED_FEATURE   = -2,
    HVMM_STATE_BUSY                 = -3,
    HVMM_STATE_BAD_ACCESS           = -4,
    HVMM_STATE_NOT_FOUND            = -5,
    HVMM_STATE_IGNORED              = -6,
    HVMM_STATE_FAIL                 = -7
}hvmm_state_t;



#endif //__HVMM_TYPE_H__
