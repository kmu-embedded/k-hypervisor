#ifndef __VM_CONFIG_H__
#define __VM_CONFIG_H__

#include <stdint.h>
#include <core/vm/vm.h>

struct vm_config {
    uint8_t nr_vcpus;
    uint32_t va_offsets;
    addr_t pa_start;
    vmcb_type_t vmcb_type;
};

extern struct vm_config vm_conf[];

#endif
