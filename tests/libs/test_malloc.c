#include "test_malloc.h"
#include <lib/malloc.h>
#include <stdio.h>
#include <drivers/pl01x.h>
#include <lib/null.h>

void malloc_simple()
{
    void *mem = malloc(213);
    printf("mem: ");
    uart_print_hex32((unsigned int)mem);
    printf("\n");

    free(mem);
}

void malloc_lots()
{
    void *mem[1024];
    int i;

    for (i = 0; i < 1024; i++) {
        mem[i] = malloc(123);
    }

    for (i--; i >= 0; i--) {
        free(mem[i]);
    }
}

void free_simple()
{
    /* check we can free NULL */
    free(NULL);
}

void test_malloc(void)
{
    malloc_simple();
    malloc_lots();
    free_simple();
}
