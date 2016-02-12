include ../../../../../config.mk

ROOT_PATH	= ../../../../..
OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/lib/c/src/sys-baremetal/arch-arm

C_FILES		= $(wildcard $(CURDIR)/*.c)
OBJS		= $(C_FILES:$(CURDIR)/%.c=$(OBJS_DIR)/%.o)

all: $(OBJS)

$(OBJS_DIR)/%.o: $(CURDIR)/%.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
