#include <core/context_switch.h>
#include <core/vm/vm.h>
#include <core/vm/vcpu.h>
#include <types.h>
#include <stdio.h>

hvmm_status_t do_context_switch(vcpuid_t current_id, vcpuid_t next_id, struct core_regs *current_regs)
{
    struct vcpu *current = vcpu_find(current_id);
    struct vcpu *next = vcpu_find(next_id);

    if (current_id == VCPUID_INVALID) {
        vm_restore(next->vmid);
        vcpu_restore(next, current_regs);
        return HVMM_STATUS_SUCCESS;
    }

    if (current->vmid != next->vmid) {
        vcpu_save(current, current_regs);
        vm_save(current->vmid);
        // TODO(casionwoo) : vm_save, vm_restore two functions will be merged later.
        vm_restore(next->vmid);
        vcpu_restore(next, current_regs);
    }
    return HVMM_STATUS_SUCCESS;
}
