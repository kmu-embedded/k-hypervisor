#ifndef __VM_POOL_H__
#define __VM_POOL_H__

#include <vm.h>
#include <hvmm_type.h>

#define MAX_VM_SIZE     8


struct vmcb* vmp_create();
hvmm_state_t vmp_delete(unsigned int vmid);
struct vmcb* vmp_get(unsigned int vmid);

#endif //__VM_POOL_H__
