#include <vdev.h>
#include <stdio.h>
#include <debug.h>
#include <arch/armv7.h>
#include <stdlib.h>

#define MAX_VDEV    256

static struct list_head vdev_list;

struct vdev_module *vdev_modules[MAX_VDEV];

hvmm_status_t vdev_register(struct vdev_module *module)
{
    struct vdev_instance *vdev_instance = malloc(sizeof(struct vdev_instance));

    if (module == NULL) {
        debug_print("Failed register vdev %s\n", module->name);
        return HVMM_STATUS_NOT_FOUND;
    }

    vdev_instance->module = module;
    LIST_ADDTAIL(&vdev_instance->head, &vdev_list);

    return HVMM_STATUS_SUCCESS;
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

    LIST_INITHEAD(&vdev_list);

    if (!cpu) {
        for (fn = __vdev_module_start; fn < __vdev_module_end; fn++) {
            if (vdev_module_initcall(*fn)) {
                debug_print("vdev : low initial call error\n");
                return HVMM_STATUS_UNKNOWN_ERROR;
            }
        }
    }

    return 0;
}

void vdev_create(struct vdev_instance *vdevs, vmid_t vmid)
{
    LIST_INITHEAD(&vdevs->head);

    // FIXME(casionwoo) : This copy of all the vdevs would be replaced to .config file.
    //                    In other words copy only vdevs that VMCB needs
    struct vdev_instance *vdev_instance;
    list_for_each_entry(struct vdev_instance, vdev_instance, &vdev_list, head) {
        struct vdev_instance *vdev = malloc(sizeof(struct vdev_instance));

        vdev->owner = vmid;
        vdev->module = vdev_instance->module;
        vdev->module->create(&vdev->pdata);

        LIST_ADDTAIL(&vdev->head, &vdevs->head);
    }
}

