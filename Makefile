# Makefile
# TODO(wonseok): Add configuration file for board.

-include $(CURDIR)/.config

######################################################
# MAKEFILE VERBOSE OPTION
######################################################
ifeq ($(CONFIG_GCC_V), y)
	Q =
else
	Q = @
endif

######################################################
# DEFINE BUILD VARIABLES and PATHs
######################################################
SOURCE_PATH:=.
export SOURCE_PATH
BUILD_PATH:=./build
export BUILD_PATH

obj-y :=
export obj-y

TARGET = $(CONFIG_TARGET_NAME:"%"=%)
export TARGET

include ${SOURCE_PATH}/arch/arm/Makefile
include ${SOURCE_PATH}/core/Makefile
include ${SOURCE_PATH}/drivers/Makefile
include ${SOURCE_PATH}/lib/c/src/Makefile
include ${SOURCE_PATH}/platform/Makefile

obj-y:= $(obj-y:%.o=${BUILD_PATH}/%.o)

DIRECTORIES += ${BUILD_PATH}/arch/arm
DIRECTORIES += ${BUILD_PATH}/core
DIRECTORIES += ${BUILD_PATH}/drivers
DIRECTORIES += ${BUILD_PATH}/lib/c/src
DIRECTORIES += ${BUILD_PATH}/platform
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
CPU:=$(CONFIG_CPU_NAME:"%"=%)
ARMV:=$(CONFIG_ARCH_NAME:"%"=%)

ASFLAGS+= -Wa,-mcpu=${CPU} -Wa,-march=${ARMV}

CFLAGS= -nodefaultlibs -nostartfiles -nostdlib -nostdinc -ffreestanding
CFLAGS+= -Wall -Werror
CFLAGS+= -mcpu=${CPU} -marm
CFLAGS += --std=c99

CFLAGS += -include $(SOURCE_PATH)/include/generated/autoconf.h

DEFINES+=-DCONFIG_C99
ASFLAGS+=${DEFINES} -D__ASSEMBLY__
CFLAGS+=${DEFINES}
# BUILD: Passed --std==gnu90, --std==gnu99, --std=gnu11

ifeq ($(CONFIG_DEBUG), y)
	DFLAGS+= -ggdb -g3
endif

INCLUDES= -I${SOURCE_PATH}/include
INCLUDES+= -I${SOURCE_PATH}/lib/c/include
INCLUDES+= -I${SOURCE_PATH}/platform/${TARGET}

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

${ELF}: ${obj-y} ${TARGET}.lds
	${Q}echo "[LD] $@"
	${Q}${LD} ${LDFLAGS} ${obj-y} -e __start -T ${BUILD_PATH}/${TARGET}.lds -o $@

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
	${Q}rm -rf ${obj-y} ${ELF} ${BUILD_PATH}/${MACHINE}.lds ${SOURCE_PATH}/*.axf
	${Q}if [ -d $(BUILD_PATH) ]; then rm -r ${BUILD_PATH}; fi

distclean: clean
	@find . \( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
        -o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \) \
        -type f -print | xargs rm -f
	@find . \( -name 'fixdep' -o -name 'docproc' -o -name 'split-include' \
        -o -name 'autoconf.h' -o -name '.config' -o -name '.config.old' \
        -o -name 'qconf' -o -name 'gconf' -o -name 'kxgettext' \
        -o -name 'mconf' -o -name 'conf' -o -name 'lxdialog' \) \
        -type f -print | xargs rm -f


######################################################
# KCONFIG
######################################################

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	else if [ -x /bin/bash ]; then echo /bin/bash; \
	else echo sh; fi ; fi)

MAKEFLAGS += --include-dir=$(SOURCE_PATH)

HOSTCC = gcc
HOSTCFLAGS :=

srctree := $(SOURCE_PATH)
include $(srctree)/scripts/Kbuild.include

export srctree CONFIG_SHELL HOSTCC HOSTCFLAGS

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) -s $(build)=scripts/basic

%config: scripts_basic FORCE
	$(Q)$(MAKE) -s $(build)=scripts/kconfig $@

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)

