#ifndef __CONTEXT_SWITCH_TO_H__
#define __CONTEXT_SWITCH_TO_H__

#include "../types.h"
#include <arch_regs.h>

hvmm_status_t do_context_switch(vcpuid_t from_id, vcpuid_t to_id, struct core_regs *current_core_regs);

#endif /* __CONTEXT_SWITCH_TO_H__ */
