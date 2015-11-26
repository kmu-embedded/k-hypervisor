#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "k_r_malloc.h"

typedef __SIZE_TYPE__ size_t;
extern HEADER *freep;

void init_heap(); //(unsigned int *head_base,  unsigned int *heap_end);
void *malloc(size_t nbytes);
void free(void *ap);

#endif

