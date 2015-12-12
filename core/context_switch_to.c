#include <context_switch_to.h>
#include <vcpu.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>

hvmm_status_t context_switch_to(vcpuid_t from_id, vcpuid_t to_id, struct core_regs *current_core_regs)
{
    struct vcpu *from_vcpu = vcpu_find(from_id);
    struct vcpu *to_vcpu = vcpu_find(to_id);

    vcpu_save(from_vcpu, current_core_regs);

    // TODO(casionwoo) : commends will be enabled after vm is implemented
//    if (from_vcpu->vmid != to_vcpu->vmid) {
        memory_save();
        interrupt_save(from_id);
        vdev_save(from_id);

        vdev_restore(to_id);
        interrupt_restore(to_id);
        memory_restore(to_id);
//    }

    vcpu_restore(to_vcpu, current_core_regs);

    return HVMM_STATUS_SUCCESS;
}
