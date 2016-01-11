#include <memory.h>
#include <stage1_mm.h>
#include <stdio.h>

hvmm_status_t memory_init()
{
    if (stage1_mm_init() != HVMM_STATUS_SUCCESS) {
        printf("[%s] stage1_mm_init() failed\n");
        return HVMM_STATUS_UNKNOWN_ERROR;
    }
    if (enable_stage1_mmu() != HVMM_STATUS_SUCCESS ) {
        printf("[%s] enable_stage1_mmu() failed\n");
        return HVMM_STATUS_UNKNOWN_ERROR;
    }

    return HVMM_STATUS_SUCCESS;
}
