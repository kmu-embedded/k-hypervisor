include ../../../config.mk

ROOT_PATH   = ../../..
OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/arch/arm/v7

ASM_FILES	= $(wildcard $(CURDIR)/*.S)
OBJS		= $(ASM_FILES:$(CURDIR)/%.S=$(OBJS_DIR)/%.o)

all: $(OBJS)

$(OBJS_DIR)/%.o: $(CURDIR)/%.S
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
