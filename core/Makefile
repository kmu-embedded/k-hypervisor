include ${SOURCE_PATH}/core/sched/Makefile
include ${SOURCE_PATH}/core/vm/Makefile

objs-y :=

objs-y += main.o
objs-y += vdev.o
objs-y += timer.o

SUBDIRECTORIES += core/sched core/vm

obj-y += $(patsubst %, core/%, ${objs-y})
