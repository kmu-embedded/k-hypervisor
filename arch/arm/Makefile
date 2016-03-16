include ${SOURCE_PATH}/arch/arm/v7/Makefile

ARCH_ASMS+= $(patsubst %, v7/%, ${ARMV7_ASMS})

ARCH_SRCS+= hvc_trap.c
ARCH_SRCS+= init.c
ARCH_SRCS+= io.c
ARCH_SRCS+= irq.c
ARCH_SRCS+= lpae.c
ARCH_SRCS+= paging.c
ARCH_SRCS+= cpu.c

SUBDIRECTORIES += arch/arm/v7
