include ${SOURCE_PATH}/lib/c/src/sys-baremetal/arch-arm/Makefile

LIBC_BM_SRCS += sys_abort.c
LIBC_BM_SRCS += $(patsubst %, arch-arm/%, ${LIBC_BM_ARCH_SRCS})

SUBDIRECTORIES += arch-arm
