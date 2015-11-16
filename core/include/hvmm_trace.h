#ifndef __HVMM_TRACE_H__
#define __HVMM_TRACE_H__

//#include <lib/printf.h>
#include <lib/print.h>

#ifdef DEBUG
#define HVMM_TRACE_ENTER()              \
    do {                                \
        printf(__func__);           \
        printf("() - enter\n\r");   \
    } while (0)

#define HVMM_TRACE_EXIT()               \
    do {                                \
        printf(__func__);           \
        printf("() - exit\n\r");    \
    } while (0)

#define HVMM_TRACE_HEX32(label, value)  \
    do {                                \
        printf(label);              \
        printf("0x%08x", value);        \
        printf("\n\r");             \
    } while (0)
#else
#define HVMM_TRACE_ENTER()
#define HVMM_TRACE_EXIT()
#define HVMM_TRACE_HEX32(label, value)
#endif

#define hyp_abort_infinite() { while (1) ; }

#endif
