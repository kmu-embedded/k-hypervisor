# Makefile
# TODO(wonseok): Add configuration file for board.

TARGET         = rtsm
DEFINES+=-DSERIAL_PL01X
#TARGET          = lager
#DEFINES+=-DSERIAL_SH
#TARGET        = odroidxu
#DEFINES+=-DSERIAL_S5P -DCONFIG_MCT
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
include ${SOURCE_PATH}/platform/Makefile

ASMS+= $(patsubst %, arch/arm/%, ${ARCH_ASMS})
ASMS+= $(patsubst %, lib/c/src/%, ${LIBC_ASMS})

SRCS+= $(patsubst %, arch/arm/%, ${ARCH_SRCS})
SRCS+= $(patsubst %, core/%, ${CORE_SRCS})
SRCS+= $(patsubst %, drivers/%, ${DRIVER_SRCS})
SRCS+= $(patsubst %, lib/c/src/%, ${LIBC_SRCS})
SRCS+= $(patsubst %, platform/%, ${PLAT_SRCS})

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
CFLAGS += --std=c99

#DEFINES= -D__CONFIG_MUTEX__ #-D__CONFIG_SMP__
#DEFINES+=-D__CONFIG_SMP__
DEFINES+=-D__CONFIG_TICKLESS_TIMER__ #-D__TEST_TIMER__
DEFINES+=-DCONFIG_C99
ASFLAGS+=${DEFINES}
CFLAGS+=${DEFINES}
# BUILD: Passed --std==gnu90, --std==gnu99, --std=gnu11

DEBUG=y
ifdef DEBUG
    DFLAGS+= -ggdb -g3
endif

INCLUDES= -I${SOURCE_PATH}/include
INCLUDES+= -I${SOURCE_PATH}/lib/c/include
INCLUDES+= -I${SOURCE_PATH}/platform/${TARGET}

#DEFINES+=-D__CONFIG_ATAGS

######################################################
# OUTPUT FILENAMES
######################################################
PROJECT			= khypervisor
LD_SCRIPT		= platform/${TARGET}/${TARGET}.lds.S
OUTPUT			= khypervisor-${TARGET}
ELF				= ${OUTPUT}.axf
MAP				= ${OUTPUT}.map
BIN				= ${OUTPUT}.bin

######################################################
# BUILD RULES
######################################################
all: ${ELF} ${TARGET}.lds ${MAP} ${BIN}

${ELF}: ${OBJS} ${TARGET}.lds
	${Q}echo "[LD] $@"
	${Q}${LD} ${LDFLAGS} ${OBJS} -e __start -T ${BUILD_PATH}/${TARGET}.lds -o $@

${TARGET}.lds: ${LD_SCRIPT} | ${DIRECTORIES}
	${Q}echo "[LD SCRIPT] $@"
	${Q}${CC} ${CFLAGS} ${INCLUDES} -E -P -o ${BUILD_PATH}/$@ -x c $<

${BUILD_PATH}/%.o: %.S | ${DIRECTORIES}
	${Q}echo "[AS] $(notdir $@)"
	${Q}${CC} ${ASFLAGS} ${DFLAGS} ${INCLUDES} -c $< -o $@

${BUILD_PATH}/%.o: %.c | ${DIRECTORIES}
	${Q}echo "[CC] $(notdir $@)"
	${Q}${CC} ${CFLAGS} ${DFLAGS} ${INCLUDES} -c $< -o $@

${DIRECTORIES}:
	${Q}mkdir -p ${DIRECTORIES}

${MAP}: ${ELF}
	${Q}echo "[NM] $@"
	${Q}${NM} $< > ${BUILD_PATH}/$@

${BIN}: ${ELF}
	${Q}echo "[BIN] $@"
	${Q}${OBJCOPY} -O binary $< ${BUILD_PATH}/$@

style:
	astyle --max-instatement-indent=120 --style=otbs --pad-header --recursive --indent=spaces=4 --pad-oper "*.c"
	astyle --max-instatement-indent=120 --style=otbs --pad-header --recursive --indent=spaces=4 --pad-oper "*.h"
	${Q}find . -name '*.[chsCHS]' -exec chmod 664 {} \;
	${Q}find . -name '*.orig' -exec rm {} \;


clean:
	${Q}echo "[CLEAN] ${PROJECT}"
	${Q}rm -rf ${OBJS} ${ELF} ${BUILD_PATH}/${MACHINE}.lds
	${Q}if [ -d $(BUILD_PATH) ]; then rm -r ${BUILD_PATH}; fi
