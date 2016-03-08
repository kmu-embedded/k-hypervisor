include ../../../../config.mk

ROOT_PATH	= ../../../..
#OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/lib/c/src/sys-baremetal

SRCS		= $(wildcard *.c)
OBJS		= $(SRCS:%.c=%.o)

all: $(OBJS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

#clean:
	#rm -rf $(OBJS_DIR)

#.PHONY: clean
