# Author has to know the difference among '?=', '=', '+=' and ':='

CROSS_COMPILE		?= arm-linux-gnueabihf-
PROCESSOR			?= cortex-a15

DEBUG_FLAG			= -g
CONFIG_FLAG			= -mcpu=$(PROCESSOR) -marm

MACH				?= rtsm
PROJECT				?= khypervisor
TARGET				?= khypervisor-$(MACH)

PROJECT_DIR			?= .
ARCH				?= $(PROJECT_DIR)/arch/arm
ARCH_VERSION		?= $(ARCH)/v7
LIB					?= $(PROJECT_DIR)/lib
DRIVER				?= $(PROJECT_DIR)/drivers
