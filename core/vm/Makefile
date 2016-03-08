include ../../config.mk

ROOT_PATH   = ../..
#OBJS_DIR	= $(ROOT_PATH)/$(BUILD)/core/sched

SRCS		= $(wildcard *.c)
OBJS		= $(SRCS:%.c=%.o)

all: $(OBJS)

%.o: %.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
