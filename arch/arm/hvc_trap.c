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
    case HSR_EC_UNKNOWN:
    case HSR_EC_WFI_WFE:
    case HSR_EC_MCR_MRC_CP15:
    case HSR_EC_MCRR_MRRC_CP15:
    case HSR_EC_MCR_MRC_CP14:
    case HSR_EC_LDC_STC_CP14:
    case HSR_EC_HCRTR_CP0_CP13:
    case HSR_EC_MRC_VMRS_CP10:
    case HSR_EC_BXJ:
    case HSR_EC_MRRC_CP14:
    case HSR_EC_SVC:
    case HSR_EC_SMC:
    case HSR_EC_PABT_FROM_GUEST:
    case HSR_EC_PABT_FROM_HYP_MODE:
	case HSR_EC_DABT_FROM_HYP_MODE:
	case HSR_EC_HVC:
        goto trap_error;
	case HSR_EC_DABT_FROM_GUEST:
	{
		switch (iss.dabt.dfsc) {
		case FSR_TRANS_FAULT(1) ... FSR_TRANS_FAULT(3):
			printf("FSR_TRANS_FAULT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_ACCESS_FAULT(1) ... FSR_ACCESS_FAULT(3):
		{
			uint32_t fipa = read_hpfar() << 8;
			struct vmcb *vm = get_current_vm();
			struct vdev_instance *instance = NULL;
			fipa |= (read_hdfar() & HPFAR_FIPA_PAGE_MASK);

			list_for_each_entry(struct vdev_instance, instance, &vm->vdevs.head, head)
			{
				uint32_t vdev_base = instance->module->base;
				uint32_t vdev_size = instance->module->size;

				if (vdev_base <= fipa && fipa <= vdev_base + vdev_size) {
					uint32_t offset = fipa - vdev_base;
					if (iss.dabt.wnr == 1) {
						if (instance->module->write(instance->pdata, offset,
								&(regs->gpr[iss.dabt.srt])) < 0) {
							goto trap_error;
						}
					} else {
						regs->gpr[iss.dabt.srt] = instance->module->read(
								instance->pdata, offset);
						if (regs->gpr[iss.dabt.srt] < 0) {
							goto trap_error;
						}
					}
				}
			}
		}
			break;

		case FSR_PERM_FAULT(1) ... FSR_PERM_FAULT(3):
			printf("FSR_PERM_FAULT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_SYNC_ABORT:
			printf("FSR_SYNC_ABORT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_ASYNC_ABORT:
			printf("FSR_ASYNC_ABORT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_ABORT_ON_TABLE_WALK(1) ... FSR_ABORT_ON_TABLE_WALK(3):
			printf("FSR_ABORT_ON_TABLE_WALK  %x\n", iss.dabt.dfsc);
			break;

		case FSR_SYNC_PERORR:
			printf("FSR_SYNC_PERORR  %x\n", iss.dabt.dfsc);
			break;

		case FSR_ASYNC_PERORR:
			printf("FSR_ASYNC_PERORR  %x\n", iss.dabt.dfsc);
			break;

		case FSR_PERORR_ON_TABLE_WALK(1) ... FSR_PERORR_ON_TABLE_WALK(3):
			printf("FSR_PERORR_ON_TABLE_WALK  %x\n", iss.dabt.dfsc);
			break;

		case FSR_ALINGMENT_FAULT:
			printf("FSR_ALINGMENT_FAULT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_DEBUG_EVENT:
			printf("FSR_DEBUG_EVENT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_TLB_CONFLICT:
			printf("FSR_TLB_CONFLICT  %x\n", iss.dabt.dfsc);
			break;

		case FSR_DOMAIN_FAULT(1) ... FSR_DOMAIN_FAULT(3):
			printf("FSR_DOMAIN_FAULT  %x\n", iss.dabt.dfsc);
			break;

		}

		break;
	}
	default:
        goto trap_error;
    }

	regs->pc += 4;

    return 0;

trap_error:
	printf("[hyp] do_hvc_trap: unknown hsr.iss= %x\n", hsr.entry.iss);
	printf("[hyp] hsr.ec= %x\n", hsr.entry.ec);
    printf("[hyp] hsr= %x\n", hsr.raw);
    printf("guest pc is %x\n", regs->pc);
    while(1) ;

    return -1;
}
