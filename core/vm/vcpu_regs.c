#include <vcpu_regs.h>
#include <stdio.h>
#include <armv7_p15.h>
#include <scheduler.h>
#include <rtsm-config.h>

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

static void core_regs_init(struct core_regs *core_regs)
{
    int i = 0;

    /* TODO(casionwoo) : Why PC should be this value */
    core_regs->pc = CFG_GUEST_START_ADDRESS;
    core_regs->cpsr =
        (CPSR_ASYNC_ABT_BIT | CPSR_IRQ_BIT | CPSR_FIQ_BIT | CPSR_MODE_SVC | VALUE_ZERO);

    for (i = 0; i < ARCH_REGS_NUM_GPR ; i++) {
        core_regs->gpr[i] = 0;
    }
}

static void cop_regs_init(struct cop_regs *cop_regs)
{
    uint32_t vmpidr = read_vmpidr();

    cop_regs->vbar  = 0;
    cop_regs->ttbr0 = 0;
    cop_regs->ttbr1 = 0;
    cop_regs->ttbcr = 0;
    cop_regs->sctlr = 0;

    // TODO(casionwoo) : Modify the way of initialize vmpidr later?
    vmpidr &= 0xFFFFFFFC;
    /* have to fix it
     * vmpidr is to be the virtual cpus's core id.
     * so this value have to determined by situation.
     * ex) linux guest's secondary vcpu, bm guest vcpu .. etc.
     */
    vmpidr |= 0;
    cop_regs->vmpidr = vmpidr;
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

static void core_regs_copy(struct core_regs *dst, struct core_regs *src)
{
    int i;

    dst->cpsr = src->cpsr;
    dst->pc = src->pc;
    dst->lr = src->lr;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
        dst->gpr[i] = src->gpr[i];
    }
}

static void core_regs_save(struct core_regs *core_regs, struct core_regs *current_core_regs)
{
    core_regs_copy(core_regs, current_core_regs);
}

static void core_regs_restore(struct core_regs *core_regs, struct core_regs *current_core_regs)
{
    core_regs_copy(current_core_regs, core_regs);
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

static void cop_regs_save(struct cop_regs *cop_regs)
{
    cop_regs->vbar  = read_vbar();
    cop_regs->ttbr0 = read_ttbr0();
    cop_regs->ttbr1 = read_ttbr1();
    cop_regs->ttbcr = read_ttbcr();
    cop_regs->sctlr = read_sctlr();
    cop_regs->vmpidr = read_vmpidr();
}

static void cop_regs_restore(struct cop_regs *cop_regs)
{
    write_vbar(cop_regs->vbar);
    write_ttbr0(cop_regs->ttbr0);
    write_ttbr1(cop_regs->ttbr1);
    write_ttbcr(cop_regs->ttbcr);
    write_sctlr(cop_regs->sctlr);
    write_vmpidr(cop_regs->vmpidr);
}

static char *_modename(uint8_t mode)
{
    char *name = "Unknown";
    switch (mode) {
    case CPSR_MODE_USER:
        name = "User";
        break;
    case CPSR_MODE_FIQ:
        name = "FIQ";
        break;
    case CPSR_MODE_IRQ:
        name = "IRQ";
        break;
    case CPSR_MODE_SVC:
        name = "Supervisor";
        break;
    case CPSR_MODE_MON:
        name = "Monitor";
        break;
    case CPSR_MODE_ABT:
        name = "Abort";
        break;
    case CPSR_MODE_HYP:
        name = "Hyp";
        break;
    case CPSR_MODE_UND:
        name = "Undefined";
        break;
    case CPSR_MODE_SYS:
        name = "System";
        break;
    }
    return name;
}

void print_core_regs(struct core_regs *core_regs)
{
    int i;

    printf("cpsr: 0x%08x\n", core_regs->cpsr);
    printf("cpsr mode(%x) : %s\n", core_regs->cpsr & CPSR_MODE_MASK, _modename(core_regs->cpsr & CPSR_MODE_MASK));
    printf("  pc: 0x%08x\n", core_regs->pc);
    printf("  lr: 0x%08x\n", core_regs->lr);
    printf(" gpr:\n\r");
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
        printf("     0x%08x\n", core_regs->gpr[i]);
    }

    printf("cntpct: %llu\n", read_cntpct());
    printf("cnth_tval:0x%08x\n", read_cnthp_tval());

}

hvmm_status_t vcpu_regs_init(struct vcpu_regs *vcpu_regs)
{
    struct context_regs *context_regs = &vcpu_regs->context_regs;

    // TODO(casionwoo) : What is the meanning of bellow line
    // Initialize loader status for reboot
    // core_regs->gpr[10] = 0;

    core_regs_init(&vcpu_regs->core_regs);
    cop_regs_init(&context_regs->cop_regs);
    banked_regs_init(&context_regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vcpu_regs_save(struct vcpu_regs *vcpu_regs, struct core_regs *current_regs)
{
    struct context_regs *context_regs = &vcpu_regs->context_regs;

    if (!current_regs)
        return HVMM_STATUS_SUCCESS;

    core_regs_save(&vcpu_regs->core_regs, current_regs);
    cop_regs_save(&context_regs->cop_regs);
    banked_regs_save(&context_regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vcpu_regs_restore(struct vcpu_regs *vcpu_regs, struct core_regs *current_regs)
{
    struct context_regs *context_regs = &vcpu_regs->context_regs;

    // 'current_regs == 0' means there is no vcpu are running on hypervisor.
    // We need a way to boot first vcpu up as below.
    if (!current_regs) {
        __set_vcpu_context_first_time(&vcpu_regs->core_regs);
        return HVMM_STATUS_SUCCESS;
    }

    core_regs_restore(&vcpu_regs->core_regs, current_regs);
    cop_regs_restore(&context_regs->cop_regs);
    banked_regs_restore(&context_regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

void print_vcpu_regs(struct vcpu_regs *vcpu_regs)
{
    print_core_regs(&vcpu_regs->core_regs);

    /* TODO(casionwoo) : Decide if print bellow count registers */
    printf("cntpct: %llu\n", read_cntpct());
    printf("cnth_tval:0x%08x\n", read_cnthp_tval());
}

