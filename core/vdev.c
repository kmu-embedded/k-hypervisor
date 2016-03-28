#include <vdev.h>
#include <stdio.h>
#include <debug.h>
#include <arch/armv7.h>

#define MAX_VDEV    256

struct vdev_module *vdev_modules[MAX_VDEV];
static int nr_vdevs;

hvmm_status_t vdev_register(struct vdev_module *module)
{
    int i;
    hvmm_status_t result = HVMM_STATUS_BUSY;

    for (i = 0; i < MAX_VDEV; i++) {
        if (!vdev_modules[i]) {
            vdev_modules[i] = module;
            result = HVMM_STATUS_SUCCESS;
            break;
        }
    }

    if (result != HVMM_STATUS_SUCCESS) {
        debug_print("vdev : Failed registering vdev '%s', max %d full\n",
                    module->name, MAX_VDEV);
    }

    return result;
}

struct vdev_module *get_vdev(uint32_t fipa)
{
    int32_t i;

	for (i = 0; i < nr_vdevs; i++) {
		struct vdev_module *vdev = vdev_modules[i];
		if (vdev->base == fipa) {
			return vdev;
		}
	}
	return NULL;
}

hvmm_status_t vdev_module_initcall(initcall_t fn)
{
    return	fn();
}


#include "../include/asm/macro.h"
hvmm_status_t SECTION(".init.vdev") vdev_init(void)
{
    initcall_t *fn;
    uint32_t cpu = smp_processor_id();

    if (!cpu) {
        for (fn = __vdev_module_middle_end; fn < __vdev_module_low_end; fn++) {
            if (vdev_module_initcall(*fn)) {
                debug_print("vdev : low initial call error\n");
                return HVMM_STATUS_UNKNOWN_ERROR;
            }
            nr_vdevs++;
        }
    }

    return 0;
}
