include ${SOURCE_PATH}/drivers/vdev/Makefile

DRIVER_SRCS+= $(patsubst %, vdev/%, ${VDEV_SRCS})

DRIVER_SRCS+= generic_timer.c
ifeq (${TARGET},rtsm)
DRIVER_SRCS+= serial_pl01x.c
#DRIVER_SRCS+= sp804.c
endif
ifeq (${TARGET},lager)
DRIVER_SRCS+= serial_sh.c
endif
ifeq (${TARGET},odroidxu)
DRIVER_SRCS+= serial_s5p.c
DRIVER_SRCS+= mct.c
endif
DRIVER_SRCS+= gic-v2.c

SUBDIRECTORIES += drivers/vdev
