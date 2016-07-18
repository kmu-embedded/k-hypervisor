#include <stdio.h>
#include <string.h>
#include <arch/armv7.h>
#include <arch_regs.h>
#include <config.h>
#include <arch/cpsr.h>

static void init_core_regs(struct core_regs *regs)
{
    memset(regs, 0, sizeof(struct core_regs));
    /*
     * R1 : Machine Number
     * R2 : Atags Address
     */
    regs->gpr[1] = MAHINE_TYPE;
    regs->gpr[2] = CFG_GUEST_ATAGS_START_ADDRESS;

    regs->pc = CFG_GUEST_START_ADDRESS;
    regs->cpsr = (CPSR_ASYNC_ABT_BIT | CPSR_IRQ_BIT | CPSR_FIQ_BIT | CPSR_MODE_SVC);
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
