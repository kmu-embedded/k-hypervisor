#include <stdio.h>
#include <arch/armv7.h>
#include <vdev.h>
#include <core/vm/vcpu.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>
#include "lpae.h"

int handle_data_abort(struct core_regs *regs, uint32_t iss)
{
    int ret = -1;
    uint32_t fipa = read_cp32(HPFAR) << IPA_SHIFT;
    fipa |= (read_cp32(HDFAR) & PAGE_OFFSET_MASK);
    fipa = read_cp32(HPFAR) << IPA_SHIFT;

    switch (DFSC(iss)) {
        // TODO: remove unused cases.
    case TRANS_FAULT(1) ... TRANS_FAULT(3):
        printf("Translation Fault!!\n");
        printf("\tWe do not allow translation fault from guest in stage 2 address\n");
        printf("\ttranslation, so if you see this message, you have to add a mapping\n");
        printf("\ttable into platform/<your target>/guest.c\n");
        printf("\tfault address is here: 0x%08x\n", fipa);

        ret = 0;
        break;

    case ACCESS_FAULT(1) ... ACCESS_FAULT(3):
        vdev_handler(regs, iss);
        ret = 0;
        break;

    case PERM_FAULT(1) ... PERM_FAULT(3):
        printf("PERM_FAULT: fipa 0x%08x\n", fipa);
        break;

    case SYNC_ABORT:
        printf("SYNC_FAULT: fipa 0x%08x\n", fipa);
        break;

    case ASYNC_ABORT:
        printf("ASYNC_ABORT\n");
        break;

    case ABORT_ON_TABLE_WALK(1) ... ABORT_ON_TABLE_WALK(3):
        printf("ABORT_ON_TABLE_WALK\n");
        break;

    case SYNC_PERORR:
        printf("SYNC_PERORR\n");
        break;

    case ASYNC_PERORR:
        printf("ASYNC_PERORR\n");
        break;

    case PERORR_ON_TABLE_WALK(1) ... PERORR_ON_TABLE_WALK(3):
        printf("PERORR_ON_TABLE_WALK\n");
        break;

    case ALINGMENT_FAULT: {
        uint32_t hcr = read_cp32(HCR);

        printf("Alignment Fault!!\n");
        printf("ARM defines the root cause of alignment fault taken into hypervisor as belows\n");

        if (regs->cpsr == CPSR_MODE(HYP)) {
            printf("\t1. When the processor is in Hyp mode.\n");
        } else {
            if ((hcr & HCR_BIT(TGE)) != 1) {
                printf("\t2. When the processor is in a PL1 or PL0 mode and the exception is generated\n");
                printf("\t   because the Non-secure PL1&0 stage 2 translation identifies the target of\n");
                printf("\t   an unaligned access as Device or Strongly-ordered memory.\n");
                printf("\t   NOTE: please check memory attribute at platform/<target>/guest.c\n");
            } else {
                printf("\t3. The processor is in the PL0 mode and HCR.TGE is set to 1.\n");
                printf("\t   For more information see Synchronous external abort, when HCR.TGE is\n");
                printf("\t   set to 1 on page B1-1193.\n");
            }
        }
        printf("HCR: %x\n",  hcr);
        printf("HDCR %x\n", read_cp32(HDCR));
        printf("HPFAR 0x%08x is UNK\n", read_cp32(HPFAR));
        printf("HDFAR 0x%08x is available\n", fipa);
        break;
    }
    case DEBUG_EVENT:
        printf("DEBUG_EVENT\n");
        break;

    case TLB_CONFLICT:
        printf("TLB_CONFLICT\n");
        break;

    case DOMAIN_FAULT(1) ... DOMAIN_FAULT(3):
        printf("DOMAIN_FAULT");
        break;

    default:
        break;
    }
    return ret;
}
