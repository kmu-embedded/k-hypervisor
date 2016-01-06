include $(CONFIG_PATH)

OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/arch/arm/v7

CC		= $(CROSS_COMPILE)gcc

INCLUDES	= -I $(ROOT_PATH)/$(INC)
INCLUDES	+= -I $(ROOT_PATH)/$(INC)/$(LIB)/c
INCLUDES	+= -I $(ROOT_PATH)/$(INC)/$(LIB)/bsd
INCLUDES	+= -I $(ROOT_PATH)/$(INC)/$(LIB)
INCLUDES	+= -I $(ROOT_PATH)/$(INC)/$(CORE)
INCLUDES	+= -I $(ROOT_PATH)/$(INC)/$(DRV)

INCLUDES	+= -I $(ROOT_PATH)/$(ARCH)/$(ARM)

INCLUDES	+= -I $(ROOT_PATH)/$(CORE)/libhw
INCLUDES	+= -I $(ROOT_PATH)/$(CORE)
INCLUDES	+= -I $(ROOT_PATH)/$(CORE)/$(INC)

INCLUDES	+= -I $(ROOT_PATH)/$(LIB)

INCLUDES	+= -I $(ROOT_PATH)/$(TESTS)
INCLUDES	+= -I $(ROOT_PATH)/$(TESTS)/libs

CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall

ASM_FILES	= $(wildcard $(CURDIR)/*.S)
OBJS		= $(ASM_FILES:$(CURDIR)/%.S=$(OBJS_DIR)/%.o)

all: $(OBJS)

$(OBJS_DIR)/%.o: $(CURDIR)/%.S
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`	
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
