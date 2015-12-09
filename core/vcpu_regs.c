#include <stdio.h>
#include <hvmm_trace.h>
#include <vcpu.h>
#include <vcpu_regs.h>
#include <vgic.h>
#include <scheduler.h>

#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

#define GUEST_VERBOSE_ALL       0xFF
#define GUEST_VERBOSE_LEVEL_0   0x01
#define GUEST_VERBOSE_LEVEL_1   0x02
#define GUEST_VERBOSE_LEVEL_2   0x04
#define GUEST_VERBOSE_LEVEL_3   0x08
#define GUEST_VERBOSE_LEVEL_4   0x10
#define GUEST_VERBOSE_LEVEL_5   0x20
#define GUEST_VERBOSE_LEVEL_6   0x40
#define GUEST_VERBOSE_LEVEL_7   0x80

static void context_copy_regs(struct core_regs *core_regs_dst, struct core_regs *core_regs_src)
{
    int i;
    core_regs_dst->cpsr = core_regs_src->cpsr;
    core_regs_dst->pc = core_regs_src->pc;
    core_regs_dst->lr = core_regs_src->lr;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        core_regs_dst->gpr[i] = core_regs_src->gpr[i];
}

/* banked registers */
static void context_init_banked(struct banked_regs *banked_regs)
{
    banked_regs->sp_usr = 0;
    banked_regs->spsr_svc = 0;
    banked_regs->sp_svc = 0;
    banked_regs->lr_svc = 0;
    banked_regs->spsr_abt = 0;
    banked_regs->sp_abt = 0;
    banked_regs->lr_abt = 0;
    banked_regs->spsr_und = 0;
    banked_regs->sp_und = 0;
    banked_regs->lr_und = 0;
    banked_regs->spsr_irq = 0;
    banked_regs->sp_irq = 0;
    banked_regs->lr_irq = 0;
    banked_regs->spsr_fiq = 0;
    banked_regs->lr_fiq = 0;
    banked_regs->r8_fiq = 0;
    banked_regs->r9_fiq = 0;
    banked_regs->r10_fiq = 0;
    banked_regs->r11_fiq = 0;
    banked_regs->r12_fiq = 0;
    /* Cortex-A15 processor does not support sp_fiq */
}

static void context_save_banked(struct banked_regs *banked_regs)
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

static void context_restore_banked(struct banked_regs *banked_regs)
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
static void context_copy_banked(struct banked_regs *banked_dst, struct banked_regs *banked_src)
{
    banked_dst->sp_usr   = banked_src->sp_usr;
    banked_dst->spsr_svc = banked_src->spsr_svc;
    banked_dst->sp_svc   = banked_src->sp_svc;
    banked_dst->lr_svc   = banked_src->lr_svc;
    banked_dst->spsr_abt = banked_src->spsr_abt;
    banked_dst->sp_abt   = banked_src->sp_abt;
    banked_dst->lr_abt   = banked_src->lr_abt;
    banked_dst->spsr_und = banked_src->spsr_und;
    banked_dst->sp_und   = banked_src->sp_und;
    banked_dst->lr_und   = banked_src->sp_und;
    banked_dst->spsr_irq = banked_src->spsr_irq;
    banked_dst->sp_irq   = banked_src->sp_irq;
    banked_dst->lr_irq   = banked_src->lr_irq;
    banked_dst->spsr_fiq = banked_src->spsr_fiq;
    banked_dst->lr_fiq   = banked_src->lr_fiq;
    banked_dst->r8_fiq   = banked_src->r8_fiq;
    banked_dst->r9_fiq   = banked_src->r9_fiq;
    banked_dst->r10_fiq  = banked_src->r10_fiq;
    banked_dst->r11_fiq  = banked_src->r11_fiq;
    banked_dst->r12_fiq  = banked_src->r12_fiq;
}
/* Co-processor state management: init/save/restore */
static void context_init_cops(struct cop_regs *cop_regs)
{
    uint32_t vmpidr = read_vmpidr();

    cop_regs->vbar = 0;
    cop_regs->ttbr0 = 0;
    cop_regs->ttbr1 = 0;
    cop_regs->ttbcr = 0;
    cop_regs->sctlr = 0;

    vmpidr &= 0xFFFFFFFC;
    /* have to fix it
     * vmpidr is to be the virtual cpus's core id.
     * so this value have to determined by situation.
     * ex) linux guest's secondary vcpu, bm guest vcpu .. etc.
     */
    vmpidr |= 0;
    cop_regs->vmpidr = vmpidr;

}

