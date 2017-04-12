#include <core/kmus.h>
#include <stdio.h>

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

void kmus_start(vmid_t normal_id, vmid_t kmus_id, struct core_regs *regs)
{
    unsigned long cpsr;

    // stop the VM (unregister from scheduler)
    vm_suspend(normal_id, regs);

    // copy all of the VM
    vm_copy(normal_id, kmus_id, regs);

    // change the type of back-up VM
    struct vmcb *vm = vm_find(kmus_id);
    vm->vcpu[0]->type = VCPU_NORMAL;

    // start the VM (register VM to scheduler)
    vm_start(kmus_id);

    // delete the VM struct (free the memory)
    vm_delete(normal_id);

    // enable irq for waiting next tick
    cpsr = __get_cpsr();
    printf("CPSR IRQ bit[%x]\n", cpsr );
    cpsr &= (~CPSR_BIT(I));
    __set_cpsr(cpsr);
    printf("CPSR IRQ bit[%x]\n", cpsr);

    while(1);
    // Shoule be never reach here

    return;
}
