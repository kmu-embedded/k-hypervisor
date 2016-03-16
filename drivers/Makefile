include ${SOURCE_PATH}/drivers/vdev/Makefile

DRIVER_SRCS+= $(patsubst %, vdev/%, ${VDEV_SRCS})

DRIVER_SRCS+= generic_timer.c
DRIVER_SRCS+= pl01x.c
DRIVER_SRCS+= gic-v2.c

SUBDIRECTORIES += drivers/vdev
