#include <lib/print.h>

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

    return 0;
}
