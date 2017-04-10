#include <stdio.h>
#include <string.h>
#include <arch/armv7.h>
#include <arch_regs.h>
#include <config.h>

static void init_core_regs(struct core_regs *regs)
{
    memset(regs, 0, sizeof(struct core_regs));
    /*
     * R1 : Machine Number
     * R2 : Atags Address
     */
    regs->gpr[1] = MACHINE_TYPE;
    regs->gpr[2] = CFG_GUEST_ATAGS_START_ADDRESS;

    regs->pc = CFG_GUEST_START_ADDRESS;
    regs->cpsr = (CPSR_BIT(A) | CPSR_BIT(I) | CPSR_BIT(F));
    regs->cpsr |= CPSR_MODE(SVC);
    printf("regs->cpsr %x\n", regs->cpsr);
}

static void init_cp15(struct cp15 *cp15)
{
    memset(cp15, 0, sizeof(struct cp15));

    printf("vmpidr: %x\n", cp15->vmpidr);
    cp15->vmpidr = read_cp32(VMPIDR);
    printf("vmpidr: %x\n", cp15->vmpidr);

    // TODO(casionwoo) : Modify the way of initialize vmpidr later?
    cp15->vmpidr &= 0xFFFFFFFC;
    /* have to fix it
     * vmpidr is to be the virtual cpus's core id.
     * so this value have to determined by situation.
     * ex) linux guest's secondary vcpu, bm guest vcpu .. etc.
     */
    cp15->vmpidr |= 0;
}

static void save_banked_regs(struct banked_regs *banked_regs)
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

static void restore_banked_regs(struct banked_regs *banked_regs)
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

static void save_cp15(struct cp15 *cp15)
{
    cp15->vbar = read_cp32(VBAR);
    // TODO: We have to handle TTBR by guest's setting.
    cp15->ttbr0 = read_cp32(TTBR0_32);
    cp15->ttbr1 = read_cp32(TTBR1_32);
    cp15->ttbcr = read_cp32(TTBCR);
    cp15->sctlr = read_cp32(SCTLR);
    cp15->vmpidr = read_cp32(VMPIDR);

    cp15->csselr = read_cp32(CSSELR);
    cp15->cpacr = read_cp32(CPACR);

    cp15->contextidr = read_cp32(CONTEXTIDR);
    cp15->tpidrurw = read_cp32(TPIDRURW);
    cp15->tpidruro = read_cp32(TPIDRURO);
    cp15->tpidrprw = read_cp32(TPIDRPRW);

    cp15->cntkctl = read_cp32(CNTKCTL);

    cp15->dacr = read_cp32(DACR);
    cp15->par = read_cp32(PAR_32);

    cp15->mair0 = read_cp32(MAIR0);
    cp15->mair1 = read_cp32(MAIR1);
    cp15->amair0 = read_cp32(AMAIR0);
    cp15->amair1 = read_cp32(AMAIR1);

    cp15->dfar = read_cp32(DFAR);
    cp15->ifar = read_cp32(IFAR);
    cp15->dfsr = read_cp32(DFSR);
    cp15->ifsr = read_cp32(IFSR);

    cp15->adfsr = read_cp32(ADFSR);
    cp15->aifsr = read_cp32(AIFSR);

    isb();
}

static void restore_cp15(struct cp15 *cp15)
{
    write_cp32(cp15->vbar, VBAR);
    // TODO: We have to handle TTBR by guest's setting.
    write_cp32(cp15->ttbr0, TTBR0_32);
    write_cp32(cp15->ttbr1, TTBR1_32);
    write_cp32(cp15->ttbcr, TTBCR);
    write_cp32(cp15->sctlr, SCTLR);
    write_cp32(cp15->vmpidr, VMPIDR);

    write_cp32(cp15->csselr, CSSELR);
    write_cp32(cp15->cpacr, CPACR);

    write_cp32(cp15->contextidr, CONTEXTIDR);
    write_cp32(cp15->tpidrurw, TPIDRURW);
    write_cp32(cp15->tpidruro, TPIDRURO);
    write_cp32(cp15->tpidrprw, TPIDRPRW);

    write_cp32(cp15->cntkctl, CNTKCTL);

    write_cp32(cp15->dacr, DACR);
    write_cp32(cp15->par, PAR_32);

    write_cp32(cp15->mair0, MAIR0);
    write_cp32(cp15->mair1, MAIR1);
    write_cp32(cp15->amair0, AMAIR0);
    write_cp32(cp15->amair1, AMAIR1);

    write_cp32(cp15->dfar, DFAR);
    write_cp32(cp15->ifar, IFAR);
    write_cp32(cp15->dfsr, DFSR);
    write_cp32(cp15->ifsr, IFSR);

    write_cp32(cp15->adfsr, ADFSR);
    write_cp32(cp15->aifsr, AIFSR);

    isb();
}

