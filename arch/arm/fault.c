#include <stdio.h>
#include "traps.h"
#include <arch/armv7.h>
#include <vdev.h>

#define decode_fsc(iss)         (iss & 0x3f)

int handle_data_abort(void *pdata, uint32_t iss)
{
    int ret = -1;
    uint32_t fipa = read_cp32(HPFAR) << 8;
    fipa |= (read_cp32(HDFAR) & PAGE_MASK);

    switch (decode_fsc(iss)) {
        // TODO: remove unused cases.
        case FSR_TRANS_FAULT(1) ... FSR_TRANS_FAULT(3):
            printf("FSR_TRANS_FAULT: fipa 0x%08x\n", fipa);
        break;

        case FSR_ACCESS_FAULT(1) ... FSR_ACCESS_FAULT(3):
            vdev_handler(pdata, iss);
            ret = 0;
            break;

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

        case FSR_ALINGMENT_FAULT:
            printf("FSR_ALIGNMENT_FAULT: fipa 0x%08x\n", fipa);
            break;

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
