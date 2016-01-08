#include <init_malloc.h>    // for __malloc_init();
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vm_main.h>
#include <test_malloc.h>

// TODO(wonseok): This C entry point will be moved into core directory.
int init_khypervisor(void)
{
    int i;
    int *arr;

    __malloc_init();
    test_malloc();

    arr = malloc(10 * sizeof(int));
    for(i = 0; i < 10; i++) {
        arr[i] = 10 * i;
    }
    for(i = 0; i < 10; i++) {
        printf("%08d\n", arr[i]);
    }
    memset(arr, 0, sizeof(int) * 10);
    for(i = 0; i < 10; i++) {
        printf("%d\n", arr[i]);
    }

    printf("%p\n", arr);
    printf("0x%08x\n", arr);

    free(arr);

#if 1
    vm_main();
#endif
    printf("test is done\n");

    return 0;
}
