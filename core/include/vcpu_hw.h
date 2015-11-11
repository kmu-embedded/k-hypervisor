#ifndef __VCPU_HW_H__
#define __VCPU_HW_H__

#include <armv7_p15.h>
#include <hvmm_type.h>

#define ARCH_REGS_NUM_GPR    13




/* co-processor registers: cp15, cp2 */
struct regs_cop {
    unsigned int vbar;
    unsigned int ttbr0;
    unsigned int ttbr1;
    unsigned int ttbcr;
    unsigned int sctlr;
};

/* banked registers */
struct regs_banked {
    unsigned int sp_usr;
    unsigned int spsr_svc;
    unsigned int sp_svc;
    unsigned int lr_svc;
    unsigned int spsr_abt;
    unsigned int sp_abt;
    unsigned int lr_abt;
    unsigned int spsr_und;
    unsigned int sp_und;
    unsigned int lr_und;
    unsigned int spsr_irq;
    unsigned int sp_irq;
    unsigned int lr_irq;

    unsigned int spsr_fiq;
    /* Cortex-A15 processor does not support sp_fiq */
    /* unsigned int sp_fiq; */
    unsigned int lr_fiq;
    unsigned int r8_fiq;
    unsigned int r9_fiq;
    unsigned int r10_fiq;
    unsigned int r11_fiq;
    unsigned int r12_fiq;
};

/* Defines the architecture specific registers */
struct arch_regs {
    unsigned int cpsr; /* CPSR */
    unsigned int pc; /* Program Counter */
    unsigned int lr;
    unsigned int gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
} __attribute((packed));

/* Defines the architecture specific information, except general regsiters */
struct arch_context {
    struct regs_cop regs_cop;
    struct regs_banked regs_banked;
};

struct vcpu_regs{
    struct arch_regs regs;
    struct arch_context context;
    unsigned int vmpidr;
};

char *_modename(unsigned char mode);

hvmm_state_t vcpu_hw_save(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs);
hvmm_state_t vcpu_hw_restore(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs);
hvmm_state_t vcpu_hw_init(struct vcpu_regs *vcpu_regs, struct arch_regs *regs);
hvmm_state_t vcpu_hw_move(struct vcpu_regs *dst_regs, struct vcpu_regs *src_regs);

#endif //__VCPU_HW_H__
