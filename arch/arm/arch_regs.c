#include <arch_regs.h>
#include <stdio.h>
#include <arch/armv7.h>
#include <config.h>

#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

#define VALUE_ZERO      0
#define CPSR_MODE_MASK           0X1F
#define CPSR_ASYNC_ABT_DIABLE    0x100
#define CPSR_IRQ_DISABLE         0x80
#define CPSR_FIQ_DISABLE         0x40

#define CPSR_ASYNC_ABT_BIT  CPSR_ASYNC_ABT_DIABLE
#define CPSR_IRQ_BIT        CPSR_IRQ_DISABLE
#define CPSR_FIQ_BIT        CPSR_FIQ_DISABLE

static void core_regs_init(struct core_regs *regs)
{
    int i = 0;
    //uint32_t *atag_ptr;

    /* TODO(casionwoo) : Why PC should be this value */
    regs->pc = CFG_GUEST_START_ADDRESS;
	regs->cpsr = (CPSR_ASYNC_ABT_BIT | CPSR_IRQ_BIT | CPSR_FIQ_BIT
			     | CPSR_MODE_SVC | VALUE_ZERO);

    for (i = 0; i < NR_ARCH_GPR_REGS ; i++) {
        regs->gpr[i] = 0;
    }

    /*
     * R1 : Machine Number
     * R2 : Atags Address
     */
    regs->gpr[1] = MAHINE_TYPE;
    regs->gpr[2] = CFG_GUEST_ATAGS_START_ADDRESS;
}

static void cop_regs_init(struct cp15 *cp15)
{
    uint32_t vmpidr = read_vmpidr();

    cp15->vbar  = 0;
    cp15->ttbr0 = 0;
    cp15->ttbr1 = 0;
    cp15->ttbcr = 0;
    cp15->sctlr = 0;

    // TODO(casionwoo) : Modify the way of initialize vmpidr later?
    vmpidr &= 0xFFFFFFFC;
    /* have to fix it
     * vmpidr is to be the virtual cpus's core id.
     * so this value have to determined by situation.
     * ex) linux guest's secondary vcpu, bm guest vcpu .. etc.
     */
    vmpidr |= 0;
    cp15->vmpidr = vmpidr;
}

static void banked_regs_init(struct banked_regs *banked_regs)
{
    banked_regs->sp_usr   = 0;
    banked_regs->spsr_svc = 0;
    banked_regs->sp_svc   = 0;
    banked_regs->lr_svc   = 0;
    banked_regs->spsr_abt = 0;
    banked_regs->sp_abt   = 0;
    banked_regs->lr_abt   = 0;
    banked_regs->spsr_und = 0;
    banked_regs->sp_und   = 0;
    banked_regs->lr_und   = 0;
    banked_regs->spsr_irq = 0;
    banked_regs->sp_irq   = 0;
    banked_regs->lr_irq   = 0;
    banked_regs->spsr_fiq = 0;
    banked_regs->lr_fiq   = 0;
    banked_regs->r8_fiq   = 0;
    banked_regs->r9_fiq   = 0;
    banked_regs->r10_fiq  = 0;
    banked_regs->r11_fiq  = 0;
    banked_regs->r12_fiq  = 0;
    /* Cortex-A15 processor does not support sp_fiq */
}

static void core_regs_save(struct core_regs *dst, struct core_regs *src)
{
    int i;

    dst->cpsr = src->cpsr;
    dst->pc = src->pc;
    dst->lr = src->lr;
    for (i = 0; i < NR_ARCH_GPR_REGS; i++) {
        dst->gpr[i] = src->gpr[i];
    }
}

static void core_regs_restore(struct core_regs *dst, struct core_regs *src)
{
    int i;

    src->cpsr = dst->cpsr;
    src->pc = dst->pc;
    src->lr = dst->lr;
    for (i = 0; i < NR_ARCH_GPR_REGS; i++) {
        src->gpr[i] = dst->gpr[i];
    }
}

