include	../../../../config.mk

ROOT_PATH	= ../../../..
OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/lib/c/src/arch-arm

C_FILES		= $(wildcard $(CURDIR)/*.c)
ASM_FILES	= $(wildcard $(CURDIR)/*.S)

C_OBJS		= $(C_FILES:$(CURDIR)/%.c=$(OBJS_DIR)/%.o)
ASM_OBJS	= $(ASM_FILES:$(CURDIR)/%.S=$(OBJS_DIR)/%.o)

all: $(C_OBJS) $(ASM_OBJS)

$(OBJS_DIR)/%.o: $(CURDIR)/%.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

$(OBJS_DIR)/%.o: $(CURDIR)/%.S
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
