include ../../../../config.mk

OBJS_DIR	= ../../../../build/lib/c/src/arch-arm

CC		= $(CROSS_COMPILE)gcc

INCLUDES	= -I../../../../$(INC) -I../../../../$(INC)/$(LIB)/c -I../../../../$(INC)/$(LIB)/bsd -I../../../../$(INC)/$(LIB)
INCLUDES	+= -I../../../../$(INC)/$(DRV)
INCLUDES	+= -I../../../../$(CORE)/libhw -I../../../../tests -I../../../../$(CORE) -I../../../../$(CORE)/include -I../../../../include/lib -I../../../../lib -I../../../../include/core
INCLUDES	+= -I../../../../tests/libs -I../../../../arch/arm

CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall

C_FILES		= $(wildcard *.c)
ASM_FILES	= $(wildcard *.S)

C_OBJS		= $(C_FILES:%.c=$(OBJS_DIR)/%.o)
ASM_OBJS	= $(ASM_FILES:%.S=$(OBJS_DIR)/%.o)


all: $(C_OBJS) $(ASM_OBJS)

$(OBJS_DIR)/%.o: %.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

$(OBJS_DIR)/%.o: %.S
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
