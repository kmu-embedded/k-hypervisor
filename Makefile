include config.mk

ASM_FILES	= $(wildcard $(ARCH_VERSION)/*.S)
C_FILES 	= $(wildcard $(ARCH)/*.c) $(wildcard $(DRIVER)/*.c) $(wildcard $(LIB)/*.c)
OBJS 		:= $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)

BIN			= $(BUILD_DIR)/$(TARGET).bin
LD_SCRIPT	= $(PROJECT).lds.S
OUTPUT 		= $(TARGET).axf
MAP			= $(TARGET).map

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
INCLUDES    = -I ./include
CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib $(DEBUG_FLAG)
CPPFLAGS	+= -Wall -Werror

all: $(OBJS) $(OUTPUT) $(MAP)

$(MAP): $(OUTPUT)
	$(NM) $< > $@

clean:
	rm -f $(MAP) $(OUTPUT) $(BIN) \
	$(TARGET).lds $(OBJS)

$(OUTPUT): $(TARGET).lds $(OBJS)
	$(CC) $(CPPFLAGS) -e __start -T $(TARGET).lds -o $@ $(OBJS)


$(BIN): $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT) $(BIN)

$(OBJ_DIR)%.o: $(ARCH)%.S
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(TARGET).lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all clean config.mk
