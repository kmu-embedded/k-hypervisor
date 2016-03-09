#include ../../config.mk

include ${SOURCE_PATH}/arch/arm/v7/Makefile
include ${SOURCE_PATH}/arch/arm/platform/Makefile

ARCH_ASMS+= $(patsubst %, v7/%, ${ARMV7_ASMS})
ARCH_OBJS+= $(ARCH_ASMS:%.S=%.o)

ARCH_SRCS+= $(patsubst %, platform/%, ${PLATFORM_SRCS})
ARCH_SRCS+= arch_init.c
ARCH_SRCS+= gic-v2.c
ARCH_SRCS+= lpae.c
ARCH_SRCS+= mm.c
ARCH_SRCS+= trap.c
ARCH_SRCS+= vgic.c

ARCH_OBJS+= $(ARCH_SRCS:%.c=%.o)

#INCLUDES	+= -I./
#all: $(ARCH_OBJS)

# alternatives: .S.o:
#%.o: %.S
#	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#
## alternatives: .S.o:
#%.o: %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#
#clean:
#	rm -rf $(ARCH_OBJS_DIR)
#
#.PHONY: clean
