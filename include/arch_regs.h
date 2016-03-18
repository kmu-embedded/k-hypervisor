#ifndef __ARCH_REGS_H__
#define __ARCH_REGS_H__

#include <stdint.h>
#include "types.h"

#define NR_ARCH_GPR_REGS    13

/* co-processor registers: cp15, cp2 */
struct cp15 {
    uint32_t vbar;
    uint32_t ttbr0;
    uint32_t ttbr1;
    uint32_t ttbcr;
    uint32_t sctlr;
    uint32_t vmpidr;
};

/* banked registers */
struct banked_regs {
    uint32_t sp_usr;

    uint32_t spsr_svc;
    uint32_t sp_svc;
    uint32_t lr_svc;
    uint32_t spsr_abt;
    uint32_t sp_abt;
    uint32_t lr_abt;
    uint32_t spsr_und;
    uint32_t sp_und;
    uint32_t lr_und;
    uint32_t spsr_irq;
    uint32_t sp_irq;
    uint32_t lr_irq;

    uint32_t spsr_fiq;
    /* unsupported sp_fiq, uint32_t sp_fiq; */
    uint32_t lr_fiq;
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
};

/* Defines the architecture core registers */
struct core_regs {
    uint32_t cpsr; 	/* CPSR */
    uint32_t pc; 	/* Program Counter */
    uint32_t lr;
    uint32_t gpr[NR_ARCH_GPR_REGS]; /* R0 - R12 */
} __attribute((packed));

struct arch_regs {
    struct core_regs core_regs;
    struct cp15 cp15;
    struct banked_regs banked_regs;
};

/* moved from scheduler.h */
hvmm_status_t arch_regs_init(struct arch_regs *regs);
hvmm_status_t arch_regs_save(struct arch_regs *regs, struct core_regs *current_regs);
hvmm_status_t arch_regs_restore(struct arch_regs *regs, struct core_regs *current_regs);

#endif
