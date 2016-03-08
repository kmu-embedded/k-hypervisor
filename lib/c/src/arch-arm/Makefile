include	../../../../config.mk

ROOT_PATH	= ../../../..
#OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/lib/c/src/arch-arm

SRCS		= $(wildcard *.c)
ASM_FILES	= $(wildcard *.S)

C_OBJS		= $(SRCS:%.c=%.o)
ASM_OBJS	= $(ASM_FILES:%.S=%.o)

all: $(C_OBJS) $(ASM_OBJS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.S
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
