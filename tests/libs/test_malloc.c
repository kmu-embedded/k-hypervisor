#include "test_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

void malloc_simple()
{
    void *mem = malloc(213);
    printf("mem: %p\n", mem);
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
