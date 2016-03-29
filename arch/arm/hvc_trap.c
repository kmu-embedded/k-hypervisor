#include <stdio.h>
#include <arch/armv7.h>
#include <vdev.h>

#include <core/vm.h>
#include <core/scheduler.h>

#include "hvc_trap.h"

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
		list_for_each_entry(struct vdev_instance, instance, &vm->vdevs.head, head)
		{
			if (fipa == instance->module->base) {
				fipa |= (read_hdfar() & HPFAR_FIPA_PAGE_MASK);
				uint32_t offset = fipa - instance->module->base;

				if (iss.dabt.wnr == 1) {
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
	printf("[hyp] do_hvc_trap:unknown hsr.iss= %x\n", hsr.entry.iss);
	printf("[hyp] hsr.ec= %x\n", hsr.entry.ec);
    printf("[hyp] hsr= %x\n", hsr.raw);
    printf("guest pc is %x\n", regs->pc);
    while(1) ;

    return -1;
}
