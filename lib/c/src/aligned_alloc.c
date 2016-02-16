#include <stdlib.h>
#include <stdio.h>

void *
aligned_alloc(size_t nbytes, size_t alignment)
{
    void *p;
    void **aligned_p;
    int offset = alignment - 1 + sizeof(void *);

    p = (void *) malloc (nbytes + offset);
    if (p == NULL) {
        return p;
    }

    aligned_p = (void **)(((size_t)(p) + offset) & ~(alignment - 1));
    aligned_p[-1] = p;

    return aligned_p;
}

void
aligned_free(void *p)
{
    free(((void**)p)[-1]);
}
