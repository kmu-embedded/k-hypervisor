#ifndef __GUEST_MEMORY_HW_H__
#define __GUEST_MEMORY_HW_H__
#include <hvmm_types.h>

hvmm_status_t guest_memory_set_vmid_ttbl(vmid_t vmid, char *ttbl);
void guest_memory_stage2_enable(int enable);

#endif //__GUEST_MEMORY_HW__
