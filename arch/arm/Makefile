include ${SOURCE_PATH}/arch/arm/v7/Makefile

ARCH_ASMS+= $(patsubst %, v7/%, ${ARMV7_ASMS})

ARCH_SRCS+= arch_init.c
ARCH_SRCS+= gic-v2.c
ARCH_SRCS+= lpae.c
ARCH_SRCS+= mm.c
ARCH_SRCS+= io.c
ARCH_SRCS+= trap.c
ARCH_SRCS+= vgic.c

SUBDIRECTORIES += arch/arm/v7
