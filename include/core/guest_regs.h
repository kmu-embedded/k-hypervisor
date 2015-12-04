#ifndef __GUEST_REGS_H__
#define __GUEST_REGS_H__

#define ARCH_REGS_NUM_GPR    13

struct arch_regs {
    uint32_t cpsr; /* CPSR */
    uint32_t pc; /* Program Counter */
    uint32_t lr;
    uint32_t gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));

#endif /* __GUEST_REGS_H__ */
