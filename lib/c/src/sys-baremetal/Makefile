include ${SOURCE_PATH}/lib/c/src/sys-baremetal/arch-arm/Makefile

objs-y :=

SUBDIRECTORIES += arch-arm

objs-y += sys_abort.o

obj-y += $(patsubst %, lib/c/src/sys-baremetal/%, ${objs-y})
