#ifndef __KMUS_H__
#define __KMUS_H__

#include <core/vm/vcpu.h>
#include <core/vm/vm.h>

void kmus_start(vmid_t normal_id, vmid_t kmus_id, struct core_regs *regs);

#endif /* __KMUS_H__ */

