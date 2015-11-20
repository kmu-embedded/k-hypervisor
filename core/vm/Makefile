SRC_DIR		= .
OBJDIR		= .
C_FILES 	= $(wildcard $(SRC_DIR)/*.c)
OBJS 		:= $(addprefix $(OBJDIR)/,$(notdir $(ASM_FILES:.S=.o))) $(addprefix $(OBJDIR)/,$(notdir $(C_FILES:.c=.o)))

TARGET		= outer
BIN			= $(BUILD_DIR)/$(TARGET).bin
OUTPUT 		= $(TARGET).axf
MAP			= $(TARGET).map

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
INCLUDES    = -I ../include -I ../../include/lib/
CONFIG_FLAGS= -g
CPPFLAGS	+= $(CONFIG_FLAGS) $(INCLUDES) -Werror

all: $(OBJS) $(OUTPUT) $(MAP)

$(MAP): $(OUTPUT)
	$(NM) $< > $@

clean:
	rm -f $(MAP) $(OUTPUT) $(BIN) \
	$(TARGET).lds $(OBJS)

$(OUTPUT): $(OBJS)
	$(CC) -o $@ $(OBJS)

$(BIN): $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT) $(BIN)

$(OBJ_DIR)%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) -static -I. -c -o $@ $<

%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all clean config.mk
