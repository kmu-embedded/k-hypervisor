#include <platform.h>
#include <lpae.h>

#include <stdio.h>
#include <memory.h>
#include <armv7_p15.h>

//#define TEST_ALIGNED_ALLOC

#if TEST_ALIGNED_ALLOC
static void test_malloc(void);
static void test_aligned_alloc(void);
#endif

void init_platform()
{
    pl01x_init(115200, 24000000);

    __malloc_init();
#if TEST_ALIGNED_ALLOC
    test_malloc();
    test_aligned_alloc();
    while(1) ;
#endif
    stage1_pgtable_create();
    stage1_pgtable_init();
    stage1_mmu_init();
}

static void test_malloc(void)
{
    int r = 3, c = 4;
    int *arr = (int *)malloc(r * c * sizeof(int));

    int i, j, count = 0;
    for (i = 0; i <  r; i++)
        for (j = 0; j < c; j++)
            *(arr + i*c + j) = ++count;

    for (i = 0; i <  r; i++)
        for (j = 0; j < c; j++)
            printf("%d ", *(arr + i*c + j));
}

static void test_aligned_alloc(void)
{
    pgentry *ptr1, *ptr2, *ptr3;
    ptr1 = aligned_alloc(sizeof(pgentry) * 4, 4096);
    ptr2 = aligned_alloc(sizeof(pgentry) * 4 * 512, 4096);
    ptr3 = aligned_alloc(sizeof(pgentry) * 4 * 512 * 512, 4096);

    printf("[%s] %p\n", __func__, ptr1);
    printf("[%s] %p\n", __func__, ptr2);
    printf("[%s] %p\n", __func__, ptr3);
}
