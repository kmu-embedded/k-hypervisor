#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>

#define __ARCH_MUTEX_UNLOCKED   0
#define __ARCH_MUTEX_LOCKED     1

extern void lock_mutex(void * mutex);
extern void unlock_mutex(void * mutex);

typedef struct {
    volatile uint32_t mutex;
} mutex_t;

#define MUTEX_UNLOCKED(m) \
    { .mutex = __ARCH_MUTEX_UNLOCKED, }

#define DEFINE_MUTEX(m) mutex_t m = MUTEX_UNLOCKED(m);

#endif //__MUTEX_H__
