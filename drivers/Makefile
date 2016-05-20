include ${SOURCE_PATH}/drivers/vdev/Makefile
include ${SOURCE_PATH}/drivers/serial/Makefile

objs-y :=

SUBDIRECTORIES += drivers/vdev
SUBDIRECTORIES += drivers/serial

objs-y += generic_timer.o
objs-y += gic-v2.o
objs-$(CONFIG_MCT) += mct.o

obj-y += $(patsubst %, drivers/%, ${objs-y})
