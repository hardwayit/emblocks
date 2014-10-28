include $(EMBROOT)/common/makerules.mak
include $(EMBROOT)/common/test/makerules.mak

EMB_LOCAL_CFLAGS=-I$(EMBROOT)/libs/string/inc -I$(EMBROOT)/libs/config/inc -I$(EMBROOT)/libs/nvm/inc

CFLAGS:=$(EMB_CFLAGS) $(EMB_TEST_CFLAGS) $(EMB_LOCAL_CFLAGS)
LDFLAGS:=$(EMB_LDFLAGS) $(EMB_TEST_LDFLAGS) $(EMB_LOCAL_LDFLAGS)

CC:=gcc
LD:=gcc
ECHO:=echo
RM:=rm -f

TARGET=config-tests

.PHONY: all clean

SOURCES:=$(wildcard ./src/*.c)
INT_OBJECTS:=$(SOURCES:%.c=%.o)
EXT_OBJECTS:=$(EMB_OBJECTS)
OBJECTS:=$(INT_OBJECTS) $(EXT_OBJECTS)

all: $(TARGET)

$(TARGET): $(INT_OBJECTS)
	@$(ECHO) "LD $(OBJECTS)"
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

%.o:%.c
	@$(ECHO) "CC $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(ECHO) "RM $(INT_OBJECTS)"
	@$(RM) $(INT_OBJECTS)

