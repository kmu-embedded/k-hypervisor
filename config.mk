# Author has to know the difference among '?=', '=', '+=' and ':='

CROSS_COMPILE		?= arm-linux-gnueabihf-
PROCESSOR		?= cortex-a15

DEBUG_FLAG		= -g
CONFIG_FLAG		= -mcpu=$(PROCESSOR) -marm

MACHINE			?= rtsm
PROJECT			?= khypervisor
TARGET			?= khypervisor-$(MACHINE)

# Desciption of directory structure
ROOT			?= .
ARCH			?= arch/arm
VERSION			?= v7
CORE			?= core
TESTS			?= tests
LIB			?= lib
DRV			?= drivers
INC			?= include
MKDIR			?= mkdir
BUILD			?= build

MAKE			?= make
