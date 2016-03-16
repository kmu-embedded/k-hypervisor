#include <stdio.h>
#include <debug_print.h>
#include <hvmm_trace.h>
#include "trap.h"
#include <arch/armv7.h>

#include <core/irq.h>
#include <core/scheduler.h>
#include <core/vdev.h>

#include "../../drivers/gic-v2.h"
hvmm_status_t _hyp_dabort(struct core_regs *regs)
{
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}

#include <irq-chip.h>
hvmm_status_t _hyp_irq(struct core_regs *regs)
{
    uint32_t irq;

    irq = irq_hw->ack();
    irq_service_routine(irq, (void *)regs);

    return HVMM_STATUS_SUCCESS;
}

hvmm_status_t _hyp_unhandled(struct core_regs *regs)
{
    hyp_abort_infinite();
    return HVMM_STATUS_UNKNOWN_ERROR;
}

enum hyp_hvc_result _hyp_hvc(struct core_regs *regs)
{
    return _hyp_hvc_service(regs);
}

static void _trap_dump_bregs(void)
{
    uint32_t spsr, lr, sp;

    debug_print(" - banked regs\n");
    asm volatile(" mrs     %0, sp_usr\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_usr\n\t" : "=r"(lr) : : "memory", "cc");
    debug_print(" - usr: sp:%x lr:%x\n", sp, lr);
    asm volatile(" mrs     %0, spsr_svc\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    debug_print(" - svc: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t" : "=r"(lr) : : "memory", "cc");
    debug_print(" - irq: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
}

enum hyp_hvc_result _hyp_hvc_service(struct core_regs *regs)
{
    int32_t vdev_num = -1;
    uint32_t hsr = read_hsr();
    uint32_t ec = (hsr & HSR_EC_BIT) >> EXTRACT_EC;
    uint32_t iss = hsr & HSR_ISS_BIT;
    uint32_t far = read_hdfar();
    uint32_t fipa;
    uint32_t srt;
    struct arch_vdev_trigger_info info;
    int level = VDEV_LEVEL_LOW;

    fipa = (read_hpfar() & HPFAR_FIPA_MASK) >> HPFAR_FIPA_SHIFT;
    fipa = fipa << HPFAR_FIPA_PAGE_SHIFT;
    fipa = fipa | (far & HPFAR_FIPA_PAGE_MASK);
    info.ec = ec;
    info.iss = iss;
    info.fipa = fipa;
    info.sas = (iss & ISS_SAS_MASK) >> ISS_SAS_SHIFT;
    srt = (iss & ISS_SRT_MASK) >> ISS_SRT_SHIFT;
    info.value = &(regs->gpr[srt]);

    switch (ec) {
    case TRAP_EC_ZERO_UNKNOWN:
    case TRAP_EC_ZERO_WFI_WFE:
    case TRAP_EC_ZERO_MCR_MRC_CP15:
    case TRAP_EC_ZERO_MCRR_MRRC_CP15:
    case TRAP_EC_ZERO_MCR_MRC_CP14:
    case TRAP_EC_ZERO_LDC_STC_CP14:
    case TRAP_EC_ZERO_HCRTR_CP0_CP13:
    case TRAP_EC_ZERO_MRC_VMRS_CP10:
    case TRAP_EC_ZERO_BXJ:
    case TRAP_EC_ZERO_MRRC_CP14:
    case TRAP_EC_NON_ZERO_SVC:
    case TRAP_EC_NON_ZERO_SMC:
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_OTHER_MODE:
    case TRAP_EC_NON_ZERO_PREFETCH_ABORT_FROM_HYP_MODE:
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_HYP_MODE:
        level = VDEV_LEVEL_HIGH;
        break;
    case TRAP_EC_NON_ZERO_HVC:
        level = VDEV_LEVEL_MIDDLE;
        break;
    case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE:
        level = VDEV_LEVEL_LOW;
        break;
    default:
        debug_print("[hyp] _hyp_hvc_service:unknown hsr.iss= %x\n", iss);
        debug_print("[hyp] hsr.ec= %x\n", ec);
        debug_print("[hyp] hsr= %x\n", hsr);
        goto trap_error;
    }

    vdev_num = vdev_find(level, &info, regs);
    if (vdev_num < 0) {
        debug_print("[hvc] cann't search vdev number\n\r");
        goto trap_error;
    }

    if (iss & ISS_WNR) {
        if (vdev_write(level, vdev_num, &info, regs) < 0) {
            goto trap_error;
        }
    } else {
        if (vdev_read(level, vdev_num, &info, regs) < 0) {
            goto trap_error;
        }
    }
    vdev_post(level, vdev_num, &info, regs);

    return HYP_RESULT_ERET;
trap_error:
    _trap_dump_bregs();
    debug_print("fipa is %x guest pc is %x\n", fipa, regs->pc);
    hyp_abort_infinite();
}
