#include <stdio.h>
#include <stdlib.h>
#include <vdev.h>
#include <core/scheduler.h>
#include <core/vm/vm.h>
#include <arch/armv7.h>
#include <core/kmus.h>

#define MAX_VDEV    256

static struct list_head vdev_list;

void vdev_handler(struct core_regs *regs, uint32_t iss)
{
    struct vmcb *vm = get_current_vm();
    uint32_t fipa = 0;
    uint8_t wnr = WNR(iss);
    uint8_t srt = SRT(iss);

    fipa = read_cp32(HPFAR) << IPA_SHIFT;
    fipa |= (read_cp32(HDFAR) & PAGE_OFFSET_MASK);

    struct vdev_instance *instance;
    list_for_each_entry(struct vdev_instance, instance, &vm->vdevs.head, head) {
        uint32_t vdev_base = instance->module->base;
        uint32_t vdev_size = instance->module->size;

        if (vdev_base <= fipa && fipa < vdev_base + vdev_size) {
            uint32_t offset = fipa - vdev_base;
            if (wnr) {
                instance->module->write(instance->pdata, offset, &(regs->gpr[srt]));
            } else {
                regs->gpr[srt] = instance->module->read(instance->pdata, offset);
            }

            break;
        }
    }
}
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

void vdev_delete(struct vdev_instance *vdevs)
{
    LIST_DEL(&vdevs->head);
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

void vdev_copy(struct vdev_instance *vdev_from, struct vdev_instance *vdev_to)
{
    struct vdev_instance *from_instance;
    struct vdev_instance *to_instance;
    list_for_each_entry(struct vdev_instance, from_instance, &vdev_from->head, head) {
        list_for_each_entry(struct vdev_instance, to_instance, &vdev_to->head, head) {
            if (from_instance->module == NULL || to_instance->module == NULL)
                return;

            if (from_instance->module->base == to_instance->module->base) {
                printf("kmus [%s] vdev copy\n", from_instance->module->name);
                if (from_instance->module->copy)
                    from_instance->module->copy(&from_instance->pdata, &to_instance->pdata);

            }
        }
    }

}

