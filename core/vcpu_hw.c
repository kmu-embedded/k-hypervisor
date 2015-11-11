#include <vcpu_hw.h>

#define CPSR_MODE_USER  0x10
#define CPSR_MODE_FIQ   0x11
#define CPSR_MODE_IRQ   0x12
#define CPSR_MODE_SVC   0x13
#define CPSR_MODE_MON   0x16
#define CPSR_MODE_ABT   0x17
#define CPSR_MODE_HYP   0x1A
#define CPSR_MODE_UND   0x1B
#define CPSR_MODE_SYS   0x1F

void context_copy_regs(struct arch_regs *regs_dst, struct arch_regs *regs_src)
{
    int i;
    regs_dst->cpsr = regs_src->cpsr;
    regs_dst->pc = regs_src->pc;
    regs_dst->lr = regs_src->lr;
    for (i = 0; i < ARCH_REGS_NUM_GPR; i++)
        regs_dst->gpr[i] = regs_src->gpr[i];
}

/* banked registers */
void context_init_banked(struct regs_banked *regs_banked)
{
    regs_banked->sp_usr     = 0;
    regs_banked->spsr_svc   = 0;
    regs_banked->sp_svc     = 0;
    regs_banked->lr_svc     = 0;
    regs_banked->spsr_abt   = 0;
    regs_banked->sp_abt     = 0;
    regs_banked->lr_abt     = 0;
    regs_banked->spsr_und   = 0;
    regs_banked->sp_und     = 0;
    regs_banked->lr_und     = 0;
    regs_banked->spsr_irq   = 0;
    regs_banked->sp_irq     = 0;
    regs_banked->lr_irq     = 0;
    regs_banked->spsr_fiq   = 0;
    regs_banked->lr_fiq     = 0;
    regs_banked->r8_fiq     = 0;
    regs_banked->r9_fiq     = 0;
    regs_banked->r10_fiq    = 0;
    regs_banked->r11_fiq    = 0;
    regs_banked->r12_fiq    = 0;
    /* Cortex-A15 processor does not support sp_fiq */
}
/*
void context_save_banked(struct regs_banked *regs_banked)
{
    //USR banked register 
    asm volatile(" mrs     %0, sp_usr\n\t"
            : "=r"(regs_banked->sp_usr) : : "memory", "cc");
    //SVC banked register
    asm volatile(" mrs     %0, spsr_svc\n\t"
            : "=r"(regs_banked->spsr_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t"
            : "=r"(regs_banked->sp_svc) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t"
            : "=r"(regs_banked->lr_svc) : : "memory", "cc");
    //ABT banked register
    asm volatile(" mrs     %0, spsr_abt\n\t"
            : "=r"(regs_banked->spsr_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_abt\n\t"
            : "=r"(regs_banked->sp_abt) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_abt\n\t"
            : "=r"(regs_banked->lr_abt) : : "memory", "cc");
    //UND banked register
    asm volatile(" mrs     %0, spsr_und\n\t"
            : "=r"(regs_banked->spsr_und) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_und\n\t"
            : "=r"(regs_banked->sp_und) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_und\n\t"
            : "=r"(regs_banked->lr_und) : : "memory", "cc");
    //IRQ banked register
    asm volatile(" mrs     %0, spsr_irq\n\t"
            : "=r"(regs_banked->spsr_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t"
            : "=r"(regs_banked->sp_irq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t"
            : "=r"(regs_banked->lr_irq) : : "memory", "cc");
    //FIQ banked register  R8_fiq ~ R12_fiq, LR and SPSR
    asm volatile(" mrs     %0, spsr_fiq\n\t"
            : "=r"(regs_banked->spsr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_fiq\n\t"
            : "=r"(regs_banked->lr_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r8_fiq\n\t"
            : "=r"(regs_banked->r8_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r9_fiq\n\t"
            : "=r"(regs_banked->r9_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r10_fiq\n\t"
            : "=r"(regs_banked->r10_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r11_fiq\n\t"
            : "=r"(regs_banked->r11_fiq) : : "memory", "cc");
    asm volatile(" mrs     %0, r12_fiq\n\t"
            : "=r"(regs_banked->r12_fiq) : : "memory", "cc");
}

void context_restore_banked(struct regs_banked *regs_banked)
{
    //USR banked register
    asm volatile(" msr    sp_usr, %0\n\t"
            : : "r"(regs_banked->sp_usr) : "memory", "cc");
    //SVC banked register
    asm volatile(" msr    spsr_svc, %0\n\t"
            : : "r"(regs_banked->spsr_svc) : "memory", "cc");
    asm volatile(" msr    sp_svc, %0\n\t"
            : : "r"(regs_banked->sp_svc) : "memory", "cc");
    asm volatile(" msr    lr_svc, %0\n\t"
            : : "r"(regs_banked->lr_svc) : "memory", "cc");
    //ABT banked register
    asm volatile(" msr    spsr_abt, %0\n\t"
            : : "r"(regs_banked->spsr_abt) : "memory", "cc");
    asm volatile(" msr    sp_abt, %0\n\t"
            : : "r"(regs_banked->sp_abt) : "memory", "cc");
    asm volatile(" msr    lr_abt, %0\n\t"
            : : "r"(regs_banked->lr_abt) : "memory", "cc");
    //UND banked register
    asm volatile(" msr    spsr_und, %0\n\t"
            : : "r"(regs_banked->spsr_und) : "memory", "cc");
    asm volatile(" msr    sp_und, %0\n\t"
            : : "r"(regs_banked->sp_und) : "memory", "cc");
    asm volatile(" msr    lr_und, %0\n\t"
            : : "r"(regs_banked->lr_und) : "memory", "cc");
    //IRQ banked register
    asm volatile(" msr     spsr_irq, %0\n\t"
            : : "r"(regs_banked->spsr_irq) : "memory", "cc");
    asm volatile(" msr     sp_irq, %0\n\t"
            : : "r"(regs_banked->sp_irq) : "memory", "cc");
    asm volatile(" msr     lr_irq, %0\n\t"
            : : "r"(regs_banked->lr_irq) : "memory", "cc");
    //FIQ banked register
    asm volatile(" msr     spsr_fiq, %0\n\t"
            : : "r"(regs_banked->spsr_fiq) : "memory", "cc");
    asm volatile(" msr     lr_fiq, %0\n\t"
            : : "r"(regs_banked->lr_fiq) : "memory", "cc");
    asm volatile(" msr    r8_fiq, %0\n\t"
            : : "r"(regs_banked->r8_fiq) : "memory", "cc");
    asm volatile(" msr    r9_fiq, %0\n\t"
            : : "r"(regs_banked->r9_fiq) : "memory", "cc");
    asm volatile(" msr    r10_fiq, %0\n\t"
            : : "r"(regs_banked->r10_fiq) : "memory", "cc");
    asm volatile(" msr    r11_fiq, %0\n\t"
            : : "r"(regs_banked->r11_fiq) : "memory", "cc");
    asm volatile(" msr    r12_fiq, %0\n\t"
            : : "r"(regs_banked->r12_fiq) : "memory", "cc");
}
*/
void context_copy_banked(struct regs_banked *banked_dst, struct regs_banked *banked_src)
{
    banked_dst->sp_usr      = banked_src->sp_usr;
    banked_dst->spsr_svc    = banked_src->spsr_svc;
    banked_dst->sp_svc      = banked_src->sp_svc;
    banked_dst->lr_svc      = banked_src->lr_svc;
    banked_dst->spsr_abt    = banked_src->spsr_abt;
    banked_dst->sp_abt      = banked_src->sp_abt;
    banked_dst->lr_abt      = banked_src->lr_abt;
    banked_dst->spsr_und    = banked_src->spsr_und;
    banked_dst->sp_und      = banked_src->sp_und;
    banked_dst->lr_und      = banked_src->sp_und;
    banked_dst->spsr_irq    = banked_src->spsr_irq;
    banked_dst->sp_irq      = banked_src->sp_irq;
    banked_dst->lr_irq      = banked_src->lr_irq;
    banked_dst->spsr_fiq    = banked_src->spsr_fiq;
    banked_dst->lr_fiq      = banked_src->lr_fiq;
    banked_dst->r8_fiq      = banked_src->r8_fiq;
    banked_dst->r9_fiq      = banked_src->r9_fiq;
    banked_dst->r10_fiq     = banked_src->r10_fiq;
    banked_dst->r11_fiq     = banked_src->r11_fiq;
    banked_dst->r12_fiq     = banked_src->r12_fiq;
}

