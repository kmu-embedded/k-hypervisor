#include <stdio.h>
#include "traps.h"
#include <arch/armv7.h>
#include <vdev.h>
#include <core/vm/vcpu.h>
#include <core/vm/vm.h>
#include <core/scheduler.h>

#define decode_fsc(iss)         (iss & 0x3f)

#define decode_wnr(iss)         (iss & (1 << 6))
#define decode_srt(iss)         ((iss & 0xF0000) >> 16)


extern uint32_t linux_smp_pen;

int handle_data_abort(struct core_regs *regs, uint32_t iss)
{
    int ret = -1;
    uint32_t fipa = read_cp32(HPFAR) << 8;
    fipa |= (read_cp32(HDFAR) & PAGE_MASK);

    switch (decode_fsc(iss)) {
        // TODO: remove unused cases.
        case FSR_TRANS_FAULT(1) ... FSR_TRANS_FAULT(3):
            fipa = read_cp32(HPFAR) << 8;
            printf("Translation Fault!!\n");
            printf("\tWe do not allow translation fault from guest in stage 2 address\n");
            printf("\ttranslation, so if you see this message, you have to add a mapping\n");
            printf("\ttable into platform/<your target>/guest.c\n");
            printf("\tfault address is here: 0x%08x\n", fipa);

            ret = 0;
            break;

        case FSR_ACCESS_FAULT(1) ... FSR_ACCESS_FAULT(3):
            {
                uint8_t wnr = decode_wnr(iss);
                uint8_t srt = decode_srt(iss);

                if (fipa == 0x1c010030){
                    printf("========================== ACCESS_FAULT: fipa 0x%08x\n", fipa);
                    printf("WnR: %s\n", !wnr ? "read" : "write");
                    printf("SRT: r%d: %x\n", srt, regs->gpr[srt]);

#if 1
                    struct vmcb *vm = get_current_vm();
                    struct vcpu *target_vcpu = vm->vcpu[1];
                    target_vcpu->regs.core_regs.pc = regs->gpr[srt];
                    target_vcpu = vm->vcpu[2];
                    target_vcpu->regs.core_regs.pc = regs->gpr[srt];

                    target_vcpu = vm->vcpu[3];
                    target_vcpu->regs.core_regs.pc = regs->gpr[srt];
#endif

                    if (wnr) {
                        writel((regs->gpr[srt]), fipa);
                    } else {
                        regs->gpr[srt] = readl(fipa);
                    }
                    linux_smp_pen = 1;
                } else if (fipa >= 0x1c010000 && fipa < 0x1c010030) {

                    if (wnr) {
                        writel((regs->gpr[srt]), fipa);
                    } else {
                        regs->gpr[srt] = readl(fipa);
                    }
                } else if (fipa > 0x1c010030 && fipa <= 0x1c011000) {
                    if (wnr) {
                        writel((regs->gpr[srt]), fipa);
                    } else {
                        regs->gpr[srt] = readl(fipa);
                    }
                } else {
                    vdev_handler(regs, iss);
                }

                ret = 0;
                break;
            }

        case FSR_PERM_FAULT(1) ... FSR_PERM_FAULT(3):
            printf("FSR_PERM_FAULT: fipa 0x%08x\n", fipa);
            break;

        case FSR_SYNC_ABORT:
            printf("FSR_SYNC_FAULT: fipa 0x%08x\n", fipa);
            break;

        case FSR_ASYNC_ABORT:
            printf("FSR_ASYNC_ABORT\n");
            break;

        case FSR_ABORT_ON_TABLE_WALK(1) ... FSR_ABORT_ON_TABLE_WALK(3):
            printf("FSR_ABORT_ON_TABLE_WALK\n");
            break;

        case FSR_SYNC_PERORR:
            printf("FSR_SYNC_PERORR\n");
            break;

        case FSR_ASYNC_PERORR:
            printf("FSR_ASYNC_PERORR\n");
            break;

        case FSR_PERORR_ON_TABLE_WALK(1) ... FSR_PERORR_ON_TABLE_WALK(3):
            printf("FSR_PERORR_ON_TABLE_WALK\n");
            break;

        case FSR_ALINGMENT_FAULT: {
            fipa |= (read_cp32(HDFAR) & PAGE_MASK);
            uint32_t hcr = read_cp32(HCR);
            printf("Alignment Fault!!\n");
            printf("ARM defines the root cause of alignment fault taken into hypervisor as belows\n");
            if (regs->cpsr == 0x1A) {
                printf("\t1. When the processor is in Hyp mode.\n");
            } else {
                if ((hcr & (1 << 27)) != 1) {
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
        case FSR_DEBUG_EVENT:
            printf("FSR_DEBUG_EVENT\n");
            break;

        case FSR_TLB_CONFLICT:
            printf("FSR_TLB_CONFLICT\n");
            break;

        case FSR_DOMAIN_FAULT(1) ... FSR_DOMAIN_FAULT(3):
            printf("FSR_DOMAIN_FAULT");
            break;

        default:
            break;
    }
    return ret;
}
