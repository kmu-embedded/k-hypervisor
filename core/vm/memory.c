#include <memory.h>
//#include <stage1_mm.h>
#include <lpaed.h>
#include <stdio.h>

hvmm_status_t memory_init()
{
    if (stage1_pgtable_init() != HVMM_STATUS_SUCCESS) {
        printf("[%s] stage1_mm_init() failed\n");
        return HVMM_STATUS_UNKNOWN_ERROR;
    }
    if (stage1_mmu_init() != HVMM_STATUS_SUCCESS ) {
        printf("[%s] enable_stage1_mmu() failed\n");
        return HVMM_STATUS_UNKNOWN_ERROR;
    }

    return HVMM_STATUS_SUCCESS;
}
