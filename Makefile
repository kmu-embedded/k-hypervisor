# Makefile
#
SOURCE_PATH=.
export SOURCE_PATH
BUILD_PATH=.
export BUILD_PATH

include ${SOURCE_PATH}/arch/arm/Makefile
include ${SOURCE_PATH}/core/Makefile
include ${SOURCE_PATH}/drivers/Makefile
include ${SOURCE_PATH}/lib/c/src/Makefile

ASMS+= $(patsubst %, arch/arm/%, ${ARCH_ASMS})
ASMS+= $(patsubst %, lib/c/src/%, ${LIBC_ASMS})

SRCS+= $(patsubst %, arch/arm/%, ${ARCH_SRCS})
SRCS+= $(patsubst %, core/%, ${CORE_SRCS})
SRCS+= $(patsubst %, drivers/%, ${DRIVER_SRCS})
SRCS+= $(patsubst %, lib/c/src/%, ${LIBC_SRCS})

OBJS+= $(ASMS:%.S=%.o)
OBJS+= $(SRCS:%.c=%.o)

V ?= 1
ifeq ($V, 1)
    Q = @
else
ifeq ($V, 2)
    Q =
endif
endif


######################################################
# TOOLCHAINE VARIABLES
######################################################
CROSS_COMPILE=arm-linux-gnueabihf-
CC=${CROSS_COMPILE}gcc
LD=${CROSS_COMPILE}ld
NM=${CROSS_COMPILE}nm
OBJCOPY=${CROSS_COMPILE}objcopy

######################################################
# FLAGS
######################################################
CPU:=cortex-a15
ARMV:=armv7-a

CFLAGS= -nodefaultlibs -nostartfiles -nostdlib -nostdinc -ffreestanding
CFLAGS+= -Wall
CFLAGS+= -mcpu=${CPU} -marm
DEFINES= -D__CONFIG_MUTEX__#-D__CONFIG_SMP__ #-D__TEST_TIMER__
CFLAGS+=${DEFINES}

#CONFIG_99=y
ifeq (${CONFIG_99}, y)
CFLAGS+= --std=c99 -DCONFIG_C99
endif

ASFLAGS+= -Wa,-mcpu=${CPU} -Wa,-march=${ARMV}

#DEBUG=y
ifdef DEBUG
    CFLAGS+= -ggdb -g3
endif

INCLUDES= -I${SOURCE_PATH}/include
INCLUDES+= -I${SOURCE_PATH}/arch/arm
INCLUDES+= -I${SOURCE_PATH}/arch/arm/platform
INCLUDES+= -I${SOURCE_PATH}/drivers/vdev
INCLUDES+= -I${SOURCE_PATH}/lib/c/include

######################################################
# OUTPUT FILENAMES
######################################################
MACHINE			= rtsm
PROJECT			= khypervisor
TARGET			= khypervisor-$(MACHINE)
LD_SCRIPT		= ${PROJECT}.lds.S
OUTPUT			= ${TARGET}.axf
MAP				= ${PROJECT}.map
BIN				= ${BUILD_PATH}/${TARGET}.bin

######################################################
# BUILD RULES
######################################################
all: ${OUTPUT} ${LD_SCRIPT} ${OBJS}
%.o: %.S
	${Q}echo "[AS] $@"
	${Q}${CC} ${ASFLAGS} ${INCLUDES} -c $< -o ${BUILD_PATH}/$@

%.o: %.c
	${Q}echo "[CC] $(notdir $@)"
	${Q}${CC} ${CFLAGS} ${INCLUDES} -c $< -o ${BUILD_PATH}/$@

${MACHINE}.lds: ${LD_SCRIPT}
	${Q}echo "[LD SCRIPT] $@"
	${Q}${CC} ${CFLAGS} ${INCLUDES} -E -P -o ${BUILD_PATH}/$@ -x c $<

${OUTPUT}: ${OBJS} ${MACHINE}.lds
	${Q}echo "[LD] $@"
	${Q}${LD} ${LDFLAGS} ${OBJS} -e __start -T ${BUILD_PATH}/${MACHINE}.lds -o ${BUILD_PATH}/$@

clean:
	${Q}echo "clean ${OBJS}"
	${Q}rm -rf ${BUILD_PATH}/${OBJS} ${BUILD_PATH}/${OUTPUT} ${BUILD_PATH}/${MACHINE}.lds
