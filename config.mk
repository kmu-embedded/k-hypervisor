# Author has to know the difference among '?=', '=', '+=' and ':='

CROSS_COMPILE		?= arm-linux-gnueabihf-
PROCESSOR			?= cortex-a15

DEBUG_FLAG			= -g
CONFIG_FLAG			= -mcpu=$(PROCESSOR) -marm

MACH				?= rtsm
PROJECT				?= khypervisor
TARGET				?= khypervisor-$(MACH)

CURDIR				?= $(PWD)
ARCH				?= $(CURDIR)/arch/arm
ARCH_VERSION		?= $(ARCH)/v7
LIB					?= $(CURDIR)/lib
DRIVER				?= $(CURDIR)/drivers
