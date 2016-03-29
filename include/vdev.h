#ifndef __VDEV_H_
#define __VDEV_H_

#include "types.h"
#include <stdint.h>
#include <lib/list.h>

#define VDEV_ERROR -1
#define VDEV_NOT_FOUND -1

typedef int (*initcall_t)(void);


extern initcall_t __vdev_module_start[];
extern initcall_t __vdev_module_end[];

#define __define_vdev_module(fn) \
    static initcall_t __initcall_##fn __attribute__((__used__)) \
    __attribute__((__section__(".vdev_module.init"))) = fn

#define vdev_module_init(fn)        __define_vdev_module(fn)

struct list_head vdev_list;

struct vdev_module {
    uint32_t id;

    uint32_t base;
    uint32_t size;

    const char *name;

    int32_t (* create) (void **pdata);
    int32_t (* read) (void *pdata, uint32_t offset);
    int32_t (* write) (void *pdata, uint32_t offset, uint32_t *addr);
};

struct vdev_instance {
	const struct vdev_module *module;
	void *pdata;
	vmid_t owner;

	struct list_head head;
};

hvmm_status_t vdev_register(struct vdev_module *module);
hvmm_status_t vdev_init(void);

const struct vdev_module * vdev_find(uint32_t fipa);

#endif /* __VDEV_H_ */
