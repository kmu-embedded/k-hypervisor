#include <lib/malloc.h>
#include <lib/k_r_malloc.h>
#include <lib/null.h>
#include <lib/print.h>

extern unsigned int __begin_heap;
extern unsigned int __end_heap;
unsigned int heap_start;
unsigned int heap_end;

void init_heap()
{
    heap_start = (unsigned int)&__begin_heap;
    heap_end = (unsigned int)&__end_heap;
}

#define ROUND_UP(n, size)           ((((n) + (size) - 1)) & (~((size) - 1)))

HEADER *morecore(unsigned nu)
{
    unsigned int nb;
    unsigned int cp;
    HEADER *up;

    cp = heap_start;
    nb = ROUND_UP(nu * sizeof(HEADER), NALLOC);
    if(heap_start + nb > heap_end) {
        return NULL;
    }
    heap_start += nb;
    up = (HEADER *)cp;
    up->s.size = nb / sizeof(HEADER);
    free((void *)(up + 1));

    return freep;
}
