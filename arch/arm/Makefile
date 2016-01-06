include $(CONFIG_PATH)

OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/arch/arm

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

C_FILES		= $(wildcard $(CURDIR)/*.c)
OBJS		= $(C_FILES:$(CURDIR)/%.c=$(OBJS_DIR)/%.o)

all: $(OBJS)

$(OBJS_DIR)/%.o: $(CURDIR)/%.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
