#include ../config.mk

INCLUDES	+= -I./

include ${SOURCE_PATH}/drivers/vdev/Makefile

DRIVER_SRCS+= $(patsubst %, vdev/%, ${VDEV_SRCS})
DRIVER_SRCS+= generic_timer.c
DRIVER_SRCS+= pl01x.c
DRIVER_OBJS+= $(DRIVER_SRCS:%.c=%.o)

#all: $(DRIVER_OBJS)

# alternatives: .S.o:
#%.o: %.c
#	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
#
#clean:
#	rm -rf $(DRIVER_OBJS)
#
##.PHONY: clean
