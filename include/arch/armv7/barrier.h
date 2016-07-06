#ifndef __BARRIER_H__
#define __BARRIER_H__

#include <asm/asm.h>

#define isb() 		asm __volatile__ ("isb" : : : "memory")
#define dsb(option) asm __volatile__ ("dsb " #option : : : "memory")
#define dmb(option) asm __volatile__ ("dmb " #option : : : "memory")

#define mb()    	dmb(sy)
#define rmb()   	dmb(sy)
#define wmb()   	dmb(st)

#define smp_mb()    dmb(ish)
#define smp_rmb()   dmb(ish)
#define smp_wmb()   dmb(ishst)

#endif /* __BARRIER_H__ */
