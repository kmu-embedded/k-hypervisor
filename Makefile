include config.mk

ASM_FILES	= $(wildcard $(ARCH)/$(VERSION)/*.S)		\
			  $(wildcard $(LIB)/c/src/arch-arm/*.S)

C_FILES 	= $(wildcard $(CORE)/*.c) 				\
			  $(wildcard $(CORE)/libhw/*.c) 		\
			  $(wildcard $(CORE)/vdev/*.c) 			\
			  $(wildcard tests/*.c) 				\
			  $(wildcard tests/libs/*.c) 				\
			  $(wildcard $(ARCH)/*.c) 				\
			  $(wildcard $(DRV)/*.c) 			\
			  $(wildcard $(LIB)/c/src/*.c)				\
			  $(wildcard $(LIB)/c/src/arch-arm/*.c)		\
			  $(wildcard $(LIB)/c/src/sys-baremetal/arch-arm/*.c)			\
			  $(wildcard $(LIB)/c/src/sys-baremetal/*.c)

OBJS 		:= $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

BIN			= $(BUILD_DIR)/$(TARGET).bin
LD_SCRIPT	= $(PROJECT).lds.S
OUTPUT 		= $(TARGET).axf
MAP			= $(PROJECT).map

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy


INCLUDES	= -I $(INC) -I $(INC)/$(LIB)/c -I $(INC)/$(LIB)/bsd -I $(INC)/$(LIB)
INCLUDES	+= -I $(INC)/$(DRV)
INCLUDES    += -I $(CORE)/libhw -I tests -I $(CORE) -I $(CORE)/include -I include/lib -I ./lib -I ./include/core
INCLUDES	+= -I tests/libs -I ./arch/arm

CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall

all: $(OBJS) $(OUTPUT) $(MAP)

$(MAP): $(OUTPUT)
	$(NM) $< > $@

clean:
	rm -f $(MAP) $(OUTPUT) $(BIN) \
	$(MACHINE).lds $(OBJS)

$(OUTPUT): $(MACHINE).lds $(OBJS)
	$(CC) $(CPPFLAGS) -e __start -T $(MACHINE).lds -o $@ $(OBJS)


$(BIN): $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT) $(BIN)

$(OBJ_DIR)%.o: $(ARCH)%.S
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(MACHINE).lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all clean config.mk
