#ifndef __VDEV_H_
#define __VDEV_H_

#include <stdint.h>
#include <types.h>
#include <lib/list.h>
#include <arch/armv7.h>
#include <arch/irq.h>

typedef hvmm_status_t (*initcall_t)(void);

extern initcall_t __vdev_module_start[];
extern initcall_t __vdev_module_end[];

#define __define_vdev_module(fn) \
    static initcall_t __initcall_##fn __attribute__((__used__)) \
    __attribute__((__section__(".vdev_module.init"))) = fn

#define vdev_module_init(fn)        __define_vdev_module(fn)

struct vdev_module {
    uint32_t id;

    uint32_t base;
    uint32_t size;

    const char *name;

    int32_t (* create) (void **pdata);
    int32_t (* read) (void *pdata, uint32_t offset);
    int32_t (* write) (void *pdata, uint32_t offset, uint32_t *addr);
    int32_t (* handle_irq) (void);
    int32_t (* copy) (void **pdata_from, void **pdata_to);

    struct list_head head;
};

struct vdev_instance {
    const struct vdev_module *module;
    void *pdata;

    vmid_t owner;

    struct list_head head;
};

void vdev_register(struct vdev_module *module);
void vdev_init(void);

void vdev_create(struct vdev_instance *, vmid_t);
void vdev_delete(struct vdev_instance *vdevs);
void vdev_handler(struct core_regs *regs, uint32_t iss);
void vdev_copy(struct vdev_instance *vdev_from, struct vdev_instance *vdev_to);

#endif /* __VDEV_H_ */