static void banked_regs_save(struct banked_regs *banked_regs)
{
    /* USR banked register */
    asm volatile(" mrs     %0, sp_usr\n\t"
                 : "=r"(banked_regs->sp_usr) : : "memory", "cc");
    /* SVC banked register */
    asm volatile(" mrs     %0, spsr_svc\n\t"
                 : "=r"(banked_regs->spsr_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t"
                 : "=r"(banked_regs->sp_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t"
                 : "=r"(banked_regs->lr_svc) : : "memory", "cc");
    /* ABT banked register */
    asm volatile(" mrs     %0, spsr_abt\n\t"
                 : "=r"(banked_regs->spsr_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_abt\n\t"
                 : "=r"(banked_regs->sp_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_abt\n\t"
                 : "=r"(banked_regs->lr_abt) : : "memory", "cc");
    /* UND banked register */
    asm volatile(" mrs     %0, spsr_und\n\t"
                 : "=r"(banked_regs->spsr_und) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_und\n\t"
                 : "=r"(banked_regs->sp_und) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_und\n\t"
                 : "=r"(banked_regs->lr_und) : : "memory", "cc");
    /* IRQ banked register */
    asm volatile(" mrs     %0, spsr_irq\n\t"
                 : "=r"(banked_regs->spsr_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t"
                 : "=r"(banked_regs->sp_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t"
                 : "=r"(banked_regs->lr_irq) : : "memory", "cc");
    /* FIQ banked register  R8_fiq ~ R12_fiq, LR and SPSR */
    asm volatile(" mrs     %0, spsr_fiq\n\t"
                 : "=r"(banked_regs->spsr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_fiq\n\t"
                 : "=r"(banked_regs->lr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r8_fiq\n\t"
                 : "=r"(banked_regs->r8_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r9_fiq\n\t"
                 : "=r"(banked_regs->r9_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r10_fiq\n\t"
                 : "=r"(banked_regs->r10_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r11_fiq\n\t"
                 : "=r"(banked_regs->r11_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r12_fiq\n\t"
                 : "=r"(banked_regs->r12_fiq) : : "memory", "cc");
}

static void banked_regs_restore(struct banked_regs *banked_regs)
{
    /* USR banked register */
    asm volatile(" msr    sp_usr, %0\n\t"
                 : : "r"(banked_regs->sp_usr) : "memory", "cc");
    /* SVC banked register */
    asm volatile(" msr    spsr_svc, %0\n\t"
                 : : "r"(banked_regs->spsr_svc) : "memory", "cc");
    asm volatile(" msr    sp_svc, %0\n\t"
                 : : "r"(banked_regs->sp_svc) : "memory", "cc");
    asm volatile(" msr    lr_svc, %0\n\t"
                 : : "r"(banked_regs->lr_svc) : "memory", "cc");
    /* ABT banked register */
    asm volatile(" msr    spsr_abt, %0\n\t"
                 : : "r"(banked_regs->spsr_abt) : "memory", "cc");
    asm volatile(" msr    sp_abt, %0\n\t"
                 : : "r"(banked_regs->sp_abt) : "memory", "cc");
    asm volatile(" msr    lr_abt, %0\n\t"
                 : : "r"(banked_regs->lr_abt) : "memory", "cc");
    /* UND banked register */
    asm volatile(" msr    spsr_und, %0\n\t"
                 : : "r"(banked_regs->spsr_und) : "memory", "cc");
    asm volatile(" msr    sp_und, %0\n\t"
                 : : "r"(banked_regs->sp_und) : "memory", "cc");
    asm volatile(" msr    lr_und, %0\n\t"
                 : : "r"(banked_regs->lr_und) : "memory", "cc");
    /* IRQ banked register */
    asm volatile(" msr     spsr_irq, %0\n\t"
                 : : "r"(banked_regs->spsr_irq) : "memory", "cc");
    asm volatile(" msr     sp_irq, %0\n\t"
                 : : "r"(banked_regs->sp_irq) : "memory", "cc");
    asm volatile(" msr     lr_irq, %0\n\t"
                 : : "r"(banked_regs->lr_irq) : "memory", "cc");
    /* FIQ banked register */
    asm volatile(" msr     spsr_fiq, %0\n\t"
                 : : "r"(banked_regs->spsr_fiq) : "memory", "cc");
    asm volatile(" msr     lr_fiq, %0\n\t"
                 : : "r"(banked_regs->lr_fiq) : "memory", "cc");
    asm volatile(" msr    r8_fiq, %0\n\t"
                 : : "r"(banked_regs->r8_fiq) : "memory", "cc");
    asm volatile(" msr    r9_fiq, %0\n\t"
                 : : "r"(banked_regs->r9_fiq) : "memory", "cc");
    asm volatile(" msr    r10_fiq, %0\n\t"
                 : : "r"(banked_regs->r10_fiq) : "memory", "cc");
    asm volatile(" msr    r11_fiq, %0\n\t"
                 : : "r"(banked_regs->r11_fiq) : "memory", "cc");
    asm volatile(" msr    r12_fiq, %0\n\t"
                 : : "r"(banked_regs->r12_fiq) : "memory", "cc");
}

static void cop_regs_save(struct cp15 *cp15)
{
    cp15->vbar  = read_vbar();
    cp15->ttbr0 = read_ttbr0();
    cp15->ttbr1 = read_ttbr1();
    cp15->ttbcr = read_ttbcr();
    cp15->sctlr = read_sctlr();
    cp15->vmpidr = read_vmpidr();
}

static void cop_regs_restore(struct cp15 *cp15)
{
    write_vbar(cp15->vbar);
    write_ttbr0(cp15->ttbr0);
    write_ttbr1(cp15->ttbr1);
    write_ttbcr(cp15->ttbcr);
    write_sctlr(cp15->sctlr);
    write_vmpidr(cp15->vmpidr);
}

void print_core_regs(struct core_regs *regs)
{
    int i;

    printf("cpsr: 0x%08x\n", regs->cpsr);
    printf("cpsr mode(%x)\n", regs->cpsr & CPSR_MODE_MASK);
    printf("  pc: 0x%08x\n", regs->pc);
    printf("  lr: 0x%08x\n", regs->lr);
    printf(" gpr:\n\r");
    for (i = 0; i < NR_ARCH_GPR_REGS; i++) {
        printf("     0x%08x\n", regs->gpr[i]);
    }

    printf("cntpct: %llu\n", read_cntpct());
    printf("cnth_tval:0x%08x\n", read_cnthp_tval());

}

hvmm_status_t arch_regs_init(struct arch_regs *regs)
{
    // TODO(casionwoo) : Initialize loader status for reboot
    core_regs_init(&regs->core_regs);
    cop_regs_init(&regs->cp15);
    banked_regs_init(&regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t arch_regs_save(struct arch_regs *regs, struct core_regs *current_regs)
{
    if (!current_regs) {
        return HVMM_STATUS_SUCCESS;
    }

    core_regs_save(&regs->core_regs, current_regs);
    cop_regs_save(&regs->cp15);
    banked_regs_save(&regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

extern void __set_vcpu_context_first_time(struct core_regs *regs);
hvmm_status_t arch_regs_restore(struct arch_regs *regs, struct core_regs *current_regs)
{
    // 'current_regs == 0' means there is no vcpu are running on hypervisor.
    // We need a way to boot first vcpu up as below.
    if (!current_regs) {
        __set_vcpu_context_first_time(&regs->core_regs);
        return HVMM_STATUS_SUCCESS;
    }

    core_regs_restore(&regs->core_regs, current_regs);
    cop_regs_restore(&regs->cp15);
    banked_regs_restore(&regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}
