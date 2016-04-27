include ${SOURCE_PATH}/arch/arm/v7/Makefile

ARCH_ASMS+= $(patsubst %, v7/%, ${ARMV7_ASMS})

ARCH_SRCS+= cp15.c
ARCH_SRCS+= traps.c
ARCH_SRCS+= init.c
ARCH_SRCS+= io.c
ARCH_SRCS+= irq.c
ARCH_SRCS+= lpae.c
ARCH_SRCS+= paging.c
ARCH_SRCS+= cpu.c
ARCH_SRCS+= arch_regs.c
ARCH_SRCS+= fault.c

SUBDIRECTORIES += arch/arm/v7
