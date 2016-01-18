#include <context_switch_to.h>
#include <vm.h>
#include <vcpu.h>
#include <interrupt.h>

hvmm_status_t context_switch_to(vcpuid_t current_id, vcpuid_t next_id, struct core_regs *current_regs)
{
    struct vcpu *current = vcpu_find(current_id);
    struct vcpu *next = vcpu_find(next_id);

    vcpu_save(current, current_regs);
    if (current->vmid != next->vmid) {
        // sched_start 때는 save 필요 없음
        vm_save(current_id);
        // TODO(casionwoo) : vm_save, vm_restore two functions will be merged later.
        vm_restore(next_id);
    }
    vcpu_restore(next, current_regs);

    return HVMM_STATUS_SUCCESS;
}
