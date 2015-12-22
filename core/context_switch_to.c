#include <context_switch_to.h>
#include <vm.h>
#include <vcpu.h>
#include <interrupt.h>

extern struct vcpu *_current_guest[NUM_CPUS];
extern int _current_guest_vmid[NUM_CPUS];

hvmm_status_t context_switch_to(vcpuid_t current_id, vcpuid_t next_id, struct core_regs *current_regs)
{
    struct vcpu *current = vcpu_find(current_id);
    struct vcpu *next = vcpu_find(next_id);

    vcpu_save(current, current_regs);
    if (current->vmid != next->vmid) {
        vm_save(current_id);
        interrupt_save(current_id);

        // TODO(casionwoo) : vm_save, vm_restore two functions will be merged later.
        // TODO(casionwoo) : After interrupt is modified, below 2 lines will be removed.
        _current_guest[0] = next;
        _current_guest_vmid[0] = next_id;

        interrupt_restore(next_id);
        vm_restore(next_id);
    }
    vcpu_restore(next, current_regs);

    return HVMM_STATUS_SUCCESS;
}
