#include <core/kmus.h>
#include <stdio.h>

void kmus_start(vmid_t normal_id, vmid_t kmus_id, struct core_regs *regs)
{
    /*
        TODO(casionwoo) for Kmus:
            1. copy VMCB and vCPU from normal VM to kmus VM
            2. delete VMCB and vCPU
                2.1 when delete vCPU this function should call detach from scheduler function and in here,
                current variable should be assigned as NULL
            3. if normal VM is current scheduled VM, schedule mechanism should be called again
               for return from hyp mode to guestos mode
    */

    // stop the VM (unregister from scheduler)
    vm_suspend(normal_id, regs);

    // copy all of the VM
    vm_copy(normal_id, kmus_id, regs);

    // start the VM (register VM to scheduler)
    vm_start(kmus_id);

    // delete the VM struct (free the memory)
//    vm_delete(normal_id);


    return;
}
