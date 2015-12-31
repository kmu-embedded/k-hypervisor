include ../config.mk

OBJS_DIR	= ../build/core

CC		= $(CROSS_COMPILE)gcc

INCLUDES	= -I../$(INC) -I../$(INC)/$(LIB)/c -I../$(INC)/$(LIB)/bsd -I../$(INC)/$(LIB)
INCLUDES	+= -I../$(INC)/$(DRV)
INCLUDES	+= -I../$(CORE)/libhw -I../tests -I../$(CORE) -I../$(CORE)/include -I../include/lib -I../lib -I../include/core
INCLUDES	+= -I../tests/libs -I../arch/arm

CPPFLAGS	= $(CONFIG_FLAG) $(INCLUDES) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall

C_FILES		= $(wildcard *.c)
OBJS		= $(C_FILES:%.c=$(OBJS_DIR)/%.o)

all: $(OBJS)

$(OBJS_DIR)/%.o: %.c
	@`[ -d $(OBJS_DIR) ] || $(MKDIR) -p $(OBJS_DIR)`
	$(CC) $(CPPFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS_DIR)

.PHONY: clean
