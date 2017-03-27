#ifndef __ARMV7_H__
#define __ARMV7_H__

#include <asm/asm.h>

#include "armv7/cache.h"
#include "armv7/tlb.h"
#include "armv7/sysctrl.h"
#include "armv7/vmsa.h"
#include "armv7/generic_timer.h"
#include "armv7/smp.h"
#include "armv7/mutex.h"
#include "armv7/cp15.h"
#include "armv7/barrier.h"
#include "armv7/fsr_decode.h"
#include "armv7/local_irq.h"
#include "armv7/hsr.h"

/* co-processor registers: cp15, cp2 */
struct cp15 {
    uint32_t vbar;
    uint32_t ttbr0;
    uint32_t ttbr1;
    uint32_t ttbcr;
    uint32_t sctlr;
    uint32_t vmpidr;

    uint32_t csselr;
    uint32_t cpacr;

    uint32_t contextidr;
    uint32_t tpidrurw;
    uint32_t tpidruro;
    uint32_t tpidrprw;

    uint32_t cntkctl;

    uint32_t dacr;
    uint32_t par;

    uint32_t mair0;
    uint32_t mair1;
    uint32_t amair0;
    uint32_t amair1;

    uint32_t dfar;
    uint32_t ifar;
    uint32_t dfsr;
    uint32_t ifsr;

    uint32_t adfsr;
    uint32_t aifsr;
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

#define NR_ARCH_GPR_REGS    13

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

int handle_data_abort(struct core_regs *regs, uint32_t iss);

#endif /* armv7.h */
