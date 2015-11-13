#ifndef __CONTEXT_SWITCH_H__
#define __CONTEXT_SWITCH_H__

#include <vm.h>
#include <vcpu.h>
#include <hvmm_type.h>

//Check if absolutely needed
#include <vcpu_hw.h>

hvmm_state_t context_switch(unsigned int src_vcpuid, unsigned int dst_vcpuid, struct arch_regs *current_regs);

#endif //__CONTEXT_SWTICH_H__
