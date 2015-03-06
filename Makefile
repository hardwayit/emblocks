#/***************************************************************************
#* File		Makefile
#* Brief		
#* Created  13.10.2013
#* Updated  06.04.2015
#* Author	"MCS51", sole proprietorship Pashkova E.A.
#*
#* Copyright(C) 2013-2015, "MCS51", sole proprietorship Pashkova E.A.
#* All rights reserved.
#***************************************************************************/

CBUILD ?= arm-none-eabi

CC:=$(CBUILD)-gcc
LD:=$(CBUILD)-ld
OBJCOPY:=$(CBUILD)-objcopy
SIZE:=$(CBUILD)-size
AR:=$(CBUILD)-ar
RM:=rm
ECHO:=echo -e

TARGET:=emblocks

CFLAGS += -std=c99 -O2 -ffunction-sections -fdata-sections
CFLAGS += -I./include
CFLAGS += -I./common/include
CFLAGS += -I./libs
CFLAGS += -include ./common/inc/core.h

include libs/Makefile

OBJECTS = $(SOURCES:%.c=%.o)

.PHONY: $(TARGET) $(TARGET)_show_info clean distclean successful

$(TARGET): lib$(TARGET).a $(TARGET)_show_info successful

%.o: %.c
	@$(ECHO) "CC $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

lib$(TARGET).a: $(OBJECTS)
	@$(ECHO) "AR $@ <- $^"
	@$(AR) cr $@ $^

$(TARGET)_show_info: $(TARGET).elf
	@$(SIZE) $^

successful:
	@$(ECHO) "\n\nBuild successful"

clean:
	@$(ECHO) "RM $(OBJECTS)"
	@$(RM) -f $(OBJECTS)

distclean: clean
	@$(ECHO) "RM lib$(TARGET).a"
	@$(RM) -f lib$(TARGET).a

