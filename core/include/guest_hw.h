#ifndef _GUEST_HW_H__
#define _GUEST_HW_H__

#include <arch/arm/rtsm-config.h>
//#include <tools/tests_hypervisor/tests.h>
#include <version.h>
#include <hvmm_trace.h>
#include <vgic.h>
#include <scheduler.h>
#include <guest_regs.h>

//#define ARCH_REGS_NUM_GPR    13

/* co-processor registers: cp15, cp2 */
struct regs_cop {
    uint32_t vbar;
    uint32_t ttbr0;
    uint32_t ttbr1;
    uint32_t ttbcr;
    uint32_t sctlr;
    uint32_t vmpidr;
};

/* banked registers */
struct regs_banked {
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
    /* Cortex-A15 processor does not support sp_fiq */
    /* uint32_t sp_fiq; */
    uint32_t lr_fiq;
    uint32_t r8_fiq;
    uint32_t r9_fiq;
    uint32_t r10_fiq;
    uint32_t r11_fiq;
    uint32_t r12_fiq;
};

/* Defines the architecture specific registers */
//struct arch_regs {
//    uint32_t cpsr; /* CPSR */
//    uint32_t pc; /* Program Counter */
//    uint32_t lr;
//    uint32_t gpr[ARCH_REGS_NUM_GPR]; /* R0 - R12 */
//} __attribute((packed));

/* Defines the architecture specific information, except general regsiters */
struct arch_context {
    struct regs_cop regs_cop;
    struct regs_banked regs_banked;
};

struct vcpu_regs {
    struct arch_context context;
    struct arch_regs regs;
};

/* moved from scheduler.h */
extern void __mon_switch_to_guest_context(struct arch_regs *regs);

hvmm_status_t guest_hw_move(struct vcpu_regs *dst, struct vcpu_regs *src);
hvmm_status_t guest_hw_dump(uint8_t verbose, struct arch_regs *regs);
hvmm_status_t guest_hw_init(struct vcpu_regs *vcpu_regs);
hvmm_status_t guest_hw_restore(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs);
hvmm_status_t guest_hw_save(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs);
#endif
