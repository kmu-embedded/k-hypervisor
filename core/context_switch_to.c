#include <context_switch_to.h>
#include <vcpu.h>
#include <interrupt.h>
#include <memory.h>
#include <vdev.h>

extern struct vcpu *_current_guest[NUM_CPUS];
extern int _current_guest_vmid[NUM_CPUS];
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

       // TODO(casionwoo) : these 2 lines will be removed after vmem, virq is generated.
       _current_guest[0] = to_vcpu;
       _current_guest_vmid[0] = to_id;

        vdev_restore(to_id);
        interrupt_restore(to_id);
        memory_restore(to_id);
//    }

    vcpu_restore(to_vcpu, current_core_regs);

    return HVMM_STATUS_SUCCESS;
}
