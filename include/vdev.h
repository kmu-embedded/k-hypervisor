#ifndef __VDEV_H_
#define __VDEV_H_

#include "types.h"
#include "core/vm/vcpu.h"

#define VDEV_ERROR -1
#define VDEV_NOT_FOUND -1

typedef int (*initcall_t)(void);

extern initcall_t __vdev_module_high_start[];
extern initcall_t __vdev_module_high_end[];
extern initcall_t __vdev_module_middle_end[];
extern initcall_t __vdev_module_low_end[];

#define __define_vdev_module(level, fn, id) \
    static initcall_t __initcall_##fn##id __attribute__((__used__)) \
    __attribute__((__section__(".vdev_module" level ".init"))) = fn

#define vdev_module_high_init(fn)       __define_vdev_module("0", fn, 1)
#define vdev_module_middle_init(fn)     __define_vdev_module("1", fn, 2)
#define vdev_module_low_init(fn)        __define_vdev_module("2", fn, 3)


struct vdev_module {
    uint32_t id;

    uint32_t base;

    uint32_t size;

    const char *name;

    int32_t (*read) (uint32_t offset, uint32_t *addr);

    int32_t (*write) (uint32_t offset, uint32_t *addr);

    struct vdev_ops *ops;

};

hvmm_status_t vdev_register(struct vdev_module *module);
hvmm_status_t vdev_init(void);
struct vdev_module * get_vdev(uint32_t fipa);
#endif /* __VDEV_H_ */