/* Co-processor state management: init/save/restore */
void context_init_cops(struct regs_cop *regs_cop)
{
    regs_cop->vbar  = 0;
    regs_cop->ttbr0 = 0;
    regs_cop->ttbr1 = 0;
    regs_cop->ttbcr = 0;
    regs_cop->sctlr = 0;
}

void context_save_cops(struct regs_cop *regs_cop)
{
/*
    regs_cop->vbar  = read_vbar();
    regs_cop->ttbr0 = read_ttbr0();
    regs_cop->ttbr1 = read_ttbr1();
    regs_cop->ttbcr = read_ttbcr();
    regs_cop->sctlr = read_sctlr();
*/
}

void context_restore_cops(struct regs_cop *regs_cop)
{
/*
    write_vbar(regs_cop->vbar);
    write_ttbr0(regs_cop->ttbr0);
    write_ttbr1(regs_cop->ttbr1);
    write_ttbcr(regs_cop->ttbcr);
    write_sctlr(regs_cop->sctlr);
*/
}

#ifndef DEBUG
char *_modename(unsigned char mode)
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

hvmm_state_t vcpu_hw_save(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs)
{
    struct arch_regs *regs = &vcpu_regs->regs;
    struct arch_context *context = &vcpu_regs->context;

    if (!current_regs)
        return HVMM_STATE_SUCCESS;

    //vcpu_regs-> vmpidr = read_vmpidr();

    //context_copy_regs(regs, current_regs);
    //context_save_cops(&context->regs_cop);
    //context_save_banked(&context->regs_banked);
/*
    printh("vcpu_hw_save  context: saving vmid[%d] mode(%x):%s pc:0x%x\n",
            _current_vcpu[0]->vmid,
            regs->cpsr & 0x1F,
            _modename(regs->cpsr & 0x1F),
            regs->pc);
*/

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_hw_restore(struct vcpu_regs *vcpu_regs, struct arch_regs *current_regs)
{
    struct arch_context *context = &vcpu_regs->context;

    //write_vmpidr(vcpu_regs->vmpidr);

    if (!current_regs) {
        /* init -> hyp mode -> vcpu */
        /*
         * The actual context switching (Hyp to Normal mode)
         * handled in the asm code
         */
        //__mon_switch_to_vcpu_context(&vcpu_regs->regs);
        return HVMM_STATE_SUCCESS;
    }

    /* vcpu -> hyp -> vcpu */
    //context_copy_regs(current_regs, &vcpu_regs->regs);
    //context_restore_cops(&context->regs_cop);
    //context_restore_banked(&context->regs_banked);

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_hw_init(struct vcpu_regs *vcpu_regs, struct arch_regs *regs)
{
    struct arch_context *context = &vcpu_regs->context;
    unsigned int vmpidr;

    //vmpidr = read_vmpidr();
    vmpidr &= 0xFFFFFFFC;
    /* have to fix it
     * vmpidr is to be the virtual cpus's core id.
     * so this value have to determined by situation.
     * ex) linux vcpu's secondary vcpu, bm vcpu vcpu .. etc.
     */
    vmpidr |= 0;
    vcpu_regs->vmpidr = vmpidr;

    regs->pc = 0x80000000;//CFG_GUEST_START_ADDRESS;
    /* Initialize loader status for reboot */
    regs->gpr[10] = 0;
    /* supervisor mode */
    regs->cpsr = 0x1d3;
    /* regs->gpr[] = whatever */
    //context_init_cops(&context->regs_cop);
    //context_init_banked(&context->regs_banked);

    return HVMM_STATE_SUCCESS;
}

hvmm_state_t vcpu_hw_move(struct vcpu_regs *dst_regs, struct vcpu_regs *src_regs)
{
    //context_copy_regs(&(dst_regs->regs), &(src_regs->regs));
    //context_copy_banked(&(dst_regs->context.regs_banked), &(src_regs->context.regs_banked));
}

