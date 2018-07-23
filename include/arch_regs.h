#ifndef __ARCH_REGS_H__
#define __ARCH_REGS_H__

#include <stdint.h>
#include <types.h>
#include <arch/armv7.h>

/* moved from scheduler.h */
hvmm_status_t arch_regs_init(struct arch_regs *regs);
hvmm_status_t arch_regs_save(struct arch_regs *regs, struct core_regs *current_regs);
hvmm_status_t arch_regs_restore(struct arch_regs *regs, struct core_regs *current_regs);

void arch_regs_copy(struct arch_regs *from, struct arch_regs *to, struct core_regs *regs);

#endif
