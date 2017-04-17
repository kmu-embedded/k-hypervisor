#include <core/kmus.h>
#include <stdio.h>
#include <drivers/gic-v2.h>
#include <arch/gicv2_bit.h>

static inline unsigned __get_cpsr(void)
{
    unsigned long retval;
    asm volatile (" mrs %0, cpsr" : "=r" (retval) :  );
    return retval;
}

static inline void __set_cpsr(unsigned val)
{
    asm volatile (" msr cpsr, %0":  :"r" (val));
}

void kmus_snapshot(vmid_t normal_id, vmid_t kmus_id, struct core_regs *regs)
{
    printf("%s start\n", __func__);

    int i, j;
    struct vcpu *vcpu = NULL;
    struct vmcb *vm = vm_find(normal_id);

    for (i = 0; i < vm->num_vcpus; i++) {
        vcpu = vm->vcpu[i];
        arch_regs_save(&vcpu->regs, regs);

        for (j = 0; j < GICv2.num_lr; j++)
            vcpu->lr[j] = GICH_READ(GICH_LR(j));

        vcpu->vmcr = GICH_READ(GICH_VMCR);
    }
    vm_copy(normal_id, kmus_id, regs);

    printf("%s end\n", __func__);
}

void kmus_start(vmid_t normal_id, vmid_t kmus_id, struct core_regs *regs)
{
    printf("%s start\n", __func__);
    unsigned long cpsr;

    // stop the VM (unregister from scheduler)
    vm_suspend(normal_id, regs);

    // change the type of back-up VM
    struct vmcb *vm = vm_find(kmus_id);
    vm->vcpu[0]->type = VCPU_NORMAL;

    // start the VM (register VM to scheduler)
    vm_start(kmus_id);

    // delete the VM struct (free the memory)
    vm_delete(normal_id);

    // enable irq for waiting next tick
    cpsr = __get_cpsr();
    cpsr &= (~CPSR_BIT(I));
    __set_cpsr(cpsr);

    printf("%s end\n", __func__);

    while(1);
    // Shoule be never reach here

    return;
}
