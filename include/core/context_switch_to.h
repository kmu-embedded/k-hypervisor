#ifndef __CONTEXT_SWITCH_TO_H__
#define __CONTEXT_SWITCH_TO_H__

#include <hvmm_types.h>
#include <core/vm/vcpu_regs.h>

hvmm_status_t context_switch_to(vcpuid_t from_id, vcpuid_t to_id, struct core_regs *current_core_regs);

#endif /* __CONTEXT_SWITCH_TO_H__ */
