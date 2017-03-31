#include <core/kmus.h>

void kmus_start(vmid_t noraml_id, vmid_t kmus_id, struct core_regs *regs)
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

    return;
}
