#include <stdio.h>
#include <stdlib.h>
#include <asm/asm.h>
#include <arch/armv7.h>

#include <debug.h>

#include <vdev.h>

#include "hvc_trap.h"

static void _trap_dump_bregs(void)
{
    uint32_t spsr, lr, sp;

    printf(" - banked regs\n");
    asm volatile(" mrs     %0, sp_usr\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_usr\n\t" : "=r"(lr) : : "memory", "cc");
    printf(" - usr: sp:%x lr:%x\n", sp, lr);
    asm volatile(" mrs     %0, spsr_svc\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_svc\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_svc\n\t" : "=r"(lr) : : "memory", "cc");
    printf(" - svc: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
    asm volatile(" mrs     %0, spsr_irq\n\t" : "=r"(spsr) : : "memory", "cc");
    asm volatile(" mrs     %0, sp_irq\n\t" : "=r"(sp) : : "memory", "cc");
    asm volatile(" mrs     %0, lr_irq\n\t" : "=r"(lr) : : "memory", "cc");
    printf(" - irq: spsr:%x sp:%x lr:%x\n", spsr, sp, lr);
}

#include <core/vm.h>
#include <core/scheduler.h>
int do_hvc_trap(struct core_regs *regs)
{
    hsr_t hsr;
	iss_t iss;

	hsr.raw = read_hsr();
	iss.raw = hsr.entry.iss;


    switch (hsr.entry.ec) {
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
	case TRAP_EC_NON_ZERO_HVC:
	case TRAP_EC_NON_ZERO_DATA_ABORT_FROM_OTHER_MODE:
	{

		uint32_t fipa = read_hpfar() << 8;
		struct vmcb *vm = get_current_vm();
		struct vdev_instance *instance = NULL;
		list_for_each_entry(struct vdev_instance, instance, &vm->vdevs_head, head)
		{
			if (fipa == instance->module->base) {
				fipa |= (read_hdfar() & HPFAR_FIPA_PAGE_MASK);
				uint32_t offset = fipa - instance->module->base;

				if (hsr.entry.iss & ISS_WNR) {
					if (instance->module->write(instance->pdata, offset, &(regs->gpr[iss.dabt.srt])) < 0) {
						goto trap_error;
					}
				} else {
					regs->gpr[iss.dabt.srt] = instance->module->read(instance->pdata, offset);
					if (regs->gpr[iss.dabt.srt] < 0) {
						goto trap_error;
					}
				}


			}
		}
	}
		break;
	default:
        goto trap_error;
    }

    if (hsr.entry.il == 0) {
    	regs->pc += 2;
    } else {
    	regs->pc += 4;
    }

    return 0;

trap_error:
    _trap_dump_bregs();
	debug_print("[hyp] do_hvc_trap:unknown hsr.iss= %x\n", hsr.entry.iss);
	debug_print("[hyp] hsr.ec= %x\n", hsr.entry.ec);
    debug_print("[hyp] hsr= %x\n", hsr.raw);
    printf("guest pc is %x\n", regs->pc);
    while(1) ;

    return -1;
}
