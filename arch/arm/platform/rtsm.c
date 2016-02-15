#include <platform.h>

#include <stdio.h>
#include <memory.h>
#include <armv7_p15.h>

void init_platform()
{
    pl01x_init(115200, 24000000);

    __malloc_init();
    stage1_pgtable_init();
    stage1_mmu_init();
}
