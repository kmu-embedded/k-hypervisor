#include <stdio.h>
#include <stdlib.h>
#include <vdev.h>

#define MAX_VDEV    256

static struct list_head vdev_list;

void vdev_register(struct vdev_module *module)
{
    if (module == NULL) {
        printf("Failed register vdev %s\n", module->name);
        return;
    }

    LIST_ADDTAIL(&module->head, &vdev_list);
}

void vdev_create(struct vdev_instance *vdevs, vmid_t vmid)
{
    LIST_INITHEAD(&vdevs->head);

    struct vdev_module *module;
    list_for_each_entry(struct vdev_module, module, &vdev_list, head) {
        struct vdev_instance *vdev = malloc(sizeof(struct vdev_instance));

        vdev->owner = vmid;
        vdev->module = module;
        vdev->module->create(&vdev->pdata);

        LIST_ADDTAIL(&vdev->head, &vdevs->head);
    }
}

static uint32_t vdev_module_initcall(initcall_t fn)
{
    return	fn();
}

#include <asm/macro.h>
void SECTION(".init.vdev") vdev_init(void)
{
    LIST_INITHEAD(&vdev_list);

    initcall_t *fn;
    for (fn = __vdev_module_start; fn < __vdev_module_end; fn++) {
        vdev_module_initcall(*fn);
    }
}
