include config.mk

SRC_DIR		= .
OBJDIR		= .
ASM_FILES 	= head.S vector.S
C_FILES 	= $(wildcard $(SRC_DIR)/*.c)
OBJS 		:= $(addprefix $(OBJDIR)/,$(notdir $(ASM_FILES:.S=.o))) $(addprefix $(OBJDIR)/,$(notdir $(C_FILES:.c=.o)))

BIN			= $(BUILD_DIR)/$(TARGET).bin
LD_SCRIPT	= $(PROJECT).lds.S
OUTPUT 		= $(TARGET).axf
MAP			= $(TARGET).map

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
INCLUDES    = -I .
CPPFLAGS	+= $(CONFIG_FLAGS) $(INCLUDES)

all: $(OBJS) $(OUTPUT) $(MAP)

$(MAP): $(OUTPUT)
	$(NM) $< > $@

clean:
	rm -f $(MAP) $(OUTPUT) $(BIN) \
	$(TARGET).lds $(OBJS)

$(OUTPUT): $(TARGET).lds $(OBJS)
	$(LD) -o $@ $(OBJS) --script=$(TARGET).lds

$(BIN): $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT) $(BIN)

$(OBJ_DIR)%.o: $(SRC_DIR)/%.S
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(OBJ_DIR)%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) -I. -c -o $@ $<

$(TARGET).lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all clean config.mk
