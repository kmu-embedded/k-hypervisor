include ../../../config.mk

ROOT_PATH   = ../../..
#OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/arch/arm/v7

ASM_FILES	= $(wildcard *.S)
OBJS		= $(ASM_FILES:%.S=%.o)

all: $(OBJS)

%.o: %.S
	#@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
