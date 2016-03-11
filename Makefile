# Makefile
# TODO(wonseok): Add configuration file for board.

######################################################
# MAKEFILE VERBOSE OPTION
######################################################
V ?= 1
ifeq ($V, 1)
    Q = @
else
ifeq ($V, 2)
    Q =
endif
endif

######################################################
# DEFINE BUILD VARIABLES and PATHs
######################################################
SOURCE_PATH:=.
export SOURCE_PATH
BUILD_PATH:=./build
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

OBJS+= $(ASMS:%.S=${BUILD_PATH}/%.o)
OBJS+= $(SRCS:%.c=${BUILD_PATH}/%.o)

DIRECTORIES += ${BUILD_PATH}/arch/arm
DIRECTORIES += ${BUILD_PATH}/core
DIRECTORIES += ${BUILD_PATH}/drivers
DIRECTORIES += ${BUILD_PATH}/lib/c/src
DIRECTORIES += $(addprefix ${BUILD_PATH}/, ${SUBDIRECTORIES})

######################################################
# DEFINE TOOLCHAINE VARIABLES
######################################################
CROSS_COMPILE?=arm-linux-gnueabihf-
CC=${CROSS_COMPILE}gcc
LD=${CROSS_COMPILE}ld
NM=${CROSS_COMPILE}nm
OBJCOPY=${CROSS_COMPILE}objcopy

######################################################
# DEFINE FLAGS
######################################################
CPU:=cortex-a15
ARMV:=armv7-a

ASFLAGS+= -Wa,-mcpu=${CPU} -Wa,-march=${ARMV}

CFLAGS= -nodefaultlibs -nostartfiles -nostdlib -nostdinc -ffreestanding
CFLAGS+= -Wall -Werror
CFLAGS+= -mcpu=${CPU} -marm
DEFINES= -D__CONFIG_MUTEX__#-D__CONFIG_SMP__ #-D__TEST_TIMER__
CFLAGS+=${DEFINES}

# BUILD: Passed --std==gnu90, --std==gnu99, --std=gnu11
# CONFIG_C99=y
ifeq (${CONFIG_C99}, y)
CFLAGS+= --std=c99 -DCONFIG_C99
endif

DEBUG=y
ifdef DEBUG
    DFLAGS+= -ggdb -g3
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
all: ${OUTPUT} ${MACHINE}.lds

${OUTPUT}: ${OBJS} ${MACHINE}.lds
	${Q}echo "[LD] $@"
	${Q}${LD} ${LDFLAGS} ${OBJS} -e __start -T ${BUILD_PATH}/${MACHINE}.lds -o $@

${MACHINE}.lds: ${LD_SCRIPT} | ${DIRECTORIES}
	${Q}echo "[LD SCRIPT] $@"
	${Q}${CC} ${CFLAGS} ${INCLUDES} -E -P -o ${BUILD_PATH}/$@ -x c $<

${BUILD_PATH}/%.o: %.S | ${DIRECTORIES}
	${Q}echo "[AS] $@"
	${Q}${CC} ${ASFLAGS} ${DFLAGS} ${INCLUDES} -c $< -o $@

${BUILD_PATH}/%.o: %.c | ${DIRECTORIES}
	${Q}echo "[CC] $(notdir $@)"
	${Q}${CC} ${CFLAGS} ${DFLAGS} ${INCLUDES} -c $< -o $@

${DIRECTORIES}:
	@echo mkdir ${DIRECTORIES}
	mkdir -p ${DIRECTORIES}

style:
	astyle --max-instatement-indent=120 --style=otbs --pad-header --recursive --indent=spaces=4 --pad-oper "*.c"
	astyle --max-instatement-indent=120 --style=otbs --pad-header --recursive --indent=spaces=4 --pad-oper "*.h"

clean:
	${Q}echo "clean ${OBJS}"
	${Q}rm -rf ${OBJS} ${OUTPUT} ${BUILD_PATH}/${MACHINE}.lds
	${Q}if [ -d $(BUILD_PATH) ]; then rm -r ${BUILD_PATH}; fi
