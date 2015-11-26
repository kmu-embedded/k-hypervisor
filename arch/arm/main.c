#include <test_malloc.h>
#include <lib/print.h>
#include <vm_main.h>
#include <lib/malloc.h>

// TODO(wonseok): This C entry point will be moved into core directory.
int init_khypervisor(void)
{
    int num;
    long base;
    num = 10;
    base = 2;
    num = num / base;
    num = num % base;

    init_print();
    printf("test %d\n", num);

    init_heap();
    test_malloc();
#if 0
    vm_main();
#endif
    printf("test is done\n");

    return 0;
}
