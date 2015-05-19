## Makefile for UAKEH
## Copyright (C) 2015 Andrea Merello
##
##   This file is based on the Makefiles in the libopencm3-examples project.
##   Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##   Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
##   Copyright (C) 2013 Frantisek Burian <BuFran@seznam.cz>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

PREFIX?= arm-none-eabi
LDSCRIPT=stm32-f103c8.ld
OPENCM3_DIR := $(realpath libopencm3)
LIBNAME=opencm3_stm32f1
STMFAMILY=STM32F1
FP_FLAGS ?= -msoft-float
ARCH_FLAGS=-mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

CC              := $(PREFIX)-gcc
CXX             := $(PREFIX)-g++
LD              := $(PREFIX)-gcc
AR              := $(PREFIX)-ar
AS              := $(PREFIX)-as
OBJCOPY         := $(PREFIX)-objcopy
OBJDUMP         := $(PREFIX)-objdump
GDB             := $(PREFIX)-gdb

INCLUDE_DIR     = $(OPENCM3_DIR)/include
LIB_DIR         = $(OPENCM3_DIR)/lib
SCRIPT_DIR      = $(OPENCM3_DIR)/scripts

###############################################################################
# C flags

CFLAGS		+= -Os -g -I$(INCLUDE_DIR) -D$(STMFAMILY)
CFLAGS		+= -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS		+= -Wredundant-decls
CFLAGS		+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# Linker flags

LDFLAGS		+= --static -nostartfiles
LDFLAGS		+= -L$(LIB_DIR)
LDFLAGS		+= -T$(LDSCRIPT)
LDFLAGS		+= -Wl,-Map=$(*).map
LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),99)
LDFLAGS		+= -Wl,--print-gc-sections
endif

###############################################################################
# Used libraries

LDLIBS		+= -l$(LIBNAME)
LDLIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################

.SUFFIXES: .elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

OBJS=cdcacm.o main.o cmd.o debug_printf.o gpio.o

all: lib usbcontrol.elf
	@printf " -------------------  WARNING!!!  -------------------\n"
	@printf " ----                                            ----\n"
	@printf " ---- make sure you have no ModemManager running ----\n"
	@printf " ---- because it is known it misrecognize our    ----\n"
	@printf " ---- device as a modem, and it tires to         ----\n"
	@printf " ---- communicate with it, screwing up things..  ----\n"
	@printf " ----                                            ----\n"
	@printf " ---- (this message always show up, we does not  ----\n"
	@printf " ---- try to detect if you really have any modem ----\n"
	@printf " ---- manager actually runnin ..)                ----\n"
	@printf " ----------------------------------------------------\n"

%.elf %.map: $(OBJS) $(LDSCRIPT) $(LIB_DIR)/lib$(LIBNAME).a
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(LDLIBS) -o $(*).elf

%.o: %.c
	@printf "  CC      $(*).c\n"
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $(*).o -c $(*).c

clean:
	@printf "  CLEAN\n"
	$(Q)$(RM) *.o *.d *.elf *.bin *.hex *.srec *.list *.map


lib:
	@if [ ! "`ls -A libopencm3`" ] ; then \
		printf "######## ERROR ########\n"; \
		printf "\tlibopencm3 not found\n"; \
		printf "\tPlease symlink main lib dir here:\n"; \
		printf "######## ERROR ########\n"; \
		exit 1; \
	fi

.PHONY: lib
