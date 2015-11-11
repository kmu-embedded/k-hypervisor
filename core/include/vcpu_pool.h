#ifndef __VCPU_POOL_H__
#define __VCPU_POOL_H__

#include <vcpu.h>
#include <hvmm_type.h>

#define MAX_VCPU_SIZE 32


struct vcpu* vcpup_create();
hvmm_state_t vcpup_delete(unsigned int vcpuid);
struct vcpu* vcpup_get(unsigned int vcpuid);

unsigned int vcpup_available_vcpus();

#endif // __VCPU_POOL_H__
