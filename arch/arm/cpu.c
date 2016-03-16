#include "cpu.h"

void SECTION(".init.arch") cpu_init()
{
    uint32_t vector_base = (uint32_t) &__hvc_vector;
    write_hvbar(vector_base);
    assert(read_hvbar() == vector_base);
    // TODO(wonseok): D-Cache invalidate and TLB flush.
    // TODO(casionwoo): add cache operation in arch/arm
}
