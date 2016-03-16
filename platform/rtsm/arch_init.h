#ifndef __ARCH_INIT_H__
#define __ARCH_INIT_H__

#include <stdint.h>
#include <arch/armv7.h>    //smp_processor_id()
#include <assert.h>
#include <asm/asm.h>

void irq_chip_init();

#endif
