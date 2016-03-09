include ${SOURCE_PATH}/core/sched/Makefile
include ${SOURCE_PATH}/core/vm/Makefile

CORE_SRCS+= $(patsubst %, sched/%, ${SCHED_SRCS})
CORE_SRCS+= $(patsubst %, vm/%, ${VM_SRCS})

CORE_SRCS+= interrupt.c
CORE_SRCS+= main.c
CORE_SRCS+= vdev.c
CORE_SRCS+= timer.c
