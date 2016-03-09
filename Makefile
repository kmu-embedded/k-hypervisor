include config.mk

SOURCE_PATH=.
export SOURCE_PATH

include ${SOURCE_PATH}/arch/arm/Makefile
include ${SOURCE_PATH}/core/Makefile
include ${SOURCE_PATH}/drivers/Makefile
include ${SOURCE_PATH}/lib/c/src/Makefile

ASMS+= $(patsubst %, arch/arm/%, ${ARCH_ASMS})
SRCS+= $(patsubst %, arch/arm/%, ${ARCH_SRCS})

SRCS+= $(patsubst %, core/%, ${CORE_SRCS})
SRCS+= $(patsubst %, drivers/%, ${DRIVER_SRCS})

SRCS+= $(patsubst %, lib/c/src/%, ${LIBC_SRCS})
ASMS+= $(patsubst %, lib/c/src/%, ${LIBC_ASMS})

OBJS+= $(ASMS:%.S=%.o)
OBJS+= $(SRCS:%.c=%.o)

ARCH_DIRS		= $(ROOT)/$(ARCH)
CORE_DIRS		= $(ROOT)/$(CORE)
DRIVERS_DIRS	= $(ROOT)/$(DRV)
LIB_DIRS		= $(ROOT)/$(LIB)/c/src

BIN				= $(BUILD_DIR)/$(TARGET).bin
LD_SCRIPT		= $(PROJECT).lds.S
OUTPUT			= $(TARGET).axf
MAP				= $(PROJECT).map

INCLUDES += -I${SOURCE_PATH}/arch/arm
INCLUDES += -I${SOURCE_PATH}/drivers/vdev

#all:	ARCH CORE DRIVERS LIB TESTS
#all: $(OUTPUT) $(MAP) $(BIN) $(OBJS)
all: $(OBJS)
%.o: %.S
	echo $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# alternatives: .S.o:
%.o: %.c
	echo $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS)

#.PHONY: all clean config.mk
