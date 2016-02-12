# Author has to know the difference among '?=', '=', '+=' and ':='

CROSS_COMPILE	= arm-linux-gnueabihf-
PROCESSOR		= cortex-a15

DEBUG_FLAG		= -g
CONFIG_FLAG		= -mcpu=$(PROCESSOR) -marm

MACHINE			= rtsm
PROJECT			= khypervisor
TARGET			= khypervisor-$(MACHINE)

# Desciption of directory structure
ROOT			= $(PWD)
ARCH			= arch/arm
VERSION			= v7
CORE			= core
TESTS			= tests
LIB				= lib
DRV				= drivers
INC				= include
MKDIR			= mkdir
BUILD			= build
MAKE			= make

INCLUDES		=  -I$(ROOT)/
INCLUDES		+= -I$(ROOT)/$(INC)
INCLUDES		+= -I$(ROOT)/$(INC)/board
INCLUDES		+= -I$(ROOT)/$(INC)/$(LIB)/bsd
INCLUDES		+= -I$(ROOT)/$(INC)/$(LIB)
INCLUDES		+= -I$(ROOT)/$(INC)/$(CORE)
INCLUDES		+= -I$(ROOT)/$(INC)/$(DRV)
INCLUDES		+= -I$(ROOT)/$(INC)/$(ARCH)/$(ARM)

INCLUDES		+= -I$(ROOT)/$(ARCH)/$(ARM)

INCLUDES		+= -I$(ROOT)/$(CORE)/libhw
INCLUDES		+= -I$(ROOT)/$(CORE)
INCLUDES		+= -I$(ROOT)/$(CORE)/$(INC)

INCLUDES		+= -I$(ROOT)/$(LIB)
INCLUDES		+= -I$(ROOT)/$(LIB)/c/include

INCLUDES		+= -I$(ROOT)/$(TESTS)
INCLUDES		+= -I$(ROOT)/$(TESTS)/libs

INCLUDES		+= -I$(ROOT)/$(INC)/asm

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
NM			= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy


CPPFLAGS	= $(CONFIG_FLAG) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles $(DEBUG_FLAG)
CPPFLAGS	+= -Wall

