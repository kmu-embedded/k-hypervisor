#include <stdio.h>
#include <arch/armv7.h>
#include <vdev.h>

#include <core/vm/vm.h>
#include <core/vm/vcpu.h>

#include "hvc_trap.h"

int do_hvc_trap(struct core_regs *regs)
{
    hsr_t hsr;
    iss_t iss;
    uint32_t fipa;

    hsr.raw = read_hsr();
    iss.raw = hsr.entry.iss;

    fipa = read_hpfar() << 8;
    fipa |= (read_hdfar() & PAGE_MASK);

    switch (hsr.entry.ec) {
    case HSR_EC_UNKNOWN:
        break;
    case HSR_EC_WFI_WFE:
    case HSR_EC_MCR_MRC_CP15:
        break;
    case HSR_EC_MCRR_MRRC_CP15:
    case HSR_EC_MCR_MRC_CP14:
        break;
    case HSR_EC_LDC_STC_CP14:
    case HSR_EC_HCRTR_CP0_CP13:
    case HSR_EC_MRC_VMRS_CP10:
        break;
    case HSR_EC_BXJ:
    case HSR_EC_MRRC_CP14:
    case HSR_EC_SVC:
    case HSR_EC_SMC:
    case HSR_EC_PABT_FROM_GUEST:
    case HSR_EC_PABT_FROM_HYP_MODE:
    case HSR_EC_DABT_FROM_HYP_MODE:
    case HSR_EC_HVC:
        goto trap_error;
    case HSR_EC_DABT_FROM_GUEST: {
        switch (iss.dabt.dfsc) {
        case FSR_TRANS_FAULT(1) ... FSR_TRANS_FAULT(3):
            fipa = read_hpfar() << 8;
            fipa |= (read_hdfar() & PAGE_MASK);
            printf("FSR_TRANS_FAULT: fipa 0x%08x\n", fipa);

            uint32_t ID = 0;
            ID = READ_CP15(15, 0, 0, 0, 0, 5);
            printf("%s\n", MRC_CP32(15, 0, 0, 0, 0, 5));
            printf("%d\n", ID & MPIDR_MASK & MPIDR_CPUID_MASK);

            goto trap_error;

        case FSR_ACCESS_FAULT(1) ... FSR_ACCESS_FAULT(3):
            vdev_handler(regs, iss);
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
    printf("fipa : %x\n", fipa);
    while (1) ;

    return -1;
}