void print_core_regs(struct core_regs *regs)
{
    int i;

    printf("cpsr: 0x%08x\n", regs->cpsr);
    printf("cpsr mode(%x)\n", regs->cpsr & CPSR(M));
    printf("  pc: 0x%08x\n", regs->pc);
    printf("  lr: 0x%08x\n", regs->lr);
    printf(" gpr:\n\r");
    for (i = 0; i < NR_ARCH_GPR_REGS; i++) {
        printf("     0x%08x\n", regs->gpr[i]);
    }
}

hvmm_status_t arch_regs_init(struct arch_regs *regs)
{
    // TODO(casionwoo) : Initialize loader status for reboot
    init_core_regs(&regs->core_regs);
    init_cp15(&regs->cp15);
    memset(&regs->banked_regs, 0, sizeof(struct banked_regs));

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t arch_regs_save(struct arch_regs *regs, struct core_regs *current_regs)
{
    if (!current_regs) {
        return HVMM_STATUS_SUCCESS;
    }

    memcpy(&regs->core_regs, current_regs, sizeof(struct core_regs));
    save_cp15(&regs->cp15);
    save_banked_regs(&regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

extern void __set_vcpu_context_first_time(struct core_regs *regs);
hvmm_status_t arch_regs_restore(struct arch_regs *regs, struct core_regs *current_regs)
{
    // 'current_regs == 0' means there is no vcpu are running on hypervisor.
    // We need a way to boot first vcpu up as below.
    if (!current_regs) {
        printf("%s[%d]\n", __func__, __LINE__);
        restore_cp15(&regs->cp15);
        __set_vcpu_context_first_time(&regs->core_regs);

        /* Never reach here */
        return HVMM_STATUS_SUCCESS;
    }

    restore_cp15(&regs->cp15);
    restore_banked_regs(&regs->banked_regs);
    memcpy(current_regs, &regs->core_regs, sizeof(struct core_regs));

    return HVMM_STATUS_SUCCESS;
}

static void cp15_copy(struct cp15 *from, struct cp15 *to)
{
    to->vbar = from->vbar;
    to->ttbr0 = from->ttbr0;
    to->ttbr1 = from->ttbr1;
    to->ttbcr = from->ttbcr;
    to->sctlr = from->sctlr;
    to->vmpidr = from->vmpidr;

    to->csselr = from->csselr;
    to->cpacr = from->cpacr;

    to->contextidr = from->contextidr;
    to->tpidrurw = from->tpidrurw;
    to->tpidruro = from->tpidruro;
    to->tpidrprw = from->tpidrprw;

    to->cntkctl = from->cntkctl;

    to->dacr = from->dacr;
    to->par = from->par;

    to->mair0 = from->mair0;
    to->mair1 = from->mair1;
    to->amair0 = from->amair0;
    to->amair1 = from->amair1;

    to->dfar = from->dfar;
    to->ifar = from->ifar;
    to->dfsr = from->dfsr;
    to->ifsr = from->ifsr;

    to->adfsr = from->adfsr;
    to->aifsr = from->aifsr;
}

static void banked_regs_copy(struct banked_regs *from, struct banked_regs *to)
{
    to->sp_usr = from->sp_usr;

    to->spsr_svc = from->spsr_svc;
    to->sp_svc = from->sp_svc;
    to->lr_svc = from->lr_svc;
    to->spsr_abt = from->spsr_abt;
    to->sp_abt = from->sp_abt;
    to->lr_abt = from->lr_abt;
    to->spsr_und = from->spsr_und;
    to->sp_und = from->sp_und;
    to->lr_und = from->lr_und;
    to->spsr_irq = from->spsr_irq;
    to->sp_irq = from->sp_irq;
    to->lr_irq = from->lr_irq;

    to->spsr_fiq = from->spsr_fiq;
    to->lr_fiq = from->lr_fiq;
    to->r8_fiq = from->r8_fiq;
    to->r9_fiq = from->r9_fiq;
    to->r10_fiq = from->r10_fiq;
    to->r11_fiq = from->r11_fiq;
    to->r12_fiq = from->r12_fiq;
}

static void core_regs_copy(struct core_regs *from, struct core_regs *to, struct core_regs *regs)
{
    to->cpsr = from->cpsr; 	/* CPSR */
    to->pc = from->pc; 	/* Program Counter */
    to->pc += 4;
    to->lr = from->lr;
    int i = 0;
    for(i = 0; i < NR_ARCH_GPR_REGS; i++)
        to->gpr[i] = from->gpr[i]; /* R0 - R12 */
}

void arch_regs_copy(struct arch_regs *from, struct arch_regs *to, struct core_regs *regs)
{
    /*
        TODO(casionwoo):
            1. cp15 copy
            2. banked_regs copy
            3. core_regs copy
    */
    cp15_copy(&from->cp15, &to->cp15);
    banked_regs_copy(&from->banked_regs, &to->banked_regs);
    core_regs_copy(&from->core_regs, &to->core_regs, regs);
}
