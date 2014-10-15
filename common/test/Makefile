include $(EMBROOT)/common/makerules.mak
include $(EMBROOT)/common/test/makerules.mak

CFLAGS:=$(EMB_CFLAGS) $(EMB_TEST_CFLAGS)
LDFLAGS:=$(EMB_LDFLAGS) $(EMB_TEST_LDFLAGS)

CC:=gcc
LD:=gcc
ECHO:=echo
RM:=rm -f

.PHONY: all clean

SOURCES:=$(wildcard ./*.c)
INT_OBJECTS:=$(SOURCES:%.c=%.o)
EXT_OBJECTS:=$(EMB_OBJECTS)
OBJECTS:=$(INT_OBJECTS) $(EXT_OBJECTS)

all: $(OBJECTS)

%.o:%.c
	@$(ECHO) "CC $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@$(ECHO) "RM $(INT_OBJECTS)"
	@$(RM) $(INT_OBJECTS)

