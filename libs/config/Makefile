include $(EMBROOT)/common/makerules.mak
include $(EMBROOT)/common/test/makerules.mak

CFLAGS:=$(EMB_CFLAGS) $(EMB_TEST_CFLAGS) -I./inc
LDFLAGS:=$(EMB_LDFLAGS) $(EMB_TEST_LDFLAGS)

CC:=gcc
LD:=gcc
ECHO:=echo
RM:=rm -f

.PHONY: all clean

TARGET:=config_test

SOURCES:=$(wildcard ./src/*.c) $(wildcard ./tests/src/*.c)
INT_OBJECTS:=$(SOURCES:%.c=%.o)
EXT_OBJECTS:=$(EMB_OBJECTS) $(EMB_TEST_OBJECTS)
OBJECTS:=$(INT_OBJECTS) $(EXT_OBJECTS)

all: $(TARGET)

$(TARGET): $(INT_OBJECTS)

%.o:%.c
	@$(ECHO) "CC $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET): $(INT_OBJECTS)
	@$(ECHO) "LD $(OBJECTS)"
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	@$(ECHO) "RM $(INT_OBJECTS) $(TARGET)"
	@$(RM) $(INT_OBJECTS) $(TARGET)

