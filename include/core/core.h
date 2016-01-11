#ifndef __CORE_H__
#define __CORE_H__

#include <hvmm_types.h>
#include <interrupt.h>

extern struct guest_virqmap _guest_virqmap[];

hvmm_status_t hypervisor_init();

#endif /* __CORE_H__ */

