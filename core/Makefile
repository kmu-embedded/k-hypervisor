#include ../config.mk

INCLUDES	+= -I./

include ${SOURCE_PATH}/core/sched/Makefile
include ${SOURCE_PATH}/core/vm/Makefile

CORE_SRCS+= $(patsubst %, sched/%, ${SCHED_SRCS})
CORE_SRCS+= $(patsubst %, vm/%, ${VM_SRCS})
CORE_SRCS+= interrupt.c
CORE_SRCS+= main.c
CORE_SRCS+= vdev.c
CORE_SRCS+= timer.c

#CORE_OBJS+= $(CORE_SRCS:%.c=%.o)

#all: $(CORE_OBJS)

# alternatives: .S.o:
#%.o: %.S
#	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#
## alternatives: .S.o:
#%.o: %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#
#clean:
#	rm -rf $(CORE_OBJS)
#
#.PHONY: clean
