objs-y :=

objs-y += vdev_gicd.o
objs-$(CONFIG_VDEV_SAMPLE) += vdev_sample.o
objs-$(CONFIG_VDEV_SP804) += vdev_sp804.o
objs-$(CONFIG_VDEV_SYSREG) += vdev_sysreg.o

obj-y += $(patsubst %, drivers/vdev/%, ${objs-y})

