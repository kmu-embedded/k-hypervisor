#include <stdio.h>
#include <stdlib.h>
#include <vdev.h>

#define MAX_VDEV    256

static struct list_head vdev_list;

void vdev_register(struct vdev_module *module)
{
    struct vdev_instance *vdev_instance = malloc(sizeof(struct vdev_instance));

    if (module == NULL) {
        printf("Failed register vdev %s\n", module->name);
        return;
    }

    vdev_instance->module = module;
    LIST_ADDTAIL(&vdev_instance->head, &vdev_list);
}

uint32_t vdev_module_initcall(initcall_t fn)
{
    return	fn();
}

#include <asm/macro.h>
void SECTION(".init.vdev") vdev_init(void)
{
    initcall_t *fn;

    LIST_INITHEAD(&vdev_list);

	for (fn = __vdev_module_start; fn < __vdev_module_end; fn++) {
		vdev_module_initcall(*fn);
	}
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

