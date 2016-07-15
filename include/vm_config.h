#ifndef __VM_CONFIG_H__
#define __VM_CONFIG_H__

#include <stdint.h>
#include <generated/autoconf.h>

struct vm_config {
    uint8_t nr_vcpus;
    //addr_t va_start;
    //uint32_t va_offsets;
    //addr_t pa_start;
};

struct vm_config vm_conf[256] = {
    { 2 },
    { 2 },
    { 1 }
};

#endif