static void context_save_cops(struct cop_regs *cop_regs)
{
    cop_regs->vbar = read_vbar();
    cop_regs->ttbr0 = read_ttbr0();
    cop_regs->ttbr1 = read_ttbr1();
    cop_regs->ttbcr = read_ttbcr();
    cop_regs->sctlr = read_sctlr();
    cop_regs->vmpidr = read_vmpidr();
}

static void context_restore_cops(struct cop_regs *cop_regs)
{
    write_vbar(cop_regs->vbar);
    write_ttbr0(cop_regs->ttbr0);
    write_ttbr1(cop_regs->ttbr1);
    write_ttbcr(cop_regs->ttbcr);
    write_sctlr(cop_regs->sctlr);
    write_vmpidr(cop_regs->vmpidr);
}

#ifndef DEBUG
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
#endif

hvmm_status_t vcpu_regs_save(struct vcpu_regs *vcpu_regs, struct core_regs *current_regs)
{
    struct core_regs *core_regs = &vcpu_regs->core_regs;
    struct context_regs *context_regs = &vcpu_regs->context_regs;

    if (!current_regs)
        return HVMM_STATUS_SUCCESS;

    context_copy_regs(core_regs, current_regs);
    context_save_cops(&context_regs->cop_regs);
    context_save_banked(&context_regs->banked_regs);
    printf("vcpu_regs_save  context: saving vmid[%d] mode(%x):%s pc:0x%x\n",
            _current_guest[0]->vcpuid,
           core_regs->cpsr & 0x1F,
           _modename(core_regs->cpsr & 0x1F),
           core_regs->pc);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vcpu_regs_restore(struct vcpu_regs *vcpu_regs, struct core_regs *current_regs)
{
    struct context_regs *context_regs = &vcpu_regs->context_regs;

    if (!current_regs) {
        /* init -> hyp mode -> guest */
        /*
         * The actual context switching (Hyp to Normal mode)
         * handled in the asm code
         */
        __mon_switch_to_guest_context(&vcpu_regs->core_regs);
        return HVMM_STATUS_SUCCESS;
    }

    /* guest -> hyp -> guest */
    context_copy_regs(current_regs, &vcpu_regs->core_regs);
    context_restore_cops(&context_regs->cop_regs);
    context_restore_banked(&context_regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

//hvmm_status_t vcpu_regs_init(struct vcpu *guest, struct core_regs *regs)
hvmm_status_t vcpu_regs_init(struct vcpu_regs *vcpu_regs)
{
    struct context_regs *context_regs = &vcpu_regs->context_regs;
    struct core_regs *core_regs = &vcpu_regs->core_regs;

    core_regs->pc = CFG_GUEST_START_ADDRESS;
    /* Initialize loader status for reboot */
    core_regs->gpr[10] = 0;
    /* supervisor mode */
    core_regs->cpsr = 0x1d3;
    /* regs->gpr[] = whatever */
    context_init_cops(&context_regs->cop_regs);
    context_init_banked(&context_regs->banked_regs);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vcpu_regs_dump(uint8_t verbose, struct core_regs *core_regs)
{
    if (verbose & GUEST_VERBOSE_LEVEL_0) {
        printf("cpsr: 0x%08x\n", core_regs->cpsr);
        printf("  pc: 0x%08x\n", core_regs->pc);
        printf("  lr: 0x%08x\n", core_regs->lr);
        {
            int i;
            printf(" gpr:\n\r");
            for (i = 0; i < ARCH_REGS_NUM_GPR; i++) {
                printf("     0x%08x\n", core_regs->gpr[i]);
            }
        }
    }
    if (verbose & GUEST_VERBOSE_LEVEL_1) {
        uint32_t lr = 0;
        asm volatile("mov  %0, lr" : "=r"(lr) : : "memory", "cc");
        printf("context: restoring vmid[%d] mode(%x):%s pc:0x%x lr:0x%x\n",
                _current_guest[0]->vcpuid,
                _current_guest[0]->vcpu_regs.core_regs.cpsr & 0x1F,
                _modename(_current_guest[0]->vcpu_regs.core_regs.cpsr & 0x1F),
                _current_guest[0]->vcpu_regs.core_regs.pc, lr);

    }
    if (verbose & GUEST_VERBOSE_LEVEL_2) {
        uint64_t pct = read_cntpct();
        uint32_t tval = read_cnthp_tval();
        printf("cntpct: %llu\n", pct);
        printf("cnth_tval:0x%08x\n", tval);
    }
    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t vcpu_regs_move(struct vcpu_regs *dst, struct vcpu_regs *src)
{
    context_copy_regs(&(dst->core_regs), &(src->core_regs));
    context_copy_banked(&(dst->context_regs.banked_regs),
            &(src->context_regs.banked_regs));
    return HVMM_STATUS_SUCCESS;
}

