#include <vdev.h>
#define DEBUG
#include <stdio.h>
#include <core/scheduler.h>
#include <config.h>
#include <stdlib.h>
#include <core/vm.h>

#define SAMPLE_BASE_ADDR 0x3FFFF000


struct vdev_sample_regs {
    uint32_t axis_x;
    uint32_t axis_y;
    uint32_t axis_z;
};

static int32_t vdev_sample_read(void *pdata, uint32_t offset);
static int32_t vdev_sample_write(void *pdata, uint32_t offset, uint32_t *addr);
int32_t vdev_sample_create_instance(void **pdata);

struct vdev_module vdev_sample = {
		.name = "vdev_sample",
		.base = SAMPLE_BASE_ADDR,
		.size = 4096,
		.read = vdev_sample_read,
		.write = vdev_sample_write,
		.create = vdev_sample_create_instance,
};

int32_t vdev_sample_create_instance(void **pdata)
{
	*pdata = malloc(sizeof(struct vdev_sample_regs));
	return 0;
}

static int32_t vdev_sample_read(void *pdata, uint32_t offset)
{
	struct vdev_sample_regs *sample_regs = pdata;

	/* READ */
	switch (offset) {
	case 0x0:
		return sample_regs->axis_x;
		break;
	case 0x4:
		return sample_regs->axis_y;
		break;
	case 0x8:
		return sample_regs->axis_x + sample_regs->axis_y;
		break;
	}
	return 0;
}

static int32_t vdev_sample_write(void *pdata, uint32_t offset, uint32_t *addr)
{

	struct vdev_sample_regs *sample_regs = pdata;

	/* WRITE */
	switch (offset) {
	case 0x0:
		sample_regs->axis_x = readl(addr);
		break;
	case 0x4:
		sample_regs->axis_y = readl(addr);
		break;
	case 0x8:
		/* read-only register, ignored, but no error */
		break;
	}
	return 0;
}


hvmm_status_t vdev_sample_init()
{
    hvmm_status_t result = HVMM_STATUS_BUSY;

    result = vdev_register(&vdev_sample);
    if (result == HVMM_STATUS_SUCCESS) {
        printf("vdev registered:'%s'\n", vdev_sample.name);
    } else {
        printf("%s: Unable to register vdev:'%s' code=%x\n",
               __func__, vdev_sample.name, result);
    }

    return result;
}
vdev_module_init(vdev_sample_init);
