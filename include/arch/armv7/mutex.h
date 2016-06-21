#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>

#ifdef CONFIG_SMP
#define __ARCH_MUTEX_UNLOCKED   0xFFFFFFFF
#define __ARCH_MUTEX_LOCKED     1

extern void init_mutex(void * mutex);
extern void lock_mutex(void * mutex);
extern void unlock_mutex(void * mutex);
extern void is_mutex_locked(void * mutex);
#else
#define __ARCH_MUTEX_UNLOCKED   0
#define __ARCH_MUTEX_LOCKED     1

extern void lock_mutex(void * mutex);
extern void unlock_mutex(void * mutex);
#endif

typedef struct {
    volatile uint32_t mutex;
} mutex_t;

#define MUTEX_UNLOCKED(m) \
    { .mutex = __ARCH_MUTEX_UNLOCKED, }

#define DEFINE_MUTEX(m) mutex_t m = MUTEX_UNLOCKED(m);

#endif //__MUTEX_H__

